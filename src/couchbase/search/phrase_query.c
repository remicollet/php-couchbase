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
 * A FTS query that matches several terms (a "phrase") as is. The order of the terms mater and no further processing is
 * applied to them, so they must appear in the index exactly as provided.  Usually for debugging purposes, prefer
 * MatchPhraseQuery.
 */
#include "couchbase.h"

typedef struct {

    double boost;
    char *field;
    zval terms;
    zend_object std;
} pcbc_phrase_search_query_t;

static inline pcbc_phrase_search_query_t *pcbc_phrase_search_query_fetch_object(zend_object *obj)
{
    return (pcbc_phrase_search_query_t *)((char *)obj - XtOffsetOf(pcbc_phrase_search_query_t, std));
}
#define Z_PHRASE_SEARCH_QUERY_OBJ(zo) (pcbc_phrase_search_query_fetch_object(zo))
#define Z_PHRASE_SEARCH_QUERY_OBJ_P(zv) (pcbc_phrase_search_query_fetch_object(Z_OBJ_P(zv)))

#define LOGARGS(lvl) LCB_LOG_##lvl, NULL, "pcbc/phrase_search_query", __FILE__, __LINE__

zend_class_entry *pcbc_phrase_search_query_ce;
/* {{{ proto void PhraseSearchQuery::__construct() */
PHP_METHOD(PhraseSearchQuery, __construct)
{
    throw_pcbc_exception("Accessing private constructor.", LCB_EINVAL);
}
/* }}} */

/* {{{ proto \Couchbase\PhraseSearchQuery PhraseSearchQuery::field(string $field)
 */
PHP_METHOD(PhraseSearchQuery, field)
{
    pcbc_phrase_search_query_t *obj;
    char *field = NULL;
    int rv;
    size_t field_len;

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &field, &field_len);
    if (rv == FAILURE) {
        RETURN_NULL();
    }

    obj = Z_PHRASE_SEARCH_QUERY_OBJ_P(getThis());
    if (obj->field) {
        efree(obj->field);
    }
    obj->field = estrndup(field, field_len);

    RETURN_ZVAL(getThis(), 1, 0);
} /* }}} */

/* {{{ proto \Couchbase\PhraseSearchQuery PhraseSearchQuery::boost(double $boost)
 */
PHP_METHOD(PhraseSearchQuery, boost)
{
    pcbc_phrase_search_query_t *obj;
    double boost = 0;
    int rv;

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "d", &boost);
    if (rv == FAILURE) {
        RETURN_NULL();
    }

    obj = Z_PHRASE_SEARCH_QUERY_OBJ_P(getThis());
    obj->boost = boost;

    RETURN_ZVAL(getThis(), 1, 0);
} /* }}} */

/* {{{ proto array PhraseSearchQuery::jsonSerialize()
 */
PHP_METHOD(PhraseSearchQuery, jsonSerialize)
{
    pcbc_phrase_search_query_t *obj;
    int rv;

    rv = zend_parse_parameters_none();
    if (rv == FAILURE) {
        RETURN_NULL();
    }

    obj = Z_PHRASE_SEARCH_QUERY_OBJ_P(getThis());
    array_init(return_value);
    ADD_ASSOC_ZVAL_EX(return_value, "terms", &obj->terms);
    PCBC_ADDREF_P(&obj->terms);
    if (obj->field) {
        ADD_ASSOC_STRING(return_value, "field", obj->field);
    }
    if (obj->boost >= 0) {
        ADD_ASSOC_DOUBLE_EX(return_value, "boost", obj->boost);
    }
} /* }}} */

ZEND_BEGIN_ARG_INFO_EX(ai_PhraseSearchQuery_none, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_PhraseSearchQuery_field, 0, 0, 1)
ZEND_ARG_INFO(0, field)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_PhraseSearchQuery_boost, 0, 0, 1)
ZEND_ARG_INFO(0, boost)
ZEND_END_ARG_INFO()

// clang-format off
zend_function_entry phrase_search_query_methods[] = {
    PHP_ME(PhraseSearchQuery, __construct, ai_PhraseSearchQuery_none, ZEND_ACC_PRIVATE | ZEND_ACC_FINAL | ZEND_ACC_CTOR)
    PHP_ME(PhraseSearchQuery, jsonSerialize, ai_PhraseSearchQuery_none, ZEND_ACC_PUBLIC)
    PHP_ME(PhraseSearchQuery, boost, ai_PhraseSearchQuery_boost, ZEND_ACC_PUBLIC)
    PHP_ME(PhraseSearchQuery, field, ai_PhraseSearchQuery_field, ZEND_ACC_PUBLIC)
    PHP_FE_END
};
// clang-format on

void pcbc_phrase_search_query_init(zval *return_value, zval *args, int num_args TSRMLS_DC)
{
    pcbc_phrase_search_query_t *obj;

    object_init_ex(return_value, pcbc_phrase_search_query_ce);
    obj = Z_PHRASE_SEARCH_QUERY_OBJ_P(return_value);
    obj->boost = -1;
    obj->field = NULL;

    ZVAL_UNDEF(&obj->terms);
    array_init(&obj->terms);

    if (num_args && args) {
        int i;
        for (i = 0; i < num_args; ++i) {
            zval *id;
            id = &args[i];
            if (Z_TYPE_P(id) != IS_STRING) {
                pcbc_log(LOGARGS(WARN), "term has to be a string (skipping argument #%d)", i);
                continue;
            }
            add_next_index_zval(&obj->terms, id);
            PCBC_ADDREF_P(id);
        }
    }
}

zend_object_handlers phrase_search_query_handlers;

static void phrase_search_query_free_object(zend_object *object TSRMLS_DC) /* {{{ */
{
    pcbc_phrase_search_query_t *obj = Z_PHRASE_SEARCH_QUERY_OBJ(object);

    if (obj->field != NULL) {
        efree(obj->field);
    }
    zval_ptr_dtor(&obj->terms);

    zend_object_std_dtor(&obj->std TSRMLS_CC);
} /* }}} */

static zend_object *phrase_search_query_create_object(zend_class_entry *class_type TSRMLS_DC)
{
    pcbc_phrase_search_query_t *obj = NULL;

    obj = PCBC_ALLOC_OBJECT_T(pcbc_phrase_search_query_t, class_type);

    zend_object_std_init(&obj->std, class_type TSRMLS_CC);
    object_properties_init(&obj->std, class_type);

    obj->std.handlers = &phrase_search_query_handlers;
    return &obj->std;
}

static HashTable *pcbc_phrase_search_query_get_debug_info(zval *object, int *is_temp TSRMLS_DC) /* {{{ */
{
    pcbc_phrase_search_query_t *obj = NULL;
    zval retval;

    *is_temp = 1;
    obj = Z_PHRASE_SEARCH_QUERY_OBJ_P(object);

    array_init(&retval);
    ADD_ASSOC_ZVAL_EX(&retval, "terms", &obj->terms);
    PCBC_ADDREF_P(&obj->terms);
    if (obj->field) {
        ADD_ASSOC_STRING(&retval, "field", obj->field);
    }
    if (obj->boost >= 0) {
        ADD_ASSOC_DOUBLE_EX(&retval, "boost", obj->boost);
    }
    return Z_ARRVAL(retval);
} /* }}} */

PHP_MINIT_FUNCTION(PhraseSearchQuery)
{
    zend_class_entry ce;

    INIT_NS_CLASS_ENTRY(ce, "Couchbase", "PhraseSearchQuery", phrase_search_query_methods);
    pcbc_phrase_search_query_ce = zend_register_internal_class(&ce TSRMLS_CC);
    pcbc_phrase_search_query_ce->create_object = phrase_search_query_create_object;
    PCBC_CE_DISABLE_SERIALIZATION(pcbc_phrase_search_query_ce);

    zend_class_implements(pcbc_phrase_search_query_ce TSRMLS_CC, 1, pcbc_json_serializable_ce);
    zend_class_implements(pcbc_phrase_search_query_ce TSRMLS_CC, 1, pcbc_search_query_part_ce);

    memcpy(&phrase_search_query_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    phrase_search_query_handlers.get_debug_info = pcbc_phrase_search_query_get_debug_info;
    phrase_search_query_handlers.free_obj = phrase_search_query_free_object;
    phrase_search_query_handlers.offset = XtOffsetOf(pcbc_phrase_search_query_t, std);

    zend_register_class_alias("\\CouchbasePhraseSearchQuery", pcbc_phrase_search_query_ce);
    return SUCCESS;
}
