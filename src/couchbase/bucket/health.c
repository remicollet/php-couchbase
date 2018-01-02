/**
 *     Copyright 2018 Couchbase, Inc.
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

#define LOGARGS(instance, lvl) LCB_LOG_##lvl, instance, "pcbc/health", __FILE__, __LINE__

typedef struct {
    opcookie_res header;
    PCBC_ZVAL val;
} opcookie_health_res;

static lcb_error_t proc_health_results(zval *return_value, opcookie *cookie TSRMLS_DC)
{
    opcookie_health_res *res;
    lcb_error_t err = LCB_SUCCESS;

    err = opcookie_get_first_error(cookie);

    if (err == LCB_SUCCESS) {
        FOREACH_OPCOOKIE_RES(opcookie_health_res, res, cookie)
        {
            ZVAL_ZVAL(return_value, PCBC_P(res->val), 1, 0);
        }
    }

    FOREACH_OPCOOKIE_RES(opcookie_health_res, res, cookie)
    {
        zval_ptr_dtor(&res->val);
    }
    return err;
}

void ping_callback(lcb_t instance, int cbtype, const lcb_RESPBASE *rb)
{
    opcookie_health_res *result = ecalloc(1, sizeof(opcookie_health_res));
    const lcb_RESPPING *resp = (const lcb_RESPPING *)rb;

    TSRMLS_FETCH();

    result->header.err = resp->rc;
    if (resp->rc == LCB_SUCCESS) {
        int last_error = 0;
        PCBC_ZVAL_ALLOC(result->val);
        PCBC_JSON_COPY_DECODE(PCBC_P(result->val), resp->json, resp->njson, PHP_JSON_OBJECT_AS_ARRAY, last_error);
        if (last_error != 0) {
            pcbc_log(LOGARGS(instance, WARN), "Failed to decode PING response as JSON: json_last_error=%d", last_error);
        }
    }
    opcookie_push((opcookie *)resp->cookie, &result->header);
    (void)instance;
    (void)cbtype;
}

PHP_METHOD(Bucket, ping)
{
    pcbc_bucket_t *obj = Z_BUCKET_OBJ_P(getThis());
    char *report_id = NULL;
    pcbc_str_arg_size report_id_len = 0;
    opcookie *cookie;
    lcb_CMDPING cmd = {0};
    long services = LCB_PINGSVC_F_KV | LCB_PINGSVC_F_N1QL | LCB_PINGSVC_F_VIEWS | LCB_PINGSVC_F_FTS;
    int rv;
    lcb_error_t err;

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|ls", &services, &report_id, &report_id_len);
    if (rv == FAILURE) {
        RETURN_NULL();
    }

    cookie = opcookie_init();
    cmd.id = report_id;
    cmd.services = services;
    cmd.options = LCB_PINGOPT_F_JSON;
    err = lcb_ping3(obj->conn->lcb, cookie, &cmd);
    if (err != LCB_SUCCESS) {
        throw_lcb_exception(err);
    }
    lcb_wait(obj->conn->lcb);
    err = proc_health_results(return_value, cookie TSRMLS_CC);
    opcookie_destroy(cookie);
    if (err != LCB_SUCCESS) {
        throw_lcb_exception(err);
    }
}

void diag_callback(lcb_t instance, int cbtype, const lcb_RESPBASE *rb)
{
    opcookie_health_res *result = ecalloc(1, sizeof(opcookie_health_res));
    const lcb_RESPDIAG *resp = (const lcb_RESPDIAG *)rb;

    TSRMLS_FETCH();

    result->header.err = resp->rc;
    if (resp->rc == LCB_SUCCESS) {
        int last_error = 0;
        PCBC_ZVAL_ALLOC(result->val);
        PCBC_JSON_COPY_DECODE(PCBC_P(result->val), resp->json, resp->njson, PHP_JSON_OBJECT_AS_ARRAY, last_error);
        if (last_error != 0) {
            pcbc_log(LOGARGS(instance, WARN), "Failed to decode PING response as JSON: json_last_error=%d", last_error);
        }
    }
    opcookie_push((opcookie *)resp->cookie, &result->header);
    (void)instance;
    (void)cbtype;
}

PHP_METHOD(Bucket, diag)
{
    pcbc_bucket_t *obj = Z_BUCKET_OBJ_P(getThis());
    char *report_id = NULL;
    pcbc_str_arg_size report_id_len = 0;
    opcookie *cookie;
    lcb_CMDDIAG cmd = {0};
    int rv;
    lcb_error_t err;

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|s", &report_id, &report_id_len);
    if (rv == FAILURE) {
        RETURN_NULL();
    }

    cookie = opcookie_init();
    cmd.id = report_id;
    err = lcb_diag(obj->conn->lcb, cookie, &cmd);
    if (err != LCB_SUCCESS) {
        throw_lcb_exception(err);
    }
    lcb_wait(obj->conn->lcb);
    err = proc_health_results(return_value, cookie TSRMLS_CC);
    opcookie_destroy(cookie);
    if (err != LCB_SUCCESS) {
        throw_lcb_exception(err);
    }
}
