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

extern zend_class_entry *pcbc_view_result_impl_ce;
extern zend_class_entry *pcbc_view_result_entry_ce;
extern zend_class_entry *pcbc_view_meta_data_impl_ce;

struct view_cookie {
    lcb_STATUS rc;
    zval *return_value;
};

static void viewrow_callback(lcb_INSTANCE *  instance, int ignoreme, const lcb_RESPVIEW *resp)
{
    TSRMLS_FETCH();

    struct view_cookie *cookie;
    lcb_respview_cookie(resp, (void **)&cookie);
    cookie->rc = lcb_respview_status(resp);

    const lcb_RESPHTTP *http;
    lcb_respview_http_response(resp, &http);
    uint16_t htstatus;
    lcb_resphttp_http_status(http, &htstatus);

    zval *return_value = cookie->return_value;

    zend_update_property_long(pcbc_view_result_impl_ce, return_value, ZEND_STRL("status"), cookie->rc TSRMLS_CC);
    zend_update_property_long(pcbc_view_result_impl_ce, return_value, ZEND_STRL("http_status"), htstatus TSRMLS_CC);

    int last_error;
    if (cookie->rc == LCB_SUCCESS) {
        if (lcb_respview_is_final(resp)) {
            zval meta, *mval, value;
            object_init_ex(&meta, pcbc_view_meta_data_impl_ce);

            const char *value_str;
            size_t value_len;
            lcb_respview_row(resp, &value_str, &value_len);
            if (value_len) {
                PCBC_JSON_COPY_DECODE(&value, value_str, value_len, PHP_JSON_OBJECT_AS_ARRAY, last_error);
                if (last_error) {
                    pcbc_log(LOGARGS(instance, WARN), "Failed to decode VIEW meta as JSON: json_last_error=%d", last_error);
                } else {
                    HashTable *marr = Z_ARRVAL(value);

                    mval = zend_symtable_str_find(marr, ZEND_STRL("total_rows"));
                    if (mval) zend_update_property(pcbc_view_meta_data_impl_ce, &meta, ZEND_STRL("total_rows"), mval TSRMLS_CC);

                    zend_update_property(pcbc_view_result_impl_ce, return_value, ZEND_STRL("meta"), &meta TSRMLS_CC);
                }
            }
        } else {
            zval entry;
            object_init_ex(&entry, pcbc_view_result_entry_ce);

            const char *id_str;
            size_t id_len;
            lcb_respview_doc_id(resp, &id_str, &id_len);
            if (id_len) {
                zend_update_property_stringl(pcbc_view_result_entry_ce, &entry, ZEND_STRL("id"), id_str, id_len TSRMLS_CC);
            }

            const char *key_str;
            size_t key_len;
            lcb_respview_key(resp, &key_str, &key_len);
            zval key;
            if (key_len) {
                PCBC_JSON_COPY_DECODE(&key, key_str, key_len, PHP_JSON_OBJECT_AS_ARRAY, last_error);
                if (last_error) {
                    pcbc_log(LOGARGS(instance, WARN), "Failed to decode VIEW key as JSON: json_last_error=%d", last_error);
                } else {
                    zend_update_property(pcbc_view_result_entry_ce, &entry, ZEND_STRL("key"), &key TSRMLS_CC);
                }
            }

            const char *value_str;
            size_t value_len;
            lcb_respview_row(resp, &value_str, &value_len);
            zval value;
            if (value_len) {
                PCBC_JSON_COPY_DECODE(&value, value_str, value_len, PHP_JSON_OBJECT_AS_ARRAY, last_error);
                if (last_error) {
                    pcbc_log(LOGARGS(instance, WARN), "Failed to decode VIEW value as JSON: json_last_error=%d", last_error);
                } else {
                    zend_update_property(pcbc_view_result_entry_ce, &entry, ZEND_STRL("value"), &value TSRMLS_CC);
                }
            }

            const char *geometry_str;
            size_t geometry_len;
            lcb_respview_geometry(resp, &geometry_str, &geometry_len);
            zval geometry;
            if (geometry_len) {
                PCBC_JSON_COPY_DECODE(&geometry, geometry_str, geometry_len, PHP_JSON_OBJECT_AS_ARRAY, last_error);
                if (last_error) {
                    pcbc_log(LOGARGS(instance, WARN), "Failed to decode VIEW geometry as JSON: json_last_error=%d", last_error);
                } else {
                    zend_update_property(pcbc_view_result_entry_ce, &entry, ZEND_STRL("geometry"), &geometry TSRMLS_CC);
                }
            }

            const lcb_RESPGET *get;
            lcb_respview_document(resp, &get);
            zval document;
            if (get) {
                const char *doc_str;
                size_t doc_len;
                lcb_respget_value(get, &doc_str, &doc_len);
                if (doc_len) {
                    PCBC_JSON_COPY_DECODE(&document, doc_str, doc_len, PHP_JSON_OBJECT_AS_ARRAY, last_error);
                    if (last_error) {
                        pcbc_log(LOGARGS(instance, WARN), "Failed to decode VIEW document as JSON: json_last_error=%d", last_error);
                    } else {
                        zend_update_property(pcbc_view_result_entry_ce, &entry, ZEND_STRL("document"), &document TSRMLS_CC);
                    }
                }
            }

            zval *rows, rv;
            rows = zend_read_property(pcbc_view_result_impl_ce, return_value, ZEND_STRL("rows"), 0, &rv);
            add_next_index_zval(rows, &entry);
        }
    } else {
        const char *body_str;
        size_t body_len;
        lcb_resphttp_body(http, &body_str, &body_len);
        zval body;
        if (body_len) {
            PCBC_JSON_COPY_DECODE(&body, body_str, body_len, PHP_JSON_OBJECT_AS_ARRAY, last_error);
            if (last_error) {
                pcbc_log(LOGARGS(instance, WARN), "Failed to decode VIEW body as JSON: json_last_error=%d", last_error);
                zend_update_property_stringl(pcbc_view_result_impl_ce, return_value, ZEND_STRL("body_str"), body_str, body_len TSRMLS_CC);
            } else {
                zend_update_property(pcbc_view_result_impl_ce, return_value, ZEND_STRL("body"), &body TSRMLS_CC);
            }
        }
    }
}

PHP_METHOD(Bucket, viewQuery)
{
    int rv;
    zend_string *design_doc;
    zend_string *view_name;
    zval options;

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "SS|z", &design_doc, &view_name, &options);
    if (rv == FAILURE) {
        RETURN_NULL();
    }
    pcbc_bucket_t *obj = Z_BUCKET_OBJ_P(getThis());
    if (obj->type == LCB_BTYPE_EPHEMERAL) {
        throw_pcbc_exception("Ephemeral bucket do not support Couchbase Views", LCB_EINVAL);
        RETURN_NULL();
    }

    lcb_CMDVIEW *cmd;
    lcb_cmdview_create(&cmd);
    lcb_cmdview_design_document(cmd, ZSTR_VAL(design_doc), ZSTR_LEN(design_doc));
    lcb_cmdview_view_name(cmd, ZSTR_VAL(view_name), ZSTR_LEN(view_name));
    lcb_cmdview_callback(cmd, viewrow_callback);

    lcb_VIEW_HANDLE *handle = NULL;
    lcb_cmdview_handle(cmd, &handle);

    lcbtrace_SPAN *span = NULL;
    lcbtrace_TRACER *tracer = lcb_get_tracer(obj->conn->lcb);
    if (tracer) {
        span = lcbtrace_span_start(tracer, "php/view", 0, NULL);
        lcbtrace_span_add_tag_str(span, LCBTRACE_TAG_COMPONENT, pcbc_client_string);
        lcbtrace_span_add_tag_str(span, LCBTRACE_TAG_SERVICE, LCBTRACE_TAG_SERVICE_VIEW);
        lcb_cmdview_parent_span(cmd, span);
    }

    rv = object_init_ex(return_value, pcbc_view_result_impl_ce);
    if (rv != SUCCESS) {
        return;
    }
    zval rows;
    array_init(&rows);
    zend_update_property(pcbc_view_result_impl_ce, return_value, ZEND_STRL("rows"), &rows TSRMLS_CC);
    struct view_cookie cookie = {
        LCB_SUCCESS,
        return_value
    };
    lcb_STATUS err = lcb_view(obj->conn->lcb, &cookie, cmd);
    lcb_cmdview_destroy(cmd);
    if (err == LCB_SUCCESS) {
        lcb_wait(obj->conn->lcb);
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
