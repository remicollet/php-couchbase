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
 * A FTS query that matches documents on a range of values. At least one bound is required, and the
 * inclusiveness of each bound can be configured.
 */
#include "couchbase.h"

typedef struct {
    PCBC_ZEND_OBJECT_PRE
    double boost;
    char *field;
    char *min;
    char *max;
    int min_len;
    int max_len;
    zend_bool inclusive_min;
    zend_bool inclusive_max;
    PCBC_ZEND_OBJECT_POST
} pcbc_term_range_search_query_t;

#if PHP_VERSION_ID >= 70000
static inline pcbc_term_range_search_query_t *pcbc_term_range_search_query_fetch_object(zend_object *obj)
{
    return (pcbc_term_range_search_query_t *)((char *)obj - XtOffsetOf(pcbc_term_range_search_query_t, std));
}
#define Z_TERM_RANGE_SEARCH_QUERY_OBJ(zo) (pcbc_term_range_search_query_fetch_object(zo))
#define Z_TERM_RANGE_SEARCH_QUERY_OBJ_P(zv) (pcbc_term_range_search_query_fetch_object(Z_OBJ_P(zv)))
#else
#define Z_TERM_RANGE_SEARCH_QUERY_OBJ(zo) ((pcbc_term_range_search_query_t *)zo)
#define Z_TERM_RANGE_SEARCH_QUERY_OBJ_P(zv)                                                                         \
    ((pcbc_term_range_search_query_t *)zend_object_store_get_object(zv TSRMLS_CC))
#endif

#define LOGARGS(lvl) LCB_LOG_##lvl, NULL, "pcbc/term_range_search_query", __FILE__, __LINE__

zend_class_entry *pcbc_term_range_search_query_ce;

/* {{{ proto void TermRangeSearchQuery::__construct() */
PHP_METHOD(TermRangeSearchQuery, __construct)
{
    throw_pcbc_exception("Accessing private constructor.", LCB_EINVAL);
}
/* }}} */

/* {{{ proto \Couchbase\TermRangeSearchQuery TermRangeSearchQuery::field(string $field)
 */
PHP_METHOD(TermRangeSearchQuery, field)
{
    pcbc_term_range_search_query_t *obj;
    char *field = NULL;
    int rv;
    pcbc_str_arg_size field_len;

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &field, &field_len);
    if (rv == FAILURE) {
        RETURN_NULL();
    }

    obj = Z_TERM_RANGE_SEARCH_QUERY_OBJ_P(getThis());
    if (obj->field) {
        efree(obj->field);
    }
    obj->field = estrndup(field, field_len);

    RETURN_ZVAL(getThis(), 1, 0);
} /* }}} */

/* {{{ proto \Couchbase\TermRangeSearchQuery TermRangeSearchQuery::boost(string $boost)
 */
PHP_METHOD(TermRangeSearchQuery, boost)
{
    pcbc_term_range_search_query_t *obj;
    double boost = 0;
    int rv;

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "d", &boost);
    if (rv == FAILURE) {
        RETURN_NULL();
    }

    obj = Z_TERM_RANGE_SEARCH_QUERY_OBJ_P(getThis());
    obj->boost = boost;

    RETURN_ZVAL(getThis(), 1, 0);
} /* }}} */

/* {{{ proto \Couchbase\TermRangeSearchQuery TermRangeSearchQuery::min(string $min, bool $inclusive = true)
 */
PHP_METHOD(TermRangeSearchQuery, min)
{
    pcbc_term_range_search_query_t *obj;
    char *min = NULL;
    pcbc_str_arg_size min_len = 0;
    zend_bool inclusive = 0;
    int rv;

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|b", &min, &min_len, &inclusive);
    if (rv == FAILURE) {
        RETURN_NULL();
    }

    obj = Z_TERM_RANGE_SEARCH_QUERY_OBJ_P(getThis());
    if (obj->min) {
        efree(obj->min);
    }
    obj->min = estrndup(min, min_len);
    obj->min_len = min_len;
    obj->inclusive_min = inclusive;

    RETURN_ZVAL(getThis(), 1, 0);
} /* }}} */

/* {{{ proto \Couchbase\TermRangeSearchQuery TermRangeSearchQuery::max(string $max, bool $inclusive = false)
 */
PHP_METHOD(TermRangeSearchQuery, max)
{
    pcbc_term_range_search_query_t *obj;
    char *max = NULL;
    pcbc_str_arg_size max_len = 0;
    zend_bool inclusive = 0;
    int rv;

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|b", &max, &max_len, &inclusive);
    if (rv == FAILURE) {
        RETURN_NULL();
    }

    obj = Z_TERM_RANGE_SEARCH_QUERY_OBJ_P(getThis());
    if (obj->max) {
        efree(obj->max);
    }
    obj->max = estrndup(max, max_len);
    obj->max_len = max_len;
    obj->inclusive_max = inclusive;

    RETURN_ZVAL(getThis(), 1, 0);
} /* }}} */

/* {{{ proto array TermRangeSearchQuery::jsonSerialize()
 */
PHP_METHOD(TermRangeSearchQuery, jsonSerialize)
{
    pcbc_term_range_search_query_t *obj;
    int rv;

    rv = zend_parse_parameters_none();
    if (rv == FAILURE) {
        RETURN_NULL();
    }

    obj = Z_TERM_RANGE_SEARCH_QUERY_OBJ_P(getThis());
    array_init(return_value);
    if (obj->min) {
        ADD_ASSOC_STRINGL(return_value, "min", obj->min, obj->min_len);
        ADD_ASSOC_BOOL_EX(return_value, "inclusive_min", obj->inclusive_min);
    }
    if (obj->max) {
        ADD_ASSOC_STRINGL(return_value, "max", obj->max, obj->max_len);
        ADD_ASSOC_BOOL_EX(return_value, "inclusive_max", obj->inclusive_max);
    }
    if (obj->field) {
        ADD_ASSOC_STRING(return_value, "field", obj->field);
    }
    if (obj->boost >= 0) {
        ADD_ASSOC_DOUBLE_EX(return_value, "boost", obj->boost);
    }
} /* }}} */

ZEND_BEGIN_ARG_INFO_EX(ai_TermRangeSearchQuery_none, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_TermRangeSearchQuery_field, 0, 0, 1)
ZEND_ARG_INFO(0, field)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_TermRangeSearchQuery_boost, 0, 0, 1)
ZEND_ARG_INFO(0, boost)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_TermRangeSearchQuery_min, 0, 0, 2)
ZEND_ARG_INFO(0, min)
ZEND_ARG_INFO(0, inclusive)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_TermRangeSearchQuery_max, 0, 0, 2)
ZEND_ARG_INFO(0, max)
ZEND_ARG_INFO(0, inclusive)
ZEND_END_ARG_INFO()

// clang-format off
zend_function_entry term_range_search_query_methods[] = {
    PHP_ME(TermRangeSearchQuery, __construct, ai_TermRangeSearchQuery_none, ZEND_ACC_PRIVATE | ZEND_ACC_FINAL | ZEND_ACC_CTOR)
    PHP_ME(TermRangeSearchQuery, jsonSerialize, ai_TermRangeSearchQuery_none, ZEND_ACC_PUBLIC | ZEND_ACC_FINAL)
    PHP_ME(TermRangeSearchQuery, boost, ai_TermRangeSearchQuery_boost, ZEND_ACC_PUBLIC | ZEND_ACC_FINAL)
    PHP_ME(TermRangeSearchQuery, field, ai_TermRangeSearchQuery_field, ZEND_ACC_PUBLIC | ZEND_ACC_FINAL)
    PHP_ME(TermRangeSearchQuery, min, ai_TermRangeSearchQuery_min, ZEND_ACC_PUBLIC | ZEND_ACC_FINAL)
    PHP_ME(TermRangeSearchQuery, max, ai_TermRangeSearchQuery_max, ZEND_ACC_PUBLIC | ZEND_ACC_FINAL)
    PHP_FE_END
};
// clang-format on

void pcbc_term_range_search_query_init(zval *return_value TSRMLS_DC)
{
    pcbc_term_range_search_query_t *obj;

    object_init_ex(return_value, pcbc_term_range_search_query_ce);
    obj = Z_TERM_RANGE_SEARCH_QUERY_OBJ_P(return_value);
    obj->boost = -1;
    obj->field = NULL;
    obj->min = NULL;
    obj->max = NULL;
}

zend_object_handlers term_range_search_query_handlers;

static void term_range_search_query_free_object(pcbc_free_object_arg *object TSRMLS_DC) /* {{{ */
{
    pcbc_term_range_search_query_t *obj = Z_TERM_RANGE_SEARCH_QUERY_OBJ(object);

    if (obj->field != NULL) {
        efree(obj->field);
    }
    if (obj->min != NULL) {
        efree(obj->min);
    }
    if (obj->max != NULL) {
        efree(obj->max);
    }

    zend_object_std_dtor(&obj->std TSRMLS_CC);
#if PHP_VERSION_ID < 70000
    efree(obj);
#endif
} /* }}} */

static pcbc_create_object_retval term_range_search_query_create_object(zend_class_entry *class_type TSRMLS_DC)
{
    pcbc_term_range_search_query_t *obj = NULL;

    obj = PCBC_ALLOC_OBJECT_T(pcbc_term_range_search_query_t, class_type);

    zend_object_std_init(&obj->std, class_type TSRMLS_CC);
    object_properties_init(&obj->std, class_type);

#if PHP_VERSION_ID >= 70000
    obj->std.handlers = &term_range_search_query_handlers;
    return &obj->std;
#else
    {
        zend_object_value ret;
        ret.handle = zend_objects_store_put(obj, (zend_objects_store_dtor_t)zend_objects_destroy_object,
                                            term_range_search_query_free_object, NULL TSRMLS_CC);
        ret.handlers = &term_range_search_query_handlers;
        return ret;
    }
#endif
}

static HashTable *pcbc_term_range_search_query_get_debug_info(zval *object, int *is_temp TSRMLS_DC) /* {{{ */
{
    pcbc_term_range_search_query_t *obj = NULL;
#if PHP_VERSION_ID >= 70000
    zval retval;
#else
    zval retval = zval_used_for_init;
#endif

    *is_temp = 1;
    obj = Z_TERM_RANGE_SEARCH_QUERY_OBJ_P(object);

    array_init(&retval);
    if (obj->min) {
        ADD_ASSOC_STRINGL(&retval, "min", obj->min, obj->min_len);
        ADD_ASSOC_BOOL_EX(&retval, "inclusive_min", obj->inclusive_min);
    }
    if (obj->max) {
        ADD_ASSOC_STRINGL(&retval, "max", obj->max, obj->max_len);
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

PHP_MINIT_FUNCTION(TermRangeSearchQuery)
{
    zend_class_entry ce;

    INIT_NS_CLASS_ENTRY(ce, "Couchbase", "TermRangeSearchQuery", term_range_search_query_methods);
    pcbc_term_range_search_query_ce = zend_register_internal_class(&ce TSRMLS_CC);
    pcbc_term_range_search_query_ce->create_object = term_range_search_query_create_object;
    PCBC_CE_FLAGS_FINAL(pcbc_term_range_search_query_ce);
    PCBC_CE_DISABLE_SERIALIZATION(pcbc_term_range_search_query_ce);

    zend_class_implements(pcbc_term_range_search_query_ce TSRMLS_CC, 1, pcbc_json_serializable_ce);
    zend_class_implements(pcbc_term_range_search_query_ce TSRMLS_CC, 1, pcbc_search_query_part_ce);

    memcpy(&term_range_search_query_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    term_range_search_query_handlers.get_debug_info = pcbc_term_range_search_query_get_debug_info;
#if PHP_VERSION_ID >= 70000
    term_range_search_query_handlers.free_obj = term_range_search_query_free_object;
    term_range_search_query_handlers.offset = XtOffsetOf(pcbc_term_range_search_query_t, std);
#endif

    zend_register_class_alias("\\CouchbaseTermRangeSearchQuery", pcbc_term_range_search_query_ce);
    return SUCCESS;
}
