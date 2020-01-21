/**
 *     Copyright 2018-2019 Couchbase, Inc.
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

static inline pcbc_search_index_manager_t *pcbc_search_index_manager_fetch_object(zend_object *obj)
{
    return (pcbc_search_index_manager_t *)((char *)obj - XtOffsetOf(pcbc_search_index_manager_t, std));
}
#define Z_SEARCH_INDEX_MANAGER_OBJ(zo) (pcbc_search_index_manager_fetch_object(zo))
#define Z_SEARCH_INDEX_MANAGER_OBJ_P(zv) (pcbc_search_index_manager_fetch_object(Z_OBJ_P(zv)))

zend_class_entry *pcbc_search_index_manager_ce;
extern zend_class_entry *pcbc_password_authenticator_ce;

PHP_METHOD(SearchIndexManager, __construct)
{
    throw_pcbc_exception("Accessing private constructor.", LCB_ERR_INVALID_ARGUMENT);
}

PHP_METHOD(SearchIndexManager, listIndexes)
{
    pcbc_search_index_manager_t *obj;
    const char *path = "/api/index";
    int rv;

    obj = Z_SEARCH_INDEX_MANAGER_OBJ_P(getThis());

    rv = zend_parse_parameters_none();
    if (rv == FAILURE) {
        RETURN_NULL();
    }

    lcb_CMDHTTP *cmd;
    lcb_cmdhttp_create(&cmd, LCB_HTTP_TYPE_SEARCH);
    lcb_cmdhttp_method(cmd, LCB_HTTP_METHOD_GET);
    lcb_cmdhttp_path(cmd, path, strlen(path));
    lcb_cmdhttp_content_type(cmd, PCBC_CONTENT_TYPE_FORM, strlen(PCBC_CONTENT_TYPE_FORM));
    pcbc_http_request(return_value, obj->conn->lcb, cmd, 1, NULL TSRMLS_CC);
}

PHP_METHOD(SearchIndexManager, getIndex)
{
    pcbc_search_index_manager_t *obj;
    int rv, path_len;
    char *path, *name = NULL;
    size_t name_len = 0;

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &name, &name_len);
    if (rv == FAILURE) {
        RETURN_NULL();
    }

    path_len = spprintf(&path, 0, "/api/index/%.*s", (int)name_len, name);
    obj = Z_SEARCH_INDEX_MANAGER_OBJ_P(getThis());

    lcb_CMDHTTP *cmd;
    lcb_cmdhttp_create(&cmd, LCB_HTTP_TYPE_SEARCH);
    lcb_cmdhttp_method(cmd, LCB_HTTP_METHOD_GET);
    lcb_cmdhttp_path(cmd, path, path_len);
    lcb_cmdhttp_content_type(cmd, PCBC_CONTENT_TYPE_FORM, strlen(PCBC_CONTENT_TYPE_FORM));
    pcbc_http_request(return_value, obj->conn->lcb, cmd, 1, NULL TSRMLS_CC);
    efree(path);
}

PHP_METHOD(SearchIndexManager, deleteIndex)
{
    pcbc_search_index_manager_t *obj;
    int rv, path_len;
    char *path, *name = NULL;
    size_t name_len = 0;

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &name, &name_len);
    if (rv == FAILURE) {
        RETURN_NULL();
    }

    path_len = spprintf(&path, 0, "/api/index/%.*s", (int)name_len, name);
    obj = Z_SEARCH_INDEX_MANAGER_OBJ_P(getThis());

    lcb_CMDHTTP *cmd;
    lcb_cmdhttp_create(&cmd, LCB_HTTP_TYPE_SEARCH);
    lcb_cmdhttp_method(cmd, LCB_HTTP_METHOD_DELETE);
    lcb_cmdhttp_path(cmd, path, path_len);
    lcb_cmdhttp_content_type(cmd, PCBC_CONTENT_TYPE_FORM, strlen(PCBC_CONTENT_TYPE_FORM));
    pcbc_http_request(return_value, obj->conn->lcb, cmd, 1, NULL TSRMLS_CC);
    efree(path);
}

PHP_METHOD(SearchIndexManager, createIndex)
{
    pcbc_search_index_manager_t *obj;
    int rv, path_len;
    char *def = NULL, *name = NULL;
    size_t def_len = 0, name_len = 0;
    char *path = NULL;

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss", &name, &name_len, &def, &def_len);
    if (rv == FAILURE) {
        RETURN_NULL();
    }

    obj = Z_SEARCH_INDEX_MANAGER_OBJ_P(getThis());
    path_len = spprintf(&path, 0, "/api/index/%.*s", (int)name_len, name);

    lcb_CMDHTTP *cmd;
    lcb_cmdhttp_create(&cmd, LCB_HTTP_TYPE_SEARCH);
    lcb_cmdhttp_method(cmd, LCB_HTTP_METHOD_PUT);
    lcb_cmdhttp_path(cmd, path, path_len);
    lcb_cmdhttp_body(cmd, def, def_len);
    lcb_cmdhttp_content_type(cmd, PCBC_CONTENT_TYPE_JSON, strlen(PCBC_CONTENT_TYPE_JSON));
    pcbc_http_request(return_value, obj->conn->lcb, cmd, 1, NULL TSRMLS_CC);
    efree(path);
}

PHP_METHOD(SearchIndexManager, getIndexedDocumentsCount)
{
    pcbc_search_index_manager_t *obj;
    int rv, path_len;
    char *path, *name = NULL;
    size_t name_len = 0;

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &name, &name_len);
    if (rv == FAILURE) {
        RETURN_NULL();
    }

    path_len = spprintf(&path, 0, "/api/index/%.*s/count", (int)name_len, name);
    obj = Z_SEARCH_INDEX_MANAGER_OBJ_P(getThis());

    lcb_CMDHTTP *cmd;
    lcb_cmdhttp_create(&cmd, LCB_HTTP_TYPE_SEARCH);
    lcb_cmdhttp_method(cmd, LCB_HTTP_METHOD_GET);
    lcb_cmdhttp_path(cmd, path, path_len);
    lcb_cmdhttp_content_type(cmd, PCBC_CONTENT_TYPE_FORM, strlen(PCBC_CONTENT_TYPE_FORM));
    pcbc_http_request(return_value, obj->conn->lcb, cmd, 1, NULL TSRMLS_CC);
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

static void pcbc_search_index_manager_free_object(zend_object *object TSRMLS_DC)
{
    pcbc_search_index_manager_t *obj = Z_SEARCH_INDEX_MANAGER_OBJ(object);
    pcbc_connection_delref(obj->conn TSRMLS_CC);
    obj->conn = NULL;

    zend_object_std_dtor(&obj->std TSRMLS_CC);
}

static zend_object *pcbc_search_index_manager_create_object(zend_class_entry *class_type TSRMLS_DC)
{
    pcbc_search_index_manager_t *obj = NULL;

    obj = PCBC_ALLOC_OBJECT_T(pcbc_search_index_manager_t, class_type);

    zend_object_std_init(&obj->std, class_type TSRMLS_CC);
    object_properties_init(&obj->std, class_type);

    obj->std.handlers = &pcbc_search_index_manager_handlers;
    return &obj->std;
}

void pcbc_search_index_manager_init(zval *return_value, zval *cluster TSRMLS_DC)
{
    pcbc_search_index_manager_t *manager;

    object_init_ex(return_value, pcbc_search_index_manager_ce);
    manager = Z_SEARCH_INDEX_MANAGER_OBJ_P(return_value);
    manager->conn = Z_CLUSTER_OBJ_P(cluster)->conn;
    pcbc_connection_addref(manager->conn TSRMLS_CC);
}

static HashTable *pcbc_search_index_manager_get_debug_info(zval *object, int *is_temp TSRMLS_DC)
{
    /* pcbc_search_index_manager_t *obj = NULL; */
    zval retval;

    *is_temp = 1;
    /* obj = Z_SEARCH_INDEX_MANAGER_OBJ_P(object); */

    array_init(&retval);

    return Z_ARRVAL(retval);
}

PHP_MINIT_FUNCTION(SearchIndexManager)
{
    zend_class_entry ce;

    INIT_NS_CLASS_ENTRY(ce, "Couchbase", "SearchIndexManager", search_index_manager_methods);
    pcbc_search_index_manager_ce = zend_register_internal_class(&ce TSRMLS_CC);
    pcbc_search_index_manager_ce->create_object = pcbc_search_index_manager_create_object;
    PCBC_CE_DISABLE_SERIALIZATION(pcbc_search_index_manager_ce);

    memcpy(&pcbc_search_index_manager_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    pcbc_search_index_manager_handlers.get_debug_info = pcbc_search_index_manager_get_debug_info;
    pcbc_search_index_manager_handlers.free_obj = pcbc_search_index_manager_free_object;
    pcbc_search_index_manager_handlers.offset = XtOffsetOf(pcbc_search_index_manager_t, std);
    return SUCCESS;
}
