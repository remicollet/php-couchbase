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

#include "couchbase.h"
#include <Zend/zend_alloc.h>

#define LOGARGS(lvl) LCB_LOG_##lvl, NULL, "pcbc/search_query", __FILE__, __LINE__

zend_class_entry *pcbc_search_query_ce;

/* {{{ proto SearchQuery::__construct(string $indexName, \Couchbase\SearchQueryPart $queryPart) */
PHP_METHOD(SearchQuery, __construct)
{
    pcbc_search_query_t *obj;
    char *index_name = NULL;
    pcbc_str_arg_size index_name_len = 0;
    zval *query_part;
    int rv;

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sO", &index_name, &index_name_len, &query_part,
                               pcbc_search_query_part_ce);
    if (rv == FAILURE) {
        throw_pcbc_exception("Invalid arguments.", LCB_EINVAL);
        RETURN_NULL();
    }

    obj = Z_SEARCH_QUERY_OBJ_P(getThis());
    obj->index_name = estrndup(index_name, index_name_len);
    obj->limit = -1;
    obj->skip = -1;
    obj->explain = 0;
    obj->highlight_style = NULL;
    obj->server_side_timeout = -1;
#if PHP_VERSION_ID >= 70000
    ZVAL_ZVAL(&obj->query_part, query_part, 1, 0);
#else
    PCBC_ADDREF_P(query_part);
    obj->query_part = query_part;
#endif
    ZVAL_UNDEF(PCBC_P(obj->consistency));
    ZVAL_UNDEF(PCBC_P(obj->fields));
    ZVAL_UNDEF(PCBC_P(obj->sort));
    ZVAL_UNDEF(PCBC_P(obj->facets));
    ZVAL_UNDEF(PCBC_P(obj->highlight_fields));
}
/* }}} */

/* {{{ proto \Couchbase\SearchQuery SearchQuery::limit(int $limit)
 */
PHP_METHOD(SearchQuery, limit)
{
    pcbc_search_query_t *obj;
    long limit = 0;
    int rv;

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &limit);
    if (rv == FAILURE) {
        RETURN_NULL();
    }

    obj = Z_SEARCH_QUERY_OBJ_P(getThis());
    obj->limit = limit;

    RETURN_ZVAL(getThis(), 1, 0);
} /* }}} */

/* {{{ proto \Couchbase\SearchQuery SearchQuery::skip(int $skip)
 */
PHP_METHOD(SearchQuery, skip)
{
    pcbc_search_query_t *obj;
    long skip = 0;
    int rv;

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &skip);
    if (rv == FAILURE) {
        RETURN_NULL();
    }

    obj = Z_SEARCH_QUERY_OBJ_P(getThis());
    obj->skip = skip;

    RETURN_ZVAL(getThis(), 1, 0);
} /* }}} */

/* {{{ proto \Couchbase\SearchQuery SearchQuery::explain(bool $explain)
 */
PHP_METHOD(SearchQuery, explain)
{
    pcbc_search_query_t *obj;
    zend_bool explain = 0;
    int rv;

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "b", &explain);
    if (rv == FAILURE) {
        RETURN_NULL();
    }

    obj = Z_SEARCH_QUERY_OBJ_P(getThis());
    obj->explain = explain;

    RETURN_ZVAL(getThis(), 1, 0);
} /* }}} */

/* {{{ proto \Couchbase\SearchQuery SearchQuery::serverSideTimeout(int $serverSideTimeout)
 */
PHP_METHOD(SearchQuery, serverSideTimeout)
{
    pcbc_search_query_t *obj;
    long timeout = 0;
    int rv;

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &timeout);
    if (rv == FAILURE) {
        RETURN_NULL();
    }

    obj = Z_SEARCH_QUERY_OBJ_P(getThis());
    obj->server_side_timeout = timeout;

    RETURN_ZVAL(getThis(), 1, 0);
} /* }}} */

/* {{{ proto \Couchbase\MatchSearchQuery SearchQuery::match(string $match) */
PHP_METHOD(SearchQuery, match)
{
    char *match = NULL;
    pcbc_str_arg_size match_len = 0;
    int rv;

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &match, &match_len);
    if (rv == FAILURE) {
        return;
    }

    pcbc_match_search_query_init(return_value, match, match_len TSRMLS_CC);
} /* }}} */

/* {{{ proto \Couchbase\QueryStringSearchQuery SearchQuery::matchPhrase(string $matchPhrase) */
PHP_METHOD(SearchQuery, matchPhrase)
{
    char *match_phrase = NULL;
    pcbc_str_arg_size match_phrase_len = 0;
    int rv;

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &match_phrase, &match_phrase_len);
    if (rv == FAILURE) {
        return;
    }

    pcbc_match_phrase_search_query_init(return_value, match_phrase, match_phrase_len TSRMLS_CC);
} /* }}} */

/* {{{ proto \Couchbase\QueryStringSearchQuery SearchQuery::queryString(string $queryString) */
PHP_METHOD(SearchQuery, queryString)
{
    char *query_string = NULL;
    pcbc_str_arg_size query_string_len = 0;
    int rv;

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &query_string, &query_string_len);
    if (rv == FAILURE) {
        return;
    }

    pcbc_query_string_search_query_init(return_value, query_string, query_string_len TSRMLS_CC);
} /* }}} */

/* {{{ proto \Couchbase\RegexpSearchQuery SearchQuery::regexp(string $regexp) */
PHP_METHOD(SearchQuery, regexp)
{
    char *regexp = NULL;
    pcbc_str_arg_size regexp_len = 0;
    int rv;

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &regexp, &regexp_len);
    if (rv == FAILURE) {
        return;
    }

    pcbc_regexp_search_query_init(return_value, regexp, regexp_len TSRMLS_CC);
} /* }}} */

/* {{{ proto \Couchbase\BooleanFieldSearchQuery SearchQuery::booleanField(string $booleanField) */
PHP_METHOD(SearchQuery, booleanField)
{
    zend_bool value = 0;
    int rv;

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "b", &value);
    if (rv == FAILURE) {
        return;
    }

    pcbc_boolean_field_search_query_init(return_value, value TSRMLS_CC);
} /* }}} */

/* {{{ proto \Couchbase\DisjunctionSearchQuery SearchQuery::disjuncts(string ...$queries) */
PHP_METHOD(SearchQuery, disjuncts)
{
#if PHP_VERSION_ID >= 70000
    zval *args = NULL;
#else
    zval ***args = NULL;
#endif
    int rv, num_args = 0;

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "+", &args, &num_args);
    if (rv == FAILURE) {
        return;
    }

    pcbc_disjunction_search_query_init(return_value, args, num_args TSRMLS_CC);
#if PHP_VERSION_ID < 70000
    if (args) {
        efree(args);
    }
#endif
} /* }}} */

/* {{{ proto \Couchbase\ConjunctionSearchQuery SearchQuery::conjuncts(string ...$queries) */
PHP_METHOD(SearchQuery, conjuncts)
{
#if PHP_VERSION_ID >= 70000
    zval *args = NULL;
#else
    zval ***args = NULL;
#endif
    int rv, num_args = 0;

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "+", &args, &num_args);
    if (rv == FAILURE) {
        return;
    }

    pcbc_conjunction_search_query_init(return_value, args, num_args TSRMLS_CC);
#if PHP_VERSION_ID < 70000
    if (args) {
        efree(args);
    }
#endif
} /* }}} */

/* {{{ proto \Couchbase\PhraseSearchQuery SearchQuery::phrase(string ...$terms) */
PHP_METHOD(SearchQuery, phrase)
{
#if PHP_VERSION_ID >= 70000
    zval *args = NULL;
#else
    zval ***args = NULL;
#endif
    int rv, num_args = 0;

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "+", &args, &num_args);
    if (rv == FAILURE) {
        return;
    }

    pcbc_phrase_search_query_init(return_value, args, num_args TSRMLS_CC);
#if PHP_VERSION_ID < 70000
    if (args) {
        efree(args);
    }
#endif
} /* }}} */

/* {{{ proto \Couchbase\DocIdSearchQuery SearchQuery::docId(string ...$documentIds) */
PHP_METHOD(SearchQuery, docId)
{
#if PHP_VERSION_ID >= 70000
    zval *args = NULL;
#else
    zval ***args = NULL;
#endif
    int rv, num_args = 0;

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "+", &args, &num_args);
    if (rv == FAILURE) {
        return;
    }

    pcbc_doc_id_search_query_init(return_value, args, num_args TSRMLS_CC);
#if PHP_VERSION_ID < 70000
    if (args) {
        efree(args);
    }
#endif
} /* }}} */

/* {{{ proto \Couchbase\WildcardSearchQuery SearchQuery::wildcard(string $wildcard) */
PHP_METHOD(SearchQuery, wildcard)
{
    char *wildcard = NULL;
    pcbc_str_arg_size wildcard_len = 0;
    int rv;

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &wildcard, &wildcard_len);
    if (rv == FAILURE) {
        return;
    }

    pcbc_wildcard_search_query_init(return_value, wildcard, wildcard_len TSRMLS_CC);
} /* }}} */

/* {{{ proto \Couchbase\TermSearchQuery SearchQuery::term(string $term) */
PHP_METHOD(SearchQuery, term)
{
    char *term = NULL;
    pcbc_str_arg_size term_len = 0;
    int rv;

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &term, &term_len);
    if (rv == FAILURE) {
        return;
    }

    pcbc_term_search_query_init(return_value, term, term_len TSRMLS_CC);
} /* }}} */

/* {{{ proto \Couchbase\PrefixSearchQuery SearchQuery::prefix(string $prefix) */
PHP_METHOD(SearchQuery, prefix)
{
    char *prefix = NULL;
    pcbc_str_arg_size prefix_len = 0;
    int rv;

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &prefix, &prefix_len);
    if (rv == FAILURE) {
        return;
    }

    pcbc_prefix_search_query_init(return_value, prefix, prefix_len TSRMLS_CC);
} /* }}} */

/* {{{ proto \Couchbase\MatchNoneSearchQuery SearchQuery::matchNone() */
PHP_METHOD(SearchQuery, matchNone)
{
    int rv;

    rv = zend_parse_parameters_none();
    if (rv == FAILURE) {
        return;
    }

    pcbc_match_none_search_query_init(return_value TSRMLS_CC);
} /* }}} */

/* {{{ proto \Couchbase\MatchAllSearchQuery SearchQuery::matchAll() */
PHP_METHOD(SearchQuery, matchAll)
{
    int rv;

    rv = zend_parse_parameters_none();
    if (rv == FAILURE) {
        return;
    }

    pcbc_match_none_search_query_init(return_value TSRMLS_CC);
} /* }}} */

/* {{{ proto \Couchbase\DateRangeSearchQuery SearchQuery::dateRange() */
PHP_METHOD(SearchQuery, dateRange)
{
    int rv;

    rv = zend_parse_parameters_none();
    if (rv == FAILURE) {
        return;
    }

    pcbc_date_range_search_query_init(return_value TSRMLS_CC);
} /* }}} */

/* {{{ proto \Couchbase\NumericRangeSearchQuery SearchQuery::numericRange() */
PHP_METHOD(SearchQuery, numericRange)
{
    int rv;

    rv = zend_parse_parameters_none();
    if (rv == FAILURE) {
        return;
    }

    pcbc_numeric_range_search_query_init(return_value TSRMLS_CC);
} /* }}} */

/* {{{ proto \Couchbase\TermRangeSearchQuery SearchQuery::termRange() */
PHP_METHOD(SearchQuery, termRange)
{
    int rv;

    rv = zend_parse_parameters_none();
    if (rv == FAILURE) {
        return;
    }

    pcbc_term_range_search_query_init(return_value TSRMLS_CC);
} /* }}} */

/* {{{ proto \Couchbase\BooleanSearchQuery SearchQuery::boolean() */
PHP_METHOD(SearchQuery, boolean)
{
    int rv;

    rv = zend_parse_parameters_none();
    if (rv == FAILURE) {
        return;
    }

    pcbc_boolean_search_query_init(return_value TSRMLS_CC);
} /* }}} */

/* {{{ proto \Couchbase\GeoDistanceSearchQuery SearchQuery::geoDistance(float $longitude, float $latitude,
                                                                        string distance) */
PHP_METHOD(SearchQuery, geoDistance)
{
    double longitude = 0;
    double latitude = 0;
    char *distance = NULL;
    pcbc_str_arg_size distance_len = 0;
    int rv;

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "dds", &longitude, &latitude, &distance, &distance_len);
    if (rv == FAILURE) {
        return;
    }

    pcbc_geo_distance_search_query_init(return_value, longitude, latitude, distance, distance_len TSRMLS_CC);
} /* }}} */

/* {{{ proto \Couchbase\GeoBoundingBoxSearchQuery SearchQuery::geoBoundingBox(float $topLeftLongitude,
                     float $topLeftLongitude, float $bottomRightLongitude, float $bottomRightLatitude) */
PHP_METHOD(SearchQuery, geoBoundingBox)
{
    double top_left_longitude = 0;
    double top_left_latitude = 0;
    double bottom_right_longitude = 0;
    double bottom_right_latitude = 0;
    int rv;

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "dddd", &top_left_longitude, &top_left_latitude,
                               &bottom_right_longitude, &bottom_right_latitude);
    if (rv == FAILURE) {
        return;
    }

    pcbc_geo_bounding_box_search_query_init(return_value, top_left_longitude, top_left_latitude, bottom_right_longitude,
                                            bottom_right_latitude TSRMLS_CC);
} /* }}} */

/* {{{ proto \Couchbase\TermSearchFacet SearchQuery::termFacet(string $field, int $limit) */
PHP_METHOD(SearchQuery, termFacet)
{
    char *field = NULL;
    pcbc_str_arg_size field_len = 0;
    long limit;
    int rv;

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sl", &field, &field_len, &limit);
    if (rv == FAILURE) {
        return;
    }

    pcbc_term_search_facet_init(return_value, field, field_len, limit TSRMLS_CC);
} /* }}} */

/* {{{ proto \Couchbase\DateRangeSearchFacet SearchQuery::dateFacet(string $field, int $limit) */
PHP_METHOD(SearchQuery, dateRangeFacet)
{
    char *field = NULL;
    pcbc_str_arg_size field_len = 0;
    long limit;
    int rv;

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sl", &field, &field_len, &limit);
    if (rv == FAILURE) {
        return;
    }

    pcbc_date_range_search_facet_init(return_value, field, field_len, limit TSRMLS_CC);
} /* }}} */

/* {{{ proto \Couchbase\NumericRangeSearchFacet SearchQuery::numericFacet(string $field, int $limit) */
PHP_METHOD(SearchQuery, numericRangeFacet)
{
    char *field = NULL;
    pcbc_str_arg_size field_len = 0;
    long limit;
    int rv;

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sl", &field, &field_len, &limit);
    if (rv == FAILURE) {
        return;
    }

    pcbc_numeric_range_search_facet_init(return_value, field, field_len, limit TSRMLS_CC);
} /* }}} */

/* {{{ proto \Couchbase\SearchQuery SearchQuery::consistentWith(\Couchbase\MutationState $mutationState) */
PHP_METHOD(SearchQuery, consistentWith)
{
    pcbc_search_query_t *obj;
    int rv;
    zval *mutation_state = NULL;
    PCBC_ZVAL scan_vectors;
    pcbc_mutation_state_t *state;

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &mutation_state, pcbc_mutation_state_ce);
    if (rv == FAILURE) {
        RETURN_NULL();
    }

    state = Z_MUTATION_STATE_OBJ_P(mutation_state);
    if (state->ntokens == 0) {
        throw_pcbc_exception("Mutation state have to contain at least one token", LCB_EINVAL);
        RETURN_NULL();
    }

    obj = Z_SEARCH_QUERY_OBJ_P(getThis());
    if (!Z_ISUNDEF(obj->consistency)) {
        zval_ptr_dtor(&obj->consistency);
        ZVAL_UNDEF(PCBC_P(obj->consistency));
    }

    PCBC_ZVAL_ALLOC(scan_vectors);
    pcbc_mutation_state_export_for_search(state, PCBC_P(scan_vectors) TSRMLS_CC);

    PCBC_ZVAL_ALLOC(obj->consistency);
    array_init_size(PCBC_P(obj->consistency), 2);
    ADD_ASSOC_STRING(PCBC_P(obj->consistency), "level", "at_plus");
    {
        PCBC_ZVAL indexed_vectors;

        PCBC_ZVAL_ALLOC(indexed_vectors);
        array_init_size(PCBC_P(indexed_vectors), 1);
        add_assoc_zval_ex(PCBC_P(indexed_vectors), obj->index_name, strlen(obj->index_name) + 1, PCBC_P(scan_vectors));
        ADD_ASSOC_ZVAL_EX(PCBC_P(obj->consistency), "vectors", PCBC_P(indexed_vectors));
    }

    RETURN_ZVAL(getThis(), 1, 0);
} /* }}} */

/* {{{ proto \Couchbase\SearchQuery SearchQuery::addFacet(string $name, \Couchbase\SearchFacet $facet) */
PHP_METHOD(SearchQuery, addFacet)
{
    pcbc_search_query_t *obj;
    char *name = NULL;
    pcbc_str_arg_size name_len = 0;
    zval *facet;
    int rv;

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sO", &name, &name_len, &facet, pcbc_search_facet_ce);
    if (rv == FAILURE) {
        return;
    }

    obj = Z_SEARCH_QUERY_OBJ_P(getThis());
    if (Z_ISUNDEF(obj->facets)) {
        PCBC_ZVAL_ALLOC(obj->facets);
        array_init(PCBC_P(obj->facets));
    }

#if PHP_VERSION_ID >= 70000
    add_assoc_zval_ex(PCBC_P(obj->facets), name, name_len, facet);
#else
    add_assoc_zval_ex(PCBC_P(obj->facets), name, name_len + 1, facet);
#endif
    PCBC_ADDREF_P(facet);

    RETURN_ZVAL(getThis(), 1, 0);
} /* }}} */

/* {{{ proto \Couchbase\SearchQuery SearchQuery::fields(string ...$fields) */
PHP_METHOD(SearchQuery, fields)
{
    pcbc_search_query_t *obj;
#if PHP_VERSION_ID >= 70000
    zval *args = NULL;
#else
    zval ***args = NULL;
#endif
    int num_args = 0;
    int rv;

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "+", &args, &num_args);
    if (rv == FAILURE) {
        return;
    }

    obj = Z_SEARCH_QUERY_OBJ_P(getThis());
    if (Z_ISUNDEF(obj->fields)) {
        PCBC_ZVAL_ALLOC(obj->fields);
        array_init(PCBC_P(obj->fields));
    }
    if (num_args && args) {
        int i;
        for (i = 0; i < num_args; ++i) {
            PCBC_ZVAL *field;
#if PHP_VERSION_ID >= 70000
            field = &args[i];
#else
            field = args[i];
#endif
            if (Z_TYPE_P(PCBC_P(*field)) != IS_STRING) {
                pcbc_log(LOGARGS(WARN), "field has to be a string (skipping argument #%d)", i);
                continue;
            }
            add_next_index_zval(PCBC_P(obj->fields), PCBC_P(*field));
            PCBC_ADDREF_P(PCBC_P(*field));
        }
    }
#if PHP_VERSION_ID < 70000
    if (args) {
        efree(args);
    }
#endif

    RETURN_ZVAL(getThis(), 1, 0);
} /* }}} */

/* {{{ proto \Couchbase\SearchQuery SearchQuery::sort(string|SearchSort ...$sort) */
PHP_METHOD(SearchQuery, sort)
{
    pcbc_search_query_t *obj;
#if PHP_VERSION_ID >= 70000
    zval *args = NULL;
#else
    zval ***args = NULL;
#endif
    int num_args = 0;
    int rv;

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "+", &args, &num_args);
    if (rv == FAILURE) {
        return;
    }

    obj = Z_SEARCH_QUERY_OBJ_P(getThis());
    if (Z_ISUNDEF(obj->sort)) {
        PCBC_ZVAL_ALLOC(obj->sort);
        array_init(PCBC_P(obj->sort));
    }
    if (num_args && args) {
        int i;
        for (i = 0; i < num_args; ++i) {
            PCBC_ZVAL *field;
#if PHP_VERSION_ID >= 70000
            field = &args[i];
#else
            field = args[i];
#endif
            if (Z_TYPE_P(PCBC_P(*field)) != IS_STRING &&
                (Z_TYPE_P(PCBC_P(*field)) != IS_OBJECT ||
                 !instanceof_function(Z_OBJCE_P(PCBC_P(*field)), pcbc_search_sort_ce TSRMLS_CC))) {
                pcbc_log(LOGARGS(WARN), "field has to be a string or SearchSort (skipping argument #%d)", i);
                continue;
            }
            add_next_index_zval(PCBC_P(obj->sort), PCBC_P(*field));
            PCBC_ADDREF_P(PCBC_P(*field));
        }
    }
#if PHP_VERSION_ID < 70000
    if (args) {
        efree(args);
    }
#endif

    RETURN_ZVAL(getThis(), 1, 0);
} /* }}} */

/* {{{ proto \Couchbase\SearchQuery SearchQuery::highlight(string $style, string ...$fields)
   See SearchQuery::HIGHLIGHT_{HTML,ANSI,SIMPLE}. Pass NULL for $style to switch it off.
*/
PHP_METHOD(SearchQuery, highlight)
{
    pcbc_search_query_t *obj;
    char *style = NULL;
#if PHP_VERSION_ID >= 70000
    zval *args = NULL;
#else
    zval ***args = NULL;
#endif
    int rv, num_args = 0;
    pcbc_str_arg_size style_len;

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s+", &style, &style_len, &args, &num_args);
    if (rv == FAILURE) {
        return;
    }

    obj = Z_SEARCH_QUERY_OBJ_P(getThis());
    if (obj->highlight_style) {
        efree(obj->highlight_style);
    }
    if (!Z_ISUNDEF(obj->highlight_fields)) {
        zval_ptr_dtor(&obj->highlight_fields);
        ZVAL_UNDEF(PCBC_P(obj->highlight_fields));
    }
    if (style) {
        obj->highlight_style = estrndup(style, style_len);
        if (Z_ISUNDEF(obj->highlight_fields)) {
            PCBC_ZVAL_ALLOC(obj->highlight_fields);
            array_init(PCBC_P(obj->highlight_fields));
        }
        if (num_args && args) {
            int i;
            for (i = 0; i < num_args; ++i) {
                PCBC_ZVAL *field;
#if PHP_VERSION_ID >= 70000
                field = &args[i];
#else
                field = args[i];
#endif
                if (Z_TYPE_P(PCBC_P(*field)) != IS_STRING) {
                    pcbc_log(LOGARGS(WARN), "field has to be a string (skipping argument #%d)", i);
                    continue;
                }
                add_next_index_zval(PCBC_P(obj->highlight_fields), PCBC_P(*field));
                PCBC_ADDREF_P(PCBC_P(*field));
            }
        }
    }
#if PHP_VERSION_ID < 70000
    if (args) {
        efree(args);
    }
#endif

    RETURN_ZVAL(getThis(), 1, 0);
} /* }}} */

/* {{{ proto array SearchQuery::jsonSerialize()
 */
PHP_METHOD(SearchQuery, jsonSerialize)
{
    pcbc_search_query_t *obj;
    int rv;

    rv = zend_parse_parameters_none();
    if (rv == FAILURE) {
        RETURN_NULL();
    }

    obj = Z_SEARCH_QUERY_OBJ_P(getThis());
    array_init(return_value);
    ADD_ASSOC_STRING(return_value, "indexName", obj->index_name);
    if (obj->explain) {
        ADD_ASSOC_BOOL_EX(return_value, "explain", obj->explain);
    }
    if (obj->limit >= 0) {
        ADD_ASSOC_LONG_EX(return_value, "size", obj->limit);
    }
    if (obj->skip >= 0) {
        ADD_ASSOC_LONG_EX(return_value, "from", obj->skip);
    }
    if (obj->server_side_timeout >= 0 || !Z_ISUNDEF(obj->consistency)) {
        PCBC_ZVAL control;

        PCBC_ZVAL_ALLOC(control);
        array_init(PCBC_P(control));
        ADD_ASSOC_ZVAL_EX(return_value, "ctl", PCBC_P(control));

        if (obj->server_side_timeout >= 0) {
            ADD_ASSOC_LONG_EX(PCBC_P(control), "timeout", obj->server_side_timeout);
        }
        if (!Z_ISUNDEF(obj->consistency)) {
            ADD_ASSOC_ZVAL_EX(PCBC_P(control), "consistency", PCBC_P(obj->consistency));
            PCBC_ADDREF_P(PCBC_P(obj->consistency));
        }
    }
    if (!Z_ISUNDEF(obj->fields)) {
        ADD_ASSOC_ZVAL_EX(return_value, "fields", PCBC_P(obj->fields));
        PCBC_ADDREF_P(PCBC_P(obj->fields));
    }
    if (!Z_ISUNDEF(obj->sort)) {
        ADD_ASSOC_ZVAL_EX(return_value, "sort", PCBC_P(obj->sort));
        PCBC_ADDREF_P(PCBC_P(obj->sort));
    }
    if (!Z_ISUNDEF(obj->facets)) {
        ADD_ASSOC_ZVAL_EX(return_value, "facets", PCBC_P(obj->facets));
        PCBC_ADDREF_P(PCBC_P(obj->facets));
    }
    if (obj->highlight_style != NULL && !Z_ISUNDEF(obj->highlight_fields)) {
        PCBC_ZVAL highlight;

        PCBC_ZVAL_ALLOC(highlight);
        array_init(PCBC_P(highlight));
        ADD_ASSOC_ZVAL_EX(return_value, "highlight", PCBC_P(highlight));

        ADD_ASSOC_STRING(PCBC_P(highlight), "style", obj->highlight_style);
        ADD_ASSOC_ZVAL_EX(PCBC_P(highlight), "fields", PCBC_P(obj->highlight_fields));
        PCBC_ADDREF_P(PCBC_P(obj->highlight_fields));
    }
    if (!Z_ISUNDEF(obj->query_part)) {
        ADD_ASSOC_ZVAL_EX(return_value, "query", PCBC_P(obj->query_part));
        PCBC_ADDREF_P(PCBC_P(obj->query_part));
    }
} /* }}} */

ZEND_BEGIN_ARG_INFO_EX(ai_SearchQuery_none, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_SearchQuery_construct, 0, 0, 2)
ZEND_ARG_INFO(0, indexName)
ZEND_ARG_INFO(0, queryPart)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_SearchQuery_booleanField, 0, 0, 1)
ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_SearchQuery_conjuncts, 0, 0, 1)
PCBC_ARG_VARIADIC_INFO(0, queries)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_SearchQuery_disjuncts, 0, 0, 1)
PCBC_ARG_VARIADIC_INFO(0, queries)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_SearchQuery_docId, 0, 0, 1)
PCBC_ARG_VARIADIC_INFO(0, documentIds)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_SearchQuery_match, 0, 0, 1)
ZEND_ARG_INFO(0, match)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_SearchQuery_matchPhrase, 0, 0, 1)
ZEND_ARG_INFO(0, matchPhrase)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_SearchQuery_phrase, 0, 0, 1)
PCBC_ARG_VARIADIC_INFO(0, terms)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_SearchQuery_prefix, 0, 0, 1)
ZEND_ARG_INFO(0, prefix)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_SearchQuery_wildcard, 0, 0, 1)
ZEND_ARG_INFO(0, wildcard)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_SearchQuery_queryString, 0, 0, 1)
ZEND_ARG_INFO(0, queryString)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_SearchQuery_regexp, 0, 0, 1)
ZEND_ARG_INFO(0, regexp)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_SearchQuery_term, 0, 0, 1)
ZEND_ARG_INFO(0, term)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_SearchQuery_geoBoundingBox, 0, 0, 4)
ZEND_ARG_INFO(0, topLeftLongitude)
ZEND_ARG_INFO(0, topLeftLatitude)
ZEND_ARG_INFO(0, bottomRightLongitude)
ZEND_ARG_INFO(0, bottomRightLatitude)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_SearchQuery_geoDistance, 0, 0, 3)
ZEND_ARG_INFO(0, longitude)
ZEND_ARG_INFO(0, latitude)
ZEND_ARG_INFO(0, distance)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_SearchQuery_limit, 0, 0, 1)
ZEND_ARG_INFO(0, limit)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_SearchQuery_skip, 0, 0, 1)
ZEND_ARG_INFO(0, skip)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_SearchQuery_explain, 0, 0, 1)
ZEND_ARG_INFO(0, explain)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_SearchQuery_consistentWith, 0, 0, 1)
ZEND_ARG_INFO(0, mutationState)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_SearchQuery_serverSideTimeout, 0, 0, 1)
ZEND_ARG_INFO(0, serverSideTimeout)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_SearchQuery_fields, 0, 0, 1)
PCBC_ARG_VARIADIC_INFO(0, fields)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_SearchQuery_sort, 0, 0, 1)
PCBC_ARG_VARIADIC_INFO(0, sort)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_SearchQuery_highlight, 0, 0, 2)
ZEND_ARG_INFO(0, style)
PCBC_ARG_VARIADIC_INFO(0, fields)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_SearchQuery_facet, 0, 0, 2)
ZEND_ARG_INFO(0, field)
ZEND_ARG_INFO(0, limit)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_SearchQuery_addFacet, 0, 0, 2)
ZEND_ARG_INFO(0, name)
ZEND_ARG_INFO(0, facet)
ZEND_END_ARG_INFO()

// clang-format off
zend_function_entry search_query_methods[] = {
    PHP_ME(SearchQuery, __construct, ai_SearchQuery_construct, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
    PHP_ME(SearchQuery, jsonSerialize, ai_SearchQuery_none, ZEND_ACC_PUBLIC)
    PHP_ME(SearchQuery, booleanField, ai_SearchQuery_booleanField, ZEND_ACC_STATIC | ZEND_ACC_PUBLIC)
    PHP_ME(SearchQuery, boolean, ai_SearchQuery_none, ZEND_ACC_STATIC | ZEND_ACC_PUBLIC)
    PHP_ME(SearchQuery, conjuncts, ai_SearchQuery_conjuncts, ZEND_ACC_STATIC | ZEND_ACC_PUBLIC)
    PHP_ME(SearchQuery, dateRange, ai_SearchQuery_none, ZEND_ACC_STATIC | ZEND_ACC_PUBLIC)
    PHP_ME(SearchQuery, disjuncts, ai_SearchQuery_disjuncts, ZEND_ACC_STATIC | ZEND_ACC_PUBLIC)
    PHP_ME(SearchQuery, docId, ai_SearchQuery_docId, ZEND_ACC_STATIC | ZEND_ACC_PUBLIC)
    PHP_ME(SearchQuery, match, ai_SearchQuery_match, ZEND_ACC_STATIC | ZEND_ACC_PUBLIC)
    PHP_ME(SearchQuery, matchAll, ai_SearchQuery_none, ZEND_ACC_STATIC | ZEND_ACC_PUBLIC)
    PHP_ME(SearchQuery, matchNone, ai_SearchQuery_none, ZEND_ACC_STATIC | ZEND_ACC_PUBLIC)
    PHP_ME(SearchQuery, matchPhrase, ai_SearchQuery_matchPhrase, ZEND_ACC_STATIC | ZEND_ACC_PUBLIC)
    PHP_ME(SearchQuery, numericRange, ai_SearchQuery_none, ZEND_ACC_STATIC | ZEND_ACC_PUBLIC)
    PHP_ME(SearchQuery, phrase, ai_SearchQuery_phrase, ZEND_ACC_STATIC | ZEND_ACC_PUBLIC)
    PHP_ME(SearchQuery, prefix, ai_SearchQuery_prefix, ZEND_ACC_STATIC | ZEND_ACC_PUBLIC)
    PHP_ME(SearchQuery, queryString, ai_SearchQuery_queryString, ZEND_ACC_STATIC | ZEND_ACC_PUBLIC)
    PHP_ME(SearchQuery, regexp, ai_SearchQuery_regexp, ZEND_ACC_STATIC | ZEND_ACC_PUBLIC)
    PHP_ME(SearchQuery, term, ai_SearchQuery_term, ZEND_ACC_STATIC | ZEND_ACC_PUBLIC)
    PHP_ME(SearchQuery, termRange, ai_SearchQuery_none, ZEND_ACC_STATIC | ZEND_ACC_PUBLIC)
    PHP_ME(SearchQuery, geoBoundingBox, ai_SearchQuery_geoBoundingBox, ZEND_ACC_STATIC | ZEND_ACC_PUBLIC)
    PHP_ME(SearchQuery, geoDistance, ai_SearchQuery_geoDistance, ZEND_ACC_STATIC | ZEND_ACC_PUBLIC)
    PHP_ME(SearchQuery, wildcard, ai_SearchQuery_wildcard, ZEND_ACC_STATIC | ZEND_ACC_PUBLIC)
    PHP_ME(SearchQuery, termFacet, ai_SearchQuery_facet, ZEND_ACC_STATIC | ZEND_ACC_PUBLIC)
    PHP_ME(SearchQuery, dateRangeFacet, ai_SearchQuery_facet, ZEND_ACC_STATIC | ZEND_ACC_PUBLIC)
    PHP_ME(SearchQuery, numericRangeFacet, ai_SearchQuery_facet, ZEND_ACC_STATIC | ZEND_ACC_PUBLIC)
    PHP_ME(SearchQuery, limit, ai_SearchQuery_limit, ZEND_ACC_PUBLIC)
    PHP_ME(SearchQuery, skip, ai_SearchQuery_skip, ZEND_ACC_PUBLIC)
    PHP_ME(SearchQuery, explain, ai_SearchQuery_explain, ZEND_ACC_PUBLIC)
    PHP_ME(SearchQuery, serverSideTimeout, ai_SearchQuery_serverSideTimeout, ZEND_ACC_PUBLIC)
    PHP_ME(SearchQuery, consistentWith, ai_SearchQuery_consistentWith, ZEND_ACC_PUBLIC)
    PHP_ME(SearchQuery, fields, ai_SearchQuery_fields, ZEND_ACC_PUBLIC)
    PHP_ME(SearchQuery, sort, ai_SearchQuery_sort, ZEND_ACC_PUBLIC)
    PHP_ME(SearchQuery, highlight, ai_SearchQuery_highlight, ZEND_ACC_PUBLIC)
    PHP_ME(SearchQuery, addFacet, ai_SearchQuery_addFacet, ZEND_ACC_PUBLIC)
    PHP_FE_END
};
// clang-format on

zend_object_handlers search_query_handlers;

static void search_query_free_object(pcbc_free_object_arg *object TSRMLS_DC) /* {{{ */
{
    pcbc_search_query_t *obj = Z_SEARCH_QUERY_OBJ(object);

    if (obj->index_name != NULL) {
        efree(obj->index_name);
    }
    if (obj->highlight_style != NULL) {
        efree(obj->highlight_style);
    }
    if (!Z_ISUNDEF(obj->query_part)) {
        zval_ptr_dtor(&obj->query_part);
        ZVAL_UNDEF(PCBC_P(obj->query_part));
    }
    if (!Z_ISUNDEF(obj->consistency)) {
        zval_ptr_dtor(&obj->consistency);
        ZVAL_UNDEF(PCBC_P(obj->consistency));
    }
    if (!Z_ISUNDEF(obj->fields)) {
        zval_ptr_dtor(&obj->fields);
        ZVAL_UNDEF(PCBC_P(obj->fields));
    }
    if (!Z_ISUNDEF(obj->sort)) {
        zval_ptr_dtor(&obj->sort);
        ZVAL_UNDEF(PCBC_P(obj->sort));
    }
    if (!Z_ISUNDEF(obj->facets)) {
        zval_ptr_dtor(&obj->facets);
        ZVAL_UNDEF(PCBC_P(obj->facets));
    }
    if (!Z_ISUNDEF(obj->highlight_fields)) {
        zval_ptr_dtor(&obj->highlight_fields);
        ZVAL_UNDEF(PCBC_P(obj->highlight_fields));
    }

    zend_object_std_dtor(&obj->std TSRMLS_CC);
#if PHP_VERSION_ID < 70000
    efree(obj);
#endif
} /* }}} */

static pcbc_create_object_retval search_query_create_object(zend_class_entry *class_type TSRMLS_DC)
{
    pcbc_search_query_t *obj = NULL;

    obj = PCBC_ALLOC_OBJECT_T(pcbc_search_query_t, class_type);

    zend_object_std_init(&obj->std, class_type TSRMLS_CC);
    object_properties_init(&obj->std, class_type);

#if PHP_VERSION_ID >= 70000
    obj->std.handlers = &search_query_handlers;
    return &obj->std;
#else
    {
        zend_object_value ret;
        ret.handle = zend_objects_store_put(obj, (zend_objects_store_dtor_t)zend_objects_destroy_object,
                                            search_query_free_object, NULL TSRMLS_CC);
        ret.handlers = &search_query_handlers;
        return ret;
    }
#endif
}

static HashTable *pcbc_search_query_get_debug_info(zval *object, int *is_temp TSRMLS_DC) /* {{{ */
{
    pcbc_search_query_t *obj = NULL;
#if PHP_VERSION_ID >= 70000
    zval retval;
#else
    zval retval = zval_used_for_init;
#endif

    *is_temp = 1;
    obj = Z_SEARCH_QUERY_OBJ_P(object);

    array_init(&retval);
    ADD_ASSOC_STRING(&retval, "indexName", obj->index_name);
    if (obj->explain == 0) {
        ADD_ASSOC_BOOL_EX(&retval, "explain", obj->explain);
    }
    if (obj->limit >= 0) {
        ADD_ASSOC_LONG_EX(&retval, "limit", obj->limit);
    }
    if (obj->skip >= 0) {
        ADD_ASSOC_LONG_EX(&retval, "skip", obj->skip);
    }
    if (obj->server_side_timeout >= 0) {
        ADD_ASSOC_LONG_EX(&retval, "serverSideTimeout", obj->server_side_timeout);
    }
    if (!Z_ISUNDEF(obj->fields)) {
        ADD_ASSOC_ZVAL_EX(&retval, "fields", PCBC_P(obj->fields));
        PCBC_ADDREF_P(PCBC_P(obj->fields));
    }
    if (!Z_ISUNDEF(obj->sort)) {
        ADD_ASSOC_ZVAL_EX(&retval, "sort", PCBC_P(obj->sort));
        PCBC_ADDREF_P(PCBC_P(obj->sort));
    }
    if (!Z_ISUNDEF(obj->facets)) {
        ADD_ASSOC_ZVAL_EX(&retval, "facets", PCBC_P(obj->facets));
        PCBC_ADDREF_P(PCBC_P(obj->facets));
    }
    if (obj->highlight_style != NULL) {
        ADD_ASSOC_STRING(&retval, "highlightStyle", obj->highlight_style);
    }
    if (!Z_ISUNDEF(obj->highlight_fields)) {
        ADD_ASSOC_ZVAL_EX(&retval, "highlightFields", PCBC_P(obj->highlight_fields));
        PCBC_ADDREF_P(PCBC_P(obj->highlight_fields));
    }
    if (!Z_ISUNDEF(obj->consistency)) {
        ADD_ASSOC_ZVAL_EX(&retval, "consistency", PCBC_P(obj->consistency));
        PCBC_ADDREF_P(PCBC_P(obj->consistency));
    }
    if (!Z_ISUNDEF(obj->query_part)) {
        ADD_ASSOC_ZVAL_EX(&retval, "queryPart", PCBC_P(obj->query_part));
        PCBC_ADDREF_P(PCBC_P(obj->query_part));
    } else {
        ADD_ASSOC_NULL_EX(&retval, "queryPart");
    }

    return Z_ARRVAL(retval);
} /* }}} */

PHP_MINIT_FUNCTION(SearchQuery)
{
    zend_class_entry ce;

    INIT_NS_CLASS_ENTRY(ce, "Couchbase", "SearchQuery", search_query_methods);
    pcbc_search_query_ce = zend_register_internal_class(&ce TSRMLS_CC);
    pcbc_search_query_ce->create_object = search_query_create_object;
    PCBC_CE_DISABLE_SERIALIZATION(pcbc_search_query_ce);

    zend_class_implements(pcbc_search_query_ce TSRMLS_CC, 1, pcbc_json_serializable_ce);

    zend_declare_class_constant_stringl(pcbc_search_query_ce, ZEND_STRL("HIGHLIGHT_HTML"), ZEND_STRL("html") TSRMLS_CC);
    zend_declare_class_constant_stringl(pcbc_search_query_ce, ZEND_STRL("HIGHLIGHT_ANSI"), ZEND_STRL("ansi") TSRMLS_CC);
    zend_declare_class_constant_stringl(pcbc_search_query_ce, ZEND_STRL("HIGHLIGHT_SIMPLE"),
                                        ZEND_STRL("simple") TSRMLS_CC);

    memcpy(&search_query_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    search_query_handlers.get_debug_info = pcbc_search_query_get_debug_info;
#if PHP_VERSION_ID >= 70000
    search_query_handlers.free_obj = search_query_free_object;
    search_query_handlers.offset = XtOffsetOf(pcbc_search_query_t, std);
#endif

    zend_register_class_alias("\\CouchbaseSearchQuery", pcbc_search_query_ce);
    return SUCCESS;
}
