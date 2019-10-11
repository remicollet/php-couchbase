/**
 *     Copyright 2016-2019 Couchbase, Inc.
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

#define LOGARGS(instance, lvl) LCB_LOG_##lvl, instance, "pcbc/http", __FILE__, __LINE__

typedef struct {
    opcookie_res header;
    zval bytes;
} opcookie_http_res;

void http_callback(lcb_INSTANCE *instance, int cbtype, const lcb_RESPHTTP *resp)
{
    opcookie_http_res *result = ecalloc(1, sizeof(opcookie_http_res));
    TSRMLS_FETCH();

    result->header.err = lcb_resphttp_status(resp);
    if (result->header.err != LCB_SUCCESS) {
        pcbc_log(LOGARGS(instance, WARN), "Failed to perform HTTP request: rc=%d", (int)result->header.err);
    }
    opcookie *cookie;
    lcb_resphttp_cookie(resp, (void **)&cookie);

    ZVAL_UNDEF(&result->bytes);
    const char *body = NULL;
    size_t nbody = 0;
    lcb_resphttp_body(resp, &body, &nbody);
    if (nbody) {
        if (cookie->json_response) {
            int last_error;

            PCBC_JSON_COPY_DECODE(&result->bytes, body, nbody, PHP_JSON_OBJECT_AS_ARRAY, last_error);
            if (last_error != 0) {
                pcbc_log(LOGARGS(instance, WARN), "Failed to decode value as JSON: json_last_error=%d", last_error);
                ZVAL_NULL(&result->bytes);
            }
        } else {
            PCBC_STRINGL(result->bytes, body, nbody);
        }
    } else {
        ZVAL_NULL(&result->bytes);
    }

    opcookie_push(cookie, &result->header);
}

static lcb_STATUS proc_http_results(zval *return_value, opcookie *cookie TSRMLS_DC)
{
    opcookie_http_res *res;
    lcb_STATUS err = LCB_SUCCESS;

    // Any error should cause everything to fail... for now?
    err = opcookie_get_first_error(cookie);

    if (err == LCB_SUCCESS) {
        int has_value = 0;
        FOREACH_OPCOOKIE_RES(opcookie_http_res, res, cookie)
        {
            if (has_value == 0) {
                ZVAL_ZVAL(return_value, &res->bytes, 1, 0);
                has_value = 1;
            } else {
                err = LCB_ERROR;
                break;
            }
        }
    }

    FOREACH_OPCOOKIE_RES(opcookie_http_res, res, cookie)
    {
        zval_ptr_dtor(&res->bytes);
    }

    return err;
}

void pcbc_http_request(zval *return_value, lcb_INSTANCE *conn, lcb_CMDHTTP *cmd, int json_response TSRMLS_DC)
{
    lcb_STATUS err;
    opcookie *cookie;

    cookie = opcookie_init();
    cookie->json_response = json_response;
    err = lcb_http(conn, cookie, cmd);
    lcb_cmdhttp_destroy(cmd);
    if (err == LCB_SUCCESS) {
        lcb_wait(conn);
        err = proc_http_results(return_value, cookie TSRMLS_CC);
    }
    opcookie_destroy(cookie);
    if (err != LCB_SUCCESS) {
        throw_lcb_exception(err, NULL);
    }
}

/*
 * vim: et ts=4 sw=4 sts=4
 */
