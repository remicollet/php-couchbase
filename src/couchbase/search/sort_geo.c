/**
 *     Copyright 2018 Couchbase, Inc.
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
 * Sort by a location and unit in the hits.
 */
#include "couchbase.h"

typedef struct {
    PCBC_ZEND_OBJECT_PRE
    zend_bool descending;
    char *field;
    double lon;
    double lat;
    char *unit;
    PCBC_ZEND_OBJECT_POST
} pcbc_search_sort_geo_distance_t;

#if PHP_VERSION_ID >= 70000
static inline pcbc_search_sort_geo_distance_t *pcbc_search_sort_geo_distance_fetch_object(zend_object *obj)
{
    return (pcbc_search_sort_geo_distance_t *)((char *)obj - XtOffsetOf(pcbc_search_sort_geo_distance_t, std));
}
#define Z_SEARCH_SORT_GEO_DISTANCE_OBJ(zo) (pcbc_search_sort_geo_distance_fetch_object(zo))
#define Z_SEARCH_SORT_GEO_DISTANCE_OBJ_P(zv) (pcbc_search_sort_geo_distance_fetch_object(Z_OBJ_P(zv)))
#else
#define Z_SEARCH_SORT_GEO_DISTANCE_OBJ(zo) ((pcbc_search_sort_geo_distance_t *)zo)
#define Z_SEARCH_SORT_GEO_DISTANCE_OBJ_P(zv)                                                                           \
    ((pcbc_search_sort_geo_distance_t *)zend_object_store_get_object(zv TSRMLS_CC))
#endif

#define LOGARGS(lvl) LCB_LOG_##lvl, NULL, "pcbc/search_sort_geo_distance", __FILE__, __LINE__

zend_class_entry *pcbc_search_sort_geo_distance_ce;

/* {{{ proto void SearchSortGeoDistance::__construct() */
PHP_METHOD(SearchSortGeoDistance, __construct)
{
    throw_pcbc_exception("Accessing private constructor.", LCB_EINVAL);
}
/* }}} */

/* {{{ proto \Couchbase\SearchSortGeoDistance SearchSortGeoDistance::descending(boolean $val)
 */
PHP_METHOD(SearchSortGeoDistance, descending)
{
    pcbc_search_sort_geo_distance_t *obj;
    zend_bool descending = 0;
    int rv;

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "b", &descending);
    if (rv == FAILURE) {
        RETURN_NULL();
    }

    obj = Z_SEARCH_SORT_GEO_DISTANCE_OBJ_P(getThis());
    obj->descending = descending;

    RETURN_ZVAL(getThis(), 1, 0);
} /* }}} */

/* {{{ proto \Couchbase\SearchSortGeoDistance SearchSortGeoDistance::unit(string $unit)
 */
PHP_METHOD(SearchSortGeoDistance, unit)
{
    pcbc_search_sort_geo_distance_t *obj;
    char *unit = NULL;
    pcbc_str_arg_size unit_len;
    int rv;

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &unit, &unit_len);
    if (rv == FAILURE) {
        return;
    }

    obj = Z_SEARCH_SORT_GEO_DISTANCE_OBJ_P(getThis());
    if (obj->unit) {
        efree(obj->unit);
        obj->unit = NULL;
    }
    if (unit) {
        obj->unit = estrndup(unit, unit_len);
    }

    RETURN_ZVAL(getThis(), 1, 0);
} /* }}} */

/* {{{ proto array SearchSortGeoDistance::jsonSerialize()
 */
PHP_METHOD(SearchSortGeoDistance, jsonSerialize)
{
    pcbc_search_sort_geo_distance_t *obj;
    int rv;

    rv = zend_parse_parameters_none();
    if (rv == FAILURE) {
        RETURN_NULL();
    }

    obj = Z_SEARCH_SORT_GEO_DISTANCE_OBJ_P(getThis());
    array_init(return_value);
    ADD_ASSOC_STRING(return_value, "by", "geo_distance");
    ADD_ASSOC_BOOL_EX(return_value, "descending", obj->descending);
    ADD_ASSOC_STRING(return_value, "field", obj->field);
    {
        PCBC_ZVAL location;

        PCBC_ZVAL_ALLOC(location);
        array_init_size(PCBC_P(location), 2);
        ADD_ASSOC_ZVAL_EX(return_value, "location", PCBC_P(location));
        add_next_index_double(PCBC_P(location), obj->lon);
        add_next_index_double(PCBC_P(location), obj->lat);
    }
    if (obj->unit != NULL) {
        ADD_ASSOC_STRING(return_value, "unit", obj->unit);
    }
} /* }}} */

ZEND_BEGIN_ARG_INFO_EX(ai_SearchSortGeoDistance_none, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_SearchSortGeoDistance_descending, 0, 0, 1)
ZEND_ARG_INFO(0, descending)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_SearchSortGeoDistance_unit, 0, 0, 1)
ZEND_ARG_INFO(0, missing)
ZEND_END_ARG_INFO()

// clang-format off
zend_function_entry search_sort_geo_distance_methods[] = {
    PHP_ME(SearchSortGeoDistance, __construct, ai_SearchSortGeoDistance_none, ZEND_ACC_PRIVATE | ZEND_ACC_FINAL | ZEND_ACC_CTOR)
    PHP_ME(SearchSortGeoDistance, jsonSerialize, ai_SearchSortGeoDistance_none, ZEND_ACC_PUBLIC)
    PHP_ME(SearchSortGeoDistance, descending, ai_SearchSortGeoDistance_descending, ZEND_ACC_PUBLIC)
    PHP_ME(SearchSortGeoDistance, unit, ai_SearchSortGeoDistance_unit, ZEND_ACC_PUBLIC)
    PHP_FE_END
};
// clang-format on

void pcbc_search_sort_geo_distance_init(zval *return_value, const char *field, int field_len, double lon,
                                        double lat TSRMLS_DC)
{
    pcbc_search_sort_geo_distance_t *obj;

    object_init_ex(return_value, pcbc_search_sort_geo_distance_ce);
    obj = Z_SEARCH_SORT_GEO_DISTANCE_OBJ_P(return_value);
    obj->descending = 0;
    obj->field = estrndup(field, field_len);
    obj->lon = lon;
    obj->lat = lat;
    obj->unit = NULL;
}

zend_object_handlers search_sort_geo_distance_handlers;

static void search_sort_geo_distance_free_object(pcbc_free_object_arg *object TSRMLS_DC) /* {{{ */
{
    pcbc_search_sort_geo_distance_t *obj = Z_SEARCH_SORT_GEO_DISTANCE_OBJ(object);

    if (obj->field) {
        efree(obj->field);
    }
    if (obj->unit) {
        efree(obj->unit);
    }
    zend_object_std_dtor(&obj->std TSRMLS_CC);
#if PHP_VERSION_ID < 70000
    efree(obj);
#endif
} /* }}} */

static pcbc_create_object_retval search_sort_geo_distance_create_object(zend_class_entry *class_type TSRMLS_DC)
{
    pcbc_search_sort_geo_distance_t *obj = NULL;

    obj = PCBC_ALLOC_OBJECT_T(pcbc_search_sort_geo_distance_t, class_type);

    zend_object_std_init(&obj->std, class_type TSRMLS_CC);
    object_properties_init(&obj->std, class_type);

#if PHP_VERSION_ID >= 70000
    obj->std.handlers = &search_sort_geo_distance_handlers;
    return &obj->std;
#else
    {
        zend_object_value ret;
        ret.handle = zend_objects_store_put(obj, (zend_objects_store_dtor_t)zend_objects_destroy_object,
                                            search_sort_geo_distance_free_object, NULL TSRMLS_CC);
        ret.handlers = &search_sort_geo_distance_handlers;
        return ret;
    }
#endif
}

static HashTable *pcbc_search_sort_geo_distance_get_debug_info(zval *object, int *is_temp TSRMLS_DC) /* {{{ */
{
    pcbc_search_sort_geo_distance_t *obj = NULL;
#if PHP_VERSION_ID >= 70000
    zval retval;
#else
    zval retval = zval_used_for_init;
#endif

    *is_temp = 1;
    obj = Z_SEARCH_SORT_GEO_DISTANCE_OBJ_P(object);

    array_init(&retval);
    ADD_ASSOC_STRING(&retval, "by", "field");
    ADD_ASSOC_BOOL_EX(&retval, "descending", obj->descending);
    ADD_ASSOC_STRING(&retval, "field", obj->field);
    ADD_ASSOC_DOUBLE_EX(&retval, "lat", obj->lat);
    ADD_ASSOC_DOUBLE_EX(&retval, "lon", obj->lon);
    if (obj->unit != NULL) {
        ADD_ASSOC_STRING(&retval, "unit", obj->unit);
    }
    return Z_ARRVAL(retval);
} /* }}} */

PHP_MINIT_FUNCTION(SearchSortGeoDistance)
{
    zend_class_entry ce;

    INIT_NS_CLASS_ENTRY(ce, "Couchbase", "SearchSortGeoDistance", search_sort_geo_distance_methods);
    pcbc_search_sort_geo_distance_ce = zend_register_internal_class_ex(&ce, pcbc_search_sort_ce
#if PHP_VERSION_ID < 70000
                                                                       ,
                                                                       NULL
#endif
                                                                           TSRMLS_CC);
    pcbc_search_sort_geo_distance_ce->create_object = search_sort_geo_distance_create_object;
    PCBC_CE_DISABLE_SERIALIZATION(pcbc_search_sort_geo_distance_ce);

    zend_class_implements(pcbc_search_sort_geo_distance_ce TSRMLS_CC, 1, pcbc_json_serializable_ce);

    memcpy(&search_sort_geo_distance_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    search_sort_geo_distance_handlers.get_debug_info = pcbc_search_sort_geo_distance_get_debug_info;
#if PHP_VERSION_ID >= 70000
    search_sort_geo_distance_handlers.free_obj = search_sort_geo_distance_free_object;
    search_sort_geo_distance_handlers.offset = XtOffsetOf(pcbc_search_sort_geo_distance_t, std);
#endif
    return SUCCESS;
}
