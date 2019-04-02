/**
 *     Copyright 2019 Couchbase, Inc.
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
 * Base class for all FTS sort options in querying.
 */

#include "couchbase.h"

zend_class_entry *pcbc_search_sort_ce;

/* {{{ proto void SearchSort::__construct()
   Should not be called directly */
PHP_METHOD(SearchSort, __construct)
{
    throw_pcbc_exception("Accessing private constructor.", LCB_EINVAL);
}

/* {{{ proto \Couchbase\SearchSortId SearchSort::id() */
PHP_METHOD(SearchSort, id)
{
    int rv;

    rv = zend_parse_parameters_none();
    if (rv == FAILURE) {
        return;
    }

    pcbc_search_sort_id_init(return_value TSRMLS_CC);
} /* }}} */

/* {{{ proto \Couchbase\SearchSortScore SearchSort::score() */
PHP_METHOD(SearchSort, score)
{
    int rv;

    rv = zend_parse_parameters_none();
    if (rv == FAILURE) {
        return;
    }

    pcbc_search_sort_score_init(return_value TSRMLS_CC);
} /* }}} */

/* {{{ proto \Couchbase\SearchSortField SearchSort::field(string $field) */
PHP_METHOD(SearchSort, field)
{
    char *field = NULL;
    size_t field_len = 0;
    long limit;
    int rv;

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &field, &field_len);
    if (rv == FAILURE) {
        return;
    }
    if (rv == FAILURE) {
        return;
    }

    pcbc_search_sort_field_init(return_value, field, field_len TSRMLS_CC);
} /* }}} */

/* {{{ proto \Couchbase\SearchSortField SearchSort::geoDistance(string $field, double $longitude, double $latitude) */
PHP_METHOD(SearchSort, geoDistance)
{
    char *field = NULL;
    size_t field_len = 0;
    double lon = 0, lat = 0;
    long limit;
    int rv;

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sdd", &field, &field_len, &lon, &lat);
    if (rv == FAILURE) {
        return;
    }
    if (rv == FAILURE) {
        return;
    }

    pcbc_search_sort_geo_distance_init(return_value, field, field_len, lon, lat TSRMLS_CC);
} /* }}} */

ZEND_BEGIN_ARG_INFO_EX(ai_SearchSort_none, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_SearchSort_id, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_SearchSort_score, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_SearchSort_field, 0, 0, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_SearchSort_geoDistance, 0, 0, 3)
ZEND_END_ARG_INFO()

// clang-format off
static const zend_function_entry search_sort_methods[] = {
    PHP_ME(SearchSort, __construct, ai_SearchSort_none, ZEND_ACC_PRIVATE | ZEND_ACC_CTOR)
    PHP_ME(SearchSort, id, ai_SearchSort_id, ZEND_ACC_STATIC | ZEND_ACC_PUBLIC)
    PHP_ME(SearchSort, score, ai_SearchSort_score, ZEND_ACC_STATIC | ZEND_ACC_PUBLIC)
    PHP_ME(SearchSort, field, ai_SearchSort_field, ZEND_ACC_STATIC | ZEND_ACC_PUBLIC)
    PHP_ME(SearchSort, geoDistance, ai_SearchSort_geoDistance, ZEND_ACC_STATIC | ZEND_ACC_PUBLIC)
    PHP_FE_END
};
// clang-format on

PHP_MINIT_FUNCTION(SearchSort)
{
    zend_class_entry ce;
    INIT_NS_CLASS_ENTRY(ce, "Couchbase", "SearchSort", search_sort_methods);
    pcbc_search_sort_ce = zend_register_internal_class(&ce TSRMLS_CC);
    return SUCCESS;
}
