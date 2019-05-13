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
 * A facet that categorizes hits into numerical ranges (or buckets) provided by the user.
 */
#include "couchbase.h"

typedef struct {

    double boost;
    char *field;
    int limit;
    zval ranges; /* array or arrays like ["range-name" => ["min" => ..., "max" => ...], ...] */
    zend_object std;
} pcbc_numeric_range_search_facet_t;

static inline pcbc_numeric_range_search_facet_t *pcbc_numeric_range_search_facet_fetch_object(zend_object *obj)
{
    return (pcbc_numeric_range_search_facet_t *)((char *)obj - XtOffsetOf(pcbc_numeric_range_search_facet_t, std));
}
#define Z_NUMERIC_RANGE_SEARCH_FACET_OBJ(zo) (pcbc_numeric_range_search_facet_fetch_object(zo))
#define Z_NUMERIC_RANGE_SEARCH_FACET_OBJ_P(zv) (pcbc_numeric_range_search_facet_fetch_object(Z_OBJ_P(zv)))

#define LOGARGS(lvl) LCB_LOG_##lvl, NULL, "pcbc/numeric_search_facet", __FILE__, __LINE__

zend_class_entry *pcbc_numeric_range_search_facet_ce;

/* {{{ proto void NumericRangeSearchFacet::__construct() */
PHP_METHOD(NumericRangeSearchFacet, __construct)
{
    throw_pcbc_exception("Accessing private constructor.", LCB_EINVAL);
}
/* }}} */

/* {{{ proto \Couchbase\NumericRangeSearchFacet NumericRangeSearchFacet::addRange(string $name, double $min, double
 * $max)
 */
PHP_METHOD(NumericRangeSearchFacet, addRange)
{
    pcbc_numeric_range_search_facet_t *obj;
    char *name = NULL;
    zval range;
    int rv;
    size_t name_len = 0;
    double min = 0, max = 0;
    zend_bool min_null = 0, max_null = 0;

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sd!d!", &name, &name_len, &min, &min_null, &max, &max_null);
    if (rv == FAILURE) {
        RETURN_NULL();
    }

    obj = Z_NUMERIC_RANGE_SEARCH_FACET_OBJ_P(getThis());

    ZVAL_UNDEF(&range);
    array_init_size(&range, 3);
    ADD_ASSOC_STRINGL(&range, "name", name, name_len);
    if (!min_null) {
        ADD_ASSOC_DOUBLE_EX(&range, "min", min);
    }
    if (!max_null) {
        ADD_ASSOC_DOUBLE_EX(&range, "max", max);
    }
    add_next_index_zval(&obj->ranges, &range);

    RETURN_ZVAL(getThis(), 1, 0);
} /* }}} */

/* {{{ proto array NumericRangeSearchFacet::jsonSerialize()
 */
PHP_METHOD(NumericRangeSearchFacet, jsonSerialize)
{
    pcbc_numeric_range_search_facet_t *obj;
    int rv;

    rv = zend_parse_parameters_none();
    if (rv == FAILURE) {
        RETURN_NULL();
    }

    obj = Z_NUMERIC_RANGE_SEARCH_FACET_OBJ_P(getThis());
    array_init(return_value);
    ADD_ASSOC_STRING(return_value, "field", obj->field);
    ADD_ASSOC_LONG_EX(return_value, "size", obj->limit);
    ADD_ASSOC_ZVAL_EX(return_value, "numeric_ranges", &obj->ranges);
    PCBC_ADDREF_P(&obj->ranges);
} /* }}} */

ZEND_BEGIN_ARG_INFO_EX(ai_NumericRangeSearchFacet_none, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_NumericRangeSearchFacet_addRange, 0, 0, 3)
ZEND_ARG_INFO(0, name)
ZEND_ARG_INFO(0, min)
ZEND_ARG_INFO(0, max)
ZEND_END_ARG_INFO()

// clang-format off
zend_function_entry numeric_search_facet_methods[] = {
    PHP_ME(NumericRangeSearchFacet, __construct, ai_NumericRangeSearchFacet_none, ZEND_ACC_PRIVATE | ZEND_ACC_FINAL | ZEND_ACC_CTOR)
    PHP_ME(NumericRangeSearchFacet, jsonSerialize, ai_NumericRangeSearchFacet_none, ZEND_ACC_PUBLIC)
    PHP_ME(NumericRangeSearchFacet, addRange, ai_NumericRangeSearchFacet_addRange, ZEND_ACC_PUBLIC)
    PHP_FE_END
};
// clang-format on

void pcbc_numeric_range_search_facet_init(zval *return_value, char *field, int field_len, int limit TSRMLS_DC)
{
    pcbc_numeric_range_search_facet_t *obj;

    object_init_ex(return_value, pcbc_numeric_range_search_facet_ce);
    obj = Z_NUMERIC_RANGE_SEARCH_FACET_OBJ_P(return_value);
    obj->field = estrndup(field, field_len);
    obj->limit = limit;

    ZVAL_UNDEF(&obj->ranges);
    array_init(&obj->ranges);
}

zend_object_handlers numeric_search_facet_handlers;

static void numeric_search_facet_free_object(zend_object *object TSRMLS_DC) /* {{{ */
{
    pcbc_numeric_range_search_facet_t *obj = Z_NUMERIC_RANGE_SEARCH_FACET_OBJ(object);

    if (obj->field != NULL) {
        efree(obj->field);
    }
    zval_ptr_dtor(&obj->ranges);

    zend_object_std_dtor(&obj->std TSRMLS_CC);
} /* }}} */

static zend_object *numeric_search_facet_create_object(zend_class_entry *class_type TSRMLS_DC)
{
    pcbc_numeric_range_search_facet_t *obj = NULL;

    obj = PCBC_ALLOC_OBJECT_T(pcbc_numeric_range_search_facet_t, class_type);

    zend_object_std_init(&obj->std, class_type TSRMLS_CC);
    object_properties_init(&obj->std, class_type);

    obj->std.handlers = &numeric_search_facet_handlers;
    return &obj->std;
}

static HashTable *pcbc_numeric_range_search_facet_get_debug_info(zval *object, int *is_temp TSRMLS_DC) /* {{{ */
{
    pcbc_numeric_range_search_facet_t *obj = NULL;
    zval retval;

    *is_temp = 1;
    obj = Z_NUMERIC_RANGE_SEARCH_FACET_OBJ_P(object);

    array_init(&retval);
    ADD_ASSOC_STRING(&retval, "field", obj->field);
    ADD_ASSOC_LONG_EX(&retval, "limit", obj->limit);
    ADD_ASSOC_ZVAL_EX(&retval, "numeric_ranges", &obj->ranges);
    PCBC_ADDREF_P(&obj->ranges);
    return Z_ARRVAL(retval);
} /* }}} */

PHP_MINIT_FUNCTION(NumericRangeSearchFacet)
{
    zend_class_entry ce;

    INIT_NS_CLASS_ENTRY(ce, "Couchbase", "NumericRangeSearchFacet", numeric_search_facet_methods);
    pcbc_numeric_range_search_facet_ce = zend_register_internal_class(&ce TSRMLS_CC);
    pcbc_numeric_range_search_facet_ce->create_object = numeric_search_facet_create_object;
    PCBC_CE_DISABLE_SERIALIZATION(pcbc_numeric_range_search_facet_ce);

    zend_class_implements(pcbc_numeric_range_search_facet_ce TSRMLS_CC, 1, pcbc_json_serializable_ce);
    zend_class_implements(pcbc_numeric_range_search_facet_ce TSRMLS_CC, 1, pcbc_search_facet_ce);

    memcpy(&numeric_search_facet_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    numeric_search_facet_handlers.get_debug_info = pcbc_numeric_range_search_facet_get_debug_info;
    numeric_search_facet_handlers.free_obj = numeric_search_facet_free_object;
    numeric_search_facet_handlers.offset = XtOffsetOf(pcbc_numeric_range_search_facet_t, std);

     
    return SUCCESS;
}
