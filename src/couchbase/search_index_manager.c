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
#include <ext/standard/php_array.h>
#include <ext/standard/php_http.h>
#include <ext/standard/url.h>

#define LOGARGS(instance, lvl) LCB_LOG_##lvl, instance, "pcbc/search_index_manager", __FILE__, __LINE__

zend_class_entry *pcbc_search_index_manager_ce;
extern zend_class_entry *pcbc_password_authenticator_ce;

PHP_METHOD(SearchIndexManager, __construct)
{
    throw_pcbc_exception("Accessing private constructor.", LCB_EINVAL);
}

PHP_METHOD(SearchIndexManager, listIndexes)
{
    pcbc_search_index_manager_t *obj;
    lcb_CMDHTTP cmd = {0};
    const char *path = "/api/index";
    int rv;

    obj = Z_SEARCH_INDEX_MANAGER_OBJ_P(getThis());

    rv = zend_parse_parameters_none();
    if (rv == FAILURE) {
        RETURN_NULL();
    }

    cmd.type = LCB_HTTP_TYPE_FTS;
    cmd.method = LCB_HTTP_METHOD_GET;
    LCB_CMD_SET_KEY(&cmd, path, strlen(path));
    cmd.content_type = PCBC_CONTENT_TYPE_FORM;
    pcbc_http_request(return_value, obj->conn->lcb, &cmd, 1 TSRMLS_CC);
}

PHP_METHOD(SearchIndexManager, getIndex)
{
    pcbc_search_index_manager_t *obj;
    lcb_CMDHTTP cmd = {0};
    int rv, path_len;
    char *path, *name = NULL;
    pcbc_str_arg_size name_len = 0;

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &name, &name_len);
    if (rv == FAILURE) {
        RETURN_NULL();
    }

    path_len = spprintf(&path, 0, "/api/index/%.*s", (int)name_len, name);
    obj = Z_SEARCH_INDEX_MANAGER_OBJ_P(getThis());

    cmd.type = LCB_HTTP_TYPE_FTS;
    cmd.method = LCB_HTTP_METHOD_GET;
    LCB_CMD_SET_KEY(&cmd, path, path_len);
    cmd.content_type = PCBC_CONTENT_TYPE_FORM;
    pcbc_http_request(return_value, obj->conn->lcb, &cmd, 1 TSRMLS_CC);
    efree(path);
}

PHP_METHOD(SearchIndexManager, deleteIndex)
{
    pcbc_search_index_manager_t *obj;
    lcb_CMDHTTP cmd = {0};
    int rv, path_len;
    char *path, *name = NULL;
    pcbc_str_arg_size name_len = 0;

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &name, &name_len);
    if (rv == FAILURE) {
        RETURN_NULL();
    }

    path_len = spprintf(&path, 0, "/api/index/%.*s", (int)name_len, name);
    obj = Z_SEARCH_INDEX_MANAGER_OBJ_P(getThis());

    cmd.type = LCB_HTTP_TYPE_FTS;
    cmd.method = LCB_HTTP_METHOD_DELETE;
    LCB_CMD_SET_KEY(&cmd, path, path_len);
    cmd.content_type = PCBC_CONTENT_TYPE_FORM;
    pcbc_http_request(return_value, obj->conn->lcb, &cmd, 1 TSRMLS_CC);
    efree(path);
}

PHP_METHOD(SearchIndexManager, createIndex)
{
    pcbc_search_index_manager_t *obj;
    lcb_CMDHTTP cmd = {0};
    int rv, path_len;
    char *def = NULL, *name = NULL;
    pcbc_str_arg_size def_len = 0, name_len = 0;
    char *path = NULL;

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss", &name, &name_len, &def, &def_len);
    if (rv == FAILURE) {
        RETURN_NULL();
    }

    obj = Z_SEARCH_INDEX_MANAGER_OBJ_P(getThis());
    path_len = spprintf(&path, 0, "/api/index/%.*s", (int)name_len, name);

    cmd.type = LCB_HTTP_TYPE_FTS;
    cmd.method = LCB_HTTP_METHOD_PUT;
    LCB_CMD_SET_KEY(&cmd, path, path_len);
    cmd.body = def;
    cmd.nbody = def_len;
    cmd.content_type = PCBC_CONTENT_TYPE_JSON;
    pcbc_http_request(return_value, obj->conn->lcb, &cmd, 1 TSRMLS_CC);
    efree(path);
}

PHP_METHOD(SearchIndexManager, getIndexedDocumentsCount)
{
    pcbc_search_index_manager_t *obj;
    lcb_CMDHTTP cmd = {0};
    int rv, path_len;
    char *path, *name = NULL;
    pcbc_str_arg_size name_len = 0;

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &name, &name_len);
    if (rv == FAILURE) {
        RETURN_NULL();
    }

    path_len = spprintf(&path, 0, "/api/index/%.*s/count", (int)name_len, name);
    obj = Z_SEARCH_INDEX_MANAGER_OBJ_P(getThis());

    cmd.type = LCB_HTTP_TYPE_FTS;
    cmd.method = LCB_HTTP_METHOD_GET;
    LCB_CMD_SET_KEY(&cmd, path, path_len);
    cmd.content_type = PCBC_CONTENT_TYPE_FORM;
    pcbc_http_request(return_value, obj->conn->lcb, &cmd, 1 TSRMLS_CC);
    efree(path);
}

ZEND_BEGIN_ARG_INFO_EX(ai_SearchIndexManager_none, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_SearchIndexManager_getIndex, 0, 0, 1)
ZEND_ARG_INFO(0, name)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_SearchIndexManager_deleteIndex, 0, 0, 1)
ZEND_ARG_INFO(0, name)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_SearchIndexManager_createIndex, 0, 0, 2)
ZEND_ARG_INFO(0, name)
ZEND_ARG_INFO(0, indexDefinition)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_SearchIndexManager_getIndexedDocumentsCount, 0, 0, 1)
ZEND_ARG_INFO(0, name)
ZEND_END_ARG_INFO()

// clang-format off
zend_function_entry search_index_manager_methods[] = {
    PHP_ME(SearchIndexManager, __construct, ai_SearchIndexManager_none, ZEND_ACC_PRIVATE | ZEND_ACC_FINAL | ZEND_ACC_CTOR)
    PHP_ME(SearchIndexManager, listIndexes, ai_SearchIndexManager_none, ZEND_ACC_PUBLIC)
    PHP_ME(SearchIndexManager, getIndex, ai_SearchIndexManager_getIndex, ZEND_ACC_PUBLIC)
    PHP_ME(SearchIndexManager, createIndex, ai_SearchIndexManager_createIndex, ZEND_ACC_PUBLIC)
    PHP_ME(SearchIndexManager, deleteIndex, ai_SearchIndexManager_deleteIndex, ZEND_ACC_PUBLIC)
    PHP_ME(SearchIndexManager, getIndexedDocumentsCount, ai_SearchIndexManager_getIndexedDocumentsCount, ZEND_ACC_PUBLIC)

    /* these aliases might be deprecated later or replace original methods */
    PHP_MALIAS(SearchIndexManager, listIndexDefinitions, listIndexes, ai_SearchIndexManager_none, ZEND_ACC_PUBLIC)
    PHP_MALIAS(SearchIndexManager, listIndexDefinition, getIndex, ai_SearchIndexManager_getIndex, ZEND_ACC_PUBLIC)
    PHP_MALIAS(SearchIndexManager, getIndexDefinition, getIndex, ai_SearchIndexManager_getIndex, ZEND_ACC_PUBLIC)
    PHP_MALIAS(SearchIndexManager, getIndexedDocumentCount, getIndexedDocumentsCount, ai_SearchIndexManager_getIndexedDocumentsCount, ZEND_ACC_PUBLIC)
    PHP_FE_END
};
// clang-format on

zend_object_handlers pcbc_search_index_manager_handlers;

static void pcbc_search_index_manager_free_object(pcbc_free_object_arg *object TSRMLS_DC) /* {{{ */
{
    pcbc_search_index_manager_t *obj = Z_SEARCH_INDEX_MANAGER_OBJ(object);
    pcbc_connection_delref(obj->conn TSRMLS_CC);
    obj->conn = NULL;

    zend_object_std_dtor(&obj->std TSRMLS_CC);
#if PHP_VERSION_ID < 70000
    efree(obj);
#endif
} /* }}} */

static pcbc_create_object_retval pcbc_search_index_manager_create_object(zend_class_entry *class_type TSRMLS_DC)
{
    pcbc_search_index_manager_t *obj = NULL;

    obj = PCBC_ALLOC_OBJECT_T(pcbc_search_index_manager_t, class_type);

    zend_object_std_init(&obj->std, class_type TSRMLS_CC);
    object_properties_init(&obj->std, class_type);

#if PHP_VERSION_ID >= 70000
    obj->std.handlers = &pcbc_search_index_manager_handlers;
    return &obj->std;
#else
    {
        zend_object_value ret;
        ret.handle = zend_objects_store_put(obj, (zend_objects_store_dtor_t)zend_objects_destroy_object,
                                            pcbc_search_index_manager_free_object, NULL TSRMLS_CC);
        ret.handlers = &pcbc_search_index_manager_handlers;
        return ret;
    }
#endif
}

void pcbc_search_index_manager_init(zval *return_value, pcbc_bucket_manager_t *bucket_manager TSRMLS_DC)
{
    pcbc_search_index_manager_t *manager;

    object_init_ex(return_value, pcbc_search_index_manager_ce);
    manager = Z_SEARCH_INDEX_MANAGER_OBJ_P(return_value);
    manager->conn = bucket_manager->conn;
    pcbc_connection_addref(manager->conn TSRMLS_CC);
}

static HashTable *pcbc_search_index_manager_get_debug_info(zval *object, int *is_temp TSRMLS_DC) /* {{{ */
{
/* pcbc_search_index_manager_t *obj = NULL; */
#if PHP_VERSION_ID >= 70000
    zval retval;
#else
    zval retval = zval_used_for_init;
#endif

    *is_temp = 1;
    /* obj = Z_SEARCH_INDEX_MANAGER_OBJ_P(object); */

    array_init(&retval);

    return Z_ARRVAL(retval);
} /* }}} */

PHP_MINIT_FUNCTION(SearchIndexManager)
{
    zend_class_entry ce;

    INIT_NS_CLASS_ENTRY(ce, "Couchbase", "SearchIndexManager", search_index_manager_methods);
    pcbc_search_index_manager_ce = zend_register_internal_class(&ce TSRMLS_CC);
    pcbc_search_index_manager_ce->create_object = pcbc_search_index_manager_create_object;
    PCBC_CE_DISABLE_SERIALIZATION(pcbc_search_index_manager_ce);

    memcpy(&pcbc_search_index_manager_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    pcbc_search_index_manager_handlers.get_debug_info = pcbc_search_index_manager_get_debug_info;
#if PHP_VERSION_ID >= 70000
    pcbc_search_index_manager_handlers.free_obj = pcbc_search_index_manager_free_object;
    pcbc_search_index_manager_handlers.offset = XtOffsetOf(pcbc_search_index_manager_t, std);
#endif
    return SUCCESS;
}
