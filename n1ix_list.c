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
#include "n1ix_spec.h"

typedef struct {
    opcookie_res header;
    zapval *specs;
    int nspecs;
} opcookie_n1ix_list_res;

static void n1ix_list_callback(lcb_t instance, int cbtype, const lcb_RESPN1XMGMT *resp)
{
    opcookie_n1ix_list_res *result = ecalloc(1, sizeof(opcookie_n1ix_list_res));
    int i;
    TSRMLS_FETCH();

    result->header.err = resp->rc;
    if (result->header.err == LCB_QUERY_ERROR) {
        php_error_docref(NULL TSRMLS_CC, E_WARNING,
                         "failed to list indexes. %d: %.*s",
                         (int)resp->inner->htresp->htstatus,
                         (int)resp->inner->nrow,
                         (char *)resp->inner->row);
    }
    result->nspecs = resp->nspecs;
    result->specs = ecalloc(result->nspecs, sizeof(zapval));
    for (i = 0; i < result->nspecs; ++i) {
        const lcb_N1XSPEC *spec = resp->specs[i];
        zapval value, rawjson, json;

        zapval_alloc_stringl(rawjson, spec->rawjson, spec->nrawjson);
        zapval_alloc(json);
        php_json_decode(zapval_zvalptr(json),
                        zapval_strval_p(&rawjson),
                        zapval_strlen_p(&rawjson),
                        1, PHP_JSON_PARSER_DEFAULT_DEPTH TSRMLS_CC);
        zapval_alloc_null(value);
        pcbc_make_n1ix_spec(zapval_zvalptr(value), json TSRMLS_CC);
        result->specs[i] = value;
    }

    opcookie_push((opcookie*)resp->cookie, &result->header);
}

static lcb_error_t proc_n1ix_list_results(bucket_object *bucket,
                                          zval *return_value,
                                          opcookie *cookie TSRMLS_DC)
{
    opcookie_n1ix_list_res *result = (opcookie_n1ix_list_res*)opcookie_next_res(cookie, NULL);
    lcb_error_t err = opcookie_get_first_error(cookie);
    int i;

    if (result) {
        if (err == LCB_SUCCESS) {
            array_init(return_value);
            for (i = 0; i < result->nspecs; ++i) {
                add_index_zval(return_value, i, zapval_zvalptr(result->specs[i]));
            }
        }
        efree(result->specs);
    }

    return err;
}

PHP_METHOD(Bucket, n1ix_list)
{
    bucket_object *data = PCBC_PHP_THISOBJ();
    lcb_CMDN1XMGMT cmd = { 0 };
    opcookie *cookie;
    lcb_error_t err;

    if (zend_parse_parameters_none() == FAILURE) {
        throw_pcbc_exception("Invalid arguments.", LCB_EINVAL);
        RETURN_NULL();
    }

    cmd.callback = n1ix_list_callback;

    cookie = opcookie_init();

    err = lcb_n1x_list(data->conn->lcb, cookie, &cmd);

    if (err == LCB_SUCCESS) {
        lcb_wait(data->conn->lcb);

        err = proc_n1ix_list_results(data, return_value, cookie TSRMLS_CC);
    }

    opcookie_destroy(cookie);

    if (err != LCB_SUCCESS) {
        throw_lcb_exception(err);
    }
}
