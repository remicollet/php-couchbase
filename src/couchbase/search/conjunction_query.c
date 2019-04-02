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

/**
 * A compound FTS query that performs a logical AND between all its sub-queries (conjunction).
 */
#include "couchbase.h"

typedef struct {

    double boost;
    zval queries;
    zend_object std;
} pcbc_conjunction_search_query_t;

static inline pcbc_conjunction_search_query_t *pcbc_conjunction_search_query_fetch_object(zend_object *obj)
{
    return (pcbc_conjunction_search_query_t *)((char *)obj - XtOffsetOf(pcbc_conjunction_search_query_t, std));
}
#define Z_CONJUNCTION_SEARCH_QUERY_OBJ(zo) (pcbc_conjunction_search_query_fetch_object(zo))
#define Z_CONJUNCTION_SEARCH_QUERY_OBJ_P(zv) (pcbc_conjunction_search_query_fetch_object(Z_OBJ_P(zv)))

#define LOGARGS(lvl) LCB_LOG_##lvl, NULL, "pcbc/conjunction_search_query", __FILE__, __LINE__

zend_class_entry *pcbc_conjunction_search_query_ce;

/* {{{ proto void ConjunctionSearchQuery::__construct() */
PHP_METHOD(ConjunctionSearchQuery, __construct)
{
    throw_pcbc_exception("Accessing private constructor.", LCB_EINVAL);
}
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
    zval *args = NULL;
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
            zval *query;
            query = &args[i];
            if (Z_TYPE_P(query) != IS_OBJECT ||
                !instanceof_function(Z_OBJCE_P(query), pcbc_search_query_part_ce TSRMLS_CC)) {
                pcbc_log(LOGARGS(WARN), "query has to implement SearchQueryPart interface (skipping argument #%d)", i);
                continue;
            }
            add_next_index_zval(&obj->queries, query);
            PCBC_ADDREF_P(query);
        }
    }
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
    ADD_ASSOC_ZVAL_EX(return_value, "conjuncts", &obj->queries);
    PCBC_ADDREF_P(&obj->queries);
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
    PHP_ME(ConjunctionSearchQuery, jsonSerialize, ai_ConjunctionSearchQuery_none, ZEND_ACC_PUBLIC)
    PHP_ME(ConjunctionSearchQuery, boost, ai_ConjunctionSearchQuery_boost, ZEND_ACC_PUBLIC)
    PHP_ME(ConjunctionSearchQuery, every, ai_ConjunctionSearchQuery_every, ZEND_ACC_PUBLIC)
    PHP_FE_END
};
// clang-format on

void pcbc_conjunction_search_query_init(zval *return_value, zval *args, int num_args TSRMLS_DC)
{
    pcbc_conjunction_search_query_t *obj;

    object_init_ex(return_value, pcbc_conjunction_search_query_ce);
    obj = Z_CONJUNCTION_SEARCH_QUERY_OBJ_P(return_value);
    obj->boost = -1;

    ZVAL_UNDEF(&obj->queries);
    array_init(&obj->queries);

    if (num_args && args) {
        int i;
        for (i = 0; i < num_args; ++i) {
            zval *query;
            query = &args[i];
            if (Z_TYPE_P(query) != IS_OBJECT ||
                !instanceof_function(Z_OBJCE_P(query), pcbc_search_query_part_ce TSRMLS_CC)) {
                pcbc_log(LOGARGS(WARN), "query has to implement SearchQueryPart interface (skipping argument #%d)", i);
                continue;
            }
            add_next_index_zval(&obj->queries, query);
            PCBC_ADDREF_P(query);
        }
    }
}

zend_object_handlers conjunction_search_query_handlers;

static void conjunction_search_query_free_object(zend_object *object TSRMLS_DC) /* {{{ */
{
    pcbc_conjunction_search_query_t *obj = Z_CONJUNCTION_SEARCH_QUERY_OBJ(object);

    zval_ptr_dtor(&obj->queries);

    zend_object_std_dtor(&obj->std TSRMLS_CC);
} /* }}} */

static zend_object *conjunction_search_query_create_object(zend_class_entry *class_type TSRMLS_DC)
{
    pcbc_conjunction_search_query_t *obj = NULL;

    obj = PCBC_ALLOC_OBJECT_T(pcbc_conjunction_search_query_t, class_type);

    zend_object_std_init(&obj->std, class_type TSRMLS_CC);
    object_properties_init(&obj->std, class_type);

    obj->std.handlers = &conjunction_search_query_handlers;
    return &obj->std;
}

static HashTable *pcbc_conjunction_search_query_get_debug_info(zval *object, int *is_temp TSRMLS_DC) /* {{{ */
{
    pcbc_conjunction_search_query_t *obj = NULL;
    zval retval;

    *is_temp = 1;
    obj = Z_CONJUNCTION_SEARCH_QUERY_OBJ_P(object);

    array_init(&retval);
    ADD_ASSOC_ZVAL_EX(&retval, "queries", &obj->queries);
    PCBC_ADDREF_P(&obj->queries);
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
    PCBC_CE_DISABLE_SERIALIZATION(pcbc_conjunction_search_query_ce);

    zend_class_implements(pcbc_conjunction_search_query_ce TSRMLS_CC, 1, pcbc_json_serializable_ce);
    zend_class_implements(pcbc_conjunction_search_query_ce TSRMLS_CC, 1, pcbc_search_query_part_ce);

    memcpy(&conjunction_search_query_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    conjunction_search_query_handlers.get_debug_info = pcbc_conjunction_search_query_get_debug_info;
    conjunction_search_query_handlers.free_obj = conjunction_search_query_free_object;
    conjunction_search_query_handlers.offset = XtOffsetOf(pcbc_conjunction_search_query_t, std);

    zend_register_class_alias("\\CouchbaseConjunctionSearchQuery", pcbc_conjunction_search_query_ce);
    return SUCCESS;
}
