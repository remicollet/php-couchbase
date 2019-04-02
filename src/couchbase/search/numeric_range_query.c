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
 * A FTS query that matches documents on a range of values. At least one bound is required, and the
 * inclusiveness of each bound can be configured.
 */
#include "couchbase.h"

typedef struct {

    double boost;
    char *field;
    double min;
    double max;
    zend_bool inclusive_min;
    zend_bool inclusive_max;
    zend_bool min_set;
    zend_bool max_set;
    zend_object std;
} pcbc_numeric_range_search_query_t;

static inline pcbc_numeric_range_search_query_t *pcbc_numeric_range_search_query_fetch_object(zend_object *obj)
{
    return (pcbc_numeric_range_search_query_t *)((char *)obj - XtOffsetOf(pcbc_numeric_range_search_query_t, std));
}
#define Z_NUMERIC_RANGE_SEARCH_QUERY_OBJ(zo) (pcbc_numeric_range_search_query_fetch_object(zo))
#define Z_NUMERIC_RANGE_SEARCH_QUERY_OBJ_P(zv) (pcbc_numeric_range_search_query_fetch_object(Z_OBJ_P(zv)))

#define LOGARGS(lvl) LCB_LOG_##lvl, NULL, "pcbc/numeric_range_search_query", __FILE__, __LINE__

zend_class_entry *pcbc_numeric_range_search_query_ce;

/* {{{ proto void NumericRangeSearchQuery::__construct() */
PHP_METHOD(NumericRangeSearchQuery, __construct)
{
    throw_pcbc_exception("Accessing private constructor.", LCB_EINVAL);
}
/* }}} */

/* {{{ proto \Couchbase\NumericRangeSearchQuery NumericRangeSearchQuery::field(string $field)
 */
PHP_METHOD(NumericRangeSearchQuery, field)
{
    pcbc_numeric_range_search_query_t *obj;
    char *field = NULL;
    int rv;
    size_t field_len;

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &field, &field_len);
    if (rv == FAILURE) {
        RETURN_NULL();
    }

    obj = Z_NUMERIC_RANGE_SEARCH_QUERY_OBJ_P(getThis());
    if (obj->field) {
        efree(obj->field);
    }
    obj->field = estrndup(field, field_len);

    RETURN_ZVAL(getThis(), 1, 0);
} /* }}} */

/* {{{ proto \Couchbase\NumericRangeSearchQuery NumericRangeSearchQuery::boost(double $boost)
 */
PHP_METHOD(NumericRangeSearchQuery, boost)
{
    pcbc_numeric_range_search_query_t *obj;
    double boost = 0;
    int rv;

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "d", &boost);
    if (rv == FAILURE) {
        RETURN_NULL();
    }

    obj = Z_NUMERIC_RANGE_SEARCH_QUERY_OBJ_P(getThis());
    obj->boost = boost;

    RETURN_ZVAL(getThis(), 1, 0);
} /* }}} */

/* {{{ proto \Couchbase\NumericRangeSearchQuery NumericRangeSearchQuery::min(double $min, bool $inclusive = true)
 */
PHP_METHOD(NumericRangeSearchQuery, min)
{
    pcbc_numeric_range_search_query_t *obj;
    double min = 0;
    zend_bool inclusive = 1;
    int rv;

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "d|b", &min, &inclusive);
    if (rv == FAILURE) {
        RETURN_NULL();
    }

    obj = Z_NUMERIC_RANGE_SEARCH_QUERY_OBJ_P(getThis());
    obj->min = min;
    obj->min_set = 1;
    obj->inclusive_min = inclusive;

    RETURN_ZVAL(getThis(), 1, 0);
} /* }}} */

/* {{{ proto \Couchbase\NumericRangeSearchQuery NumericRangeSearchQuery::max(double $max, bool $inclusive = false)
 */
PHP_METHOD(NumericRangeSearchQuery, max)
{
    pcbc_numeric_range_search_query_t *obj;
    double max = 0;
    zend_bool inclusive = 0;
    int rv;

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "d|b", &max, &inclusive);
    if (rv == FAILURE) {
        RETURN_NULL();
    }

    obj = Z_NUMERIC_RANGE_SEARCH_QUERY_OBJ_P(getThis());
    obj->max = max;
    obj->max_set = 1;
    obj->inclusive_max = inclusive;

    RETURN_ZVAL(getThis(), 1, 0);
} /* }}} */

/* {{{ proto array NumericRangeSearchQuery::jsonSerialize()
 */
PHP_METHOD(NumericRangeSearchQuery, jsonSerialize)
{
    pcbc_numeric_range_search_query_t *obj;
    int rv;

    rv = zend_parse_parameters_none();
    if (rv == FAILURE) {
        RETURN_NULL();
    }

    obj = Z_NUMERIC_RANGE_SEARCH_QUERY_OBJ_P(getThis());
    array_init(return_value);
    if (obj->min_set) {
        ADD_ASSOC_DOUBLE_EX(return_value, "min", obj->min);
        ADD_ASSOC_BOOL_EX(return_value, "inclusive_min", obj->inclusive_min);
    }
    if (obj->max_set) {
        ADD_ASSOC_DOUBLE_EX(return_value, "max", obj->max);
        ADD_ASSOC_BOOL_EX(return_value, "inclusive_max", obj->inclusive_max);
    }
    if (obj->field) {
        ADD_ASSOC_STRING(return_value, "field", obj->field);
    }
    if (obj->boost >= 0) {
        ADD_ASSOC_DOUBLE_EX(return_value, "boost", obj->boost);
    }
} /* }}} */

ZEND_BEGIN_ARG_INFO_EX(ai_NumericRangeSearchQuery_none, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_NumericRangeSearchQuery_field, 0, 0, 1)
ZEND_ARG_INFO(0, field)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_NumericRangeSearchQuery_boost, 0, 0, 1)
ZEND_ARG_INFO(0, boost)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_NumericRangeSearchQuery_min, 0, 0, 2)
ZEND_ARG_INFO(0, min)
ZEND_ARG_INFO(0, inclusive)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_NumericRangeSearchQuery_max, 0, 0, 2)
ZEND_ARG_INFO(0, max)
ZEND_ARG_INFO(0, inclusive)
ZEND_END_ARG_INFO()

// clang-format off
zend_function_entry numeric_range_search_query_methods[] = {
    PHP_ME(NumericRangeSearchQuery, __construct, ai_NumericRangeSearchQuery_none, ZEND_ACC_PRIVATE | ZEND_ACC_FINAL | ZEND_ACC_CTOR)
    PHP_ME(NumericRangeSearchQuery, jsonSerialize, ai_NumericRangeSearchQuery_none, ZEND_ACC_PUBLIC)
    PHP_ME(NumericRangeSearchQuery, boost, ai_NumericRangeSearchQuery_boost, ZEND_ACC_PUBLIC)
    PHP_ME(NumericRangeSearchQuery, field, ai_NumericRangeSearchQuery_field, ZEND_ACC_PUBLIC)
    PHP_ME(NumericRangeSearchQuery, min, ai_NumericRangeSearchQuery_min, ZEND_ACC_PUBLIC)
    PHP_ME(NumericRangeSearchQuery, max, ai_NumericRangeSearchQuery_max, ZEND_ACC_PUBLIC)
    PHP_FE_END
};
// clang-format on

void pcbc_numeric_range_search_query_init(zval *return_value TSRMLS_DC)
{
    pcbc_numeric_range_search_query_t *obj;

    object_init_ex(return_value, pcbc_numeric_range_search_query_ce);
    obj = Z_NUMERIC_RANGE_SEARCH_QUERY_OBJ_P(return_value);
    obj->boost = -1;
    obj->field = NULL;
}

zend_object_handlers numeric_range_search_query_handlers;

static void numeric_range_search_query_free_object(zend_object *object TSRMLS_DC) /* {{{ */
{
    pcbc_numeric_range_search_query_t *obj = Z_NUMERIC_RANGE_SEARCH_QUERY_OBJ(object);

    if (obj->field != NULL) {
        efree(obj->field);
    }

    zend_object_std_dtor(&obj->std TSRMLS_CC);
} /* }}} */

static zend_object *numeric_range_search_query_create_object(zend_class_entry *class_type TSRMLS_DC)
{
    pcbc_numeric_range_search_query_t *obj = NULL;

    obj = PCBC_ALLOC_OBJECT_T(pcbc_numeric_range_search_query_t, class_type);

    zend_object_std_init(&obj->std, class_type TSRMLS_CC);
    object_properties_init(&obj->std, class_type);

    obj->std.handlers = &numeric_range_search_query_handlers;
    return &obj->std;
}

static HashTable *pcbc_numeric_range_search_query_get_debug_info(zval *object, int *is_temp TSRMLS_DC) /* {{{ */
{
    pcbc_numeric_range_search_query_t *obj = NULL;
    zval retval;

    *is_temp = 1;
    obj = Z_NUMERIC_RANGE_SEARCH_QUERY_OBJ_P(object);

    array_init(&retval);
    if (obj->min_set) {
        ADD_ASSOC_DOUBLE_EX(&retval, "min", obj->min);
        ADD_ASSOC_BOOL_EX(&retval, "inclusive_min", obj->inclusive_min);
    }
    if (obj->max_set) {
        ADD_ASSOC_DOUBLE_EX(&retval, "max", obj->max);
        ADD_ASSOC_BOOL_EX(&retval, "inclusive_max", obj->inclusive_max);
    }
    if (obj->field) {
        ADD_ASSOC_STRING(&retval, "field", obj->field);
    }
    if (obj->boost >= 0) {
        ADD_ASSOC_DOUBLE_EX(&retval, "boost", obj->boost);
    }
    return Z_ARRVAL(retval);
} /* }}} */

PHP_MINIT_FUNCTION(NumericRangeSearchQuery)
{
    zend_class_entry ce;

    INIT_NS_CLASS_ENTRY(ce, "Couchbase", "NumericRangeSearchQuery", numeric_range_search_query_methods);
    pcbc_numeric_range_search_query_ce = zend_register_internal_class(&ce TSRMLS_CC);
    pcbc_numeric_range_search_query_ce->create_object = numeric_range_search_query_create_object;
    PCBC_CE_DISABLE_SERIALIZATION(pcbc_numeric_range_search_query_ce);

    zend_class_implements(pcbc_numeric_range_search_query_ce TSRMLS_CC, 1, pcbc_json_serializable_ce);
    zend_class_implements(pcbc_numeric_range_search_query_ce TSRMLS_CC, 1, pcbc_search_query_part_ce);

    memcpy(&numeric_range_search_query_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    numeric_range_search_query_handlers.get_debug_info = pcbc_numeric_range_search_query_get_debug_info;
    numeric_range_search_query_handlers.free_obj = numeric_range_search_query_free_object;
    numeric_range_search_query_handlers.offset = XtOffsetOf(pcbc_numeric_range_search_query_t, std);

    zend_register_class_alias("\\CouchbaseNumericRangeSearchQuery", pcbc_numeric_range_search_query_ce);
    return SUCCESS;
}
