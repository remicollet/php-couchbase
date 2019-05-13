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

PHP_METHOD(BucketManager, listDesignDocuments)
{
    pcbc_bucket_manager_t *obj;
    char *path;
    int rv, path_len;

    obj = Z_BUCKET_MANAGER_OBJ_P(getThis());

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

PHP_METHOD(BucketManager, insertDesignDocument)
{
    pcbc_bucket_manager_t *obj;
    char *path, *name = NULL;
    int rv, path_len;
    size_t name_len = 0;
    zval *document;
    smart_str buf = {0};
    int last_error;

    obj = Z_BUCKET_MANAGER_OBJ_P(getThis());

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sz", &name, &name_len, &document);
    if (rv == FAILURE) {
        return;
    }

    path_len = spprintf(&path, 0, "/_design/%*s", (int)name_len, name);

    lcb_CMDHTTP *cmd;

    lcb_cmdhttp_create(&cmd, LCB_HTTP_TYPE_VIEW);
    lcb_cmdhttp_method(cmd, LCB_HTTP_METHOD_GET);
    lcb_cmdhttp_path(cmd, path, path_len);
    lcb_cmdhttp_content_type(cmd, PCBC_CONTENT_TYPE_FORM, strlen(PCBC_CONTENT_TYPE_FORM));
    pcbc_http_request(return_value, obj->conn->lcb, cmd, 1 TSRMLS_CC);
    zval_dtor(return_value);
    if (!php_array_exists(return_value, "error")) {
        efree(path);
        throw_pcbc_exception("Design document already exists", LCB_KEY_EEXISTS);
        RETURN_NULL();
    }

    lcb_cmdhttp_create(&cmd, LCB_HTTP_TYPE_VIEW);
    lcb_cmdhttp_method(cmd, LCB_HTTP_METHOD_PUT);
    lcb_cmdhttp_path(cmd, path, path_len);
    lcb_cmdhttp_content_type(cmd, PCBC_CONTENT_TYPE_JSON, strlen(PCBC_CONTENT_TYPE_JSON));
    PCBC_JSON_ENCODE(&buf, document, 0, last_error);
    if (last_error != 0) {
        pcbc_log(LOGARGS(obj, WARN), "Failed to encode design document as JSON: json_last_error=%d", last_error);
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
            throw_pcbc_exception(error, LCB_ERROR);
            if (owned) {
                efree(error);
            }
            zval_dtor(return_value);
            RETURN_NULL();
        }
    }
}

PHP_METHOD(BucketManager, upsertDesignDocument)
{
    pcbc_bucket_manager_t *obj;
    char *path, *name = NULL;
    int rv, path_len = 0;
    size_t name_len = 0;
    zval *document;
    smart_str buf = {0};
    int last_error;

    obj = Z_BUCKET_MANAGER_OBJ_P(getThis());

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
            throw_pcbc_exception(error, LCB_ERROR);
            if (owned) {
                efree(error);
            }
            zval_dtor(return_value);
        }
    }
}

PHP_METHOD(BucketManager, removeDesignDocument)
{
    pcbc_bucket_manager_t *obj;
    char *path, *name = NULL;
    int rv, path_len;
    size_t name_len = 0;

    obj = Z_BUCKET_MANAGER_OBJ_P(getThis());

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

PHP_METHOD(BucketManager, getDesignDocument)
{
    pcbc_bucket_manager_t *obj;
    char *path, *name = NULL;
    int rv, path_len;
    size_t name_len = 0;

    obj = Z_BUCKET_MANAGER_OBJ_P(getThis());

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

PHP_METHOD(BucketManager, listN1qlIndexes)
{
    pcbc_bucket_manager_t *obj;
    int rv;

    rv = zend_parse_parameters_none();
    if (rv == FAILURE) {
        RETURN_NULL();
    }

    obj = Z_BUCKET_MANAGER_OBJ_P(getThis());

    pcbc_n1ix_list(obj, return_value TSRMLS_CC);
}

PHP_METHOD(BucketManager, createN1qlPrimaryIndex)
{
    pcbc_bucket_manager_t *obj;
    char *name = NULL;
    int rv;
    size_t name_len = 0;
    zend_bool ignore_if_exist = 0, defer = 0;
    lcb_CMDN1XMGMT cmd = {0};

    obj = Z_BUCKET_MANAGER_OBJ_P(getThis());

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|sbb", &name, &name_len, &ignore_if_exist, &defer);
    if (rv == FAILURE) {
        return;
    }

    cmd.spec.ixtype = LCB_N1XSPEC_T_GSI;
    cmd.spec.flags = LCB_N1XSPEC_F_PRIMARY;
    if (defer) {
        cmd.spec.flags |= LCB_N1XSPEC_F_DEFER;
    }

    cmd.spec.name = name;
    cmd.spec.nname = name_len;

    cmd.spec.keyspace = obj->conn->bucketname;
    cmd.spec.nkeyspace = strlen(obj->conn->bucketname);

    pcbc_n1ix_create(obj, &cmd, ignore_if_exist, return_value TSRMLS_CC);
}

PHP_METHOD(BucketManager, createN1qlIndex)
{
    pcbc_bucket_manager_t *obj;
    lcb_CMDN1XMGMT cmd = {0};
    char *name = NULL, *where = NULL;
    int rv, last_error;
    size_t name_len = 0, where_len = 0;
    zend_bool ignore_if_exist = 0, defer = 0;
    zval *fields;
    smart_str buf = {0};

    obj = Z_BUCKET_MANAGER_OBJ_P(getThis());

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sa|sbb", &name, &name_len, &fields, &where, &where_len,
                               &ignore_if_exist, &defer);
    if (rv == FAILURE) {
        return;
    }

    PCBC_JSON_ENCODE(&buf, fields, 0, last_error);
    if (last_error != 0) {
        pcbc_log(LOGARGS(obj, WARN), "Failed to encode index fields as JSON: json_last_error=%d", last_error);
        smart_str_free(&buf);
        RETURN_NULL();
    } else {
        smart_str_0(&buf);
        PCBC_SMARTSTR_SET(buf, cmd.spec.fields, cmd.spec.nfields);
    }
    cmd.spec.ixtype = LCB_N1XSPEC_T_GSI;
    cmd.spec.flags = 0;
    if (defer) {
        cmd.spec.flags |= LCB_N1XSPEC_F_DEFER;
    }

    cmd.spec.name = name;
    cmd.spec.nname = name_len;
    cmd.spec.keyspace = obj->conn->bucketname;
    cmd.spec.nkeyspace = strlen(obj->conn->bucketname);
    cmd.spec.cond = where;
    cmd.spec.ncond = where_len;

    pcbc_n1ix_create(obj, &cmd, ignore_if_exist, return_value TSRMLS_CC);
    smart_str_free(&buf);

}

PHP_METHOD(BucketManager, dropN1qlPrimaryIndex)
{
    pcbc_bucket_manager_t *obj;
    lcb_CMDN1XMGMT cmd = {0};
    char *name = NULL;
    int rv;
    size_t name_len = 0;
    zend_bool ignore_if_not_exist = 0;

    obj = Z_BUCKET_MANAGER_OBJ_P(getThis());

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|sb", &name, &name_len, &ignore_if_not_exist);
    if (rv == FAILURE) {
        return;
    }
    cmd.spec.name = name;
    cmd.spec.nname = name_len;

    cmd.spec.keyspace = obj->conn->bucketname;
    cmd.spec.nkeyspace = strlen(obj->conn->bucketname);

    cmd.spec.ixtype = LCB_N1XSPEC_T_GSI;
    cmd.spec.flags = LCB_N1XSPEC_F_PRIMARY;
    pcbc_n1ix_drop(obj, &cmd, ignore_if_not_exist, return_value TSRMLS_CC);
}

PHP_METHOD(BucketManager, dropN1qlIndex)
{
    pcbc_bucket_manager_t *obj;
    lcb_CMDN1XMGMT cmd = {0};
    char *name = NULL;
    int rv;
    size_t name_len = 0;
    zend_bool ignore_if_not_exist = 0;

    obj = Z_BUCKET_MANAGER_OBJ_P(getThis());

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|bb", &name, &name_len, &ignore_if_not_exist);
    if (rv == FAILURE) {
        return;
    }
    cmd.spec.name = name;
    cmd.spec.nname = name_len;

    cmd.spec.keyspace = obj->conn->bucketname;
    cmd.spec.nkeyspace = strlen(obj->conn->bucketname);

    cmd.spec.ixtype = LCB_N1XSPEC_T_GSI;
    pcbc_n1ix_drop(obj, &cmd, ignore_if_not_exist, return_value TSRMLS_CC);
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

ZEND_BEGIN_ARG_INFO_EX(ai_BucketManager_getDesignDocument, 0, 0, 1)
ZEND_ARG_INFO(0, name)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_BucketManager_upsertDesignDocument, 0, 0, 2)
ZEND_ARG_INFO(0, name)
ZEND_ARG_INFO(0, document)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_BucketManager_createN1qlPrimaryIndex, 0, 0, 3)
ZEND_ARG_INFO(0, customName)
ZEND_ARG_INFO(0, ignoreIfExist)
ZEND_ARG_INFO(0, defer)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_BucketManager_createN1qlIndex, 0, 0, 5)
ZEND_ARG_INFO(0, indexName)
ZEND_ARG_INFO(0, fields)
ZEND_ARG_INFO(0, whereClause)
ZEND_ARG_INFO(0, ignoreIfExist)
ZEND_ARG_INFO(0, defer)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_BucketManager_dropN1qlPrimaryIndex, 0, 0, 2)
ZEND_ARG_INFO(0, customName)
ZEND_ARG_INFO(0, ignoreIfNotExist)
ZEND_ARG_INFO(0, defer)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_BucketManager_dropN1qlIndex, 0, 0, 2)
ZEND_ARG_INFO(0, indexName)
ZEND_ARG_INFO(0, ignoreIfNotExist)
ZEND_ARG_INFO(0, defer)
ZEND_END_ARG_INFO()

// clang-format off
zend_function_entry bucket_manager_methods[] = {
    PHP_ME(BucketManager, __construct, ai_BucketManager_none, ZEND_ACC_PRIVATE | ZEND_ACC_FINAL | ZEND_ACC_CTOR)
    PHP_ME(BucketManager, info, ai_BucketManager_none, ZEND_ACC_PUBLIC)
    PHP_ME(BucketManager, flush, ai_BucketManager_none, ZEND_ACC_PUBLIC)
    PHP_ME(BucketManager, listDesignDocuments, ai_BucketManager_none, ZEND_ACC_PUBLIC)
    PHP_MALIAS(BucketManager, getDesignDocuments, listDesignDocuments, ai_BucketManager_none, ZEND_ACC_PUBLIC | ZEND_ACC_DEPRECATED)
    PHP_ME(BucketManager, getDesignDocument, ai_BucketManager_getDesignDocument, ZEND_ACC_PUBLIC)
    PHP_ME(BucketManager, removeDesignDocument, ai_BucketManager_getDesignDocument, ZEND_ACC_PUBLIC)
    PHP_ME(BucketManager, upsertDesignDocument, ai_BucketManager_upsertDesignDocument, ZEND_ACC_PUBLIC)
    PHP_ME(BucketManager, insertDesignDocument, ai_BucketManager_upsertDesignDocument, ZEND_ACC_PUBLIC)
    PHP_ME(BucketManager, listN1qlIndexes, ai_BucketManager_none, ZEND_ACC_PUBLIC)
    PHP_ME(BucketManager, createN1qlPrimaryIndex, ai_BucketManager_createN1qlPrimaryIndex, ZEND_ACC_PUBLIC)
    PHP_ME(BucketManager, createN1qlIndex, ai_BucketManager_createN1qlIndex, ZEND_ACC_PUBLIC)
    PHP_ME(BucketManager, dropN1qlPrimaryIndex, ai_BucketManager_dropN1qlPrimaryIndex, ZEND_ACC_PUBLIC)
    PHP_ME(BucketManager, dropN1qlIndex, ai_BucketManager_dropN1qlIndex, ZEND_ACC_PUBLIC)
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

void pcbc_bucket_manager_init(zval *return_value, zval *bucket TSRMLS_DC)
{
    pcbc_bucket_manager_t *manager;

    object_init_ex(return_value, pcbc_bucket_manager_ce);
    manager = Z_BUCKET_MANAGER_OBJ_P(return_value);
    manager->conn = Z_BUCKET_OBJ_P(bucket)->conn;
    pcbc_connection_addref(manager->conn TSRMLS_CC);
}

static HashTable *pcbc_bucket_manager_get_debug_info(zval *object, int *is_temp TSRMLS_DC)
{
    pcbc_bucket_manager_t *obj = NULL;
    zval retval;

    *is_temp = 1;
    obj = Z_BUCKET_MANAGER_OBJ_P(object);

    array_init(&retval);
    ADD_ASSOC_STRING(&retval, "bucket", obj->conn->bucketname);

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
