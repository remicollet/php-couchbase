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
 * A FTS query that matches a given term, applying further processing to it
 * like analyzers, stemming and even #fuzziness(int).
 */
#include "couchbase.h"

typedef struct {
    PCBC_ZEND_OBJECT_PRE
    double boost;
    char *field;
    char *analyzer;
    char *match;
    int prefix_length;
    int fuzziness;
    PCBC_ZEND_OBJECT_POST
} pcbc_match_search_query_t;

#if PHP_VERSION_ID >= 70000
static inline pcbc_match_search_query_t *pcbc_match_search_query_fetch_object(zend_object *obj)
{
    return (pcbc_match_search_query_t *)((char *)obj - XtOffsetOf(pcbc_match_search_query_t, std));
}
#define Z_MATCH_SEARCH_QUERY_OBJ(zo) (pcbc_match_search_query_fetch_object(zo))
#define Z_MATCH_SEARCH_QUERY_OBJ_P(zv) (pcbc_match_search_query_fetch_object(Z_OBJ_P(zv)))
#else
#define Z_MATCH_SEARCH_QUERY_OBJ(zo) ((pcbc_match_search_query_t *)zo)
#define Z_MATCH_SEARCH_QUERY_OBJ_P(zv) ((pcbc_match_search_query_t *)zend_object_store_get_object(zv TSRMLS_CC))
#endif

#define LOGARGS(lvl) LCB_LOG_##lvl, NULL, "pcbc/match_search_query", __FILE__, __LINE__

zend_class_entry *pcbc_match_search_query_ce;

extern PHP_JSON_API zend_class_entry *php_json_serializable_ce;

/* {{{ proto void MatchSearchQuery::__construct() */
PHP_METHOD(MatchSearchQuery, __construct) { throw_pcbc_exception("Accessing private constructor.", LCB_EINVAL); }
/* }}} */

/* {{{ proto \Couchbase\MatchSearchQuery MatchSearchQuery::analyzer(string $analyzer)
 */
PHP_METHOD(MatchSearchQuery, analyzer)
{
    pcbc_match_search_query_t *obj;
    char *analyzer = NULL;
    int rv;
    pcbc_str_arg_size analyzer_len;

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &analyzer, &analyzer_len);
    if (rv == FAILURE) {
        RETURN_NULL();
    }

    obj = Z_MATCH_SEARCH_QUERY_OBJ_P(getThis());
    if (obj->analyzer) {
        efree(obj->analyzer);
    }
    obj->analyzer = estrndup(analyzer, analyzer_len);

    RETURN_ZVAL(getThis(), 1, 0);
} /* }}} */

/* {{{ proto \Couchbase\MatchSearchQuery MatchSearchQuery::field(string $field)
 */
PHP_METHOD(MatchSearchQuery, field)
{
    pcbc_match_search_query_t *obj;
    char *field = NULL;
    int rv;
    pcbc_str_arg_size field_len;

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &field, &field_len);
    if (rv == FAILURE) {
        RETURN_NULL();
    }

    obj = Z_MATCH_SEARCH_QUERY_OBJ_P(getThis());
    if (obj->field) {
        efree(obj->field);
    }
    obj->field = estrndup(field, field_len);

    RETURN_ZVAL(getThis(), 1, 0);
} /* }}} */

/* {{{ proto \Couchbase\MatchSearchQuery MatchSearchQuery::prefixLength(int $prefixLength)
 */
PHP_METHOD(MatchSearchQuery, prefixLength)
{
    pcbc_match_search_query_t *obj;
    long prefix_length = 0;
    int rv;

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &prefix_length);
    if (rv == FAILURE) {
        RETURN_NULL();
    }

    obj = Z_MATCH_SEARCH_QUERY_OBJ_P(getThis());
    obj->prefix_length = prefix_length;

    RETURN_ZVAL(getThis(), 1, 0);
} /* }}} */

/* {{{ proto \Couchbase\MatchSearchQuery MatchSearchQuery::fuzziness(int $fuzziness)
 */
PHP_METHOD(MatchSearchQuery, fuzziness)
{
    pcbc_match_search_query_t *obj;
    long fuzziness = 0;
    int rv;

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &fuzziness);
    if (rv == FAILURE) {
        RETURN_NULL();
    }

    obj = Z_MATCH_SEARCH_QUERY_OBJ_P(getThis());
    obj->fuzziness = fuzziness;

    RETURN_ZVAL(getThis(), 1, 0);
} /* }}} */

/* {{{ proto \Couchbase\MatchSearchQuery MatchSearchQuery::boost(double $boost)
 */
PHP_METHOD(MatchSearchQuery, boost)
{
    pcbc_match_search_query_t *obj;
    double boost = 0;
    int rv;

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "d", &boost);
    if (rv == FAILURE) {
        RETURN_NULL();
    }

    obj = Z_MATCH_SEARCH_QUERY_OBJ_P(getThis());
    obj->boost = boost;

    RETURN_ZVAL(getThis(), 1, 0);
} /* }}} */

/* {{{ proto array MatchSearchQuery::jsonSerialize()
 */
PHP_METHOD(MatchSearchQuery, jsonSerialize)
{
    pcbc_match_search_query_t *obj;
    int rv;

    rv = zend_parse_parameters_none();
    if (rv == FAILURE) {
        RETURN_NULL();
    }

    obj = Z_MATCH_SEARCH_QUERY_OBJ_P(getThis());
    array_init(return_value);
    ADD_ASSOC_STRING(return_value, "match", obj->match);
    if (obj->field) {
        ADD_ASSOC_STRING(return_value, "field", obj->field);
    }
    if (obj->analyzer) {
        ADD_ASSOC_STRING(return_value, "analyzer", obj->analyzer);
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

ZEND_BEGIN_ARG_INFO_EX(ai_MatchSearchQuery_none, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_MatchSearchQuery_field, 0, 0, 1)
ZEND_ARG_INFO(0, field)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_MatchSearchQuery_analyzer, 0, 0, 1)
ZEND_ARG_INFO(0, analyzer)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_MatchSearchQuery_prefixLength, 0, 0, 1)
ZEND_ARG_INFO(0, prefixLength)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_MatchSearchQuery_fuzziness, 0, 0, 1)
ZEND_ARG_INFO(0, fuzziness)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_MatchSearchQuery_boost, 0, 0, 1)
ZEND_ARG_INFO(0, boost)
ZEND_END_ARG_INFO()

// clang-format off
zend_function_entry match_search_query_methods[] = {
    PHP_ME(MatchSearchQuery, __construct, ai_MatchSearchQuery_none, ZEND_ACC_PRIVATE | ZEND_ACC_FINAL | ZEND_ACC_CTOR)
    PHP_ME(MatchSearchQuery, jsonSerialize, ai_MatchSearchQuery_none, ZEND_ACC_PUBLIC | ZEND_ACC_FINAL)
    PHP_ME(MatchSearchQuery, boost, ai_MatchSearchQuery_boost, ZEND_ACC_PUBLIC | ZEND_ACC_FINAL)
    PHP_ME(MatchSearchQuery, field, ai_MatchSearchQuery_field, ZEND_ACC_PUBLIC | ZEND_ACC_FINAL)
    PHP_ME(MatchSearchQuery, analyzer, ai_MatchSearchQuery_analyzer, ZEND_ACC_PUBLIC | ZEND_ACC_FINAL)
    PHP_ME(MatchSearchQuery, prefixLength, ai_MatchSearchQuery_prefixLength, ZEND_ACC_PUBLIC | ZEND_ACC_FINAL)
    PHP_ME(MatchSearchQuery, fuzziness, ai_MatchSearchQuery_fuzziness, ZEND_ACC_PUBLIC | ZEND_ACC_FINAL)
    PHP_FE_END
};
// clang-format on

void pcbc_match_search_query_init(zval *return_value, char *match, int match_len TSRMLS_DC)
{
    pcbc_match_search_query_t *obj;

    object_init_ex(return_value, pcbc_match_search_query_ce);
    obj = Z_MATCH_SEARCH_QUERY_OBJ_P(return_value);
    obj->boost = -1;
    obj->match = estrndup(match, match_len);
    obj->field = NULL;
    obj->analyzer = NULL;
    obj->prefix_length = -1;
    obj->fuzziness = -1;
}

zend_object_handlers match_search_query_handlers;

static void match_search_query_free_object(pcbc_free_object_arg *object TSRMLS_DC) /* {{{ */
{
    pcbc_match_search_query_t *obj = Z_MATCH_SEARCH_QUERY_OBJ(object);

    if (obj->match != NULL) {
        efree(obj->match);
    }
    if (obj->field != NULL) {
        efree(obj->field);
    }
    if (obj->analyzer != NULL) {
        efree(obj->analyzer);
    }

    zend_object_std_dtor(&obj->std TSRMLS_CC);
#if PHP_VERSION_ID < 70000
    efree(obj);
#endif
} /* }}} */

static pcbc_create_object_retval match_search_query_create_object(zend_class_entry *class_type TSRMLS_DC)
{
    pcbc_match_search_query_t *obj = NULL;

    obj = PCBC_ALLOC_OBJECT_T(pcbc_match_search_query_t, class_type);

    zend_object_std_init(&obj->std, class_type TSRMLS_CC);
    object_properties_init(&obj->std, class_type);

#if PHP_VERSION_ID >= 70000
    obj->std.handlers = &match_search_query_handlers;
    return &obj->std;
#else
    {
        zend_object_value ret;
        ret.handle = zend_objects_store_put(obj, (zend_objects_store_dtor_t)zend_objects_destroy_object,
                                            match_search_query_free_object, NULL TSRMLS_CC);
        ret.handlers = &match_search_query_handlers;
        return ret;
    }
#endif
}

static HashTable *pcbc_match_search_query_get_debug_info(zval *object, int *is_temp TSRMLS_DC) /* {{{ */
{
    pcbc_match_search_query_t *obj = NULL;
#if PHP_VERSION_ID >= 70000
    zval retval;
#else
    zval retval = zval_used_for_init;
#endif

    *is_temp = 1;
    obj = Z_MATCH_SEARCH_QUERY_OBJ_P(object);

    array_init(&retval);
    ADD_ASSOC_STRING(&retval, "match", obj->match);
    if (obj->field) {
        ADD_ASSOC_STRING(&retval, "field", obj->field);
    }
    if (obj->analyzer) {
        ADD_ASSOC_STRING(&retval, "analyzer", obj->analyzer);
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

PHP_MINIT_FUNCTION(MatchSearchQuery)
{
    zend_class_entry ce;

    INIT_NS_CLASS_ENTRY(ce, "Couchbase", "MatchSearchQuery", match_search_query_methods);
    pcbc_match_search_query_ce = zend_register_internal_class(&ce TSRMLS_CC);
    pcbc_match_search_query_ce->create_object = match_search_query_create_object;
    PCBC_CE_FLAGS_FINAL(pcbc_match_search_query_ce);
    PCBC_CE_DISABLE_SERIALIZATION(pcbc_match_search_query_ce);

    zend_class_implements(pcbc_match_search_query_ce TSRMLS_CC, 1, php_json_serializable_ce);
    zend_class_implements(pcbc_match_search_query_ce TSRMLS_CC, 1, pcbc_search_query_part_ce);

    memcpy(&match_search_query_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    match_search_query_handlers.get_debug_info = pcbc_match_search_query_get_debug_info;
#if PHP_VERSION_ID >= 70000
    match_search_query_handlers.free_obj = match_search_query_free_object;
    match_search_query_handlers.offset = XtOffsetOf(pcbc_match_search_query_t, std);
#endif

    zend_register_class_alias("\\CouchbaseMatchSearchQuery", pcbc_match_search_query_ce);
    return SUCCESS;
}
