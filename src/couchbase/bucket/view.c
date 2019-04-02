/**
 *     Copyright 2017-2019 Couchbase, Inc.
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
    zval id;
    zval key;
    zval value;
} opcookie_viewrow_res;

static void viewrow_callback(lcb_t instance, int ignoreme, const lcb_RESPVIEWQUERY *resp)
{
    opcookie_viewrow_res *result = ecalloc(1, sizeof(opcookie_viewrow_res));
    opcookie *cookie = (opcookie *)resp->cookie;
    int last_error;
    TSRMLS_FETCH();

    ZVAL_UNDEF(&result->id);
    ZVAL_UNDEF(&result->key);
    ZVAL_UNDEF(&result->value);

    ZVAL_NULL(&result->id);
    ZVAL_NULL(&result->key);
    ZVAL_NULL(&result->value);

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
                PCBC_JSON_COPY_DECODE(&result->value, resp->value, resp->nvalue, json_options, last_error);
                if (last_error != 0) {
                    pcbc_log(LOGARGS(instance, WARN), "Failed to decode VIEW value as JSON: json_last_error=%d",
                             last_error);
                    PCBC_STRINGL(result->value, resp->value, resp->nvalue);
                }
            }

            if (resp->nkey) {
                if ((resp->rflags & LCB_RESP_F_FINAL) == 0) {
                    PCBC_JSON_COPY_DECODE(&result->key, resp->key, resp->nkey, json_options, last_error);
                    if (last_error != 0) {
                        pcbc_log(LOGARGS(instance, WARN), "Failed to decode VIEW key as JSON: json_last_error=%d",
                                 last_error);
                        PCBC_STRINGL(result->key, resp->key, resp->nkey);
                    }
                }
            }
        } else {
            PCBC_STRINGL(result->key, resp->key, resp->nkey);
            PCBC_STRINGL(result->value, resp->value, resp->nvalue);
        }
    } else {
        if (resp->htresp->nbody) {
            zval errval;
            char *errbody = (char *)resp->htresp->body;
            int errbody_len = resp->htresp->nbody;

            ZVAL_UNDEF(&errval);
            PCBC_JSON_COPY_DECODE(&errval, errbody, errbody_len, PHP_JSON_OBJECT_AS_ARRAY, last_error);
            if (last_error != 0) {
                pcbc_log(LOGARGS(instance, ERROR), "Failed to perform VIEW query. %d: %.*s",
                         (int)resp->htresp->htstatus, errbody_len, errbody);
            } else {
                char *error = NULL, *reason = NULL;
                int error_len = 0, reason_len = 0;
                zend_bool error_free = 0, reason_free = 0;

                error = php_array_fetch_string(&errval, "error", &error_len, &error_free);
                reason = php_array_fetch_string(&errval, "reason", &reason_len, &reason_free);
                if (error && reason) {
                    char *m = NULL;
                    spprintf(&m, 0, "Failed to perform VIEW query. error: %*s: reason: \"%*s\"", error_len, error,
                             reason_len, reason);
                    ZVAL_UNDEF(&cookie->exc);
                    pcbc_exception_init(&cookie->exc, resp->rc, m TSRMLS_CC);
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
        zval rows;

        ZVAL_UNDEF(&rows);
        array_init(&rows);

        object_init(return_value);
        add_property_zval(return_value, "rows", &rows);
        Z_DELREF_P(&rows);

        FOREACH_OPCOOKIE_RES(opcookie_viewrow_res, res, cookie)
        {
            if (res->rflags & LCB_RESP_F_FINAL) {
                if (Z_TYPE_P(&res->value) == IS_ARRAY) {
                    zval *val;
                    val = php_array_fetch(&res->value, "total_rows");
                    if (val) {
                        add_property_zval(return_value, "total_rows", val);
                    }
                }
            } else {
                zval row;

                ZVAL_UNDEF(&row);
                object_init(&row);
                add_property_zval(&row, "id", &res->id);
                add_property_zval(&row, "key", &res->key);
                add_property_zval(&row, "value", &res->value);
                add_next_index_zval(&rows, &row);
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
#ifdef LCB_TRACING
    lcbtrace_TRACER *tracer = NULL;
    lcb_VIEWHANDLE handle = NULL;
#endif

    cmd->callback = viewrow_callback;
    cookie = opcookie_init();
    cookie->json_response = json_response;
    cookie->json_options = json_options;
#ifdef LCB_TRACING
    tracer = lcb_get_tracer(bucket->conn->lcb);
    if (tracer) {
        cookie->span = lcbtrace_span_start(tracer, "php/view", 0, NULL);
        lcbtrace_span_add_tag_str(cookie->span, LCBTRACE_TAG_COMPONENT, pcbc_client_string);
        lcbtrace_span_add_tag_str(cookie->span, LCBTRACE_TAG_SERVICE, LCBTRACE_TAG_SERVICE_VIEW);
        cmd->handle = &handle;
    }
#endif
    err = lcb_view_query(bucket->conn->lcb, cookie, cmd);
    if (err == LCB_SUCCESS) {
#ifdef LCB_TRACING
        if (cookie->span) {
            lcb_view_set_parent_span(bucket->conn->lcb, handle, cookie->span);
        }
#endif
        lcb_wait(bucket->conn->lcb);
        err = proc_viewrow_results(return_value, cookie TSRMLS_CC);
    }
    if (err != LCB_SUCCESS) {
        if (Z_ISUNDEF(cookie->exc)) {
            throw_lcb_exception(err);
        } else {
            zend_throw_exception_object(&cookie->exc TSRMLS_CC);
        }
    }
#ifdef LCB_TRACING
    if (cookie->span) {
        lcbtrace_span_finish(cookie->span, LCBTRACE_NOW);
    }
#endif
    opcookie_destroy(cookie);
}
