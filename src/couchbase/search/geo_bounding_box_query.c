/**
 *     Copyright 2017 Couchbase, Inc.
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
    double top_left_longitude;
    double top_left_latitude;
    double bottom_right_longitude;
    double bottom_right_latitude;
    PCBC_ZEND_OBJECT_POST
} pcbc_geo_bounding_box_search_query_t;

#if PHP_VERSION_ID >= 70000
static inline pcbc_geo_bounding_box_search_query_t *pcbc_geo_bounding_box_search_query_fetch_object(zend_object *obj)
{
    return (pcbc_geo_bounding_box_search_query_t *)((char *)obj -
                                                    XtOffsetOf(pcbc_geo_bounding_box_search_query_t, std));
}
#define Z_GEO_BOUNDING_BOX_SEARCH_QUERY_OBJ(zo) (pcbc_geo_bounding_box_search_query_fetch_object(zo))
#define Z_GEO_BOUNDING_BOX_SEARCH_QUERY_OBJ_P(zv) (pcbc_geo_bounding_box_search_query_fetch_object(Z_OBJ_P(zv)))
#else
#define Z_GEO_BOUNDING_BOX_SEARCH_QUERY_OBJ(zo) ((pcbc_geo_bounding_box_search_query_t *)zo)
#define Z_GEO_BOUNDING_BOX_SEARCH_QUERY_OBJ_P(zv)                                                                      \
    ((pcbc_geo_bounding_box_search_query_t *)zend_object_store_get_object(zv TSRMLS_CC))
#endif

#define LOGARGS(lvl) LCB_LOG_##lvl, NULL, "pcbc/geo_bounding_box_search_query", __FILE__, __LINE__

zend_class_entry *pcbc_geo_bounding_box_search_query_ce;

/* {{{ proto void GeoBoundingBoxSearchQuery::__construct() */
PHP_METHOD(GeoBoundingBoxSearchQuery, __construct)
{
    throw_pcbc_exception("Accessing private constructor.", LCB_EINVAL);
}
/* }}} */

/* {{{ proto \Couchbase\GeoBoundingBoxSearchQuery GeoBoundingBoxSearchQuery::field(string $field)
 */
PHP_METHOD(GeoBoundingBoxSearchQuery, field)
{
    pcbc_geo_bounding_box_search_query_t *obj;
    char *field = NULL;
    int rv;
    pcbc_str_arg_size field_len;

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &field, &field_len);
    if (rv == FAILURE) {
        RETURN_NULL();
    }

    obj = Z_GEO_BOUNDING_BOX_SEARCH_QUERY_OBJ_P(getThis());
    if (obj->field) {
        efree(obj->field);
    }
    obj->field = estrndup(field, field_len);

    RETURN_ZVAL(getThis(), 1, 0);
} /* }}} */

/* {{{ proto \Couchbase\GeoBoundingBoxSearchQuery GeoBoundingBoxSearchQuery::boost(string $boost)
 */
PHP_METHOD(GeoBoundingBoxSearchQuery, boost)
{
    pcbc_geo_bounding_box_search_query_t *obj;
    double boost = 0;
    int rv;

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "d", &boost);
    if (rv == FAILURE) {
        RETURN_NULL();
    }

    obj = Z_GEO_BOUNDING_BOX_SEARCH_QUERY_OBJ_P(getThis());
    obj->boost = boost;

    RETURN_ZVAL(getThis(), 1, 0);
} /* }}} */

/* {{{ proto array GeoBoundingBoxSearchQuery::jsonSerialize()
 */
PHP_METHOD(GeoBoundingBoxSearchQuery, jsonSerialize)
{
    pcbc_geo_bounding_box_search_query_t *obj;
    int rv;
    PCBC_ZVAL top_left;
    PCBC_ZVAL bottom_right;

    rv = zend_parse_parameters_none();
    if (rv == FAILURE) {
        RETURN_NULL();
    }

    obj = Z_GEO_BOUNDING_BOX_SEARCH_QUERY_OBJ_P(getThis());
    array_init(return_value);

    PCBC_ZVAL_ALLOC(top_left);
    array_init(PCBC_P(top_left));
    add_next_index_double(PCBC_P(top_left), obj->top_left_longitude);
    add_next_index_double(PCBC_P(top_left), obj->top_left_latitude);
    ADD_ASSOC_ZVAL_EX(return_value, "top_left", PCBC_P(top_left));

    PCBC_ZVAL_ALLOC(bottom_right);
    array_init(PCBC_P(bottom_right));
    add_next_index_double(PCBC_P(bottom_right), obj->bottom_right_longitude);
    add_next_index_double(PCBC_P(bottom_right), obj->bottom_right_latitude);
    ADD_ASSOC_ZVAL_EX(return_value, "bottom_right", PCBC_P(bottom_right));

    if (obj->field) {
        ADD_ASSOC_STRING(return_value, "field", obj->field);
    }
    if (obj->boost >= 0) {
        ADD_ASSOC_DOUBLE_EX(return_value, "boost", obj->boost);
    }
} /* }}} */

ZEND_BEGIN_ARG_INFO_EX(ai_GeoBoundingBoxSearchQuery_none, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_GeoBoundingBoxSearchQuery_field, 0, 0, 1)
ZEND_ARG_INFO(0, field)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_GeoBoundingBoxSearchQuery_boost, 0, 0, 1)
ZEND_ARG_INFO(0, boost)
ZEND_END_ARG_INFO()

// clang-format off
zend_function_entry geo_bounding_box_search_query_methods[] = {
    PHP_ME(GeoBoundingBoxSearchQuery, __construct, ai_GeoBoundingBoxSearchQuery_none, ZEND_ACC_PRIVATE | ZEND_ACC_FINAL | ZEND_ACC_CTOR)
    PHP_ME(GeoBoundingBoxSearchQuery, jsonSerialize, ai_GeoBoundingBoxSearchQuery_none, ZEND_ACC_PUBLIC)
    PHP_ME(GeoBoundingBoxSearchQuery, boost, ai_GeoBoundingBoxSearchQuery_boost, ZEND_ACC_PUBLIC)
    PHP_ME(GeoBoundingBoxSearchQuery, field, ai_GeoBoundingBoxSearchQuery_field, ZEND_ACC_PUBLIC)
    PHP_FE_END
};
// clang-format on

void pcbc_geo_bounding_box_search_query_init(zval *return_value, double top_left_longitude, double top_left_latitude,
                                             double bottom_right_longitude, double bottom_right_latitude TSRMLS_DC)
{
    pcbc_geo_bounding_box_search_query_t *obj;

    object_init_ex(return_value, pcbc_geo_bounding_box_search_query_ce);
    obj = Z_GEO_BOUNDING_BOX_SEARCH_QUERY_OBJ_P(return_value);
    obj->boost = -1;
    obj->field = NULL;
    obj->top_left_longitude = top_left_longitude;
    obj->top_left_latitude = top_left_latitude;
    obj->bottom_right_longitude = bottom_right_longitude;
    obj->bottom_right_latitude = bottom_right_latitude;
}

zend_object_handlers geo_bounding_box_search_query_handlers;

static void geo_bounding_box_search_query_free_object(pcbc_free_object_arg *object TSRMLS_DC) /* {{{ */
{
    pcbc_geo_bounding_box_search_query_t *obj = Z_GEO_BOUNDING_BOX_SEARCH_QUERY_OBJ(object);

    if (obj->field != NULL) {
        efree(obj->field);
    }

    zend_object_std_dtor(&obj->std TSRMLS_CC);
#if PHP_VERSION_ID < 70000
    efree(obj);
#endif
} /* }}} */

static pcbc_create_object_retval geo_bounding_box_search_query_create_object(zend_class_entry *class_type TSRMLS_DC)
{
    pcbc_geo_bounding_box_search_query_t *obj = NULL;

    obj = PCBC_ALLOC_OBJECT_T(pcbc_geo_bounding_box_search_query_t, class_type);

    zend_object_std_init(&obj->std, class_type TSRMLS_CC);
    object_properties_init(&obj->std, class_type);

#if PHP_VERSION_ID >= 70000
    obj->std.handlers = &geo_bounding_box_search_query_handlers;
    return &obj->std;
#else
    {
        zend_object_value ret;
        ret.handle = zend_objects_store_put(obj, (zend_objects_store_dtor_t)zend_objects_destroy_object,
                                            geo_bounding_box_search_query_free_object, NULL TSRMLS_CC);
        ret.handlers = &geo_bounding_box_search_query_handlers;
        return ret;
    }
#endif
}

static HashTable *pcbc_geo_bounding_box_search_query_get_debug_info(zval *object, int *is_temp TSRMLS_DC) /* {{{ */
{
    pcbc_geo_bounding_box_search_query_t *obj = NULL;
#if PHP_VERSION_ID >= 70000
    zval retval;
#else
    zval retval = zval_used_for_init;
#endif

    *is_temp = 1;
    obj = Z_GEO_BOUNDING_BOX_SEARCH_QUERY_OBJ_P(object);

    array_init(&retval);
    ADD_ASSOC_DOUBLE_EX(&retval, "topLeftLongitude", obj->top_left_longitude);
    ADD_ASSOC_DOUBLE_EX(&retval, "topLeftLatitude", obj->top_left_latitude);
    ADD_ASSOC_DOUBLE_EX(&retval, "bottomRightLongitude", obj->bottom_right_longitude);
    ADD_ASSOC_DOUBLE_EX(&retval, "bottomRightLatitude", obj->bottom_right_latitude);
    if (obj->field) {
        ADD_ASSOC_STRING(&retval, "field", obj->field);
    }
    if (obj->boost >= 0) {
        ADD_ASSOC_DOUBLE_EX(&retval, "boost", obj->boost);
    }
    return Z_ARRVAL(retval);
} /* }}} */

PHP_MINIT_FUNCTION(GeoBoundingBoxSearchQuery)
{
    zend_class_entry ce;

    INIT_NS_CLASS_ENTRY(ce, "Couchbase", "GeoBoundingBoxSearchQuery", geo_bounding_box_search_query_methods);
    pcbc_geo_bounding_box_search_query_ce = zend_register_internal_class(&ce TSRMLS_CC);
    pcbc_geo_bounding_box_search_query_ce->create_object = geo_bounding_box_search_query_create_object;
    PCBC_CE_DISABLE_SERIALIZATION(pcbc_geo_bounding_box_search_query_ce);

    zend_class_implements(pcbc_geo_bounding_box_search_query_ce TSRMLS_CC, 1, pcbc_json_serializable_ce);
    zend_class_implements(pcbc_geo_bounding_box_search_query_ce TSRMLS_CC, 1, pcbc_search_query_part_ce);

    memcpy(&geo_bounding_box_search_query_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    geo_bounding_box_search_query_handlers.get_debug_info = pcbc_geo_bounding_box_search_query_get_debug_info;
#if PHP_VERSION_ID >= 70000
    geo_bounding_box_search_query_handlers.free_obj = geo_bounding_box_search_query_free_object;
    geo_bounding_box_search_query_handlers.offset = XtOffsetOf(pcbc_geo_bounding_box_search_query_t, std);
#endif

    zend_register_class_alias("\\CouchbaseGeoBoundingBoxSearchQuery", pcbc_geo_bounding_box_search_query_ce);
    return SUCCESS;
}
