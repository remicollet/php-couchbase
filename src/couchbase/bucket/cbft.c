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

#define LOGARGS(instance, lvl) LCB_LOG_##lvl, instance, "pcbc/cbft", __FILE__, __LINE__

typedef struct {
    opcookie_res header;
    lcb_U16 rflags;
    PCBC_ZVAL row;
} opcookie_ftsrow_res;

static void ftsrow_callback(lcb_t instance, int ignoreme, const lcb_RESPFTS *resp)
{
    opcookie_ftsrow_res *result = ecalloc(1, sizeof(opcookie_ftsrow_res));
    opcookie *cookie = (opcookie *)resp->cookie;
    TSRMLS_FETCH();

    result->header.err = resp->rc;
    if (result->header.err == LCB_HTTP_ERROR) {
        pcbc_log(LOGARGS(instance, ERROR), "Failed to search in index. %d: %.*s", (int)resp->htresp->htstatus,
                 (int)resp->nrow, (char *)resp->row);
    }
    result->rflags = resp->rflags;
    PCBC_ZVAL_ALLOC(result->row);
    if (cookie->json_response) {
        int last_error;
        int json_options = cookie->json_options;

        if (resp->rflags & LCB_RESP_F_FINAL) {
            // parse meta into arrays
            json_options |= PHP_JSON_OBJECT_AS_ARRAY;
        }
        PCBC_JSON_COPY_DECODE(PCBC_P(result->row), resp->row, resp->nrow, json_options, last_error);
        if (last_error != 0) {
            pcbc_log(LOGARGS(instance, WARN), "Failed to decode FTS row as JSON: json_last_error=%d", last_error);
            PCBC_STRINGL(result->row, resp->row, resp->nrow);
        }
    } else {
        PCBC_STRINGL(result->row, resp->row, resp->nrow);
    }
    if (result->header.err != LCB_SUCCESS) {
        zval *val;
        if (Z_TYPE_P(PCBC_P(result->row)) == IS_ARRAY && (val = php_array_fetch(PCBC_P(result->row), "errors"))) {
            zval *err = php_array_fetch(val, "0");
            if (err) {
                char *msg = NULL;
                int msg_len;
                zend_bool need_free = 0;
                long code = php_array_fetch_long(err, "code");
                msg = php_array_fetch_string(err, "msg", &msg_len, &need_free);
                if (code && msg) {
                    char *m = NULL;
                    spprintf(&m, 0, "Failed to perform FTS query. HTTP %d: code: %d, message: \"%*s\"",
                             (int)resp->htresp->htstatus, (int)code, msg_len, msg);
                    PCBC_ZVAL_ALLOC(cookie->exc);
                    pcbc_exception_init(PCBC_P(cookie->exc), code, m TSRMLS_CC);
                    if (m) {
                        efree(m);
                    }
                }
                if (msg && need_free) {
                    efree(msg);
                }
            }
        } else {
            pcbc_log(LOGARGS(instance, ERROR), "Failed to perform FTS query. %d: %.*s", (int)resp->htresp->htstatus,
                     (int)resp->nrow, (char *)resp->row);
        }
    }

    opcookie_push((opcookie *)resp->cookie, &result->header);
}

static lcb_error_t proc_ftsrow_results(pcbc_bucket_t *bucket, zval *return_value, opcookie *cookie TSRMLS_DC)
{
    opcookie_ftsrow_res *res;
    lcb_error_t err = LCB_SUCCESS;

    err = opcookie_get_first_error(cookie);

    if (err == LCB_SUCCESS) {
        PCBC_ZVAL hits;

        PCBC_ZVAL_ALLOC(hits);
        array_init(PCBC_P(hits));

        object_init(return_value);
        add_property_zval(return_value, "hits", PCBC_P(hits));
        Z_DELREF_P(PCBC_P(hits));

        FOREACH_OPCOOKIE_RES(opcookie_ftsrow_res, res, cookie)
        {
            if (res->rflags & LCB_RESP_F_FINAL) {
                PCBC_ZVAL metrics;
                zval *val;

                val = php_array_fetch(PCBC_P(res->row), "status");
                if (val) {
                    add_property_zval(return_value, "status", val);
                }
                val = php_array_fetch(PCBC_P(res->row), "facets");
                if (val) {
                    add_property_zval(return_value, "facets", val);
                }
                PCBC_ZVAL_ALLOC(metrics);
                array_init_size(PCBC_P(metrics), 3);
                ADD_ASSOC_LONG_EX(PCBC_P(metrics), "total_hits", php_array_fetch_long(PCBC_P(res->row), "total_hits"));
                ADD_ASSOC_DOUBLE_EX(PCBC_P(metrics), "max_score",
                                    php_array_fetch_double(PCBC_P(res->row), "max_score"));
                ADD_ASSOC_LONG_EX(PCBC_P(metrics), "took", php_array_fetch_long((PCBC_P(res->row)), "took"));
                add_property_zval(return_value, "metrics", PCBC_P(metrics));
                Z_DELREF_P(PCBC_P(metrics));
            } else {
                add_next_index_zval(PCBC_P(hits), PCBC_P(res->row));
                PCBC_ADDREF_P(PCBC_P(res->row));
            }
        }
    }

    FOREACH_OPCOOKIE_RES(opcookie_ftsrow_res, res, cookie)
    {
        zval_ptr_dtor(&res->row);
    }

    return err;
}

void pcbc_bucket_cbft_request(pcbc_bucket_t *bucket, lcb_CMDFTS *cmd, int json_response, int json_options,
                              zval *return_value TSRMLS_DC)
{
    opcookie *cookie;
    lcb_error_t err;

    cmd->callback = ftsrow_callback;
    cookie = opcookie_init();
    cookie->json_response = json_response;
    cookie->json_options = json_options;
    err = lcb_fts_query(bucket->conn->lcb, cookie, cmd);
    if (err == LCB_SUCCESS) {
        lcb_wait(bucket->conn->lcb);
        err = proc_ftsrow_results(bucket, return_value, cookie TSRMLS_CC);
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
