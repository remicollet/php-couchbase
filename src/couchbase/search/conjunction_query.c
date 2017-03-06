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
 * A compound FTS query that performs a logical AND between all its sub-queries (conjunction).
 */
#include "couchbase.h"

typedef struct {
    PCBC_ZEND_OBJECT_PRE
    double boost;
    PCBC_ZVAL queries;
    PCBC_ZEND_OBJECT_POST
} pcbc_conjunction_search_query_t;

#if PHP_VERSION_ID >= 70000
static inline pcbc_conjunction_search_query_t *pcbc_conjunction_search_query_fetch_object(zend_object *obj)
{
    return (pcbc_conjunction_search_query_t *)((char *)obj - XtOffsetOf(pcbc_conjunction_search_query_t, std));
}
#define Z_CONJUNCTION_SEARCH_QUERY_OBJ(zo) (pcbc_conjunction_search_query_fetch_object(zo))
#define Z_CONJUNCTION_SEARCH_QUERY_OBJ_P(zv) (pcbc_conjunction_search_query_fetch_object(Z_OBJ_P(zv)))
#else
#define Z_CONJUNCTION_SEARCH_QUERY_OBJ(zo) ((pcbc_conjunction_search_query_t *)zo)
#define Z_CONJUNCTION_SEARCH_QUERY_OBJ_P(zv)                                                                           \
    ((pcbc_conjunction_search_query_t *)zend_object_store_get_object(zv TSRMLS_CC))
#endif

#define LOGARGS(lvl) LCB_LOG_##lvl, NULL, "pcbc/conjunction_search_query", __FILE__, __LINE__

zend_class_entry *pcbc_conjunction_search_query_ce;

extern PHP_JSON_API zend_class_entry *php_json_serializable_ce;

/* {{{ proto void ConjunctionSearchQuery::__construct() */
PHP_METHOD(ConjunctionSearchQuery, __construct) { throw_pcbc_exception("Accessing private constructor.", LCB_EINVAL); }
/* }}} */

/* {{{ proto \Couchbase\ConjunctionSearchQuery ConjunctionSearchQuery::boost(double $boost)
 */
PHP_METHOD(ConjunctionSearchQuery, boost)
{
    pcbc_conjunction_search_query_t *obj;
    double boost = 0;
    int rv;

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "d", &boost);
    if (rv == FAILURE) {
        RETURN_NULL();
    }

    obj = Z_CONJUNCTION_SEARCH_QUERY_OBJ_P(getThis());
    obj->boost = boost;

    RETURN_ZVAL(getThis(), 1, 0);
} /* }}} */

/* {{{ proto \Couchbase\ConjunctionSearchQuery ConjunctionSearchQuery::every(SearchQueryPart ...$queries)
 */
PHP_METHOD(ConjunctionSearchQuery, every)
{
    pcbc_conjunction_search_query_t *obj;
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

    obj = Z_CONJUNCTION_SEARCH_QUERY_OBJ_P(getThis());

    if (num_args && args) {
        int i;
        for (i = 0; i < num_args; ++i) {
            PCBC_ZVAL *query;
#if PHP_VERSION_ID >= 70000
            query = &args[i];
#else
            query = args[i];
#endif
            if (Z_TYPE_P(PCBC_P(*query)) != IS_OBJECT ||
                !instanceof_function(Z_OBJCE_P(PCBC_P(*query)), pcbc_search_query_part_ce TSRMLS_CC)) {
                pcbc_log(LOGARGS(WARN), "query has to implement SearchQueryPart interface (skipping argument #%d)", i);
                continue;
            }
            add_next_index_zval(PCBC_P(obj->queries), PCBC_P(*query));
        }
    }
#if PHP_VERSION_ID < 70000
    if (args) {
        efree(args);
    }
#endif
    RETURN_ZVAL(getThis(), 1, 0);
} /* }}} */

/* {{{ proto array ConjunctionSearchQuery::jsonSerialize()
 */
PHP_METHOD(ConjunctionSearchQuery, jsonSerialize)
{
    pcbc_conjunction_search_query_t *obj;
    int rv;

    rv = zend_parse_parameters_none();
    if (rv == FAILURE) {
        RETURN_NULL();
    }

    obj = Z_CONJUNCTION_SEARCH_QUERY_OBJ_P(getThis());
    array_init(return_value);
    ADD_ASSOC_ZVAL_EX(return_value, "conjuncts", PCBC_P(obj->queries));
    PCBC_ADDREF_P(PCBC_P(obj->queries));
    if (obj->boost >= 0) {
        ADD_ASSOC_DOUBLE_EX(return_value, "boost", obj->boost);
    }
} /* }}} */

ZEND_BEGIN_ARG_INFO_EX(ai_ConjunctionSearchQuery_none, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_ConjunctionSearchQuery_boost, 0, 0, 1)
ZEND_ARG_INFO(0, boost)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_ConjunctionSearchQuery_every, 0, 0, 1)
PCBC_ARG_VARIADIC_INFO(0, queries)
ZEND_END_ARG_INFO()

// clang-format off
zend_function_entry conjunction_search_query_methods[] = {
    PHP_ME(ConjunctionSearchQuery, __construct, ai_ConjunctionSearchQuery_none, ZEND_ACC_PRIVATE | ZEND_ACC_FINAL | ZEND_ACC_CTOR)
    PHP_ME(ConjunctionSearchQuery, jsonSerialize, ai_ConjunctionSearchQuery_none, ZEND_ACC_PUBLIC | ZEND_ACC_FINAL)
    PHP_ME(ConjunctionSearchQuery, boost, ai_ConjunctionSearchQuery_boost, ZEND_ACC_PUBLIC | ZEND_ACC_FINAL)
    PHP_ME(ConjunctionSearchQuery, every, ai_ConjunctionSearchQuery_every, ZEND_ACC_PUBLIC | ZEND_ACC_FINAL)
    PHP_FE_END
};
// clang-format on

void pcbc_conjunction_search_query_init(zval *return_value,
#if PHP_VERSION_ID >= 70000
                                        zval *args,
#else
                                        zval ***args,
#endif
                                        int num_args TSRMLS_DC)
{
    pcbc_conjunction_search_query_t *obj;

    object_init_ex(return_value, pcbc_conjunction_search_query_ce);
    obj = Z_CONJUNCTION_SEARCH_QUERY_OBJ_P(return_value);
    obj->boost = -1;

    PCBC_ZVAL_ALLOC(obj->queries);
    array_init(PCBC_P(obj->queries));

    if (num_args && args) {
        int i;
        for (i = 0; i < num_args; ++i) {
            PCBC_ZVAL *query;
#if PHP_VERSION_ID >= 70000
            query = &args[i];
#else
            query = args[i];
#endif
            if (Z_TYPE_P(PCBC_P(*query)) != IS_OBJECT ||
                !instanceof_function(Z_OBJCE_P(PCBC_P(*query)), pcbc_search_query_part_ce TSRMLS_CC)) {
                pcbc_log(LOGARGS(WARN), "query has to implement SearchQueryPart interface (skipping argument #%d)", i);
                continue;
            }
            add_next_index_zval(PCBC_P(obj->queries), PCBC_P(*query));
        }
    }
}

zend_object_handlers conjunction_search_query_handlers;

static void conjunction_search_query_free_object(pcbc_free_object_arg *object TSRMLS_DC) /* {{{ */
{
    pcbc_conjunction_search_query_t *obj = Z_CONJUNCTION_SEARCH_QUERY_OBJ(object);

    zval_ptr_dtor(&obj->queries);

    zend_object_std_dtor(&obj->std TSRMLS_CC);
#if PHP_VERSION_ID < 70000
    efree(obj);
#endif
} /* }}} */

static pcbc_create_object_retval conjunction_search_query_create_object(zend_class_entry *class_type TSRMLS_DC)
{
    pcbc_conjunction_search_query_t *obj = NULL;

    obj = PCBC_ALLOC_OBJECT_T(pcbc_conjunction_search_query_t, class_type);

    zend_object_std_init(&obj->std, class_type TSRMLS_CC);
    object_properties_init(&obj->std, class_type);

#if PHP_VERSION_ID >= 70000
    obj->std.handlers = &conjunction_search_query_handlers;
    return &obj->std;
#else
    {
        zend_object_value ret;
        ret.handle = zend_objects_store_put(obj, (zend_objects_store_dtor_t)zend_objects_destroy_object,
                                            conjunction_search_query_free_object, NULL TSRMLS_CC);
        ret.handlers = &conjunction_search_query_handlers;
        return ret;
    }
#endif
}

static HashTable *pcbc_conjunction_search_query_get_debug_info(zval *object, int *is_temp TSRMLS_DC) /* {{{ */
{
    pcbc_conjunction_search_query_t *obj = NULL;
#if PHP_VERSION_ID >= 70000
    zval retval;
#else
    zval retval = zval_used_for_init;
#endif

    *is_temp = 1;
    obj = Z_CONJUNCTION_SEARCH_QUERY_OBJ_P(object);

    array_init(&retval);
    ADD_ASSOC_ZVAL_EX(&retval, "queries", PCBC_P(obj->queries));
    PCBC_ADDREF_P(PCBC_P(obj->queries));
    if (obj->boost >= 0) {
        ADD_ASSOC_DOUBLE_EX(&retval, "boost", obj->boost);
    }
    return Z_ARRVAL(retval);
} /* }}} */

PHP_MINIT_FUNCTION(ConjunctionSearchQuery)
{
    zend_class_entry ce;

    INIT_NS_CLASS_ENTRY(ce, "Couchbase", "ConjunctionSearchQuery", conjunction_search_query_methods);
    pcbc_conjunction_search_query_ce = zend_register_internal_class(&ce TSRMLS_CC);
    pcbc_conjunction_search_query_ce->create_object = conjunction_search_query_create_object;
    PCBC_CE_FLAGS_FINAL(pcbc_conjunction_search_query_ce);
    PCBC_CE_DISABLE_SERIALIZATION(pcbc_conjunction_search_query_ce);

    zend_class_implements(pcbc_conjunction_search_query_ce TSRMLS_CC, 1, php_json_serializable_ce);
    zend_class_implements(pcbc_conjunction_search_query_ce TSRMLS_CC, 1, pcbc_search_query_part_ce);

    memcpy(&conjunction_search_query_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    conjunction_search_query_handlers.get_debug_info = pcbc_conjunction_search_query_get_debug_info;
#if PHP_VERSION_ID >= 70000
    conjunction_search_query_handlers.free_obj = conjunction_search_query_free_object;
    conjunction_search_query_handlers.offset = XtOffsetOf(pcbc_conjunction_search_query_t, std);
#endif

    zend_register_class_alias("\\CouchbaseConjunctionSearchQuery", pcbc_conjunction_search_query_ce);
    return SUCCESS;
}
