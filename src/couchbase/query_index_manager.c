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

#define LOGARGS(manager, lvl) LCB_LOG_##lvl, manager->conn->lcb, "pcbc/query_index_manager", __FILE__, __LINE__

int pcbc_n1ix_list(pcbc_query_index_manager_t *manager, zval *return_value TSRMLS_DC);
void pcbc_n1ix_create(pcbc_query_index_manager_t *manager, lcb_CMDN1XMGMT *cmd, zend_bool ignore_if_exist,
                      zval *return_value TSRMLS_DC);
void pcbc_n1ix_drop(pcbc_query_index_manager_t *manager, lcb_CMDN1XMGMT *cmd, zend_bool ignore_if_not_exist,
                    zval *return_value TSRMLS_DC);

static inline pcbc_query_index_manager_t *pcbc_query_index_manager_fetch_object(zend_object *obj)
{
    return (pcbc_query_index_manager_t *)((char *)obj - XtOffsetOf(pcbc_query_index_manager_t, std));
}
#define Z_QUERY_INDEX_MANAGER_OBJ(zo) (pcbc_query_index_manager_fetch_object(zo))
#define Z_QUERY_INDEX_MANAGER_OBJ_P(zv) (pcbc_query_index_manager_fetch_object(Z_OBJ_P(zv)))
zend_class_entry *pcbc_query_index_manager_ce;

PHP_METHOD(QueryIndexManager, __construct)
{
    throw_pcbc_exception("Accessing private constructor.", LCB_ERR_INVALID_ARGUMENT);
}

PHP_METHOD(QueryIndexManager, getAllIndexes)
{
    pcbc_query_index_manager_t *obj;
    int rv;

    rv = zend_parse_parameters_none();
    if (rv == FAILURE) {
        RETURN_NULL();
    }

    obj = Z_QUERY_INDEX_MANAGER_OBJ_P(getThis());

    pcbc_n1ix_list(obj, return_value TSRMLS_CC);
}

PHP_METHOD(QueryIndexManager, createPrimaryIndex)
{
    pcbc_query_index_manager_t *obj;
    zend_string *bucket_name = NULL;
    int rv;
    zend_bool ignore_if_exist = 0, defer = 0;
    lcb_CMDN1XMGMT cmd = {0};

    obj = Z_QUERY_INDEX_MANAGER_OBJ_P(getThis());

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "S|bb", &bucket_name, &ignore_if_exist, &defer);
    if (rv == FAILURE) {
        return;
    }

    cmd.spec.ixtype = LCB_N1XSPEC_T_GSI;
    cmd.spec.flags = LCB_N1XSPEC_F_PRIMARY;
    if (defer) {
        cmd.spec.flags |= LCB_N1XSPEC_F_DEFER;
    }

    cmd.spec.name = ZSTR_VAL(bucket_name);
    cmd.spec.nname = ZSTR_LEN(bucket_name);
    cmd.spec.keyspace = ZSTR_VAL(bucket_name);
    cmd.spec.nkeyspace = ZSTR_LEN(bucket_name);

    pcbc_n1ix_create(obj, &cmd, ignore_if_exist, return_value TSRMLS_CC);
}

PHP_METHOD(QueryIndexManager, createIndex)
{
    pcbc_query_index_manager_t *obj;
    lcb_CMDN1XMGMT cmd = {0};
    char *where = NULL;
    zend_string *index_name = NULL, *bucket_name = NULL;
    int rv, last_error;
    size_t where_len = 0;
    zend_bool ignore_if_exist = 0, defer = 0;
    zval *fields;
    smart_str buf = {0};

    obj = Z_QUERY_INDEX_MANAGER_OBJ_P(getThis());

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "SSa|sbb", &bucket_name, &index_name, &fields, &where,
                               &where_len, &ignore_if_exist, &defer);
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

    cmd.spec.name = ZSTR_VAL(index_name);
    cmd.spec.nname = ZSTR_LEN(index_name);
    cmd.spec.keyspace = ZSTR_VAL(bucket_name);
    cmd.spec.nkeyspace = ZSTR_LEN(bucket_name);
    cmd.spec.cond = where;
    cmd.spec.ncond = where_len;

    pcbc_n1ix_create(obj, &cmd, ignore_if_exist, return_value TSRMLS_CC);
    smart_str_free(&buf);
}

PHP_METHOD(QueryIndexManager, dropPrimaryIndex)
{
    pcbc_query_index_manager_t *obj;
    lcb_CMDN1XMGMT cmd = {0};
    zend_string *bucket_name = NULL;
    int rv;
    zend_bool ignore_if_not_exist = 0;

    obj = Z_QUERY_INDEX_MANAGER_OBJ_P(getThis());

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "S|b", &bucket_name, &ignore_if_not_exist);
    if (rv == FAILURE) {
        return;
    }
    cmd.spec.name = ZSTR_VAL(bucket_name);
    cmd.spec.nname = ZSTR_LEN(bucket_name);
    cmd.spec.keyspace = ZSTR_VAL(bucket_name);
    cmd.spec.nkeyspace = ZSTR_LEN(bucket_name);

    cmd.spec.ixtype = LCB_N1XSPEC_T_GSI;
    cmd.spec.flags = LCB_N1XSPEC_F_PRIMARY;
    pcbc_n1ix_drop(obj, &cmd, ignore_if_not_exist, return_value TSRMLS_CC);
}

PHP_METHOD(QueryIndexManager, dropIndex)
{
    pcbc_query_index_manager_t *obj;
    lcb_CMDN1XMGMT cmd = {0};
    zend_string *index_name = NULL, *bucket_name = NULL;
    int rv;
    zend_bool ignore_if_not_exist = 0;

    obj = Z_QUERY_INDEX_MANAGER_OBJ_P(getThis());

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "SS|bb", &index_name, &bucket_name, &ignore_if_not_exist);
    if (rv == FAILURE) {
        return;
    }
    cmd.spec.name = ZSTR_VAL(index_name);
    cmd.spec.nname = ZSTR_LEN(index_name);
    cmd.spec.keyspace = ZSTR_VAL(bucket_name);
    cmd.spec.nkeyspace = ZSTR_LEN(bucket_name);

    cmd.spec.ixtype = LCB_N1XSPEC_T_GSI;
    pcbc_n1ix_drop(obj, &cmd, ignore_if_not_exist, return_value TSRMLS_CC);
}

ZEND_BEGIN_ARG_INFO_EX(ai_QueryIndexManager_none, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_QueryIndexManager_getAllIndexes, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_QueryIndexManager_createPrimaryIndex, 0, 0, 1)
ZEND_ARG_TYPE_INFO(0, bucketName, IS_STRING, 0)
ZEND_ARG_INFO(0, ignoreIfExist)
ZEND_ARG_INFO(0, defer)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_QueryIndexManager_createIndex, 0, 0, 2)
ZEND_ARG_TYPE_INFO(0, bucketName, IS_STRING, 0)
ZEND_ARG_TYPE_INFO(0, indexName, IS_STRING, 0)
ZEND_ARG_INFO(0, fields)
ZEND_ARG_INFO(0, whereClause)
ZEND_ARG_INFO(0, ignoreIfExist)
ZEND_ARG_INFO(0, defer)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_QueryIndexManager_dropPrimaryIndex, 0, 0, 1)
ZEND_ARG_TYPE_INFO(0, bucketName, IS_STRING, 0)
ZEND_ARG_INFO(0, ignoreIfNotExist)
ZEND_ARG_INFO(0, defer)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_QueryIndexManager_dropIndex, 0, 0, 2)
ZEND_ARG_TYPE_INFO(0, bucketName, IS_STRING, 0)
ZEND_ARG_TYPE_INFO(0, indexName, IS_STRING, 0)
ZEND_ARG_INFO(0, ignoreIfNotExist)
ZEND_ARG_INFO(0, defer)
ZEND_END_ARG_INFO()

// clang-format off
zend_function_entry query_index_manager_methods[] = {
    PHP_ME(QueryIndexManager, __construct, ai_QueryIndexManager_none, ZEND_ACC_PRIVATE | ZEND_ACC_FINAL | ZEND_ACC_CTOR)
    PHP_ME(QueryIndexManager, getAllIndexes, ai_QueryIndexManager_getAllIndexes, ZEND_ACC_PUBLIC)
    PHP_ME(QueryIndexManager, createPrimaryIndex, ai_QueryIndexManager_createPrimaryIndex, ZEND_ACC_PUBLIC)
    PHP_ME(QueryIndexManager, createIndex, ai_QueryIndexManager_createIndex, ZEND_ACC_PUBLIC)
    PHP_ME(QueryIndexManager, dropPrimaryIndex, ai_QueryIndexManager_dropPrimaryIndex, ZEND_ACC_PUBLIC)
    PHP_ME(QueryIndexManager, dropIndex, ai_QueryIndexManager_dropIndex, ZEND_ACC_PUBLIC)
    PHP_FE_END
};
// clang-format on

zend_object_handlers pcbc_query_index_manager_handlers;

static void pcbc_query_index_manager_free_object(zend_object *object TSRMLS_DC)
{
    pcbc_query_index_manager_t *obj = Z_QUERY_INDEX_MANAGER_OBJ(object);

    pcbc_connection_delref(obj->conn TSRMLS_CC);
    obj->conn = NULL;
    zend_object_std_dtor(&obj->std TSRMLS_CC);
}

static zend_object *pcbc_query_index_manager_create_object(zend_class_entry *class_type TSRMLS_DC)
{
    pcbc_query_index_manager_t *obj = NULL;

    obj = PCBC_ALLOC_OBJECT_T(pcbc_query_index_manager_t, class_type);

    zend_object_std_init(&obj->std, class_type TSRMLS_CC);
    object_properties_init(&obj->std, class_type);

    obj->std.handlers = &pcbc_query_index_manager_handlers;
    return &obj->std;
}

void pcbc_query_index_manager_init(zval *return_value, zval *cluster TSRMLS_DC)
{
    pcbc_query_index_manager_t *manager;

    object_init_ex(return_value, pcbc_query_index_manager_ce);
    manager = Z_QUERY_INDEX_MANAGER_OBJ_P(return_value);
    manager->conn = Z_CLUSTER_OBJ_P(cluster)->conn;
    pcbc_connection_addref(manager->conn TSRMLS_CC);
}

static HashTable *pcbc_query_index_manager_get_debug_info(zval *object, int *is_temp TSRMLS_DC)
{
    // pcbc_query_index_manager_t *obj = NULL;
    zval retval;

    *is_temp = 1;
    // obj = Z_QUERY_INDEX_MANAGER_OBJ_P(object);

    array_init(&retval);

    return Z_ARRVAL(retval);
}

PHP_MINIT_FUNCTION(QueryIndexManager)
{
    zend_class_entry ce;

    INIT_NS_CLASS_ENTRY(ce, "Couchbase", "QueryIndexManager", query_index_manager_methods);
    pcbc_query_index_manager_ce = zend_register_internal_class(&ce TSRMLS_CC);
    pcbc_query_index_manager_ce->create_object = pcbc_query_index_manager_create_object;
    PCBC_CE_DISABLE_SERIALIZATION(pcbc_query_index_manager_ce);

    memcpy(&pcbc_query_index_manager_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    pcbc_query_index_manager_handlers.get_debug_info = pcbc_query_index_manager_get_debug_info;
    pcbc_query_index_manager_handlers.free_obj = pcbc_query_index_manager_free_object;
    pcbc_query_index_manager_handlers.offset = XtOffsetOf(pcbc_query_index_manager_t, std);

    return SUCCESS;
}

/*
 * vim: et ts=4 sw=4 sts=4
 */
