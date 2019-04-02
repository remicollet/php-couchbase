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
#include <ext/date/php_date.h>

typedef struct {

    double boost;
    char *field;
    char *start;
    int start_len;
    char *end;
    int end_len;
    char *date_time_parser;
    zend_bool inclusive_start;
    zend_bool inclusive_end;
    zend_object std;
} pcbc_date_range_search_query_t;

static inline pcbc_date_range_search_query_t *pcbc_date_range_search_query_fetch_object(zend_object *obj)
{
    return (pcbc_date_range_search_query_t *)((char *)obj - XtOffsetOf(pcbc_date_range_search_query_t, std));
}
#define Z_DATE_RANGE_SEARCH_QUERY_OBJ(zo) (pcbc_date_range_search_query_fetch_object(zo))
#define Z_DATE_RANGE_SEARCH_QUERY_OBJ_P(zv) (pcbc_date_range_search_query_fetch_object(Z_OBJ_P(zv)))

#define LOGARGS(lvl) LCB_LOG_##lvl, NULL, "pcbc/date_range_search_query", __FILE__, __LINE__

zend_class_entry *pcbc_date_range_search_query_ce;

/* {{{ proto void DateRangeSearchQuery::__construct() */
PHP_METHOD(DateRangeSearchQuery, __construct)
{
    throw_pcbc_exception("Accessing private constructor.", LCB_EINVAL);
}
/* }}} */

/* {{{ proto \Couchbase\DateRangeSearchQuery DateRangeSearchQuery::field(string $field)
 */
PHP_METHOD(DateRangeSearchQuery, field)
{
    pcbc_date_range_search_query_t *obj;
    char *field = NULL;
    int rv;
    size_t field_len;

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &field, &field_len);
    if (rv == FAILURE) {
        RETURN_NULL();
    }

    obj = Z_DATE_RANGE_SEARCH_QUERY_OBJ_P(getThis());
    if (obj->field) {
        efree(obj->field);
    }
    obj->field = estrndup(field, field_len);

    RETURN_ZVAL(getThis(), 1, 0);
} /* }}} */

/* {{{ proto \Couchbase\DateRangeSearchQuery DateRangeSearchQuery::dateTimeParser(string $dateTimeParser)
 */
PHP_METHOD(DateRangeSearchQuery, dateTimeParser)
{
    pcbc_date_range_search_query_t *obj;
    char *date_time_parser = NULL;
    int rv;
    size_t date_time_parser_len;

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &date_time_parser, &date_time_parser_len);
    if (rv == FAILURE) {
        RETURN_NULL();
    }

    obj = Z_DATE_RANGE_SEARCH_QUERY_OBJ_P(getThis());
    if (obj->date_time_parser) {
        efree(obj->date_time_parser);
    }
    obj->date_time_parser = estrndup(date_time_parser, date_time_parser_len);

    RETURN_ZVAL(getThis(), 1, 0);
} /* }}} */

/* {{{ proto \Couchbase\DateRangeSearchQuery DateRangeSearchQuery::boost(double $boost)
 */
PHP_METHOD(DateRangeSearchQuery, boost)
{
    pcbc_date_range_search_query_t *obj;
    double boost = 0;
    int rv;

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "d", &boost);
    if (rv == FAILURE) {
        RETURN_NULL();
    }

    obj = Z_DATE_RANGE_SEARCH_QUERY_OBJ_P(getThis());
    obj->boost = boost;

    RETURN_ZVAL(getThis(), 1, 0);
} /* }}} */

/* {{{ proto \Couchbase\DateRangeSearchQuery DateRangeSearchQuery::start(string $start, bool $inclusive = true)
 *
 * The strings will be taken verbatim and supposed to be formatted with custom date time formatter (see
 * dateTimeParser). Integers interpreted as unix timestamps and represented as RFC3339 strings.
 */
PHP_METHOD(DateRangeSearchQuery, start)
{
    pcbc_date_range_search_query_t *obj;
    zval *start = NULL;
    zend_bool inclusive = 1;
    int rv;

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z|b", &start, &inclusive);
    if (rv == FAILURE) {
        RETURN_NULL();
    }

    obj = Z_DATE_RANGE_SEARCH_QUERY_OBJ_P(getThis());
    obj->inclusive_start = inclusive;
    switch (Z_TYPE_P(start)) {
    case IS_STRING:
        obj->start = estrndup(Z_STRVAL_P(start), Z_STRLEN_P(start));
        break;
    case IS_LONG: {
        zend_string *date_str = NULL;
        date_str = php_format_date(ZEND_STRL(PCBC_DATE_FORMAT_RFC3339), Z_LVAL_P(start), 1 TSRMLS_CC);
        obj->start = ZSTR_VAL(date_str);
    } break;
    default:
        throw_pcbc_exception("Date should be either formatted string or integer (Unix timestamp)", LCB_EINVAL);
        RETURN_NULL();
    }

    RETURN_ZVAL(getThis(), 1, 0);
} /* }}} */

/* {{{ proto \Couchbase\DateRangeSearchQuery DateRangeSearchQuery::end(string $end, bool $inclusive = false)
 *
 * The strings will be taken verbatim and supposed to be formatted with custom date time formatter (see
 * dateTimeParser). Integers interpreted as unix timestamps and represented as RFC3339 strings.
 */
PHP_METHOD(DateRangeSearchQuery, end)
{
    pcbc_date_range_search_query_t *obj;
    zval *end = NULL;
    zend_bool inclusive = 0;
    int rv;

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z|b", &end, &inclusive);
    if (rv == FAILURE) {
        RETURN_NULL();
    }

    obj = Z_DATE_RANGE_SEARCH_QUERY_OBJ_P(getThis());
    obj->inclusive_end = inclusive;
    switch (Z_TYPE_P(end)) {
    case IS_STRING:
        obj->end = estrndup(Z_STRVAL_P(end), Z_STRLEN_P(end));
        break;
    case IS_LONG: {
        zend_string *date_str = NULL;
        date_str = php_format_date(ZEND_STRL(PCBC_DATE_FORMAT_RFC3339), Z_LVAL_P(end), 1 TSRMLS_CC);
        obj->end = ZSTR_VAL(date_str);
    } break;
    default:
        throw_pcbc_exception("Date should be either formatted string or integer (Unix timestamp)", LCB_EINVAL);
        RETURN_NULL();
    }

    RETURN_ZVAL(getThis(), 1, 0);
} /* }}} */

/* {{{ proto array DateRangeSearchQuery::jsonSerialize()
 */
PHP_METHOD(DateRangeSearchQuery, jsonSerialize)
{
    pcbc_date_range_search_query_t *obj;
    int rv;

    rv = zend_parse_parameters_none();
    if (rv == FAILURE) {
        RETURN_NULL();
    }

    obj = Z_DATE_RANGE_SEARCH_QUERY_OBJ_P(getThis());
    array_init(return_value);
    if (obj->start) {
        ADD_ASSOC_STRING(return_value, "start", obj->start);
        ADD_ASSOC_BOOL_EX(return_value, "inclusive_start", obj->inclusive_start);
    }
    if (obj->end) {
        ADD_ASSOC_STRING(return_value, "end", obj->end);
        ADD_ASSOC_BOOL_EX(return_value, "inclusive_end", obj->inclusive_end);
    }
    if (obj->date_time_parser) {
        ADD_ASSOC_STRING(return_value, "datetime_parser", obj->date_time_parser);
    }
    if (obj->field) {
        ADD_ASSOC_STRING(return_value, "field", obj->field);
    }
    if (obj->boost >= 0) {
        ADD_ASSOC_DOUBLE_EX(return_value, "boost", obj->boost);
    }
} /* }}} */

ZEND_BEGIN_ARG_INFO_EX(ai_DateRangeSearchQuery_none, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_DateRangeSearchQuery_field, 0, 0, 1)
ZEND_ARG_INFO(0, field)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_DateRangeSearchQuery_boost, 0, 0, 1)
ZEND_ARG_INFO(0, boost)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_DateRangeSearchQuery_start, 0, 0, 2)
ZEND_ARG_INFO(0, start)
ZEND_ARG_INFO(0, inclusive)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_DateRangeSearchQuery_end, 0, 0, 2)
ZEND_ARG_INFO(0, end)
ZEND_ARG_INFO(0, inclusive)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_DateRangeSearchQuery_dateTimeParser, 0, 0, 1)
ZEND_ARG_INFO(0, dateTimeParser)
ZEND_END_ARG_INFO()

// clang-format off
zend_function_entry date_range_search_query_methods[] = {
    PHP_ME(DateRangeSearchQuery, __construct, ai_DateRangeSearchQuery_none, ZEND_ACC_PRIVATE | ZEND_ACC_FINAL | ZEND_ACC_CTOR)
    PHP_ME(DateRangeSearchQuery, jsonSerialize, ai_DateRangeSearchQuery_none, ZEND_ACC_PUBLIC)
    PHP_ME(DateRangeSearchQuery, boost, ai_DateRangeSearchQuery_boost, ZEND_ACC_PUBLIC)
    PHP_ME(DateRangeSearchQuery, field, ai_DateRangeSearchQuery_field, ZEND_ACC_PUBLIC)
    PHP_ME(DateRangeSearchQuery, start, ai_DateRangeSearchQuery_start, ZEND_ACC_PUBLIC)
    PHP_ME(DateRangeSearchQuery, end, ai_DateRangeSearchQuery_end, ZEND_ACC_PUBLIC)
    PHP_ME(DateRangeSearchQuery, dateTimeParser, ai_DateRangeSearchQuery_dateTimeParser, ZEND_ACC_PUBLIC)
    PHP_FE_END
};
// clang-format on

void pcbc_date_range_search_query_init(zval *return_value TSRMLS_DC)
{
    pcbc_date_range_search_query_t *obj;

    object_init_ex(return_value, pcbc_date_range_search_query_ce);
    obj = Z_DATE_RANGE_SEARCH_QUERY_OBJ_P(return_value);
    obj->boost = -1;
    obj->field = NULL;
}

zend_object_handlers date_range_search_query_handlers;

static void date_range_search_query_free_object(zend_object *object TSRMLS_DC) /* {{{ */
{
    pcbc_date_range_search_query_t *obj = Z_DATE_RANGE_SEARCH_QUERY_OBJ(object);

    if (obj->field != NULL) {
        efree(obj->field);
    }
    if (obj->start != NULL) {
        efree(obj->start);
    }
    if (obj->end != NULL) {
        efree(obj->end);
    }
    if (obj->date_time_parser != NULL) {
        efree(obj->date_time_parser);
    }

    zend_object_std_dtor(&obj->std TSRMLS_CC);
} /* }}} */

static zend_object *date_range_search_query_create_object(zend_class_entry *class_type TSRMLS_DC)
{
    pcbc_date_range_search_query_t *obj = NULL;

    obj = PCBC_ALLOC_OBJECT_T(pcbc_date_range_search_query_t, class_type);

    zend_object_std_init(&obj->std, class_type TSRMLS_CC);
    object_properties_init(&obj->std, class_type);

    obj->std.handlers = &date_range_search_query_handlers;
    return &obj->std;
}

static HashTable *pcbc_date_range_search_query_get_debug_info(zval *object, int *is_temp TSRMLS_DC) /* {{{ */
{
    pcbc_date_range_search_query_t *obj = NULL;
    zval retval;

    *is_temp = 1;
    obj = Z_DATE_RANGE_SEARCH_QUERY_OBJ_P(object);

    array_init(&retval);
    if (obj->start) {
        ADD_ASSOC_STRING(&retval, "start", obj->start);
        ADD_ASSOC_BOOL_EX(&retval, "inclusiveStart", obj->inclusive_start);
    }
    if (obj->end) {
        ADD_ASSOC_STRING(&retval, "end", obj->end);
        ADD_ASSOC_BOOL_EX(&retval, "inclusiveEnd", obj->inclusive_end);
    }
    if (obj->date_time_parser) {
        ADD_ASSOC_STRING(&retval, "dateTimeParser", obj->date_time_parser);
    }
    if (obj->field) {
        ADD_ASSOC_STRING(&retval, "field", obj->field);
    }
    if (obj->boost >= 0) {
        ADD_ASSOC_DOUBLE_EX(&retval, "boost", obj->boost);
    }
    return Z_ARRVAL(retval);
} /* }}} */

PHP_MINIT_FUNCTION(DateRangeSearchQuery)
{
    zend_class_entry ce;

    INIT_NS_CLASS_ENTRY(ce, "Couchbase", "DateRangeSearchQuery", date_range_search_query_methods);
    pcbc_date_range_search_query_ce = zend_register_internal_class(&ce TSRMLS_CC);
    pcbc_date_range_search_query_ce->create_object = date_range_search_query_create_object;
    PCBC_CE_DISABLE_SERIALIZATION(pcbc_date_range_search_query_ce);

    zend_class_implements(pcbc_date_range_search_query_ce TSRMLS_CC, 1, pcbc_json_serializable_ce);
    zend_class_implements(pcbc_date_range_search_query_ce TSRMLS_CC, 1, pcbc_search_query_part_ce);

    memcpy(&date_range_search_query_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    date_range_search_query_handlers.get_debug_info = pcbc_date_range_search_query_get_debug_info;
    date_range_search_query_handlers.free_obj = date_range_search_query_free_object;
    date_range_search_query_handlers.offset = XtOffsetOf(pcbc_date_range_search_query_t, std);

    zend_register_class_alias("\\CouchbaseDateRangeSearchQuery", pcbc_date_range_search_query_ce);
    return SUCCESS;
}
