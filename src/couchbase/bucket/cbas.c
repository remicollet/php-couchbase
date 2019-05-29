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

#define LOGARGS(instance, lvl) LCB_LOG_##lvl, instance, "pcbc/cbas", __FILE__, __LINE__

extern zend_class_entry *pcbc_analytics_query_result_impl_ce;
extern zend_class_entry *pcbc_meta_data_impl_ce;

struct query_cookie {
    lcb_STATUS rc;
    zval *return_value;
};

static void analytics_callback(lcb_INSTANCE *  instance, int ignoreme, const lcb_RESPANALYTICS *resp)
{
    TSRMLS_FETCH();

    struct query_cookie *cookie;
    lcb_respanalytics_cookie(resp, (void **)&cookie);
    cookie->rc = lcb_respanalytics_status(resp);
    zval *return_value = cookie->return_value;

    zend_update_property_long(pcbc_analytics_query_result_impl_ce, return_value, ZEND_STRL("status"), cookie->rc TSRMLS_CC);

    const char *row = NULL;
    size_t nrow = 0;
    lcb_respanalytics_row(resp, &row, &nrow);

    if (nrow > 0) {
        zval value;
        ZVAL_NULL(&value);

        int last_error;
        PCBC_JSON_COPY_DECODE(&value, row, nrow, PHP_JSON_OBJECT_AS_ARRAY, last_error);
        if(last_error != 0) {
            pcbc_log(LOGARGS(instance, WARN), "Failed to decode N1QL response as JSON: json_last_error=%d", last_error);
        }

        if (lcb_respanalytics_is_final(resp)) {
            zval meta, *mval;
            object_init_ex(&meta, pcbc_meta_data_impl_ce);
            HashTable *marr = Z_ARRVAL(value);

            mval = zend_symtable_str_find(marr, ZEND_STRL("status"));
            if (mval) zend_update_property(pcbc_meta_data_impl_ce, &meta, ZEND_STRL("status"), mval TSRMLS_CC);
            mval = zend_symtable_str_find(marr, ZEND_STRL("requestID"));
            if (mval) zend_update_property(pcbc_meta_data_impl_ce, &meta, ZEND_STRL("request_id"), mval TSRMLS_CC);
            mval = zend_symtable_str_find(marr, ZEND_STRL("clientContextID"));
            if (mval) zend_update_property(pcbc_meta_data_impl_ce, &meta, ZEND_STRL("client_context_id"), mval TSRMLS_CC);
            mval = zend_symtable_str_find(marr, ZEND_STRL("signature"));
            if (mval) zend_update_property(pcbc_meta_data_impl_ce, &meta, ZEND_STRL("signature"), mval TSRMLS_CC);
            mval = zend_symtable_str_find(marr, ZEND_STRL("errors"));
            if (mval) zend_update_property(pcbc_meta_data_impl_ce, &meta, ZEND_STRL("errors"), mval TSRMLS_CC);
            mval = zend_symtable_str_find(marr, ZEND_STRL("warnings"));
            if (mval) zend_update_property(pcbc_meta_data_impl_ce, &meta, ZEND_STRL("warnings"), mval TSRMLS_CC);
            mval = zend_symtable_str_find(marr, ZEND_STRL("metrics"));
            if (mval) zend_update_property(pcbc_meta_data_impl_ce, &meta, ZEND_STRL("metrics"), mval TSRMLS_CC);

            zend_update_property(pcbc_analytics_query_result_impl_ce, return_value, ZEND_STRL("meta"), &meta TSRMLS_CC);
        } else {
            zval *rows, rv;
            rows = zend_read_property(pcbc_analytics_query_result_impl_ce, return_value, ZEND_STRL("rows"), 0, &rv);
            add_next_index_zval(rows, &value);
        }
    }
}

PHP_METHOD(Bucket, analyticsQuery)
{
    lcb_STATUS err;
    zend_string *statement;
    zval *options = NULL;

    int rv = zend_parse_parameters_throw(ZEND_NUM_ARGS() TSRMLS_CC, "S|z", &statement, &options);
    if (rv == FAILURE) {
        RETURN_NULL();
    }

    pcbc_bucket_t *bucket = Z_BUCKET_OBJ_P(getThis());

    lcb_CMDANALYTICS *cmd;
    lcb_cmdanalytics_create(&cmd);
    lcb_cmdanalytics_callback(cmd, analytics_callback);
    lcb_cmdanalytics_statement(cmd, ZSTR_VAL(statement), ZSTR_LEN(statement));

    lcb_ANALYTICS_HANDLE *handle = NULL;
    lcb_cmdanalytics_handle(cmd, &handle);

    lcbtrace_SPAN *span = NULL;
    lcbtrace_TRACER *tracer = lcb_get_tracer(bucket->conn->lcb);
    if (tracer) {
        span = lcbtrace_span_start(tracer,"php/analytics", 0, NULL);
        lcbtrace_span_add_tag_str(span, LCBTRACE_TAG_COMPONENT, pcbc_client_string);
        lcbtrace_span_add_tag_str(span, LCBTRACE_TAG_SERVICE, LCBTRACE_TAG_SERVICE_ANALYTICS);
        lcb_cmdanalytics_parent_span(cmd, span);
    }
    rv = object_init_ex(return_value, pcbc_analytics_query_result_impl_ce);
    if (rv != SUCCESS) {
        return;
    }
    zval rows;
    array_init(&rows);
    zend_update_property(pcbc_analytics_query_result_impl_ce, return_value, ZEND_STRL("rows"), &rows TSRMLS_CC);
    struct query_cookie cookie = {
        LCB_SUCCESS,
        return_value
    };
    err = lcb_analytics(bucket->conn->lcb, &cookie, cmd);
    lcb_cmdanalytics_destroy(cmd);
    if (err == LCB_SUCCESS) {
        lcb_wait(bucket->conn->lcb);
        err = cookie.rc;
    }
    if (span) {
        lcbtrace_span_finish(span, LCBTRACE_NOW);
    }
    if (err != LCB_SUCCESS) {
        throw_lcb_exception(err, NULL);
    }
}

/*
 * vim: et ts=4 sw=4 sts=4
 */
