/**
 *     Copyright 2017 Couchbase, Inc.
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

#define LOGARGS(instance, lvl) LCB_LOG_##lvl, instance, "pcbc/view", __FILE__, __LINE__

typedef struct {
    opcookie_res header;
    lcb_U16 rflags;
    PCBC_ZVAL id;
    PCBC_ZVAL key;
    PCBC_ZVAL value;
} opcookie_viewrow_res;

static void viewrow_callback(lcb_t instance, int ignoreme, const lcb_RESPVIEWQUERY *resp)
{
    opcookie_viewrow_res *result = ecalloc(1, sizeof(opcookie_viewrow_res));
    opcookie *cookie = (opcookie *)resp->cookie;
    int last_error;
    TSRMLS_FETCH();

#if PHP_VERSION_ID < 70000
    MAKE_STD_ZVAL(result->id);
    MAKE_STD_ZVAL(result->key);
    MAKE_STD_ZVAL(result->value);
#endif
    result->header.err = resp->rc;
    result->rflags = resp->rflags;
    if (result->header.err == LCB_SUCCESS) {
        PCBC_STRINGL(result->id, resp->docid, resp->ndocid);
        if (cookie->json_response) {
            int json_options = cookie->json_options;

            if (resp->rflags & LCB_RESP_F_FINAL) {
                // parse meta into arrays
                json_options |= PHP_JSON_OBJECT_AS_ARRAY;
            }

            if (resp->nvalue) {
                PCBC_JSON_COPY_DECODE(PCBC_P(result->value), resp->value, resp->nvalue, json_options, last_error);
                if (last_error != 0) {
                    pcbc_log(LOGARGS(instance, WARN), "Failed to decode VIEW value as JSON: json_last_error=%d",
                             last_error);
                    PCBC_STRINGL(result->value, resp->value, resp->nvalue);
                }
            } else {
                ZVAL_NULL(PCBC_P(result->value));
            }

            if (resp->nkey) {
                if ((resp->rflags & LCB_RESP_F_FINAL) == 0) {
                    PCBC_JSON_COPY_DECODE(PCBC_P(result->key), resp->key, resp->nkey, json_options, last_error);
                    if (last_error != 0) {
                        pcbc_log(LOGARGS(instance, WARN), "Failed to decode VIEW key as JSON: json_last_error=%d",
                                 last_error);
                        PCBC_STRINGL(result->key, resp->key, resp->nkey);
                    }
                }
            } else {
                ZVAL_NULL(PCBC_P(result->key));
            }
        } else {
            PCBC_STRINGL(result->key, resp->key, resp->nkey);
            PCBC_STRINGL(result->value, resp->value, resp->nvalue);
        }
    } else {
        if (resp->htresp->nbody) {
            PCBC_ZVAL errval;
            char *errbody = (char *)resp->htresp->body;
            int errbody_len = resp->htresp->nbody;

            PCBC_ZVAL_ALLOC(errval);
            PCBC_JSON_COPY_DECODE(PCBC_P(errval), errbody, errbody_len, PHP_JSON_OBJECT_AS_ARRAY, last_error);
            if (last_error != 0) {
                pcbc_log(LOGARGS(instance, ERROR), "Failed to perform VIEW query. %d: %.*s",
                         (int)resp->htresp->htstatus, errbody_len, errbody);
            } else {
                char *error = NULL, *reason = NULL;
                int error_len = 0, reason_len = 0;
                zend_bool error_free = 0, reason_free = 0;

                error = php_array_fetch_string(PCBC_P(errval), "error", &error_len, &error_free);
                reason = php_array_fetch_string(PCBC_P(errval), "reason", &reason_len, &reason_free);
                if (error && reason) {
                    char *m = NULL;
                    spprintf(&m, 0, "Failed to perform VIEW query. error: %*s: reason: \"%*s\"", error_len, error,
                             reason_len, reason);
                    PCBC_ZVAL_ALLOC(cookie->exc);
                    pcbc_exception_init(PCBC_P(cookie->exc), resp->rc, m TSRMLS_CC);
                    if (m) {
                        efree(m);
                    }
                } else {
                    pcbc_log(LOGARGS(instance, ERROR), "Failed to perform VIEW query. %d: %.*s",
                             (int)resp->htresp->htstatus, errbody_len, errbody);
                }
                if (error && error_free) {
                    efree(error);
                }
                if (reason && reason_free) {
                    efree(reason);
                }
            }
            zval_ptr_dtor(&errval);
        }
    }

    opcookie_push((opcookie *)resp->cookie, &result->header);
}

static lcb_error_t proc_viewrow_results(zval *return_value, opcookie *cookie TSRMLS_DC)
{
    opcookie_viewrow_res *res;
    lcb_error_t err = LCB_SUCCESS;

    // Any error should cause everything to fail... for now?
    err = opcookie_get_first_error(cookie);

    if (err == LCB_SUCCESS) {
        PCBC_ZVAL rows;
#if PHP_VERSION_ID < 70000
        MAKE_STD_ZVAL(rows);
#endif
        array_init(PCBC_P(rows));

        object_init(return_value);
        add_property_zval(return_value, "rows", PCBC_P(rows));
        Z_DELREF_P(PCBC_P(rows));

        FOREACH_OPCOOKIE_RES(opcookie_viewrow_res, res, cookie)
        {
            if (res->rflags & LCB_RESP_F_FINAL) {
                if (Z_TYPE_P(PCBC_P(res->value)) == IS_ARRAY) {
                    zval *val;
                    val = php_array_fetch(PCBC_P(res->value), "total_rows");
                    if (val) {
                        add_property_zval(return_value, "total_rows", val);
                    }
                }
            } else {
                PCBC_ZVAL row;
#if PHP_VERSION_ID < 70000
                MAKE_STD_ZVAL(row);
#endif
                object_init(PCBC_P(row));
                add_property_zval(PCBC_P(row), "id", PCBC_P(res->id));
                add_property_zval(PCBC_P(row), "key", PCBC_P(res->key));
                add_property_zval(PCBC_P(row), "value", PCBC_P(res->value));
                add_next_index_zval(PCBC_P(rows), PCBC_P(row));
            }
        }
    }

    FOREACH_OPCOOKIE_RES(opcookie_viewrow_res, res, cookie)
    {
        zval_ptr_dtor(&res->id);
        zval_ptr_dtor(&res->key);
        zval_ptr_dtor(&res->value);
    }

    return err;
}

void pcbc_bucket_view_request(pcbc_bucket_t *bucket, lcb_CMDVIEWQUERY *cmd, int json_response, int json_options,
                              zval *return_value TSRMLS_DC)
{
    opcookie *cookie;
    lcb_error_t err;

    cmd->callback = viewrow_callback;
    cookie = opcookie_init();
    cookie->json_response = json_response;
    cookie->json_options = json_options;
    err = lcb_view_query(bucket->conn->lcb, cookie, cmd);
    if (err == LCB_SUCCESS) {
        lcb_wait(bucket->conn->lcb);
        err = proc_viewrow_results(return_value, cookie TSRMLS_CC);
    }
    if (err != LCB_SUCCESS) {
        if (Z_ISUNDEF(cookie->exc)) {
            throw_lcb_exception(err);
        } else {
            zend_throw_exception_object(PCBC_P(cookie->exc) TSRMLS_CC);
        }
    }
    opcookie_destroy(cookie);
}
