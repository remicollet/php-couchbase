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

#define LOGARGS(instance, lvl) LCB_LOG_##lvl, instance, "pcbc/cbft", __FILE__, __LINE__

typedef struct {
    opcookie_res header;
    lcb_U16 rflags;
    zapval row;
} opcookie_ftsrow_res;

static void ftsrow_callback(lcb_t instance, int ignoreme, const lcb_RESPFTS *resp)
{
    opcookie_ftsrow_res *result = ecalloc(1, sizeof(opcookie_ftsrow_res));
    TSRMLS_FETCH();

    result->header.err = resp->rc;
    if (result->header.err == LCB_HTTP_ERROR) {
        pcbc_log(LOGARGS(instance, ERROR), "Failed to search in index. %d: %.*s",
                         (int)resp->htresp->htstatus, (int)resp->nrow, (char *)resp->row);
    }
    result->rflags = resp->rflags;
    zapval_alloc_stringl(result->row, resp->row, resp->nrow);

    opcookie_push((opcookie*)resp->cookie, &result->header);
}

static lcb_error_t proc_ftsrow_results(bucket_object *bucket, zval *return_value,
        opcookie *cookie TSRMLS_DC)
{
    opcookie_ftsrow_res *res;
    lcb_error_t err = LCB_SUCCESS;

    err = opcookie_get_first_error(cookie);

    if (err == LCB_SUCCESS) {
        zval *results_array = NULL;
        zapval zResults;

        zapval_alloc_array(zResults);
        array_init(return_value);
        results_array = zap_hash_str_add(Z_ARRVAL_P(return_value), "results", 7,
            zapval_zvalptr(zResults));

        FOREACH_OPCOOKIE_RES(opcookie_ftsrow_res, res, cookie) {
            if (res->rflags & LCB_RESP_F_FINAL) {
                zap_hash_str_add(Z_ARRVAL_P(return_value), "meta", 4,
                        zapval_zvalptr(res->row));
                zapval_addref(res->row);
            } else {
                zap_hash_next_index_insert(
                        Z_ARRVAL_P(results_array), zapval_zvalptr(res->row));
                zapval_addref(res->row);
            }
        }
    }

    FOREACH_OPCOOKIE_RES(opcookie_ftsrow_res, res, cookie) {
        zapval_destroy(res->row);
    }

    return err;
}

PHP_METHOD(Bucket, fts_request)
{
    bucket_object *data = PCBC_PHP_THISOBJ();
    lcb_CMDFTS cmd = { 0 };
    opcookie *cookie;
    zval *zbody;
    lcb_error_t err;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &zbody) == FAILURE) {
        throw_pcbc_exception("Invalid arguments.", LCB_EINVAL);
        RETURN_NULL();
    }

    PCBC_CHECK_ZVAL_STRING(zbody, "body must be a string");

    cmd.callback = ftsrow_callback;
    cmd.query = Z_STRVAL_P(zbody);
    cmd.nquery = Z_STRLEN_P(zbody);

    cookie = opcookie_init();

    // Execute query
    err = lcb_fts_query(data->conn->lcb, cookie, &cmd);

    if (err == LCB_SUCCESS) {
        lcb_wait(data->conn->lcb);

        err = proc_ftsrow_results(data, return_value, cookie TSRMLS_CC);
    }

    opcookie_destroy(cookie);

    if (err != LCB_SUCCESS) {
        throw_lcb_exception(err);
    }
}
