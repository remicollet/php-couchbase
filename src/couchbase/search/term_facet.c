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
 * A facet that gives the number of occurrences of the most recurring terms in all hits.
 */
#include "couchbase.h"

typedef struct {
    PCBC_ZEND_OBJECT_PRE
    double boost;
    char *field;
    int limit;
    PCBC_ZEND_OBJECT_POST
} pcbc_term_search_facet_t;

#if PHP_VERSION_ID >= 70000
static inline pcbc_term_search_facet_t *pcbc_term_search_facet_fetch_object(zend_object *obj)
{
    return (pcbc_term_search_facet_t *)((char *)obj - XtOffsetOf(pcbc_term_search_facet_t, std));
}
#define Z_TERM_SEARCH_FACET_OBJ(zo) (pcbc_term_search_facet_fetch_object(zo))
#define Z_TERM_SEARCH_FACET_OBJ_P(zv) (pcbc_term_search_facet_fetch_object(Z_OBJ_P(zv)))
#else
#define Z_TERM_SEARCH_FACET_OBJ(zo) ((pcbc_term_search_facet_t *)zo)
#define Z_TERM_SEARCH_FACET_OBJ_P(zv) ((pcbc_term_search_facet_t *)zend_object_store_get_object(zv TSRMLS_CC))
#endif

#define LOGARGS(lvl) LCB_LOG_##lvl, NULL, "pcbc/term_search_facet", __FILE__, __LINE__

zend_class_entry *pcbc_term_search_facet_ce;

extern PHP_JSON_API zend_class_entry *php_json_serializable_ce;

/* {{{ proto void TermSearchFacet::__construct() */
PHP_METHOD(TermSearchFacet, __construct)
{
    throw_pcbc_exception("Accessing private constructor.", LCB_EINVAL);
}
/* }}} */

/* {{{ proto array TermSearchFacet::jsonSerialize()
 */
PHP_METHOD(TermSearchFacet, jsonSerialize)
{
    pcbc_term_search_facet_t *obj;
    int rv;

    rv = zend_parse_parameters_none();
    if (rv == FAILURE) {
        RETURN_NULL();
    }

    obj = Z_TERM_SEARCH_FACET_OBJ_P(getThis());
    array_init(return_value);
    ADD_ASSOC_STRING(return_value, "field", obj->field);
    ADD_ASSOC_LONG_EX(return_value, "size", obj->limit);
} /* }}} */

ZEND_BEGIN_ARG_INFO_EX(ai_TermSearchFacet_none, 0, 0, 0)
ZEND_END_ARG_INFO()

// clang-format off
zend_function_entry term_search_facet_methods[] = {
    PHP_ME(TermSearchFacet, __construct, ai_TermSearchFacet_none, ZEND_ACC_PRIVATE | ZEND_ACC_FINAL | ZEND_ACC_CTOR)
    PHP_ME(TermSearchFacet, jsonSerialize, ai_TermSearchFacet_none, ZEND_ACC_PUBLIC | ZEND_ACC_FINAL)
    PHP_FE_END
};
// clang-format on

void pcbc_term_search_facet_init(zval *return_value, char *field, int field_len, int limit TSRMLS_DC)
{
    pcbc_term_search_facet_t *obj;

    object_init_ex(return_value, pcbc_term_search_facet_ce);
    obj = Z_TERM_SEARCH_FACET_OBJ_P(return_value);
    obj->field = estrndup(field, field_len);
    obj->limit = limit;
}

zend_object_handlers term_search_facet_handlers;

static void term_search_facet_free_object(pcbc_free_object_arg *object TSRMLS_DC) /* {{{ */
{
    pcbc_term_search_facet_t *obj = Z_TERM_SEARCH_FACET_OBJ(object);

    if (obj->field != NULL) {
        efree(obj->field);
    }

    zend_object_std_dtor(&obj->std TSRMLS_CC);
#if PHP_VERSION_ID < 70000
    efree(obj);
#endif
} /* }}} */

static pcbc_create_object_retval term_search_facet_create_object(zend_class_entry *class_type TSRMLS_DC)
{
    pcbc_term_search_facet_t *obj = NULL;

    obj = PCBC_ALLOC_OBJECT_T(pcbc_term_search_facet_t, class_type);

    zend_object_std_init(&obj->std, class_type TSRMLS_CC);
    object_properties_init(&obj->std, class_type);

#if PHP_VERSION_ID >= 70000
    obj->std.handlers = &term_search_facet_handlers;
    return &obj->std;
#else
    {
        zend_object_value ret;
        ret.handle = zend_objects_store_put(obj, (zend_objects_store_dtor_t)zend_objects_destroy_object,
                                            term_search_facet_free_object, NULL TSRMLS_CC);
        ret.handlers = &term_search_facet_handlers;
        return ret;
    }
#endif
}

static HashTable *pcbc_term_search_facet_get_debug_info(zval *object, int *is_temp TSRMLS_DC) /* {{{ */
{
    pcbc_term_search_facet_t *obj = NULL;
#if PHP_VERSION_ID >= 70000
    zval retval;
#else
    zval retval = zval_used_for_init;
#endif

    *is_temp = 1;
    obj = Z_TERM_SEARCH_FACET_OBJ_P(object);

    array_init(&retval);
    ADD_ASSOC_STRING(&retval, "field", obj->field);
    ADD_ASSOC_LONG_EX(&retval, "limit", obj->limit);
    return Z_ARRVAL(retval);
} /* }}} */

PHP_MINIT_FUNCTION(TermSearchFacet)
{
    zend_class_entry ce;

    INIT_NS_CLASS_ENTRY(ce, "Couchbase", "TermSearchFacet", term_search_facet_methods);
    pcbc_term_search_facet_ce = zend_register_internal_class(&ce TSRMLS_CC);
    pcbc_term_search_facet_ce->create_object = term_search_facet_create_object;
    PCBC_CE_FLAGS_FINAL(pcbc_term_search_facet_ce);
    PCBC_CE_DISABLE_SERIALIZATION(pcbc_term_search_facet_ce);

    zend_class_implements(pcbc_term_search_facet_ce TSRMLS_CC, 1, php_json_serializable_ce);
    zend_class_implements(pcbc_term_search_facet_ce TSRMLS_CC, 1, pcbc_search_facet_ce);

    memcpy(&term_search_facet_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    term_search_facet_handlers.get_debug_info = pcbc_term_search_facet_get_debug_info;
#if PHP_VERSION_ID >= 70000
    term_search_facet_handlers.free_obj = term_search_facet_free_object;
    term_search_facet_handlers.offset = XtOffsetOf(pcbc_term_search_facet_t, std);
#endif

    zend_register_class_alias("\\CouchbaseTermSearchFacet", pcbc_term_search_facet_ce);
    return SUCCESS;
}
