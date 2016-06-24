/**
 *     Copyright 2016 Couchbase, Inc.
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
#include "ext/standard/php_var.h"
#include "ext/json/php_json.h"
#include "exception.h"
#include "paramparser.h"
#include "zap.h"
#include "bucket.h"
#include "cas.h"
#include "metadoc.h"
#include "docfrag.h"
#include "token.h"
#include "transcoding.h"
#include "opcookie.h"

typedef struct {
    opcookie_res header;
    zapval value;
    zapval cas;
    zapval token;
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
        cas_encode(&result->cas, rb->cas TSRMLS_CC);
    }
    mutinfo = lcb_resp_get_mutation_token(cbtype, rb);
    if (mutinfo == NULL) {
        zapval_alloc_null(result->token);
    } else {
        const char *bucketname;
        zapval_alloc(result->token);
        lcb_cntl(instance, LCB_CNTL_GET, LCB_CNTL_BUCKETNAME, &bucketname);
        pcbc_make_token(zapval_zvalptr(result->token), bucketname,
                        LCB_MUTATION_TOKEN_VB(mutinfo),
                        LCB_MUTATION_TOKEN_ID(mutinfo),
                        LCB_MUTATION_TOKEN_SEQ(mutinfo) TSRMLS_CC);
    }
    zapval_alloc_array(result->value);
    while (lcb_sdresult_next(resp, &cur, &vii)) {
        zapval value, res, code;
        size_t index = oix++;
        int ntmp;
        char *tmp;

        if (cbtype == LCB_CALLBACK_SDMUTATE) {
            index = cur.index;
        }
        if (cur.nvalue > 0) {
            ntmp = cur.nvalue + 1;
            tmp = emalloc(ntmp);
            memcpy(tmp, cur.value, cur.nvalue);
            tmp[ntmp - 1] = 0;
            zapval_alloc(res);
            php_json_decode(zapval_zvalptr(res), tmp, ntmp - 1, 1, PHP_JSON_PARSER_DEFAULT_DEPTH TSRMLS_CC);
            efree(tmp);
        } else {
            zapval_alloc_null(res);
        }
        zapval_alloc_array(value);

        zapval_add_assoc_zval_ex(value, "value", res);

        zapval_alloc_long(code, cur.status);
        zapval_add_assoc_zval_ex(value, "code", code);

        add_index_zval(zapval_zvalptr(result->value), index, zapval_zvalptr(value));
    }

    opcookie_push((opcookie*)rb->cookie, &result->header);
}

static lcb_error_t proc_subdoc_results(bucket_object *bucket, zval *return_value,
                                       opcookie *cookie TSRMLS_DC)
{
    opcookie_subdoc_res *res;
    lcb_error_t err = LCB_SUCCESS;

    FOREACH_OPCOOKIE_RES(opcookie_subdoc_res, res, cookie) {
        if (res->header.err == LCB_SUCCESS) {
            pcbc_make_docfrag(return_value, &res->value, &res->cas, &res->token TSRMLS_CC);
        } else {
            pcbc_make_docfrag_error(return_value, res->header.err,
                                    res->header.err == LCB_SUBDOC_MULTI_FAILURE ? &res->value : NULL
                                    TSRMLS_CC);
        }
    }

    FOREACH_OPCOOKIE_RES(opcookie_subdoc_res, res, cookie) {
        zapval_destroy(res->value);
        if (!zap_zval_is_undef(zapval_zvalptr(res->cas))) {
            zapval_destroy(res->cas);
        }
    }

    return err;
}

typedef struct {
    int nspecs;
    lcb_SDSPEC *specs;
    smart_str *bufs;
} pcbc_sd_params;

static int extract_specs(zapval *pDest, void *argument TSRMLS_DC)
{
    pcbc_sd_params *params = (pcbc_sd_params *)argument;
    lcb_SDSPEC *spec;
    HashTable *hparams;
    zval *zparam;
    int remove_brackets = 0;

    if (!pDest || !zap_zval_is_array(zapval_zvalptr(*pDest))) {
        return ZEND_HASH_APPLY_KEEP;
    }
    hparams = zapval_arrval(*pDest);

    spec = params->specs + params->nspecs;

    zparam = zap_hash_str_find_s(hparams, "opcode");
    if (!zparam) {
        return ZEND_HASH_APPLY_KEEP;
    }
    spec->sdcmd = Z_LVAL_P(zparam);

    switch (spec->sdcmd) {
    case LCB_SDCMD_ARRAY_ADD_FIRST:
    case LCB_SDCMD_ARRAY_ADD_LAST:
    case LCB_SDCMD_ARRAY_INSERT:
        zparam = zap_hash_str_find_s(hparams, "removeBrackets");
        remove_brackets = zparam && zap_zval_boolval(zparam);
    }

    zparam = zap_hash_str_find_s(hparams, "createParents");
    if (zparam && zap_zval_boolval(zparam)) {
        spec->options |= LCB_SDSPEC_F_MKINTERMEDIATES;
    }

    zparam = zap_hash_str_find_s(hparams, "path");
    if (zparam) {
        LCB_SDSPEC_SET_PATH(spec, Z_STRVAL_P(zparam), Z_STRLEN_P(zparam));
    }

    zparam = zap_hash_str_find_s(hparams, "value");
    if (zparam) {
        char *p;
        int n;
        smart_str *buf = params->bufs + params->nspecs;
        php_json_encode(buf, zparam, 0 TSRMLS_CC);
#if PHP_VERSION_ID >= 70000
        p = zap_zstr_val(buf->s);
        n = zap_zstr_len(buf->s);
#else
        p = buf->c;
        n = buf->len;
#endif
        if (remove_brackets) {
            for (; isspace(*p) && n; n--, p++) {
            }
            for (; n && isspace(p[n-1]); n--) {
            }
            if (n < 3 || p[0] != '[' || p[n-1] != ']') {
                php_error_docref(NULL TSRMLS_CC, E_WARNING, "multivalue operation expects non-empty array");
                return ZEND_HASH_APPLY_KEEP;
            }
            p++;
            n -= 2;
        }
        LCB_SDSPEC_SET_VALUE(spec, p, n);
    }
    params->nspecs++;

    return ZEND_HASH_APPLY_KEEP;
}

PHP_METHOD(Bucket, subdoc_request)
{
    bucket_object *data = PCBC_PHP_THISOBJ();
    lcb_CMDSUBDOC cmd = { 0 };
    opcookie *cookie;
    zval *zid, *zcommands, *zcas;
    lcb_error_t err;
    int nspecs, i;
    pcbc_sd_params params = {0};

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zzz",
                              &zid, &zcommands, &zcas) == FAILURE) {
        throw_pcbc_exception("Invalid arguments.", LCB_EINVAL);
        RETURN_NULL();
    }

    PCBC_CHECK_ZVAL_STRING(zid, "key must be a string");
    PCBC_CHECK_ZVAL_ARRAY(zcommands, "commands must be an array");
    if (!zap_zval_is_null(zcas)) {
        PCBC_CHECK_ZVAL_STRING(zcas, "commands must be an string");
        cmd.cas = cas_decode(zcas TSRMLS_CC);
    }
    LCB_CMD_SET_KEY(&cmd, Z_STRVAL_P(zid), Z_STRLEN_P(zid));

    nspecs = zend_hash_num_elements(Z_ARRVAL_P(zcommands));
    params.nspecs = 0;
    params.specs = emalloc(sizeof(lcb_SDSPEC) * nspecs);
    memset(params.specs, 0, sizeof(lcb_SDSPEC) * nspecs);
    params.bufs = emalloc(sizeof(smart_str) * nspecs);
    memset(params.bufs, 0, sizeof(smart_str) * nspecs);

    zend_hash_apply_with_argument(Z_ARRVAL_P(zcommands), (apply_func_arg_t) extract_specs, &params TSRMLS_CC);
    cmd.specs = params.specs;
    cmd.nspecs = params.nspecs;

    cookie = opcookie_init();

    // Execute query
    err = lcb_subdoc3(data->conn->lcb, cookie, &cmd);

    if (err == LCB_SUCCESS) {
        lcb_wait(data->conn->lcb);

        err = proc_subdoc_results(data, return_value, cookie TSRMLS_CC);
    }

    opcookie_destroy(cookie);
    efree(params.specs);
    for (i = 0; i < nspecs; ++i) {
        smart_str_free(params.bufs + i);
    }
    efree(params.bufs);

    if (err != LCB_SUCCESS) {
        throw_lcb_exception(err);
    }
}
