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

zend_class_entry *n1ix_spec_ce;

/* {{{ proto void N1qlIndex::__construct() Should not be called directly */
PHP_METHOD(N1qlIndex, __construct) { throw_pcbc_exception("Accessing private constructor.", LCB_EINVAL); }
/* }}} */

ZEND_BEGIN_ARG_INFO_EX(ai_N1qlIndex_none, 0, 0, 0)
ZEND_END_ARG_INFO()

zend_function_entry n1ix_spec_methods[] = {
    PHP_ME(N1qlIndex, __construct, ai_N1qlIndex_none, ZEND_ACC_PRIVATE | ZEND_ACC_FINAL | ZEND_ACC_CTOR) PHP_FE_END};

#define PROPERTIES(X)                                                                                                  \
    X("name", name)                                                                                                    \
    X("isPrimary", is_primary)                                                                                         \
    X("type", type)                                                                                                    \
    X("state", state)                                                                                                  \
    X("keyspace", keyspace)                                                                                            \
    X("namespace", namespace)                                                                                          \
    X("fields", fields)                                                                                                \
    X("condition", condition)

PHP_MINIT_FUNCTION(N1qlIndex)
{
    zend_class_entry ce;

    INIT_NS_CLASS_ENTRY(ce, "Couchbase", "N1qlIndex", n1ix_spec_methods);
    n1ix_spec_ce = zend_register_internal_class(&ce TSRMLS_CC);

#define X(name, var) zend_declare_property_null(n1ix_spec_ce, name, strlen(name), ZEND_ACC_PUBLIC TSRMLS_CC);
    PROPERTIES(X)
#undef X

    zend_declare_class_constant_long(n1ix_spec_ce, ZEND_STRL("VIEW"), LCB_N1XSPEC_T_VIEW TSRMLS_CC);
    zend_declare_class_constant_long(n1ix_spec_ce, ZEND_STRL("GSI"), LCB_N1XSPEC_T_GSI TSRMLS_CC);
    zend_declare_class_constant_long(n1ix_spec_ce, ZEND_STRL("UNSPECIFIED"), LCB_N1XSPEC_T_DEFAULT TSRMLS_CC);

    return SUCCESS;
}

int pcbc_n1ix_init(zval *return_value, zval *json TSRMLS_DC)
{
    zval *val;

    object_init_ex(return_value, n1ix_spec_ce);

    val = php_array_fetch(json, "name");
    if (val) {
        zend_update_property(n1ix_spec_ce, return_value, ZEND_STRL("name"), val TSRMLS_CC);
    }

    val = php_array_fetch(json, "is_primary");
    if (val) {
        zend_update_property(n1ix_spec_ce, return_value, ZEND_STRL("isPrimary"), val TSRMLS_CC);
    }

    {
        PCBC_ZVAL type;
        char *str;
        int str_len = 0;
        zend_bool owned = 0;

        PCBC_ZVAL_ALLOC(type);
        str = php_array_fetch_string(json, "using", &str_len, &owned);
        if (str) {
            if (strcmp(str, "view") == 0) {
                ZVAL_LONG(PCBC_P(type), LCB_N1XSPEC_T_VIEW);
            } else if (strcmp(str, "gsi") == 0) {
                ZVAL_LONG(PCBC_P(type), LCB_N1XSPEC_T_GSI);
            } else {
                ZVAL_LONG(PCBC_P(type), LCB_N1XSPEC_T_DEFAULT);
            }
            if (owned) {
                efree(str);
            }
        } else {
            ZVAL_LONG(PCBC_P(type), LCB_N1XSPEC_T_DEFAULT);
        }
        zend_update_property(n1ix_spec_ce, return_value, ZEND_STRL("type"), PCBC_P(type) TSRMLS_CC);
        zval_ptr_dtor(&type);
    }

    val = php_array_fetch(json, "state");
    if (val) {
        zend_update_property(n1ix_spec_ce, return_value, ZEND_STRL("state"), val TSRMLS_CC);
    }
    val = php_array_fetch(json, "keyspace_id");
    if (val) {
        zend_update_property(n1ix_spec_ce, return_value, ZEND_STRL("keyspace"), val TSRMLS_CC);
    }
    val = php_array_fetch(json, "namespace_id");
    if (val) {
        zend_update_property(n1ix_spec_ce, return_value, ZEND_STRL("namespace"), val TSRMLS_CC);
    }
    val = php_array_fetch(json, "index_key");
    if (val) {
        zend_update_property(n1ix_spec_ce, return_value, ZEND_STRL("fields"), val TSRMLS_CC);
    }
    val = php_array_fetch(json, "condition");
    if (val) {
        zend_update_property(n1ix_spec_ce, return_value, ZEND_STRL("condition"), val TSRMLS_CC);
    }

    return SUCCESS;
}
