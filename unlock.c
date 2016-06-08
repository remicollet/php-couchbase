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
#include "transcoding.h"
#include "opcookie.h"

typedef struct {
    opcookie_res header;
    zapval key;
} opcookie_unlock_res;

void unlock_callback(lcb_t instance, int cbtype, const lcb_RESPBASE *rb)
{
    opcookie_unlock_res *result = ecalloc(1, sizeof(opcookie_unlock_res));
    const lcb_RESPUNLOCK *resp = (const lcb_RESPUNLOCK *)rb;
    TSRMLS_FETCH();

    result->header.err = resp->rc;
    zapval_alloc_stringl(result->key, resp->key, resp->nkey);

    opcookie_push((opcookie*)rb->cookie, &result->header);
}

static lcb_error_t proc_unlock_results(bucket_object *bucket, zval *return_value,
                                       opcookie *cookie, int is_mapped TSRMLS_DC)
{
    opcookie_unlock_res *res;
    lcb_error_t err = LCB_SUCCESS;

    // If we are not mapped, we need to throw any op errors
    if (is_mapped == 0) {
        err = opcookie_get_first_error(cookie);
    }

    if (err == LCB_SUCCESS) {
        FOREACH_OPCOOKIE_RES(opcookie_unlock_res, res, cookie) {
            zval *doc = bop_get_return_doc(
                return_value, &res->key, is_mapped);

            if (res->header.err == LCB_SUCCESS) {
                make_metadoc(doc, NULL, NULL, NULL TSRMLS_CC);
            } else {
                make_metadoc_error(doc, res->header.err TSRMLS_CC);
            }
        }
    }

    FOREACH_OPCOOKIE_RES(opcookie_unlock_res, res, cookie) {
        zapval_destroy(res->key);
    }

    return err;
}

// unlock($id {, $cas, $groupid}) : MetaDoc
PHP_METHOD(Bucket, unlock)
{
    bucket_object *data = PCBC_PHP_THISOBJ();
    int ii, ncmds, nscheduled;
    pcbc_pp_state pp_state;
    pcbc_pp_id id;
    zval *zcas, *zgroupid;
    opcookie *cookie;
    lcb_error_t err;

    // Note that groupid is experimental here and should not be used.
    if (pcbc_pp_begin(ZEND_NUM_ARGS() TSRMLS_CC, &pp_state,
                      "id||cas,groupid",
                      &id, &zcas, &zgroupid) != SUCCESS)
    {
        throw_pcbc_exception("Invalid arguments.", LCB_EINVAL);
        RETURN_NULL();
    }

    ncmds = pcbc_pp_keycount(&pp_state);
    cookie = opcookie_init();

    nscheduled = 0;
    for (ii = 0; pcbc_pp_next(&pp_state); ++ii) {
        lcb_CMDUNLOCK cmd = {0};

        PCBC_CHECK_ZVAL_STRING(zcas, "cas must be a string");
        PCBC_CHECK_ZVAL_STRING(zgroupid, "groupid must be a string");

        LCB_CMD_SET_KEY(&cmd, id.str, id.len);
        if (zcas) {
            cmd.cas = cas_decode(zcas TSRMLS_CC);
        }
        if (zgroupid) {
            LCB_KREQ_SIMPLE(&cmd._hashkey, Z_STRVAL_P(zgroupid), Z_STRLEN_P(zgroupid));
        }

        err = lcb_unlock3(data->conn->lcb, cookie, &cmd);
        if (err != LCB_SUCCESS) {
            break;
        }
        nscheduled++;
    }
    pcbc_assert_number_of_commands("unlock", nscheduled, ncmds);

    if (nscheduled) {
        lcb_wait(data->conn->lcb);

        err = proc_unlock_results(data, return_value, cookie, pcbc_pp_ismapped(&pp_state) TSRMLS_CC);
    }

    opcookie_destroy(cookie);

    if (err != LCB_SUCCESS) {
        throw_lcb_exception(err);
    }
}
