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
#include <ext/standard/php_array.h>
#include <ext/standard/php_http.h>
#include <ext/standard/url.h>

#define LOGARGS(instance, lvl) LCB_LOG_##lvl, instance, "pcbc/cluster_manager", __FILE__, __LINE__

#define PCBC_CLUSTER_MANAGER_RBAC_DOMAIN_LOCAL 1
#define PCBC_CLUSTER_MANAGER_RBAC_DOMAIN_EXTERNAL 2

zend_class_entry *pcbc_cluster_manager_ce;

PHP_METHOD(ClusterManager, __construct)
{
    throw_pcbc_exception("Accessing private constructor.", LCB_EINVAL);
}

PHP_METHOD(ClusterManager, listBuckets)
{
    pcbc_cluster_manager_t *obj;
    const char *path = "/pools/default/buckets";
    int rv;

    obj = Z_CLUSTER_MANAGER_OBJ_P(getThis());

    rv = zend_parse_parameters_none();
    if (rv == FAILURE) {
        RETURN_NULL();
    }

    lcb_CMDHTTP *cmd;
    lcb_cmdhttp_create(&cmd, LCB_HTTP_TYPE_MANAGEMENT);
    lcb_cmdhttp_method(cmd, LCB_HTTP_METHOD_GET);
    lcb_cmdhttp_path(cmd, path, strlen(path));
    lcb_cmdhttp_content_type(cmd, PCBC_CONTENT_TYPE_FORM, strlen(PCBC_CONTENT_TYPE_FORM));
    pcbc_http_request(return_value, obj->conn->lcb, cmd, 1 TSRMLS_CC);
}

PHP_METHOD(ClusterManager, createBucket)
{
    pcbc_cluster_manager_t *obj;
    const char *name = NULL;
    size_t name_len = 0;
    zval *options = NULL;
    int rv;
    const char *path = "/pools/default/buckets";
    zval default_options;
    smart_str buf = {0};

    obj = Z_CLUSTER_MANAGER_OBJ_P(getThis());

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|z", &name, &name_len, &options);
    if (rv == FAILURE) {
        return;
    }

    ZVAL_UNDEF(&default_options);
    array_init_size(&default_options, 5);
    ADD_ASSOC_STRING(&default_options, "name", name);
    ADD_ASSOC_STRING(&default_options, "authType", "sasl");
    ADD_ASSOC_STRING(&default_options, "bucketType", "couchbase");
    ADD_ASSOC_LONG_EX(&default_options, "ramQuotaMB", 100);
    ADD_ASSOC_LONG_EX(&default_options, "replicaNumber", 1);
    if (options && Z_TYPE_P(options) == IS_ARRAY) {
        PCBC_ARRAY_MERGE(Z_ARRVAL_P(&default_options), Z_ARRVAL_P(options));
    }

    rv = php_url_encode_hash_ex(HASH_OF(&default_options), &buf, NULL, 0, NULL, 0, NULL, 0, NULL, NULL,
                                PHP_QUERY_RFC1738 TSRMLS_CC);
    zval_ptr_dtor(&default_options);
    lcb_CMDHTTP *cmd;
    if (rv == FAILURE) {
        pcbc_log(LOGARGS(obj->conn->lcb, WARN), "Failed to encode options as RFC1738 query");
        smart_str_free(&buf);
        RETURN_NULL();
    } else {
        smart_str_0(&buf);
        lcb_cmdhttp_create(&cmd,  LCB_HTTP_TYPE_MANAGEMENT);
        lcb_cmdhttp_body(cmd, ZSTR_VAL(buf.s), ZSTR_LEN(buf.s));
        lcb_cmdhttp_method(cmd, LCB_HTTP_METHOD_POST);
        lcb_cmdhttp_path(cmd, path, strlen(path));
        lcb_cmdhttp_content_type(cmd, PCBC_CONTENT_TYPE_FORM, strlen(PCBC_CONTENT_TYPE_FORM));
        pcbc_http_request(return_value, obj->conn->lcb, cmd, 1 TSRMLS_CC);
        smart_str_free(&buf);
    }

}

PHP_METHOD(ClusterManager, removeBucket)
{
    pcbc_cluster_manager_t *obj;
    const char *name = NULL;
    size_t name_len = 0;
    char *path;
    int rv, path_len;

    obj = Z_CLUSTER_MANAGER_OBJ_P(getThis());

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &name, &name_len);
    if (rv == FAILURE) {
        return;
    }
    lcb_CMDHTTP *cmd;
    lcb_cmdhttp_create(&cmd, LCB_HTTP_TYPE_MANAGEMENT);
    lcb_cmdhttp_method(cmd, LCB_HTTP_METHOD_DELETE);
    path_len = spprintf(&path, 0, "/pools/default/buckets/%*s", (int)name_len, name);
    lcb_cmdhttp_path(cmd, path, path_len);
    lcb_cmdhttp_content_type(cmd, PCBC_CONTENT_TYPE_FORM, strlen(PCBC_CONTENT_TYPE_FORM));
    pcbc_http_request(return_value, obj->conn->lcb, cmd, 1 TSRMLS_CC);
    efree(path);
}

PHP_METHOD(ClusterManager, info)
{
    pcbc_cluster_manager_t *obj;
    const char *path = "/pools/default";
    int rv;

    obj = Z_CLUSTER_MANAGER_OBJ_P(getThis());

    rv = zend_parse_parameters_none();
    if (rv == FAILURE) {
        RETURN_NULL();
    }

    lcb_CMDHTTP *cmd;
    lcb_cmdhttp_create(&cmd, LCB_HTTP_TYPE_MANAGEMENT);
    lcb_cmdhttp_method(cmd, LCB_HTTP_METHOD_GET);
    lcb_cmdhttp_path(cmd, path, strlen(path));
    lcb_cmdhttp_content_type(cmd, PCBC_CONTENT_TYPE_FORM, strlen(PCBC_CONTENT_TYPE_FORM));
    pcbc_http_request(return_value, obj->conn->lcb, cmd, 1 TSRMLS_CC);
}

PHP_METHOD(ClusterManager, listUsers)
{
    pcbc_cluster_manager_t *obj;
    long domain = PCBC_CLUSTER_MANAGER_RBAC_DOMAIN_LOCAL;
    char *path;
    int rv;

    obj = Z_CLUSTER_MANAGER_OBJ_P(getThis());

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|l", &domain);
    if (rv == FAILURE) {
        RETURN_NULL();
    }

    switch (domain) {
    case PCBC_CLUSTER_MANAGER_RBAC_DOMAIN_LOCAL:
        path = "/settings/rbac/users/local";
        break;
    case PCBC_CLUSTER_MANAGER_RBAC_DOMAIN_EXTERNAL:
        path = "/settings/rbac/users/external";
        break;
    default:
        throw_pcbc_exception("Invalid arguments.", LCB_EINVAL);
        RETURN_NULL();
    }
    lcb_CMDHTTP *cmd;
    lcb_cmdhttp_create(&cmd, LCB_HTTP_TYPE_MANAGEMENT);
    lcb_cmdhttp_method(cmd, LCB_HTTP_METHOD_GET);
    lcb_cmdhttp_path(cmd, path, strlen(path));
    lcb_cmdhttp_content_type(cmd, PCBC_CONTENT_TYPE_FORM, strlen(PCBC_CONTENT_TYPE_FORM));
    pcbc_http_request(return_value, obj->conn->lcb, cmd, 1 TSRMLS_CC);
}

PHP_METHOD(ClusterManager, getUser)
{
    pcbc_cluster_manager_t *obj;
    const char *name = NULL;
    size_t name_len = 0;
    char *path;
    int rv, path_len;
    long domain = PCBC_CLUSTER_MANAGER_RBAC_DOMAIN_LOCAL;

    obj = Z_CLUSTER_MANAGER_OBJ_P(getThis());

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|l", &name, &name_len, &domain);
    if (rv == FAILURE) {
        return;
    }
    switch (domain) {
    case PCBC_CLUSTER_MANAGER_RBAC_DOMAIN_LOCAL:
        path_len = spprintf(&path, 0, "/settings/rbac/users/local/%*s", (int)name_len, name);
        break;
    case PCBC_CLUSTER_MANAGER_RBAC_DOMAIN_EXTERNAL:
        path_len = spprintf(&path, 0, "/settings/rbac/users/external/%*s", (int)name_len, name);
        break;
    default:
        throw_pcbc_exception("Invalid arguments.", LCB_EINVAL);
        RETURN_NULL();
    }

    lcb_CMDHTTP *cmd;
    lcb_cmdhttp_create(&cmd, LCB_HTTP_TYPE_MANAGEMENT);
    lcb_cmdhttp_method(cmd, LCB_HTTP_METHOD_GET);
    lcb_cmdhttp_path(cmd, path, path_len);
    pcbc_http_request(return_value, obj->conn->lcb, cmd, 1 TSRMLS_CC);
    efree(path);
}

PHP_METHOD(ClusterManager, removeUser)
{
    pcbc_cluster_manager_t *obj;
    const char *name = NULL;
    size_t name_len = 0;
    char *path;
    int rv, path_len;
    long domain = PCBC_CLUSTER_MANAGER_RBAC_DOMAIN_LOCAL;

    obj = Z_CLUSTER_MANAGER_OBJ_P(getThis());

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|l", &name, &name_len, &domain);
    if (rv == FAILURE) {
        return;
    }
    switch (domain) {
    case PCBC_CLUSTER_MANAGER_RBAC_DOMAIN_LOCAL:
        path_len = spprintf(&path, 0, "/settings/rbac/users/local/%*s", (int)name_len, name);
        break;
    case PCBC_CLUSTER_MANAGER_RBAC_DOMAIN_EXTERNAL:
        path_len = spprintf(&path, 0, "/settings/rbac/users/external/%*s", (int)name_len, name);
        break;
    default:
        throw_pcbc_exception("Invalid arguments.", LCB_EINVAL);
        RETURN_NULL();
    }

    lcb_CMDHTTP *cmd;
    lcb_cmdhttp_create(&cmd, LCB_HTTP_TYPE_MANAGEMENT);
    lcb_cmdhttp_method(cmd, LCB_HTTP_METHOD_DELETE);
    lcb_cmdhttp_path(cmd, path, path_len);
    lcb_cmdhttp_content_type(cmd, PCBC_CONTENT_TYPE_FORM, strlen(PCBC_CONTENT_TYPE_FORM));
    pcbc_http_request(return_value, obj->conn->lcb, cmd, 0 TSRMLS_CC);
    efree(path);
    if (Z_STRLEN_P(return_value) == 0 || (Z_STRVAL_P(return_value)[0] == '"' && Z_STRVAL_P(return_value)[1] == '"')) {
        RETURN_TRUE;
    } else {
        throw_pcbc_exception(Z_STRVAL_P(return_value), LCB_EINVAL);
        RETURN_NULL();
    }
}

PHP_METHOD(ClusterManager, upsertUser)
{
    pcbc_cluster_manager_t *obj;
    const char *name = NULL;
    size_t name_len = 0;
    zval *settings = NULL;
    char *path;
    int rv, path_len;
    smart_str buf = {0};
    zval body;
    pcbc_user_settings_t *user;
    long domain = PCBC_CLUSTER_MANAGER_RBAC_DOMAIN_LOCAL;

    obj = Z_CLUSTER_MANAGER_OBJ_P(getThis());

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sO|l", &name, &name_len, &settings, pcbc_user_settings_ce,
                               &domain);
    if (rv == FAILURE) {
        return;
    }

    user = Z_USER_SETTINGS_OBJ_P(settings);
    switch (domain) {
    case PCBC_CLUSTER_MANAGER_RBAC_DOMAIN_LOCAL:
        path_len = spprintf(&path, 0, "/settings/rbac/users/local/%*s", (int)name_len, name);
        break;
    case PCBC_CLUSTER_MANAGER_RBAC_DOMAIN_EXTERNAL:
        path_len = spprintf(&path, 0, "/settings/rbac/users/external/%*s", (int)name_len, name);
        break;
    default:
        throw_pcbc_exception("Invalid arguments.", LCB_EINVAL);
        RETURN_NULL();
    }

    ZVAL_UNDEF(&body);
    array_init_size(&body, 3);
    if (user->full_name) {
        ADD_ASSOC_STRINGL(&body, "name", user->full_name, user->full_name_len);
    }
    if (user->password) {
        ADD_ASSOC_STRINGL(&body, "password", user->password, user->password_len);
    }
    if (PCBC_SMARTSTR_LEN(user->roles)) {
        ADD_ASSOC_STRINGL(&body, "roles", PCBC_SMARTSTR_VAL(user->roles), PCBC_SMARTSTR_LEN(user->roles));
    }
    rv = php_url_encode_hash_ex(HASH_OF(&body), &buf, NULL, 0, NULL, 0, NULL, 0, NULL, NULL,
                                PHP_QUERY_RFC1738 TSRMLS_CC);
    zval_ptr_dtor(&body);
    if (rv == FAILURE) {
        pcbc_log(LOGARGS(obj->conn->lcb, WARN), "Failed to encode options as RFC1738 query");
        smart_str_free(&buf);
        RETURN_NULL();
    } else {
        smart_str_0(&buf);
        lcb_CMDHTTP *cmd;
        lcb_cmdhttp_create(&cmd, LCB_HTTP_TYPE_MANAGEMENT);
        lcb_cmdhttp_method(cmd,LCB_HTTP_METHOD_PUT);
        lcb_cmdhttp_path(cmd, path, path_len);
        lcb_cmdhttp_content_type(cmd, PCBC_CONTENT_TYPE_FORM, strlen(PCBC_CONTENT_TYPE_FORM));
        lcb_cmdhttp_body(cmd, ZSTR_VAL(buf.s), ZSTR_LEN(buf.s));
        pcbc_http_request(return_value, obj->conn->lcb, cmd, 0 TSRMLS_CC);
        smart_str_free(&buf);
        efree(path);
        if (Z_STRLEN_P(return_value) == 0 || (Z_STRVAL_P(return_value)[0] == '"' && Z_STRVAL_P(return_value)[1] == '"')) {
            RETURN_TRUE;
        } else {
            throw_pcbc_exception(Z_STRVAL_P(return_value), LCB_EINVAL);
            RETURN_NULL();
        }
    }
}

ZEND_BEGIN_ARG_INFO_EX(ai_ClusterManager_none, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_ClusterManager_removeBucket, 0, 0, 1)
ZEND_ARG_INFO(0, name)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_ClusterManager_createBucket, 0, 0, 2)
ZEND_ARG_INFO(0, name)
ZEND_ARG_INFO(0, options)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_ClusterManager_upsertUser, 0, 0, 3)
ZEND_ARG_INFO(0, name)
ZEND_ARG_INFO(0, settings)
ZEND_ARG_INFO(0, domain)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_ClusterManager_removeUser, 0, 0, 2)
ZEND_ARG_INFO(0, name)
ZEND_ARG_INFO(0, domain)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_ClusterManager_getUser, 0, 0, 2)
ZEND_ARG_INFO(0, name)
ZEND_ARG_INFO(0, domain)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_ClusterManager_listUsers, 0, 0, 1)
ZEND_ARG_INFO(0, domain)
ZEND_END_ARG_INFO()

// clang-format off
zend_function_entry cluster_manager_methods[] = {
    PHP_ME(ClusterManager, __construct, ai_ClusterManager_none, ZEND_ACC_PRIVATE | ZEND_ACC_FINAL | ZEND_ACC_CTOR)
    PHP_ME(ClusterManager, listBuckets, ai_ClusterManager_none, ZEND_ACC_PUBLIC)
    PHP_ME(ClusterManager, createBucket, ai_ClusterManager_createBucket, ZEND_ACC_PUBLIC)
    PHP_ME(ClusterManager, removeBucket, ai_ClusterManager_removeBucket, ZEND_ACC_PUBLIC)
    PHP_ME(ClusterManager, listUsers, ai_ClusterManager_listUsers, ZEND_ACC_PUBLIC)
    PHP_ME(ClusterManager, upsertUser, ai_ClusterManager_upsertUser, ZEND_ACC_PUBLIC)
    PHP_ME(ClusterManager, getUser, ai_ClusterManager_getUser, ZEND_ACC_PUBLIC)
    PHP_ME(ClusterManager, removeUser, ai_ClusterManager_removeUser, ZEND_ACC_PUBLIC)
    PHP_ME(ClusterManager, info, ai_ClusterManager_none, ZEND_ACC_PUBLIC)
    PHP_FE_END
};
// clang-format on

zend_object_handlers pcbc_cluster_manager_handlers;

static void pcbc_cluster_manager_free_object(zend_object *object TSRMLS_DC)
{
    pcbc_cluster_manager_t *obj = Z_CLUSTER_MANAGER_OBJ(object);
    pcbc_connection_delref(obj->conn TSRMLS_CC);
    obj->conn = NULL;

    zend_object_std_dtor(&obj->std TSRMLS_CC);
}

static zend_object *pcbc_cluster_manager_create_object(zend_class_entry *class_type TSRMLS_DC)
{
    pcbc_cluster_manager_t *obj = NULL;

    obj = PCBC_ALLOC_OBJECT_T(pcbc_cluster_manager_t, class_type);

    zend_object_std_init(&obj->std, class_type TSRMLS_CC);
    object_properties_init(&obj->std, class_type);

    obj->std.handlers = &pcbc_cluster_manager_handlers;
    return &obj->std;
}

static HashTable *pcbc_cluster_manager_get_debug_info(zval *object, int *is_temp TSRMLS_DC)
{
    /* pcbc_cluster_manager_t *obj = NULL; */
    zval retval;

    *is_temp = 1;
    /* obj = Z_CLUSTER_MANAGER_OBJ_P(object); */

    array_init(&retval);

    return Z_ARRVAL(retval);
}

PHP_MINIT_FUNCTION(ClusterManager)
{
    zend_class_entry ce;

    INIT_NS_CLASS_ENTRY(ce, "Couchbase", "ClusterManager", cluster_manager_methods);
    pcbc_cluster_manager_ce = zend_register_internal_class(&ce TSRMLS_CC);
    pcbc_cluster_manager_ce->create_object = pcbc_cluster_manager_create_object;
    PCBC_CE_DISABLE_SERIALIZATION(pcbc_cluster_manager_ce);

    memcpy(&pcbc_cluster_manager_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    pcbc_cluster_manager_handlers.get_debug_info = pcbc_cluster_manager_get_debug_info;
    pcbc_cluster_manager_handlers.free_obj = pcbc_cluster_manager_free_object;
    pcbc_cluster_manager_handlers.offset = XtOffsetOf(pcbc_cluster_manager_t, std);
    zend_declare_class_constant_long(pcbc_cluster_manager_ce, ZEND_STRL("RBAC_DOMAIN_LOCAL"),
                                     PCBC_CLUSTER_MANAGER_RBAC_DOMAIN_LOCAL TSRMLS_CC);
    zend_declare_class_constant_long(pcbc_cluster_manager_ce, ZEND_STRL("RBAC_DOMAIN_EXTERNAL"),
                                     PCBC_CLUSTER_MANAGER_RBAC_DOMAIN_EXTERNAL TSRMLS_CC);


    return SUCCESS;
}

/*
 * vim: et ts=4 sw=4 sts=4
 */
