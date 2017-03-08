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
 * A facet that categorizes hits into numerical ranges (or buckets) provided by the user.
 */
#include "couchbase.h"

typedef struct {
    PCBC_ZEND_OBJECT_PRE
    double boost;
    char *field;
    int limit;
    PCBC_ZVAL ranges; /* array or arrays like ["range-name" => ["min" => ..., "max" => ...], ...] */
    PCBC_ZEND_OBJECT_POST
} pcbc_numeric_range_search_facet_t;

#if PHP_VERSION_ID >= 70000
static inline pcbc_numeric_range_search_facet_t *pcbc_numeric_range_search_facet_fetch_object(zend_object *obj)
{
    return (pcbc_numeric_range_search_facet_t *)((char *)obj - XtOffsetOf(pcbc_numeric_range_search_facet_t, std));
}
#define Z_NUMERIC_RANGE_SEARCH_FACET_OBJ(zo) (pcbc_numeric_range_search_facet_fetch_object(zo))
#define Z_NUMERIC_RANGE_SEARCH_FACET_OBJ_P(zv) (pcbc_numeric_range_search_facet_fetch_object(Z_OBJ_P(zv)))
#else
#define Z_NUMERIC_RANGE_SEARCH_FACET_OBJ(zo) ((pcbc_numeric_range_search_facet_t *)zo)
#define Z_NUMERIC_RANGE_SEARCH_FACET_OBJ_P(zv)                                                                         \
    ((pcbc_numeric_range_search_facet_t *)zend_object_store_get_object(zv TSRMLS_CC))
#endif

#define LOGARGS(lvl) LCB_LOG_##lvl, NULL, "pcbc/numeric_search_facet", __FILE__, __LINE__

zend_class_entry *pcbc_numeric_range_search_facet_ce;

extern PHP_JSON_API zend_class_entry *php_json_serializable_ce;

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
    double min = 0, max = 0;
    zend_bool min_null = 0, max_null = 0;
    char *name = NULL;
    PCBC_ZVAL range;
    int rv;
    pcbc_str_arg_size name_len = 0;

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sd!d!", &name, &name_len, &min, &min_null, &max, &max_null);
    if (rv == FAILURE) {
        RETURN_NULL();
    }

    obj = Z_NUMERIC_RANGE_SEARCH_FACET_OBJ_P(getThis());

    PCBC_ZVAL_ALLOC(range);
    array_init_size(PCBC_P(range), 3);
    ADD_ASSOC_STRINGL(PCBC_P(range), "name", name, name_len);
    if (!min_null) {
        ADD_ASSOC_DOUBLE_EX(PCBC_P(range), "min", min);
    }
    if (!max_null) {
        ADD_ASSOC_DOUBLE_EX(PCBC_P(range), "max", max);
    }
    PCBC_ADDREF_P(PCBC_P(range));
    add_next_index_zval(PCBC_P(obj->ranges), PCBC_P(range));

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
    ADD_ASSOC_ZVAL_EX(return_value, "numeric_ranges", PCBC_P(obj->ranges));
    PCBC_ADDREF_P(PCBC_P(obj->ranges));
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
    PHP_ME(NumericRangeSearchFacet, jsonSerialize, ai_NumericRangeSearchFacet_none, ZEND_ACC_PUBLIC | ZEND_ACC_FINAL)
    PHP_ME(NumericRangeSearchFacet, addRange, ai_NumericRangeSearchFacet_addRange, ZEND_ACC_PUBLIC | ZEND_ACC_FINAL)
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

    PCBC_ZVAL_ALLOC(obj->ranges);
    array_init(PCBC_P(obj->ranges));
    PCBC_ADDREF_P(PCBC_P(obj->ranges));
    zval_ptr_dtor(&obj->ranges);
}

zend_object_handlers numeric_search_facet_handlers;

static void numeric_search_facet_free_object(pcbc_free_object_arg *object TSRMLS_DC) /* {{{ */
{
    pcbc_numeric_range_search_facet_t *obj = Z_NUMERIC_RANGE_SEARCH_FACET_OBJ(object);

    if (obj->field != NULL) {
        efree(obj->field);
    }

    zend_object_std_dtor(&obj->std TSRMLS_CC);
#if PHP_VERSION_ID < 70000
    efree(obj);
#endif
} /* }}} */

static pcbc_create_object_retval numeric_search_facet_create_object(zend_class_entry *class_type TSRMLS_DC)
{
    pcbc_numeric_range_search_facet_t *obj = NULL;

    obj = PCBC_ALLOC_OBJECT_T(pcbc_numeric_range_search_facet_t, class_type);

    zend_object_std_init(&obj->std, class_type TSRMLS_CC);
    object_properties_init(&obj->std, class_type);

#if PHP_VERSION_ID >= 70000
    obj->std.handlers = &numeric_search_facet_handlers;
    return &obj->std;
#else
    {
        zend_object_value ret;
        ret.handle = zend_objects_store_put(obj, (zend_objects_store_dtor_t)zend_objects_destroy_object,
                                            numeric_search_facet_free_object, NULL TSRMLS_CC);
        ret.handlers = &numeric_search_facet_handlers;
        return ret;
    }
#endif
}

static HashTable *pcbc_numeric_range_search_facet_get_debug_info(zval *object, int *is_temp TSRMLS_DC) /* {{{ */
{
    pcbc_numeric_range_search_facet_t *obj = NULL;
#if PHP_VERSION_ID >= 70000
    zval retval;
#else
    zval retval = zval_used_for_init;
#endif

    *is_temp = 1;
    obj = Z_NUMERIC_RANGE_SEARCH_FACET_OBJ_P(object);

    array_init(&retval);
    ADD_ASSOC_STRING(&retval, "field", obj->field);
    ADD_ASSOC_LONG_EX(&retval, "limit", obj->limit);
    ADD_ASSOC_ZVAL_EX(&retval, "numeric_ranges", PCBC_P(obj->ranges));
    PCBC_ADDREF_P(PCBC_P(obj->ranges));
    return Z_ARRVAL(retval);
} /* }}} */

PHP_MINIT_FUNCTION(NumericRangeSearchFacet)
{
    zend_class_entry ce;

    INIT_NS_CLASS_ENTRY(ce, "Couchbase", "NumericRangeSearchFacet", numeric_search_facet_methods);
    pcbc_numeric_range_search_facet_ce = zend_register_internal_class(&ce TSRMLS_CC);
    pcbc_numeric_range_search_facet_ce->create_object = numeric_search_facet_create_object;
    PCBC_CE_FLAGS_FINAL(pcbc_numeric_range_search_facet_ce);
    PCBC_CE_DISABLE_SERIALIZATION(pcbc_numeric_range_search_facet_ce);

    zend_class_implements(pcbc_numeric_range_search_facet_ce TSRMLS_CC, 1, php_json_serializable_ce);
    zend_class_implements(pcbc_numeric_range_search_facet_ce TSRMLS_CC, 1, pcbc_search_facet_ce);

    memcpy(&numeric_search_facet_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    numeric_search_facet_handlers.get_debug_info = pcbc_numeric_range_search_facet_get_debug_info;
#if PHP_VERSION_ID >= 70000
    numeric_search_facet_handlers.free_obj = numeric_search_facet_free_object;
    numeric_search_facet_handlers.offset = XtOffsetOf(pcbc_numeric_range_search_facet_t, std);
#endif

    zend_register_class_alias("\\CouchbaseNumericRangeSearchFacet", pcbc_numeric_range_search_facet_ce);
    return SUCCESS;
}
