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
 * Sort by a field in the hits.
 */
#include "couchbase.h"

typedef struct {
    PCBC_ZEND_OBJECT_PRE
    zend_bool descending;
    char *field;
    char *type;
    char *mode;
    char *missing;
    PCBC_ZEND_OBJECT_POST
} pcbc_search_sort_field_t;

#if PHP_VERSION_ID >= 70000
static inline pcbc_search_sort_field_t *pcbc_search_sort_field_fetch_object(zend_object *obj)
{
    return (pcbc_search_sort_field_t *)((char *)obj - XtOffsetOf(pcbc_search_sort_field_t, std));
}
#define Z_SEARCH_SORT_FIELD_OBJ(zo) (pcbc_search_sort_field_fetch_object(zo))
#define Z_SEARCH_SORT_FIELD_OBJ_P(zv) (pcbc_search_sort_field_fetch_object(Z_OBJ_P(zv)))
#else
#define Z_SEARCH_SORT_FIELD_OBJ(zo) ((pcbc_search_sort_field_t *)zo)
#define Z_SEARCH_SORT_FIELD_OBJ_P(zv) ((pcbc_search_sort_field_t *)zend_object_store_get_object(zv TSRMLS_CC))
#endif

#define LOGARGS(lvl) LCB_LOG_##lvl, NULL, "pcbc/search_sort_field", __FILE__, __LINE__

zend_class_entry *pcbc_search_sort_field_ce;

/* {{{ proto void SearchSortField::__construct() */
PHP_METHOD(SearchSortField, __construct)
{
    throw_pcbc_exception("Accessing private constructor.", LCB_EINVAL);
}
/* }}} */

/* {{{ proto \Couchbase\SearchSortField SearchSortField::descending(boolean $val)
 */
PHP_METHOD(SearchSortField, descending)
{
    pcbc_search_sort_field_t *obj;
    zend_bool descending = 0;
    int rv;

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "b", &descending);
    if (rv == FAILURE) {
        RETURN_NULL();
    }

    obj = Z_SEARCH_SORT_FIELD_OBJ_P(getThis());
    obj->descending = descending;

    RETURN_ZVAL(getThis(), 1, 0);
} /* }}} */

/* {{{ proto \Couchbase\SearchSortField SearchSortField::type(string $type)
 */
PHP_METHOD(SearchSortField, type)
{
    pcbc_search_sort_field_t *obj;
    char *type = NULL;
    pcbc_str_arg_size type_len;
    int rv;

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &type, &type_len);
    if (rv == FAILURE) {
        return;
    }

    obj = Z_SEARCH_SORT_FIELD_OBJ_P(getThis());
    if (obj->type) {
        efree(obj->type);
        obj->type = NULL;
    }
    if (type) {
        obj->type = estrndup(type, type_len);
    }

    RETURN_ZVAL(getThis(), 1, 0);
} /* }}} */

/* {{{ proto \Couchbase\SearchSortField SearchSortField::mode(string $mode)
 */
PHP_METHOD(SearchSortField, mode)
{
    pcbc_search_sort_field_t *obj;
    char *mode = NULL;
    pcbc_str_arg_size mode_len;
    int rv;

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &mode, &mode_len);
    if (rv == FAILURE) {
        return;
    }

    obj = Z_SEARCH_SORT_FIELD_OBJ_P(getThis());
    if (obj->mode) {
        efree(obj->mode);
        obj->mode = NULL;
    }
    if (mode) {
        obj->mode = estrndup(mode, mode_len);
    }

    RETURN_ZVAL(getThis(), 1, 0);
} /* }}} */

/* {{{ proto \Couchbase\SearchSortField SearchSortField::missing(string $missing)
 */
PHP_METHOD(SearchSortField, missing)
{
    pcbc_search_sort_field_t *obj;
    char *missing = NULL;
    pcbc_str_arg_size missing_len;
    int rv;

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &missing, &missing_len);
    if (rv == FAILURE) {
        return;
    }

    obj = Z_SEARCH_SORT_FIELD_OBJ_P(getThis());
    if (obj->missing) {
        efree(obj->missing);
        obj->missing = NULL;
    }
    if (missing) {
        obj->missing = estrndup(missing, missing_len);
    }

    RETURN_ZVAL(getThis(), 1, 0);
} /* }}} */

/* {{{ proto array SearchSortField::jsonSerialize()
 */
PHP_METHOD(SearchSortField, jsonSerialize)
{
    pcbc_search_sort_field_t *obj;
    int rv;

    rv = zend_parse_parameters_none();
    if (rv == FAILURE) {
        RETURN_NULL();
    }

    obj = Z_SEARCH_SORT_FIELD_OBJ_P(getThis());
    array_init(return_value);
    ADD_ASSOC_STRING(return_value, "by", "field");
    ADD_ASSOC_BOOL_EX(return_value, "desc", obj->descending);
    ADD_ASSOC_STRING(return_value, "field", obj->field);
    if (obj->type != NULL) {
        ADD_ASSOC_STRING(return_value, "type", obj->type);
    }
    if (obj->mode != NULL) {
        ADD_ASSOC_STRING(return_value, "mode", obj->mode);
    }
    if (obj->missing != NULL) {
        ADD_ASSOC_STRING(return_value, "missing", obj->missing);
    }
} /* }}} */

ZEND_BEGIN_ARG_INFO_EX(ai_SearchSortField_none, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_SearchSortField_descending, 0, 0, 1)
ZEND_ARG_INFO(0, descending)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_SearchSortField_type, 0, 0, 1)
ZEND_ARG_INFO(0, type)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_SearchSortField_mode, 0, 0, 1)
ZEND_ARG_INFO(0, mode)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_SearchSortField_missing, 0, 0, 1)
ZEND_ARG_INFO(0, missing)
ZEND_END_ARG_INFO()

// clang-format off
zend_function_entry search_sort_field_methods[] = {
    PHP_ME(SearchSortField, __construct, ai_SearchSortField_none, ZEND_ACC_PRIVATE | ZEND_ACC_FINAL | ZEND_ACC_CTOR)
    PHP_ME(SearchSortField, jsonSerialize, ai_SearchSortField_none, ZEND_ACC_PUBLIC)
    PHP_ME(SearchSortField, descending, ai_SearchSortField_descending, ZEND_ACC_PUBLIC)
    PHP_ME(SearchSortField, type, ai_SearchSortField_type, ZEND_ACC_PUBLIC)
    PHP_ME(SearchSortField, mode, ai_SearchSortField_mode, ZEND_ACC_PUBLIC)
    PHP_ME(SearchSortField, missing, ai_SearchSortField_missing, ZEND_ACC_PUBLIC)
    PHP_FE_END
};
// clang-format on

void pcbc_search_sort_field_init(zval *return_value, const char *field, int field_len TSRMLS_DC)
{
    pcbc_search_sort_field_t *obj;

    object_init_ex(return_value, pcbc_search_sort_field_ce);
    obj = Z_SEARCH_SORT_FIELD_OBJ_P(return_value);
    obj->descending = 0;
    obj->field = estrndup(field, field_len);
    obj->type = NULL;
    obj->mode = NULL;
    obj->missing = NULL;
}

zend_object_handlers search_sort_field_handlers;

static void search_sort_field_free_object(pcbc_free_object_arg *object TSRMLS_DC) /* {{{ */
{
    pcbc_search_sort_field_t *obj = Z_SEARCH_SORT_FIELD_OBJ(object);

    if (obj->field) {
        efree(obj->field);
    }
    if (obj->type) {
        efree(obj->type);
    }
    if (obj->mode) {
        efree(obj->mode);
    }
    if (obj->missing) {
        efree(obj->missing);
    }
    zend_object_std_dtor(&obj->std TSRMLS_CC);
#if PHP_VERSION_ID < 70000
    efree(obj);
#endif
} /* }}} */

static pcbc_create_object_retval search_sort_field_create_object(zend_class_entry *class_type TSRMLS_DC)
{
    pcbc_search_sort_field_t *obj = NULL;

    obj = PCBC_ALLOC_OBJECT_T(pcbc_search_sort_field_t, class_type);

    zend_object_std_init(&obj->std, class_type TSRMLS_CC);
    object_properties_init(&obj->std, class_type);

#if PHP_VERSION_ID >= 70000
    obj->std.handlers = &search_sort_field_handlers;
    return &obj->std;
#else
    {
        zend_object_value ret;
        ret.handle = zend_objects_store_put(obj, (zend_objects_store_dtor_t)zend_objects_destroy_object,
                                            search_sort_field_free_object, NULL TSRMLS_CC);
        ret.handlers = &search_sort_field_handlers;
        return ret;
    }
#endif
}

static HashTable *pcbc_search_sort_field_get_debug_info(zval *object, int *is_temp TSRMLS_DC) /* {{{ */
{
    pcbc_search_sort_field_t *obj = NULL;
#if PHP_VERSION_ID >= 70000
    zval retval;
#else
    zval retval = zval_used_for_init;
#endif

    *is_temp = 1;
    obj = Z_SEARCH_SORT_FIELD_OBJ_P(object);

    array_init(&retval);
    ADD_ASSOC_STRING(&retval, "by", "field");
    ADD_ASSOC_BOOL_EX(&retval, "desc", obj->descending);
    ADD_ASSOC_STRING(&retval, "field", obj->field);
    if (obj->type != NULL) {
        ADD_ASSOC_STRING(&retval, "type", obj->type);
    }
    if (obj->mode != NULL) {
        ADD_ASSOC_STRING(&retval, "mode", obj->mode);
    }
    if (obj->missing != NULL) {
        ADD_ASSOC_STRING(&retval, "missing", obj->missing);
    }
    return Z_ARRVAL(retval);
} /* }}} */

PHP_MINIT_FUNCTION(SearchSortField)
{
    zend_class_entry ce;

    INIT_NS_CLASS_ENTRY(ce, "Couchbase", "SearchSortField", search_sort_field_methods);
    pcbc_search_sort_field_ce = zend_register_internal_class_ex(&ce, pcbc_search_sort_ce
#if PHP_VERSION_ID < 70000
                                                                ,
                                                                NULL
#endif
                                                                    TSRMLS_CC);
    pcbc_search_sort_field_ce->create_object = search_sort_field_create_object;
    PCBC_CE_DISABLE_SERIALIZATION(pcbc_search_sort_field_ce);

    zend_class_implements(pcbc_search_sort_field_ce TSRMLS_CC, 1, pcbc_json_serializable_ce);

    zend_declare_class_constant_stringl(pcbc_search_sort_field_ce, ZEND_STRL("TYPE_AUTO"), ZEND_STRL("auto") TSRMLS_CC);
    zend_declare_class_constant_stringl(pcbc_search_sort_field_ce, ZEND_STRL("TYPE_STRING"),
                                        ZEND_STRL("string") TSRMLS_CC);
    zend_declare_class_constant_stringl(pcbc_search_sort_field_ce, ZEND_STRL("TYPE_NUMBER"),
                                        ZEND_STRL("number") TSRMLS_CC);
    zend_declare_class_constant_stringl(pcbc_search_sort_field_ce, ZEND_STRL("TYPE_DATE"), ZEND_STRL("date") TSRMLS_CC);

    zend_declare_class_constant_stringl(pcbc_search_sort_field_ce, ZEND_STRL("MODE_DEFAULT"),
                                        ZEND_STRL("default") TSRMLS_CC);
    zend_declare_class_constant_stringl(pcbc_search_sort_field_ce, ZEND_STRL("MODE_MIN"), ZEND_STRL("min") TSRMLS_CC);
    zend_declare_class_constant_stringl(pcbc_search_sort_field_ce, ZEND_STRL("MODE_MAX"), ZEND_STRL("max") TSRMLS_CC);

    zend_declare_class_constant_stringl(pcbc_search_sort_field_ce, ZEND_STRL("MISSING_FIRST"),
                                        ZEND_STRL("first") TSRMLS_CC);
    zend_declare_class_constant_stringl(pcbc_search_sort_field_ce, ZEND_STRL("MISSING_LAST"),
                                        ZEND_STRL("last") TSRMLS_CC);

    memcpy(&search_sort_field_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    search_sort_field_handlers.get_debug_info = pcbc_search_sort_field_get_debug_info;
#if PHP_VERSION_ID >= 70000
    search_sort_field_handlers.free_obj = search_sort_field_free_object;
    search_sort_field_handlers.offset = XtOffsetOf(pcbc_search_sort_field_t, std);
#endif
    return SUCCESS;
}
