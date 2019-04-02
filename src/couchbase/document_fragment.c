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

#include "couchbase.h"

zend_class_entry *pcbc_document_fragment_ce;

// clang-format off
zend_function_entry docfrag_methods[] = {
    PHP_FE_END
};
// clang-format on

int pcbc_document_fragment_init(zval *return_value, zval *value, zval *cas, zval *token TSRMLS_DC)
{
    object_init_ex(return_value, pcbc_document_fragment_ce);

    if (value) {
        zend_update_property(pcbc_document_fragment_ce, return_value, "value", sizeof("value") - 1, value TSRMLS_CC);
    }
    if (cas && !Z_ISUNDEF_P(cas)) {
        zend_update_property(pcbc_document_fragment_ce, return_value, "cas", sizeof("cas") - 1, cas TSRMLS_CC);
    }
    if (token && !Z_ISUNDEF_P(token)) {
        zend_update_property(pcbc_document_fragment_ce, return_value, "token", sizeof("token") - 1, token TSRMLS_CC);
    }

    return SUCCESS;
}

int pcbc_document_fragment_init_error(zval *return_value, opcookie_res *header, zval *value TSRMLS_DC)
{
    zval error;

    object_init_ex(return_value, pcbc_document_fragment_ce);
    ZVAL_UNDEF(&error);
    pcbc_exception_init_lcb(&error, header->err, NULL, header->err_ctx, header->err_ref TSRMLS_CC);
    zend_update_property(pcbc_document_fragment_ce, return_value, "error", sizeof("error") - 1, &error TSRMLS_CC);
    if (value) {
        zend_update_property(pcbc_document_fragment_ce, return_value, "value", sizeof("value") - 1, value TSRMLS_CC);
    }

    zval_ptr_dtor(&error);
    return SUCCESS;
}

PHP_MINIT_FUNCTION(DocumentFragment)
{
    zend_class_entry ce;

    INIT_NS_CLASS_ENTRY(ce, "Couchbase", "DocumentFragment", docfrag_methods);
    pcbc_document_fragment_ce = zend_register_internal_class(&ce TSRMLS_CC);

    zend_declare_property_null(pcbc_document_fragment_ce, "error", strlen("error"), ZEND_ACC_PUBLIC TSRMLS_CC);
    zend_declare_property_null(pcbc_document_fragment_ce, "cas", strlen("cas"), ZEND_ACC_PUBLIC TSRMLS_CC);
    zend_declare_property_null(pcbc_document_fragment_ce, "value", strlen("value"), ZEND_ACC_PUBLIC TSRMLS_CC);
    zend_declare_property_null(pcbc_document_fragment_ce, "token", strlen("token"), ZEND_ACC_PUBLIC TSRMLS_CC);

    zend_register_class_alias("\\CouchbaseDocumentFragment", pcbc_document_fragment_ce);
    return SUCCESS;
}
