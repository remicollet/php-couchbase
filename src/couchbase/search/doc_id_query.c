/**
 *     Copyright 2016-2017 Couchbase, Inc.
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

/**
 * A FTS query that matches on Couchbase document IDs. Useful to restrict the search space to a list of keys (by using
 * this in a compound query).
 */
#include "couchbase.h"

typedef struct {
    PCBC_ZEND_OBJECT_PRE
    double boost;
    char *field;
    PCBC_ZVAL doc_ids;
    PCBC_ZEND_OBJECT_POST
} pcbc_doc_id_search_query_t;

#if PHP_VERSION_ID >= 70000
static inline pcbc_doc_id_search_query_t *pcbc_doc_id_search_query_fetch_object(zend_object *obj)
{
    return (pcbc_doc_id_search_query_t *)((char *)obj - XtOffsetOf(pcbc_doc_id_search_query_t, std));
}
#define Z_DOC_ID_SEARCH_QUERY_OBJ(zo) (pcbc_doc_id_search_query_fetch_object(zo))
#define Z_DOC_ID_SEARCH_QUERY_OBJ_P(zv) (pcbc_doc_id_search_query_fetch_object(Z_OBJ_P(zv)))
#else
#define Z_DOC_ID_SEARCH_QUERY_OBJ(zo) ((pcbc_doc_id_search_query_t *)zo)
#define Z_DOC_ID_SEARCH_QUERY_OBJ_P(zv) ((pcbc_doc_id_search_query_t *)zend_object_store_get_object(zv TSRMLS_CC))
#endif

#define LOGARGS(lvl) LCB_LOG_##lvl, NULL, "pcbc/doc_id_search_query", __FILE__, __LINE__

zend_class_entry *pcbc_doc_id_search_query_ce;

extern PHP_JSON_API zend_class_entry *php_json_serializable_ce;

/* {{{ proto void DocIdSearchQuery::__construct() */
PHP_METHOD(DocIdSearchQuery, __construct) { throw_pcbc_exception("Accessing private constructor.", LCB_EINVAL); }
/* }}} */

/* {{{ proto \Couchbase\DocIdSearchQuery DocIdSearchQuery::field(string $field)
 */
PHP_METHOD(DocIdSearchQuery, field)
{
    pcbc_doc_id_search_query_t *obj;
    char *field = NULL;
    int rv;
    pcbc_str_arg_size field_len;

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &field, &field_len);
    if (rv == FAILURE) {
        RETURN_NULL();
    }

    obj = Z_DOC_ID_SEARCH_QUERY_OBJ_P(getThis());
    if (obj->field) {
        efree(obj->field);
    }
    obj->field = estrndup(field, field_len);

    RETURN_ZVAL(getThis(), 1, 0);
} /* }}} */

/* {{{ proto \Couchbase\DocIdSearchQuery DocIdSearchQuery::boost(double $boost)
 */
PHP_METHOD(DocIdSearchQuery, boost)
{
    pcbc_doc_id_search_query_t *obj;
    double boost = 0;
    int rv;

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "d", &boost);
    if (rv == FAILURE) {
        RETURN_NULL();
    }

    obj = Z_DOC_ID_SEARCH_QUERY_OBJ_P(getThis());
    obj->boost = boost;

    RETURN_ZVAL(getThis(), 1, 0);
} /* }}} */

/* {{{ proto DocIdSearchQuery DocIdSearchQuery::docIds(string ...$documentIds)
 */
PHP_METHOD(DocIdSearchQuery, docIds)
{
    pcbc_doc_id_search_query_t *obj;
#if PHP_VERSION_ID >= 70000
    zval *args = NULL;
#else
    zval ***args = NULL;
#endif
    int num_args = 0;
    int rv;

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "+", &args, &num_args);
    if (rv == FAILURE) {
        return;
    }

    obj = Z_DOC_ID_SEARCH_QUERY_OBJ_P(getThis());

    if (num_args && args) {
        int i;
        for (i = 0; i < num_args; ++i) {
            PCBC_ZVAL *id;
#if PHP_VERSION_ID >= 70000
            id = &args[i];
#else
            id = args[i];
#endif
            if (Z_TYPE_P(PCBC_P(*id)) != IS_STRING) {
                pcbc_log(LOGARGS(WARN), "id has to be a string (skipping argument #%d)", i);
                continue;
            }
            add_next_index_zval(PCBC_P(obj->doc_ids), PCBC_P(*id));
            PCBC_ADDREF_P(PCBC_P(*id));
        }
    }
#if PHP_VERSION_ID < 70000
    if (args) {
        efree(args);
    }
#endif

    RETURN_ZVAL(getThis(), 1, 0);
} /* }}} */

/* {{{ proto array DocIdSearchQuery::jsonSerialize()
 */
PHP_METHOD(DocIdSearchQuery, jsonSerialize)
{
    pcbc_doc_id_search_query_t *obj;
    int rv;

    rv = zend_parse_parameters_none();
    if (rv == FAILURE) {
        RETURN_NULL();
    }

    obj = Z_DOC_ID_SEARCH_QUERY_OBJ_P(getThis());
    array_init(return_value);
    ADD_ASSOC_ZVAL_EX(return_value, "ids", PCBC_P(obj->doc_ids));
    PCBC_ADDREF_P(PCBC_P(obj->doc_ids));
    if (obj->field) {
        ADD_ASSOC_STRING(return_value, "field", obj->field);
    }
    if (obj->boost >= 0) {
        ADD_ASSOC_DOUBLE_EX(return_value, "boost", obj->boost);
    }
} /* }}} */

ZEND_BEGIN_ARG_INFO_EX(ai_DocIdSearchQuery_none, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_DocIdSearchQuery_field, 0, 0, 1)
ZEND_ARG_INFO(0, field)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_DocIdSearchQuery_boost, 0, 0, 1)
ZEND_ARG_INFO(0, boost)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_DocIdSearchQuery_docIds, 0, 0, 1)
PCBC_ARG_VARIADIC_INFO(0, documentIds)
ZEND_END_ARG_INFO()

// clang-format off
zend_function_entry doc_id_search_query_methods[] = {
    PHP_ME(DocIdSearchQuery, __construct, ai_DocIdSearchQuery_none, ZEND_ACC_PRIVATE | ZEND_ACC_FINAL | ZEND_ACC_CTOR)
    PHP_ME(DocIdSearchQuery, jsonSerialize, ai_DocIdSearchQuery_none, ZEND_ACC_PUBLIC | ZEND_ACC_FINAL)
    PHP_ME(DocIdSearchQuery, boost, ai_DocIdSearchQuery_boost, ZEND_ACC_PUBLIC | ZEND_ACC_FINAL)
    PHP_ME(DocIdSearchQuery, field, ai_DocIdSearchQuery_field, ZEND_ACC_PUBLIC | ZEND_ACC_FINAL)
    PHP_ME(DocIdSearchQuery, docIds, ai_DocIdSearchQuery_docIds, ZEND_ACC_PUBLIC | ZEND_ACC_FINAL)
    PHP_FE_END
};
// clang-format on

void pcbc_doc_id_search_query_init(zval *return_value,
#if PHP_VERSION_ID >= 70000
                                   zval *args,
#else
                                   zval ***args,
#endif
                                   int num_args TSRMLS_DC)
{
    pcbc_doc_id_search_query_t *obj;

    object_init_ex(return_value, pcbc_doc_id_search_query_ce);
    obj = Z_DOC_ID_SEARCH_QUERY_OBJ_P(return_value);
    obj->boost = -1;
    obj->field = NULL;

    PCBC_ZVAL_ALLOC(obj->doc_ids);
    array_init(PCBC_P(obj->doc_ids));

    if (num_args && args) {
        int i;
        for (i = 0; i < num_args; ++i) {
            PCBC_ZVAL *id;
#if PHP_VERSION_ID >= 70000
            id = &args[i];
#else
            id = args[i];
#endif
            if (Z_TYPE_P(PCBC_P(*id)) != IS_STRING) {
                pcbc_log(LOGARGS(WARN), "id has to be a string (skipping argument #%d)", i);
                continue;
            }
            add_next_index_zval(PCBC_P(obj->doc_ids), PCBC_P(*id));
            PCBC_ADDREF_P(PCBC_P(*id));
        }
    }
}

zend_object_handlers doc_id_search_query_handlers;

static void doc_id_search_query_free_object(pcbc_free_object_arg *object TSRMLS_DC) /* {{{ */
{
    pcbc_doc_id_search_query_t *obj = Z_DOC_ID_SEARCH_QUERY_OBJ(object);

    if (obj->field != NULL) {
        efree(obj->field);
    }
    zval_ptr_dtor(&obj->doc_ids);

    zend_object_std_dtor(&obj->std TSRMLS_CC);
#if PHP_VERSION_ID < 70000
    efree(obj);
#endif
} /* }}} */

static pcbc_create_object_retval doc_id_search_query_create_object(zend_class_entry *class_type TSRMLS_DC)
{
    pcbc_doc_id_search_query_t *obj = NULL;

    obj = PCBC_ALLOC_OBJECT_T(pcbc_doc_id_search_query_t, class_type);

    zend_object_std_init(&obj->std, class_type TSRMLS_CC);
    object_properties_init(&obj->std, class_type);

#if PHP_VERSION_ID >= 70000
    obj->std.handlers = &doc_id_search_query_handlers;
    return &obj->std;
#else
    {
        zend_object_value ret;
        ret.handle = zend_objects_store_put(obj, (zend_objects_store_dtor_t)zend_objects_destroy_object,
                                            doc_id_search_query_free_object, NULL TSRMLS_CC);
        ret.handlers = &doc_id_search_query_handlers;
        return ret;
    }
#endif
}

static HashTable *pcbc_doc_id_search_query_get_debug_info(zval *object, int *is_temp TSRMLS_DC) /* {{{ */
{
    pcbc_doc_id_search_query_t *obj = NULL;
#if PHP_VERSION_ID >= 70000
    zval retval;
#else
    zval retval = zval_used_for_init;
#endif

    *is_temp = 1;
    obj = Z_DOC_ID_SEARCH_QUERY_OBJ_P(object);

    array_init(&retval);
    ADD_ASSOC_ZVAL_EX(&retval, "ids", PCBC_P(obj->doc_ids));
    PCBC_ADDREF_P(PCBC_P(obj->doc_ids));
    if (obj->field) {
        ADD_ASSOC_STRING(&retval, "field", obj->field);
    }
    if (obj->boost >= 0) {
        ADD_ASSOC_DOUBLE_EX(&retval, "boost", obj->boost);
    }
    return Z_ARRVAL(retval);
} /* }}} */

PHP_MINIT_FUNCTION(DocIdSearchQuery)
{
    zend_class_entry ce;

    INIT_NS_CLASS_ENTRY(ce, "Couchbase", "DocIdSearchQuery", doc_id_search_query_methods);
    pcbc_doc_id_search_query_ce = zend_register_internal_class(&ce TSRMLS_CC);
    pcbc_doc_id_search_query_ce->create_object = doc_id_search_query_create_object;
    PCBC_CE_FLAGS_FINAL(pcbc_doc_id_search_query_ce);
    PCBC_CE_DISABLE_SERIALIZATION(pcbc_doc_id_search_query_ce);

    zend_class_implements(pcbc_doc_id_search_query_ce TSRMLS_CC, 1, php_json_serializable_ce);
    zend_class_implements(pcbc_doc_id_search_query_ce TSRMLS_CC, 1, pcbc_search_query_part_ce);

    memcpy(&doc_id_search_query_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    doc_id_search_query_handlers.get_debug_info = pcbc_doc_id_search_query_get_debug_info;
#if PHP_VERSION_ID >= 70000
    doc_id_search_query_handlers.free_obj = doc_id_search_query_free_object;
    doc_id_search_query_handlers.offset = XtOffsetOf(pcbc_doc_id_search_query_t, std);
#endif

    zend_register_class_alias("\\CouchbaseDocIdSearchQuery", pcbc_doc_id_search_query_ce);
    return SUCCESS;
}
