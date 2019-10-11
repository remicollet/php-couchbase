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

#define LOGARGS(manager, lvl) LCB_LOG_##lvl, manager->conn->lcb, "pcbc/bucket_manager", __FILE__, __LINE__

static inline pcbc_bucket_manager_t *pcbc_bucket_manager_fetch_object(zend_object *obj)
{
    return (pcbc_bucket_manager_t *)((char *)obj - XtOffsetOf(pcbc_bucket_manager_t, std));
}
#define Z_BUCKET_MANAGER_OBJ(zo) (pcbc_bucket_manager_fetch_object(zo))
#define Z_BUCKET_MANAGER_OBJ_P(zv) (pcbc_bucket_manager_fetch_object(Z_OBJ_P(zv)))
zend_class_entry *pcbc_bucket_manager_ce;

PHP_METHOD(BucketManager, __construct)
{
    throw_pcbc_exception("Accessing private constructor.", LCB_EINVAL);
}

PHP_METHOD(BucketManager, info)
{
    char *path;
    int rv, path_len;

    rv = zend_parse_parameters_none();
    if (rv == FAILURE) {
        RETURN_NULL();
    }
    pcbc_bucket_manager_t *obj = Z_BUCKET_MANAGER_OBJ_P(getThis());

    lcb_CMDHTTP *cmd;
    lcb_cmdhttp_create(&cmd, LCB_HTTP_TYPE_MANAGEMENT);
    lcb_cmdhttp_method(cmd, LCB_HTTP_METHOD_GET);
    path_len = spprintf(&path, 0, "/pools/default/buckets/%s", obj->conn->bucketname);
    lcb_cmdhttp_path(cmd, path, path_len);
    lcb_cmdhttp_content_type(cmd, PCBC_CONTENT_TYPE_FORM, strlen(PCBC_CONTENT_TYPE_FORM));
    pcbc_http_request(return_value, obj->conn->lcb, cmd, 1 TSRMLS_CC);
    efree(path);
}

PHP_METHOD(BucketManager, flush)
{
    char *path;
    int rv, path_len;

    rv = zend_parse_parameters_none();
    if (rv == FAILURE) {
        RETURN_NULL();
    }
    pcbc_bucket_manager_t *obj = Z_BUCKET_MANAGER_OBJ_P(getThis());
    lcb_CMDHTTP *cmd;
    lcb_cmdhttp_create(&cmd, LCB_HTTP_TYPE_MANAGEMENT);
    lcb_cmdhttp_method(cmd, LCB_HTTP_METHOD_POST);
    path_len = spprintf(&path, 0, "/pools/default/buckets/%s/controller/doFlush", obj->conn->bucketname);
    lcb_cmdhttp_path(cmd, path, path_len);
    lcb_cmdhttp_content_type(cmd, PCBC_CONTENT_TYPE_FORM, strlen(PCBC_CONTENT_TYPE_FORM));
    pcbc_http_request(return_value, obj->conn->lcb, cmd, 1 TSRMLS_CC);
    efree(path);
}

PHP_METHOD(BucketManager, searchIndexManager)
{
    pcbc_bucket_manager_t *obj;
    int rv;

    rv = zend_parse_parameters_none();
    if (rv == FAILURE) {
        RETURN_NULL();
    }
    obj = Z_BUCKET_MANAGER_OBJ_P(getThis());
    pcbc_search_index_manager_init(return_value, obj TSRMLS_CC);
}

ZEND_BEGIN_ARG_INFO_EX(ai_BucketManager_none, 0, 0, 0)
ZEND_END_ARG_INFO()

// clang-format off
zend_function_entry bucket_manager_methods[] = {
    PHP_ME(BucketManager, __construct, ai_BucketManager_none, ZEND_ACC_PRIVATE | ZEND_ACC_FINAL | ZEND_ACC_CTOR)
    PHP_ME(BucketManager, info, ai_BucketManager_none, ZEND_ACC_PUBLIC)
    PHP_ME(BucketManager, flush, ai_BucketManager_none, ZEND_ACC_PUBLIC)
    PHP_ME(BucketManager, searchIndexManager, ai_BucketManager_none, ZEND_ACC_PUBLIC)
    PHP_FE_END
};
// clang-format on

zend_object_handlers pcbc_bucket_manager_handlers;

static void pcbc_bucket_manager_free_object(zend_object *object TSRMLS_DC)
{
    pcbc_bucket_manager_t *obj = Z_BUCKET_MANAGER_OBJ(object);

    pcbc_connection_delref(obj->conn TSRMLS_CC);
    obj->conn = NULL;
    zend_object_std_dtor(&obj->std TSRMLS_CC);
}

static zend_object *pcbc_bucket_manager_create_object(zend_class_entry *class_type TSRMLS_DC)
{
    pcbc_bucket_manager_t *obj = NULL;

    obj = PCBC_ALLOC_OBJECT_T(pcbc_bucket_manager_t, class_type);

    zend_object_std_init(&obj->std, class_type TSRMLS_CC);
    object_properties_init(&obj->std, class_type);

    obj->std.handlers = &pcbc_bucket_manager_handlers;
    return &obj->std;
}

static HashTable *pcbc_bucket_manager_get_debug_info(zval *object, int *is_temp TSRMLS_DC)
{
    pcbc_bucket_manager_t *obj = NULL;
    zval retval;

    *is_temp = 1;
    obj = Z_BUCKET_MANAGER_OBJ_P(object);

    array_init(&retval);
    add_assoc_string(&retval, "bucket", obj->conn->bucketname);

    return Z_ARRVAL(retval);
}

PHP_MINIT_FUNCTION(BucketManager)
{
    zend_class_entry ce;

    INIT_NS_CLASS_ENTRY(ce, "Couchbase", "BucketManager", bucket_manager_methods);
    pcbc_bucket_manager_ce = zend_register_internal_class(&ce TSRMLS_CC);
    pcbc_bucket_manager_ce->create_object = pcbc_bucket_manager_create_object;
    PCBC_CE_DISABLE_SERIALIZATION(pcbc_bucket_manager_ce);

    memcpy(&pcbc_bucket_manager_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    pcbc_bucket_manager_handlers.get_debug_info = pcbc_bucket_manager_get_debug_info;
    pcbc_bucket_manager_handlers.free_obj = pcbc_bucket_manager_free_object;
    pcbc_bucket_manager_handlers.offset = XtOffsetOf(pcbc_bucket_manager_t, std);

    return SUCCESS;
}

/*
 * vim: et ts=4 sw=4 sts=4
 */
