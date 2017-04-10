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
 * A FTS query that matches terms (without further analysis). Usually for debugging purposes, prefer using MatchQuery.
 */
#include "couchbase.h"

typedef struct {
    PCBC_ZEND_OBJECT_PRE
    double boost;
    char *field;
    char *term;
    int prefix_length;
    int fuzziness;
    PCBC_ZEND_OBJECT_POST
} pcbc_term_search_query_t;

#if PHP_VERSION_ID >= 70000
static inline pcbc_term_search_query_t *pcbc_term_search_query_fetch_object(zend_object *obj)
{
    return (pcbc_term_search_query_t *)((char *)obj - XtOffsetOf(pcbc_term_search_query_t, std));
}
#define Z_TERM_SEARCH_QUERY_OBJ(zo) (pcbc_term_search_query_fetch_object(zo))
#define Z_TERM_SEARCH_QUERY_OBJ_P(zv) (pcbc_term_search_query_fetch_object(Z_OBJ_P(zv)))
#else
#define Z_TERM_SEARCH_QUERY_OBJ(zo) ((pcbc_term_search_query_t *)zo)
#define Z_TERM_SEARCH_QUERY_OBJ_P(zv) ((pcbc_term_search_query_t *)zend_object_store_get_object(zv TSRMLS_CC))
#endif

#define LOGARGS(lvl) LCB_LOG_##lvl, NULL, "pcbc/term_search_query", __FILE__, __LINE__

zend_class_entry *pcbc_term_search_query_ce;

/* {{{ proto void TermSearchQuery::__construct() */
PHP_METHOD(TermSearchQuery, __construct)
{
    throw_pcbc_exception("Accessing private constructor.", LCB_EINVAL);
}
/* }}} */

/* {{{ proto \Couchbase\TermSearchQuery TermSearchQuery::field(string $field)
 */
PHP_METHOD(TermSearchQuery, field)
{
    pcbc_term_search_query_t *obj;
    char *field = NULL;
    int rv;
    pcbc_str_arg_size field_len;

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &field, &field_len);
    if (rv == FAILURE) {
        RETURN_NULL();
    }

    obj = Z_TERM_SEARCH_QUERY_OBJ_P(getThis());
    if (obj->field) {
        efree(obj->field);
    }
    obj->field = estrndup(field, field_len);

    RETURN_ZVAL(getThis(), 1, 0);
} /* }}} */

/* {{{ proto \Couchbase\TermSearchQuery TermSearchQuery::prefixLength(int $prefixLength)
 */
PHP_METHOD(TermSearchQuery, prefixLength)
{
    pcbc_term_search_query_t *obj;
    long prefix_length = 0;
    int rv;

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &prefix_length);
    if (rv == FAILURE) {
        RETURN_NULL();
    }

    obj = Z_TERM_SEARCH_QUERY_OBJ_P(getThis());
    obj->prefix_length = prefix_length;

    RETURN_ZVAL(getThis(), 1, 0);
} /* }}} */

/* {{{ proto \Couchbase\TermSearchQuery TermSearchQuery::fuzziness(int $fuzziness)
 */
PHP_METHOD(TermSearchQuery, fuzziness)
{
    pcbc_term_search_query_t *obj;
    long fuzziness = 0;
    int rv;

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &fuzziness);
    if (rv == FAILURE) {
        RETURN_NULL();
    }

    obj = Z_TERM_SEARCH_QUERY_OBJ_P(getThis());
    obj->fuzziness = fuzziness;

    RETURN_ZVAL(getThis(), 1, 0);
} /* }}} */

/* {{{ proto \Couchbase\TermSearchQuery TermSearchQuery::boost(double $boost)
 */
PHP_METHOD(TermSearchQuery, boost)
{
    pcbc_term_search_query_t *obj;
    double boost = 0;
    int rv;

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "d", &boost);
    if (rv == FAILURE) {
        RETURN_NULL();
    }

    obj = Z_TERM_SEARCH_QUERY_OBJ_P(getThis());
    obj->boost = boost;

    RETURN_ZVAL(getThis(), 1, 0);
} /* }}} */

/* {{{ proto array TermSearchQuery::jsonSerialize()
 */
PHP_METHOD(TermSearchQuery, jsonSerialize)
{
    pcbc_term_search_query_t *obj;
    int rv;

    rv = zend_parse_parameters_none();
    if (rv == FAILURE) {
        RETURN_NULL();
    }

    obj = Z_TERM_SEARCH_QUERY_OBJ_P(getThis());
    array_init(return_value);
    ADD_ASSOC_STRING(return_value, "term", obj->term);
    if (obj->field) {
        ADD_ASSOC_STRING(return_value, "field", obj->field);
    }
    if (obj->prefix_length >= 0) {
        ADD_ASSOC_LONG_EX(return_value, "prefix_length", obj->prefix_length);
    }
    if (obj->fuzziness >= 0) {
        ADD_ASSOC_LONG_EX(return_value, "fuzziness", obj->fuzziness);
    }
    if (obj->boost >= 0) {
        ADD_ASSOC_DOUBLE_EX(return_value, "boost", obj->boost);
    }
} /* }}} */

ZEND_BEGIN_ARG_INFO_EX(ai_TermSearchQuery_none, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_TermSearchQuery_field, 0, 0, 1)
ZEND_ARG_INFO(0, field)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_TermSearchQuery_prefixLength, 0, 0, 1)
ZEND_ARG_INFO(0, prefixLength)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_TermSearchQuery_fuzziness, 0, 0, 1)
ZEND_ARG_INFO(0, fuzziness)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_TermSearchQuery_boost, 0, 0, 1)
ZEND_ARG_INFO(0, boost)
ZEND_END_ARG_INFO()

// clang-format off
zend_function_entry term_search_query_methods[] = {
    PHP_ME(TermSearchQuery, __construct, ai_TermSearchQuery_none, ZEND_ACC_PRIVATE | ZEND_ACC_FINAL | ZEND_ACC_CTOR)
    PHP_ME(TermSearchQuery, jsonSerialize, ai_TermSearchQuery_none, ZEND_ACC_PUBLIC | ZEND_ACC_FINAL)
    PHP_ME(TermSearchQuery, boost, ai_TermSearchQuery_boost, ZEND_ACC_PUBLIC | ZEND_ACC_FINAL)
    PHP_ME(TermSearchQuery, field, ai_TermSearchQuery_field, ZEND_ACC_PUBLIC | ZEND_ACC_FINAL)
    PHP_ME(TermSearchQuery, prefixLength, ai_TermSearchQuery_prefixLength, ZEND_ACC_PUBLIC | ZEND_ACC_FINAL)
    PHP_ME(TermSearchQuery, fuzziness, ai_TermSearchQuery_fuzziness, ZEND_ACC_PUBLIC | ZEND_ACC_FINAL)
    PHP_FE_END
};
// clang-format on

void pcbc_term_search_query_init(zval *return_value, char *term, int term_len TSRMLS_DC)
{
    pcbc_term_search_query_t *obj;

    object_init_ex(return_value, pcbc_term_search_query_ce);
    obj = Z_TERM_SEARCH_QUERY_OBJ_P(return_value);
    obj->boost = -1;
    obj->term = estrndup(term, term_len);
    obj->field = NULL;
    obj->prefix_length = -1;
    obj->fuzziness = -1;
}

zend_object_handlers term_search_query_handlers;

static void term_search_query_free_object(pcbc_free_object_arg *object TSRMLS_DC) /* {{{ */
{
    pcbc_term_search_query_t *obj = Z_TERM_SEARCH_QUERY_OBJ(object);

    if (obj->term != NULL) {
        efree(obj->term);
    }
    if (obj->field != NULL) {
        efree(obj->field);
    }

    zend_object_std_dtor(&obj->std TSRMLS_CC);
#if PHP_VERSION_ID < 70000
    efree(obj);
#endif
} /* }}} */

static pcbc_create_object_retval term_search_query_create_object(zend_class_entry *class_type TSRMLS_DC)
{
    pcbc_term_search_query_t *obj = NULL;

    obj = PCBC_ALLOC_OBJECT_T(pcbc_term_search_query_t, class_type);

    zend_object_std_init(&obj->std, class_type TSRMLS_CC);
    object_properties_init(&obj->std, class_type);

#if PHP_VERSION_ID >= 70000
    obj->std.handlers = &term_search_query_handlers;
    return &obj->std;
#else
    {
        zend_object_value ret;
        ret.handle = zend_objects_store_put(obj, (zend_objects_store_dtor_t)zend_objects_destroy_object,
                                            term_search_query_free_object, NULL TSRMLS_CC);
        ret.handlers = &term_search_query_handlers;
        return ret;
    }
#endif
}

static HashTable *pcbc_term_search_query_get_debug_info(zval *object, int *is_temp TSRMLS_DC) /* {{{ */
{
    pcbc_term_search_query_t *obj = NULL;
#if PHP_VERSION_ID >= 70000
    zval retval;
#else
    zval retval = zval_used_for_init;
#endif

    *is_temp = 1;
    obj = Z_TERM_SEARCH_QUERY_OBJ_P(object);

    array_init(&retval);
    ADD_ASSOC_STRING(&retval, "term", obj->term);
    if (obj->field) {
        ADD_ASSOC_STRING(&retval, "field", obj->field);
    }
    if (obj->prefix_length >= 0) {
        ADD_ASSOC_LONG_EX(&retval, "prefix_length", obj->prefix_length);
    }
    if (obj->fuzziness >= 0) {
        ADD_ASSOC_LONG_EX(&retval, "fuzziness", obj->fuzziness);
    }
    if (obj->boost >= 0) {
        ADD_ASSOC_DOUBLE_EX(&retval, "boost", obj->boost);
    }
    return Z_ARRVAL(retval);
} /* }}} */

PHP_MINIT_FUNCTION(TermSearchQuery)
{
    zend_class_entry ce;

    INIT_NS_CLASS_ENTRY(ce, "Couchbase", "TermSearchQuery", term_search_query_methods);
    pcbc_term_search_query_ce = zend_register_internal_class(&ce TSRMLS_CC);
    pcbc_term_search_query_ce->create_object = term_search_query_create_object;
    PCBC_CE_FLAGS_FINAL(pcbc_term_search_query_ce);
    PCBC_CE_DISABLE_SERIALIZATION(pcbc_term_search_query_ce);

    zend_class_implements(pcbc_term_search_query_ce TSRMLS_CC, 1, pcbc_json_serializable_ce);
    zend_class_implements(pcbc_term_search_query_ce TSRMLS_CC, 1, pcbc_search_query_part_ce);

    memcpy(&term_search_query_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    term_search_query_handlers.get_debug_info = pcbc_term_search_query_get_debug_info;
#if PHP_VERSION_ID >= 70000
    term_search_query_handlers.free_obj = term_search_query_free_object;
    term_search_query_handlers.offset = XtOffsetOf(pcbc_term_search_query_t, std);
#endif

    zend_register_class_alias("\\CouchbaseTermSearchQuery", pcbc_term_search_query_ce);
    return SUCCESS;
}
