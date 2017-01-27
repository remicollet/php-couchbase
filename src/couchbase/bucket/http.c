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

#define LOGARGS(instance, lvl) LCB_LOG_##lvl, instance, "pcbc/http", __FILE__, __LINE__

typedef struct {
    opcookie_res header;
    PCBC_ZVAL bytes;
} opcookie_http_res;

void http_callback(lcb_t instance, int cbtype, const lcb_RESPBASE *rb)
{
    opcookie_http_res *result = ecalloc(1, sizeof(opcookie_http_res));
    const lcb_RESPHTTP *resp = (const lcb_RESPHTTP *)rb;
    TSRMLS_FETCH();

    result->header.err = resp->rc;
    if (result->header.err != LCB_SUCCESS) {
        pcbc_log(LOGARGS(instance, WARN), "Failed to perform HTTP request: rc=%d", (int)resp->rc);
    }

#if PHP_VERSION_ID < 70000
    MAKE_STD_ZVAL(result->bytes);
#endif
    if (resp->nbody) {
        if (((opcookie *)rb->cookie)->json_response) {
            int last_error;

            PCBC_JSON_COPY_DECODE(PCBC_P(result->bytes), resp->body, resp->nbody, PHP_JSON_OBJECT_AS_ARRAY, last_error);
            if (last_error != 0) {
                pcbc_log(LOGARGS(instance, WARN), "Failed to decode value as JSON: json_last_error=%d", last_error);
                ZVAL_NULL(PCBC_P(result->bytes));
            }
        } else {
            PCBC_STRINGL(result->bytes, resp->body, resp->nbody);
        }
    } else {
        ZVAL_NULL(PCBC_P(result->bytes));
    }

    opcookie_push((opcookie *)rb->cookie, &result->header);
}

static lcb_error_t proc_http_results(zval *return_value, opcookie *cookie TSRMLS_DC)
{
    opcookie_http_res *res;
    lcb_error_t err = LCB_SUCCESS;

    // Any error should cause everything to fail... for now?
    err = opcookie_get_first_error(cookie);

    if (err == LCB_SUCCESS) {
        int has_value = 0;
        FOREACH_OPCOOKIE_RES(opcookie_http_res, res, cookie)
        {
            if (has_value == 0) {
                ZVAL_ZVAL(return_value, PCBC_P(res->bytes), 1, 0);
                has_value = 1;
            } else {
                err = LCB_ERROR;
                break;
            }
        }
    }

    FOREACH_OPCOOKIE_RES(opcookie_http_res, res, cookie) { zval_ptr_dtor(&res->bytes); }

    return err;
}

void pcbc_http_request(zval *return_value, lcb_t conn, lcb_CMDHTTP *cmd, int json_response TSRMLS_DC)
{
    lcb_error_t err;
    opcookie *cookie;

    cookie = opcookie_init();
    cookie->json_response = json_response;
    err = lcb_http3(conn, cookie, cmd);
    if (err == LCB_SUCCESS) {
        lcb_wait(conn);
        err = proc_http_results(return_value, cookie TSRMLS_CC);
    }
    opcookie_destroy(cookie);
    if (err != LCB_SUCCESS) {
        throw_lcb_exception(err);
    }
}

PHP_METHOD(Bucket, http_request)
{
    pcbc_bucket_t *obj = Z_BUCKET_OBJ_P(getThis());
    lcb_CMDHTTP cmd = {0};
    zval *ztype, *zmethod, *zpath, *zbody, *zcontenttype;
    int rv;

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zzzzz", &ztype, &zmethod, &zpath, &zbody, &zcontenttype);
    if (rv == FAILURE) {
        RETURN_NULL();
    }

    if (Z_LVAL_P(ztype) == 1) {
        cmd.type = LCB_HTTP_TYPE_VIEW;
    } else if (Z_LVAL_P(ztype) == 2) {
        cmd.type = LCB_HTTP_TYPE_MANAGEMENT;
    } else if (Z_LVAL_P(ztype) == 3) {
        cmd.type = LCB_HTTP_TYPE_N1QL;
    } else {
        RETURN_NULL();
    }

    if (Z_LVAL_P(zmethod) == 1) {
        cmd.method = LCB_HTTP_METHOD_GET;
    } else if (Z_LVAL_P(zmethod) == 2) {
        cmd.method = LCB_HTTP_METHOD_POST;
    } else if (Z_LVAL_P(zmethod) == 3) {
        cmd.method = LCB_HTTP_METHOD_PUT;
    } else if (Z_LVAL_P(zmethod) == 4) {
        cmd.method = LCB_HTTP_METHOD_DELETE;
    } else {
        RETURN_NULL();
    }

    if (Z_LVAL_P(zcontenttype) == 1) {
        cmd.content_type = "application/json";
    } else if (Z_LVAL_P(zcontenttype) == 2) {
        cmd.content_type = "application/x-www-form-urlencoded";
    } else {
        RETURN_NULL();
    }

    if (Z_TYPE_P(zpath) == IS_STRING) {
        LCB_CMD_SET_KEY(&cmd, Z_STRVAL_P(zpath), Z_STRLEN_P(zpath));
    }
    if (Z_TYPE_P(zbody) == IS_STRING) {
        cmd.body = Z_STRVAL_P(zbody);
        cmd.nbody = Z_STRLEN_P(zbody);
    }

    pcbc_http_request(return_value, obj->conn->lcb, &cmd, 0 TSRMLS_CC);
}
