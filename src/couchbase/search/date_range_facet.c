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
 * A facet that categorizes hits inside date ranges (or buckets) provided by the user.
 */
#include "couchbase.h"
#include <ext/date/php_date.h>

typedef struct {

    double boost;
    char *field;
    int limit;
    zval ranges; /* array or arrays like ["range-name" => ["start" => ..., "end" => ...], ...] */
    zend_object std;
} date_range_search_facet_t;

static inline date_range_search_facet_t *date_range_search_facet_fetch_object(zend_object *obj)
{
    return (date_range_search_facet_t *)((char *)obj - XtOffsetOf(date_range_search_facet_t, std));
}
#define Z_DATE_RANGE_SEARCH_FACET_OBJ(zo) (date_range_search_facet_fetch_object(zo))
#define Z_DATE_RANGE_SEARCH_FACET_OBJ_P(zv) (date_range_search_facet_fetch_object(Z_OBJ_P(zv)))

#define LOGARGS(lvl) LCB_LOG_##lvl, NULL, "pcbc/date_search_facet", __FILE__, __LINE__

zend_class_entry *date_range_search_facet_ce;

/* {{{ proto void DateRangeSearchFacet::__construct() */
PHP_METHOD(DateRangeSearchFacet, __construct)
{
    throw_pcbc_exception("Accessing private constructor.", LCB_EINVAL);
}
/* }}} */

/* {{{ proto \Couchbase\DateRangeSearchFacet DateRangeSearchFacet::addRange(string $name, (string|int) $start,
 * (string|int) $end)
 */
PHP_METHOD(DateRangeSearchFacet, addRange)
{
    date_range_search_facet_t *obj;
    zval *start = NULL, *end = NULL;
    char *name = NULL;
    zval range;
    int rv;
    size_t name_len = 0;
    zend_string *date_str = NULL;

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "szz", &name, &name_len, &start, &end);
    if (rv == FAILURE) {
        RETURN_NULL();
    }

    obj = Z_DATE_RANGE_SEARCH_FACET_OBJ_P(getThis());

    ZVAL_UNDEF(&range);
    array_init_size(&range, 3);
    ADD_ASSOC_STRINGL(&range, "name", name, name_len);
    if (start) {
        switch (Z_TYPE_P(start)) {
        case IS_STRING:
            ADD_ASSOC_STRINGL(&range, "start", Z_STRVAL_P(start), Z_STRLEN_P(start));
            break;
        case IS_LONG:
            date_str = php_format_date(ZEND_STRL(PCBC_DATE_FORMAT_RFC3339), Z_LVAL_P(start), 1 TSRMLS_CC);
            add_assoc_str(&range, "start", date_str);
            break;
        case IS_NULL:
            break;
        default:
            throw_pcbc_exception("Range start should be either formatted string or integer (Unix timestamp)",
                                 LCB_EINVAL);
            zval_ptr_dtor(&range);
            RETURN_NULL();
        }
    }
    if (end) {
        switch (Z_TYPE_P(end)) {
        case IS_STRING:
            ADD_ASSOC_STRINGL(&range, "end", Z_STRVAL_P(end), Z_STRLEN_P(end));
            break;
        case IS_LONG:
            date_str = php_format_date(ZEND_STRL(PCBC_DATE_FORMAT_RFC3339), Z_LVAL_P(end), 1 TSRMLS_CC);
            add_assoc_str(&range, "end", date_str);
            break;
        case IS_NULL:
            break;
        default:
            throw_pcbc_exception("Range end should be either formatted string or integer (Unix timestamp)", LCB_EINVAL);
            zval_ptr_dtor(&range);
            RETURN_NULL();
        }
    }
    add_next_index_zval(&obj->ranges, &range);

    RETURN_ZVAL(getThis(), 1, 0);
} /* }}} */

/* {{{ proto array DateRangeSearchFacet::jsonSerialize()
 */
PHP_METHOD(DateRangeSearchFacet, jsonSerialize)
{
    date_range_search_facet_t *obj;
    int rv;

    rv = zend_parse_parameters_none();
    if (rv == FAILURE) {
        RETURN_NULL();
    }

    obj = Z_DATE_RANGE_SEARCH_FACET_OBJ_P(getThis());
    array_init(return_value);
    ADD_ASSOC_STRING(return_value, "field", obj->field);
    ADD_ASSOC_LONG_EX(return_value, "size", obj->limit);
    ADD_ASSOC_ZVAL_EX(return_value, "date_ranges", &obj->ranges);
    PCBC_ADDREF_P(&obj->ranges);
} /* }}} */

ZEND_BEGIN_ARG_INFO_EX(ai_DateRangeSearchFacet_none, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_DateRangeSearchFacet_addRange, 0, 0, 2)
ZEND_ARG_INFO(0, name)
ZEND_ARG_INFO(0, start)
ZEND_ARG_INFO(0, end)
ZEND_END_ARG_INFO()

// clang-format off
zend_function_entry date_search_facet_methods[] = {
    PHP_ME(DateRangeSearchFacet, __construct, ai_DateRangeSearchFacet_none, ZEND_ACC_PRIVATE | ZEND_ACC_FINAL | ZEND_ACC_CTOR)
    PHP_ME(DateRangeSearchFacet, jsonSerialize, ai_DateRangeSearchFacet_none, ZEND_ACC_PUBLIC)
    PHP_ME(DateRangeSearchFacet, addRange, ai_DateRangeSearchFacet_addRange, ZEND_ACC_PUBLIC)
    PHP_FE_END
};
// clang-format on

void pcbc_date_range_search_facet_init(zval *return_value, char *field, int field_len, int limit TSRMLS_DC)
{
    date_range_search_facet_t *obj;

    object_init_ex(return_value, date_range_search_facet_ce);
    obj = Z_DATE_RANGE_SEARCH_FACET_OBJ_P(return_value);
    obj->field = estrndup(field, field_len);
    obj->limit = limit;

    ZVAL_UNDEF(&obj->ranges);
    array_init(&obj->ranges);
}

zend_object_handlers date_search_facet_handlers;

static void date_search_facet_free_object(zend_object *object TSRMLS_DC) /* {{{ */
{
    date_range_search_facet_t *obj = Z_DATE_RANGE_SEARCH_FACET_OBJ(object);

    if (obj->field != NULL) {
        efree(obj->field);
    }
    zval_ptr_dtor(&obj->ranges);

    zend_object_std_dtor(&obj->std TSRMLS_CC);
} /* }}} */

static zend_object *date_search_facet_create_object(zend_class_entry *class_type TSRMLS_DC)
{
    date_range_search_facet_t *obj = NULL;

    obj = PCBC_ALLOC_OBJECT_T(date_range_search_facet_t, class_type);

    zend_object_std_init(&obj->std, class_type TSRMLS_CC);
    object_properties_init(&obj->std, class_type);

    obj->std.handlers = &date_search_facet_handlers;
    return &obj->std;
}

static HashTable *date_range_search_facet_get_debug_info(zval *object, int *is_temp TSRMLS_DC) /* {{{ */
{
    date_range_search_facet_t *obj = NULL;
    zval retval;

    *is_temp = 1;
    obj = Z_DATE_RANGE_SEARCH_FACET_OBJ_P(object);

    array_init(&retval);
    ADD_ASSOC_STRING(&retval, "field", obj->field);
    ADD_ASSOC_LONG_EX(&retval, "limit", obj->limit);
    ADD_ASSOC_ZVAL_EX(&retval, "date_ranges", &obj->ranges);
    PCBC_ADDREF_P(&obj->ranges);
    return Z_ARRVAL(retval);
} /* }}} */

PHP_MINIT_FUNCTION(DateRangeSearchFacet)
{
    zend_class_entry ce;

    INIT_NS_CLASS_ENTRY(ce, "Couchbase", "DateRangeSearchFacet", date_search_facet_methods);
    date_range_search_facet_ce = zend_register_internal_class(&ce TSRMLS_CC);
    date_range_search_facet_ce->create_object = date_search_facet_create_object;
    PCBC_CE_DISABLE_SERIALIZATION(date_range_search_facet_ce);

    zend_class_implements(date_range_search_facet_ce TSRMLS_CC, 1, pcbc_json_serializable_ce);
    zend_class_implements(date_range_search_facet_ce TSRMLS_CC, 1, pcbc_search_facet_ce);

    memcpy(&date_search_facet_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    date_search_facet_handlers.get_debug_info = date_range_search_facet_get_debug_info;
    date_search_facet_handlers.free_obj = date_search_facet_free_object;
    date_search_facet_handlers.offset = XtOffsetOf(date_range_search_facet_t, std);

    zend_register_class_alias("\\CouchbaseDateRangeSearchFacet", date_range_search_facet_ce);
    return SUCCESS;
}
