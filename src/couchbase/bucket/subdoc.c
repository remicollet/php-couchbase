/**
 *     Copyright 2016-2017 Couchbase, Inc.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 */

#include "couchbase.h"

#define LOGARGS(instance, lvl) LCB_LOG_##lvl, instance, "pcbc/subdoc", __FILE__, __LINE__

typedef struct {
    opcookie_res header;
    PCBC_ZVAL value;
    PCBC_ZVAL cas;
    PCBC_ZVAL token;
} opcookie_subdoc_res;

void subdoc_callback(lcb_t instance, int cbtype, const lcb_RESPBASE *rb)
{
    opcookie_subdoc_res *result = ecalloc(1, sizeof(opcookie_subdoc_res));
    const lcb_RESPSUBDOC *resp = (const lcb_RESPSUBDOC *)rb;
    const lcb_MUTATION_TOKEN *mutinfo;
    lcb_SDENTRY cur;
    size_t vii = 0, oix = 0;
    TSRMLS_FETCH();

    result->header.err = rb->rc;
    if (rb->rc == LCB_SUCCESS || rb->rc == LCB_SUBDOC_MULTI_FAILURE) {
        PCBC_ZVAL_ALLOC(result->cas);
        pcbc_cas_encode(PCBC_P(result->cas), rb->cas TSRMLS_CC);
    }
    mutinfo = lcb_resp_get_mutation_token(cbtype, rb);
    if (mutinfo != NULL) {
        char *bucketname;
        PCBC_ZVAL_ALLOC(result->token);
        lcb_cntl(instance, LCB_CNTL_GET, LCB_CNTL_BUCKETNAME, &bucketname);
        pcbc_mutation_token_init(PCBC_P(result->token), bucketname, mutinfo TSRMLS_CC);
    }
    PCBC_ZVAL_ALLOC(result->value);
    array_init(PCBC_P(result->value));
    while (lcb_sdresult_next(resp, &cur, &vii)) {
        PCBC_ZVAL value, res, code;
        size_t index = oix++;

        PCBC_ZVAL_ALLOC(value);
        PCBC_ZVAL_ALLOC(res);
        PCBC_ZVAL_ALLOC(code);
        if (cbtype == LCB_CALLBACK_SDMUTATE) {
            index = cur.index;
        }
        if (cur.nvalue > 0) {
            int last_error;

            PCBC_JSON_COPY_DECODE(PCBC_P(res), cur.value, cur.nvalue, PHP_JSON_OBJECT_AS_ARRAY, last_error);
            if (last_error != 0) {
                pcbc_log(LOGARGS(instance, WARN), "Failed to decode subdoc response as JSON: json_last_error=%d",
                         last_error);
            }
        } else {
            ZVAL_NULL(PCBC_P(res));
        }
        array_init(PCBC_P(value));

        ADD_ASSOC_ZVAL_EX(PCBC_P(value), "value", PCBC_P(res));
        ZVAL_LONG(PCBC_P(code), cur.status);
        ADD_ASSOC_ZVAL_EX(PCBC_P(value), "code", PCBC_P(code));
        add_index_zval(PCBC_P(result->value), index, PCBC_P(value));
    }

    opcookie_push((opcookie *)rb->cookie, &result->header);
}

static lcb_error_t proc_subdoc_results(zval *return_value, opcookie *cookie TSRMLS_DC)
{
    opcookie_subdoc_res *res;
    lcb_error_t err = LCB_SUCCESS;

    FOREACH_OPCOOKIE_RES(opcookie_subdoc_res, res, cookie)
    {
        if (res->header.err == LCB_SUCCESS) {
            pcbc_document_fragment_init(return_value, PCBC_P(res->value), PCBC_P(res->cas),
                                        PCBC_P(res->token) TSRMLS_CC);
        } else {
            pcbc_document_fragment_init_error(return_value, &res->header, res->header.err == LCB_SUBDOC_MULTI_FAILURE
                                                                                 ? PCBC_P(res->value)
                                                                                 : NULL TSRMLS_CC);
        }
    }

    FOREACH_OPCOOKIE_RES(opcookie_subdoc_res, res, cookie)
    {
        zval_ptr_dtor(&res->value);
        if (!Z_ISUNDEF(res->cas)) {
            zval_ptr_dtor(&res->cas);
        }
        if (!Z_ISUNDEF(res->token)) {
            zval_ptr_dtor(&res->token);
        }
    }

    return err;
}

typedef struct {
    int nspecs;
    lcb_SDSPEC *specs;
    smart_str *bufs;
    lcb_t instance;
} pcbc_sd_params;

void pcbc_bucket_subdoc_request(pcbc_bucket_t *obj, void *builder, int is_lookup, zval *return_value TSRMLS_DC)
{
    opcookie *cookie;
    lcb_CMDSUBDOC cmd = {0};
    pcbc_sd_spec_t *spec;
    lcb_error_t err;
    int i;

#define COPY_SPEC(b)                                                                                                   \
    do {                                                                                                               \
        if ((b)->nspecs == 0) {                                                                                        \
            return;                                                                                                    \
        }                                                                                                              \
        LCB_CMD_SET_KEY(&cmd, (b)->id, (b)->id_len);                                                                   \
        cmd.nspecs = (b)->nspecs;                                                                                      \
        cmd.specs = emalloc(sizeof(lcb_SDSPEC) * (b)->nspecs);                                                         \
        spec = (b)->head;                                                                                              \
        i = 0;                                                                                                         \
        while (spec) {                                                                                                 \
            memcpy((void *)(cmd.specs + i), &spec->s, sizeof(lcb_SDSPEC));                                             \
            spec = spec->next;                                                                                         \
            i++;                                                                                                       \
        }                                                                                                              \
    } while (0);

    if (is_lookup) {
        COPY_SPEC((pcbc_lookup_in_builder_t *)builder);
    } else {
        COPY_SPEC((pcbc_mutate_in_builder_t *)builder);
        cmd.cas = ((pcbc_mutate_in_builder_t *)builder)->cas;
        if (((pcbc_mutate_in_builder_t *)builder)->expiry) {
            cmd.exptime = ((pcbc_mutate_in_builder_t *)builder)->expiry;
        }
        switch (((pcbc_mutate_in_builder_t *)builder)->fulldoc) {
        case PCBC_SUBDOC_FULLDOC_INSERT:
            cmd.cmdflags |= LCB_CMDSUBDOC_F_INSERT_DOC;
            break;
        case PCBC_SUBDOC_FULLDOC_UPSERT:
            cmd.cmdflags |= LCB_CMDSUBDOC_F_UPSERT_DOC;
            break;
        case PCBC_SUBDOC_FULLDOC_REPLACE:
        default:
            break;
        }
    }
#undef COPY_SPEC

    cookie = opcookie_init();
    err = lcb_subdoc3(obj->conn->lcb, cookie, &cmd);

    if (err == LCB_SUCCESS) {
        lcb_wait(obj->conn->lcb);

        err = proc_subdoc_results(return_value, cookie TSRMLS_CC);
    }

    opcookie_destroy(cookie);
    efree((void *)cmd.specs);

    if (err != LCB_SUCCESS) {
        throw_lcb_exception(err);
    }
}

lcb_U32 pcbc_subdoc_options_to_flags(int is_path, int is_lookup, zval *options TSRMLS_DC)
{
    lcb_U32 flags = 0;

    if (!options) {
        return 0;
    }

    if (is_path && !is_lookup) {
        switch (Z_TYPE_P(options)) {
#if PHP_VERSION_ID >= 70000
        case IS_TRUE:
            return LCB_SDSPEC_F_MKINTERMEDIATES;
        case IS_FALSE:
            return 0;
#else
        case IS_BOOL:
            if (Z_BVAL_P(options)) {
                return LCB_SDSPEC_F_MKINTERMEDIATES;
            } else {
                return 0;
            }
#endif
        }
    }
    if (Z_TYPE_P(options) == IS_ARRAY) {
        if (is_path) {
            if (php_array_fetch_bool(options, "xattr")) {
                flags |= LCB_SDSPEC_F_XATTRPATH;
            }
            if (!is_lookup) {
                if (php_array_fetch_bool(options, "createPath")) {
                    flags |= LCB_SDSPEC_F_MKINTERMEDIATES;
                }
                if (php_array_fetch_bool(options, "expandMacroValues")) {
                    flags |= LCB_SDSPEC_F_XATTR_MACROVALUES;
                }
            }
        }
    }

    return flags;
}
