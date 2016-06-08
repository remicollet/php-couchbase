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
    zapval bytes;
} opcookie_http_res;

void http_callback(lcb_t instance, int cbtype, const lcb_RESPBASE *rb)
{
    opcookie_http_res *result = ecalloc(1, sizeof(opcookie_http_res));
    const lcb_RESPHTTP *resp = (const lcb_RESPHTTP *)rb;
    TSRMLS_FETCH();

    result->header.err = resp->rc;
    zapval_alloc_stringl(result->bytes, resp->body, resp->nbody);

    opcookie_push((opcookie*)rb->cookie, &result->header);
}

static lcb_error_t proc_http_results(bucket_object *bucket, zval *return_value,
                                     opcookie *cookie TSRMLS_DC)
{
    opcookie_http_res *res;
    lcb_error_t err = LCB_SUCCESS;

    // Any error should cause everything to fail... for now?
    err = opcookie_get_first_error(cookie);

    if (err == LCB_SUCCESS) {
        int has_value = 0;
        FOREACH_OPCOOKIE_RES(opcookie_http_res, res, cookie) {
            if (has_value == 0) {
                zap_zval_zval_p(return_value, zapval_zvalptr(res->bytes), 1, 0);
                has_value = 1;
            } else {
                err = LCB_ERROR;
                break;
            }
        }
    }

    FOREACH_OPCOOKIE_RES(opcookie_http_res, res, cookie) {
        zapval_destroy(res->bytes);
    }

    return err;
}

PHP_METHOD(Bucket, http_request)
{
    bucket_object *data = PCBC_PHP_THISOBJ();
    lcb_CMDHTTP cmd = { 0 };
    opcookie *cookie;
    zval *ztype, *zmethod, *zpath, *zbody, *zcontenttype;
    lcb_error_t err;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zzzzz",
                              &ztype, &zmethod, &zpath, &zbody, &zcontenttype) == FAILURE) {
        throw_pcbc_exception("Invalid arguments.", LCB_EINVAL);
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

    cookie = opcookie_init();

    err = lcb_http3(data->conn->lcb, cookie, &cmd);

    if (err == LCB_SUCCESS) {
        lcb_wait(data->conn->lcb);

        err = proc_http_results(data, return_value, cookie TSRMLS_CC);
    }

    opcookie_destroy(cookie);

    if (err != LCB_SUCCESS) {
        throw_lcb_exception(err);
    }
}
