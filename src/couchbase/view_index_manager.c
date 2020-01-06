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

#define LOGARGS(manager, lvl) LCB_LOG_##lvl, manager->conn->lcb, "pcbc/view_index_manager", __FILE__, __LINE__

typedef struct {
    pcbc_connection_t *conn;
    zend_object std;
} pcbc_view_index_manager_t;

static inline pcbc_view_index_manager_t *pcbc_view_index_manager_fetch_object(zend_object *obj)
{
    return (pcbc_view_index_manager_t *)((char *)obj - XtOffsetOf(pcbc_view_index_manager_t, std));
}
#define Z_VIEW_INDEX_MANAGER_OBJ(zo) (pcbc_view_index_manager_fetch_object(zo))
#define Z_VIEW_INDEX_MANAGER_OBJ_P(zv) (pcbc_view_index_manager_fetch_object(Z_OBJ_P(zv)))
zend_class_entry *pcbc_view_index_manager_ce;

PHP_METHOD(ViewIndexManager, __construct)
{
    throw_pcbc_exception("Accessing private constructor.", LCB_ERR_INVALID_ARGUMENT);
}

PHP_METHOD(ViewIndexManager, getAllDesignDocuments)
{
    pcbc_view_index_manager_t *obj;
    char *path;
    int rv, path_len;

    obj = Z_VIEW_INDEX_MANAGER_OBJ_P(getThis());

    rv = zend_parse_parameters_none();
    if (rv == FAILURE) {
        RETURN_NULL();
    }
    lcb_CMDHTTP *cmd;
    lcb_cmdhttp_create(&cmd, LCB_HTTP_TYPE_MANAGEMENT);
    lcb_cmdhttp_method(cmd, LCB_HTTP_METHOD_GET);
    path_len = spprintf(&path, 0, "/pools/default/buckets/%s/ddocs", obj->conn->bucketname);
    lcb_cmdhttp_path(cmd, path, path_len);
    lcb_cmdhttp_content_type(cmd, PCBC_CONTENT_TYPE_FORM, strlen(PCBC_CONTENT_TYPE_FORM));
    pcbc_http_request(return_value, obj->conn->lcb, cmd, 1 TSRMLS_CC);
    efree(path);
}

PHP_METHOD(ViewIndexManager, upsertDesignDocument)
{
    pcbc_view_index_manager_t *obj;
    char *path, *name = NULL;
    int rv, path_len = 0;
    size_t name_len = 0;
    zval *document;
    smart_str buf = {0};
    int last_error;

    obj = Z_VIEW_INDEX_MANAGER_OBJ_P(getThis());

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sz", &name, &name_len, &document);
    if (rv == FAILURE) {
        return;
    }

    lcb_CMDHTTP *cmd;
    lcb_cmdhttp_create(&cmd, LCB_HTTP_TYPE_VIEW);
    lcb_cmdhttp_method(cmd, LCB_HTTP_METHOD_PUT);
    path_len = spprintf(&path, 0, "/_design/%*s", (int)name_len, name);
    lcb_cmdhttp_path(cmd, path, path_len);
    lcb_cmdhttp_content_type(cmd, PCBC_CONTENT_TYPE_JSON, strlen(PCBC_CONTENT_TYPE_JSON));
    PCBC_JSON_ENCODE(&buf, document, 0, last_error);
    if (last_error != 0) {
        pcbc_log(LOGARGS(obj, WARN), "Failed to encode design document as JSON: json_last_error=%d", last_error);
        lcb_cmdhttp_destroy(cmd);
        smart_str_free(&buf);
        efree(path);
        RETURN_NULL();
    } else {
        smart_str_0(&buf);
        lcb_cmdhttp_body(cmd, ZSTR_VAL(buf.s), ZSTR_LEN(buf.s));
    }
    pcbc_http_request(return_value, obj->conn->lcb, cmd, 1 TSRMLS_CC);
    efree(path);
    smart_str_free(&buf);

    {
        char *error = NULL;
        int error_len = 0;
        zend_bool owned = 0;

        error = php_array_fetch_string(return_value, "error", &error_len, &owned);
        if (error) {
            throw_pcbc_exception(error, LCB_ERR_GENERIC);
            if (owned) {
                efree(error);
            }
            zval_dtor(return_value);
        }
    }
}

PHP_METHOD(ViewIndexManager, dropDesignDocument)
{
    pcbc_view_index_manager_t *obj;
    char *path, *name = NULL;
    int rv, path_len;
    size_t name_len = 0;

    obj = Z_VIEW_INDEX_MANAGER_OBJ_P(getThis());

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &name, &name_len);
    if (rv == FAILURE) {
        return;
    }

    lcb_CMDHTTP *cmd;
    lcb_cmdhttp_create(&cmd, LCB_HTTP_TYPE_VIEW);
    lcb_cmdhttp_method(cmd, LCB_HTTP_METHOD_DELETE);
    path_len = spprintf(&path, 0, "/_design/%*s", (int)name_len, name);
    lcb_cmdhttp_path(cmd, path, path_len);
    lcb_cmdhttp_content_type(cmd, PCBC_CONTENT_TYPE_FORM, strlen(PCBC_CONTENT_TYPE_FORM));
    pcbc_http_request(return_value, obj->conn->lcb, cmd, 1 TSRMLS_CC);
    efree(path);
}

PHP_METHOD(ViewIndexManager, getDesignDocument)
{
    pcbc_view_index_manager_t *obj;
    char *path, *name = NULL;
    int rv, path_len;
    size_t name_len = 0;

    obj = Z_VIEW_INDEX_MANAGER_OBJ_P(getThis());

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &name, &name_len);
    if (rv == FAILURE) {
        return;
    }

    lcb_CMDHTTP *cmd;
    lcb_cmdhttp_create(&cmd, LCB_HTTP_TYPE_VIEW);
    lcb_cmdhttp_method(cmd, LCB_HTTP_METHOD_GET);
    path_len = spprintf(&path, 0, "/_design/%*s", (int)name_len, name);
    lcb_cmdhttp_path(cmd, path, path_len);
    lcb_cmdhttp_content_type(cmd, PCBC_CONTENT_TYPE_FORM, strlen(PCBC_CONTENT_TYPE_FORM));
    pcbc_http_request(return_value, obj->conn->lcb, cmd, 1 TSRMLS_CC);
    efree(path);
    if (php_array_exists(return_value, "error")) {
        zval_dtor(return_value);
        RETURN_BOOL(0);
    }
}

ZEND_BEGIN_ARG_INFO_EX(ai_ViewIndexManager_none, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_ViewIndexManager_getDesignDocument, 0, 0, 1)
ZEND_ARG_INFO(0, name)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_ViewIndexManager_upsertDesignDocument, 0, 0, 2)
ZEND_ARG_INFO(0, name)
ZEND_ARG_INFO(0, document)
ZEND_END_ARG_INFO()

// clang-format off
zend_function_entry view_index_manager_methods[] = {
    PHP_ME(ViewIndexManager, __construct, ai_ViewIndexManager_none, ZEND_ACC_PRIVATE | ZEND_ACC_FINAL | ZEND_ACC_CTOR)
    PHP_ME(ViewIndexManager, getAllDesignDocuments, ai_ViewIndexManager_none, ZEND_ACC_PUBLIC)
    PHP_ME(ViewIndexManager, getDesignDocument, ai_ViewIndexManager_getDesignDocument, ZEND_ACC_PUBLIC)
    PHP_ME(ViewIndexManager, dropDesignDocument, ai_ViewIndexManager_getDesignDocument, ZEND_ACC_PUBLIC)
    PHP_ME(ViewIndexManager, upsertDesignDocument, ai_ViewIndexManager_upsertDesignDocument, ZEND_ACC_PUBLIC)
    PHP_FE_END
};
// clang-format on

void pcbc_view_index_manager_init(zval *return_value, zval *bucket TSRMLS_DC)
{
    pcbc_view_index_manager_t *manager;

    object_init_ex(return_value, pcbc_view_index_manager_ce);
    manager = Z_VIEW_INDEX_MANAGER_OBJ_P(return_value);
    manager->conn = Z_BUCKET_OBJ_P(bucket)->conn;
    pcbc_connection_addref(manager->conn TSRMLS_CC);
}

zend_object_handlers pcbc_view_index_manager_handlers;

static void pcbc_view_index_manager_free_object(zend_object *object TSRMLS_DC)
{
    pcbc_view_index_manager_t *obj = Z_VIEW_INDEX_MANAGER_OBJ(object);

    pcbc_connection_delref(obj->conn TSRMLS_CC);
    obj->conn = NULL;
    zend_object_std_dtor(&obj->std TSRMLS_CC);
}

static zend_object *pcbc_view_index_manager_create_object(zend_class_entry *class_type TSRMLS_DC)
{
    pcbc_view_index_manager_t *obj = NULL;

    obj = PCBC_ALLOC_OBJECT_T(pcbc_view_index_manager_t, class_type);

    zend_object_std_init(&obj->std, class_type TSRMLS_CC);
    object_properties_init(&obj->std, class_type);

    obj->std.handlers = &pcbc_view_index_manager_handlers;
    return &obj->std;
}

static HashTable *pcbc_view_index_manager_get_debug_info(zval *object, int *is_temp TSRMLS_DC)
{
    pcbc_view_index_manager_t *obj = NULL;
    zval retval;

    *is_temp = 1;
    obj = Z_VIEW_INDEX_MANAGER_OBJ_P(object);

    array_init(&retval);
    add_assoc_string(&retval, "bucket", obj->conn->bucketname);

    return Z_ARRVAL(retval);
}

PHP_MINIT_FUNCTION(ViewIndexManager)
{
    zend_class_entry ce;

    INIT_NS_CLASS_ENTRY(ce, "Couchbase", "ViewIndexManager", view_index_manager_methods);
    pcbc_view_index_manager_ce = zend_register_internal_class(&ce TSRMLS_CC);
    pcbc_view_index_manager_ce->create_object = pcbc_view_index_manager_create_object;
    PCBC_CE_DISABLE_SERIALIZATION(pcbc_view_index_manager_ce);

    memcpy(&pcbc_view_index_manager_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    pcbc_view_index_manager_handlers.get_debug_info = pcbc_view_index_manager_get_debug_info;
    pcbc_view_index_manager_handlers.free_obj = pcbc_view_index_manager_free_object;
    pcbc_view_index_manager_handlers.offset = XtOffsetOf(pcbc_view_index_manager_t, std);

    return SUCCESS;
}

/*
 * vim: et ts=4 sw=4 sts=4
 */
