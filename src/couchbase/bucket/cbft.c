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

#define LOGARGS(instance, lvl) LCB_LOG_##lvl, instance, "pcbc/cbft", __FILE__, __LINE__

extern zend_class_entry *pcbc_search_result_impl_ce;
extern zend_class_entry *pcbc_search_meta_data_impl_ce;
extern zend_class_entry *pcbc_search_query_ce;

struct search_cookie {
    lcb_STATUS rc;
    zval *return_value;
};

static void ftsrow_callback(lcb_INSTANCE *  instance, int ignoreme, const lcb_RESPFTS *resp)
{
    TSRMLS_FETCH();

    struct search_cookie *cookie;
    lcb_respfts_cookie(resp, (void **)&cookie);
    cookie->rc = lcb_respfts_status(resp);
    zval *return_value = cookie->return_value;

    zend_update_property_long(pcbc_search_result_impl_ce, return_value, ZEND_STRL("status"), cookie->rc TSRMLS_CC);

    const char *row = NULL;
    size_t nrow = 0;
    lcb_respfts_row(resp, &row, &nrow);

    if (nrow > 0) {
        zval value;
        ZVAL_NULL(&value);

        int last_error;
        PCBC_JSON_COPY_DECODE(&value, row, nrow, PHP_JSON_OBJECT_AS_ARRAY, last_error);
        if(last_error != 0) {
            pcbc_log(LOGARGS(instance, WARN), "Failed to decode FTS response as JSON: json_last_error=%d", last_error);
        }
        if (lcb_respfts_is_final(resp)) {
            zval meta, *mval, *mstatus;
            object_init_ex(&meta, pcbc_search_meta_data_impl_ce);
            HashTable *marr = Z_ARRVAL(value);

            mval = zend_symtable_str_find(marr, ZEND_STRL("took"));
            if (mval) zend_update_property(pcbc_search_meta_data_impl_ce, &meta, ZEND_STRL("took"), mval TSRMLS_CC);
            mval = zend_symtable_str_find(marr, ZEND_STRL("total_hits"));
            if (mval) zend_update_property(pcbc_search_meta_data_impl_ce, &meta, ZEND_STRL("total_hits"), mval TSRMLS_CC);
            mval = zend_symtable_str_find(marr, ZEND_STRL("max_score"));
            if (mval) zend_update_property(pcbc_search_meta_data_impl_ce, &meta, ZEND_STRL("max_score"), mval TSRMLS_CC);
            mval = zend_symtable_str_find(marr, ZEND_STRL("metrics"));
            if (mval) zend_update_property(pcbc_search_meta_data_impl_ce, &meta, ZEND_STRL("metrics"), mval TSRMLS_CC);
            zend_update_property(pcbc_search_result_impl_ce, return_value, ZEND_STRL("meta"), &meta TSRMLS_CC);

            mstatus = zend_symtable_str_find(marr, ZEND_STRL("status"));
            if (mstatus) {
                mval = zend_symtable_str_find(Z_ARRVAL_P(mstatus), ZEND_STRL("successful"));
                if (mval) zend_update_property(pcbc_search_meta_data_impl_ce, &meta, ZEND_STRL("success_count"), mval TSRMLS_CC);
                mval = zend_symtable_str_find(Z_ARRVAL_P(mstatus), ZEND_STRL("failed"));
                if (mval) zend_update_property(pcbc_search_meta_data_impl_ce, &meta, ZEND_STRL("error_count"), mval TSRMLS_CC);
            }

            mval = zend_symtable_str_find(marr, ZEND_STRL("facets"));
            if (mval) zend_update_property(pcbc_search_result_impl_ce, return_value, ZEND_STRL("facets"), mval TSRMLS_CC);
        } else {
            zval *hits, rv;
            hits = zend_read_property(pcbc_search_result_impl_ce, return_value, ZEND_STRL("hits"), 0, &rv);
            add_next_index_zval(hits, &value);
        }
    }
}

PHP_METHOD(Cluster, searchQuery)
{
    lcb_STATUS err;
    zend_string *index;
    zval *query;
    zval *options = NULL;
    int rv;

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "SO|z", &index, &query, pcbc_search_query_ce, &options);
    if (rv == FAILURE) {
        RETURN_NULL();
    }

    pcbc_cluster_t *cluster = Z_CLUSTER_OBJ_P(getThis());

    lcb_CMDFTS *cmd;
    lcb_cmdfts_create(&cmd);
    lcb_cmdfts_callback(cmd, ftsrow_callback);

    {
        smart_str buf = {0};
        int last_error;

        PCBC_JSON_ENCODE(&buf, query, 0, last_error);
        if (last_error != 0) {
            pcbc_log(LOGARGS(cluster->conn->lcb, WARN), "Failed to encode FTS query as JSON: json_last_error=%d",
                     last_error);
            smart_str_free(&buf);
            RETURN_NULL();
        }
        smart_str_0(&buf);
        lcb_cmdfts_query(cmd, ZSTR_VAL(buf.s), ZSTR_LEN(buf.s));
        smart_str_free(&buf);
    }

    object_init_ex(return_value, pcbc_search_result_impl_ce);
    zval hits;
    array_init(&hits);
    zend_update_property(pcbc_search_result_impl_ce, return_value, ZEND_STRL("hits"), &hits TSRMLS_CC);
    struct search_cookie cookie = {
        LCB_SUCCESS,
        return_value
    };

    lcb_FTS_HANDLE *handle = NULL;
    lcb_cmdfts_handle(cmd, &handle);
    lcbtrace_SPAN *span = NULL;
    lcbtrace_TRACER *tracer = lcb_get_tracer(cluster->conn->lcb);
    if (tracer) {
        span = lcbtrace_span_start(tracer, "php/search", 0, NULL);
        lcbtrace_span_add_tag_str(span, LCBTRACE_TAG_COMPONENT, pcbc_client_string);
        lcbtrace_span_add_tag_str(span, LCBTRACE_TAG_SERVICE, LCBTRACE_TAG_SERVICE_SEARCH);
        lcb_cmdfts_parent_span(cmd, span);
    }
    err = lcb_fts(cluster->conn->lcb, &cookie, cmd);
    lcb_cmdfts_destroy(cmd);
    if (err == LCB_SUCCESS) {
        lcb_wait(cluster->conn->lcb);
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
