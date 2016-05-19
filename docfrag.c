/**
 *     Copyright 2016 Couchbase, Inc.
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

#include "bucket.h"
#include "cas.h"
#include "transcoding.h"
#include "exception.h"

zend_class_entry *docfrag_ce;

zend_function_entry docfrag_methods[] = {
    { NULL, NULL, NULL }
};

void couchbase_init_docfrag(INIT_FUNC_ARGS) {
    zend_class_entry ce;

    INIT_CLASS_ENTRY(ce, "CouchbaseDocumentFragment", docfrag_methods);
    docfrag_ce = zend_register_internal_class(&ce TSRMLS_CC);

    zend_declare_property_null(docfrag_ce, "error", strlen("error"), ZEND_ACC_PUBLIC TSRMLS_CC);
    zend_declare_property_null(docfrag_ce, "cas", strlen("cas"), ZEND_ACC_PUBLIC TSRMLS_CC);
    zend_declare_property_null(docfrag_ce, "value", strlen("value"), ZEND_ACC_PUBLIC TSRMLS_CC);
}

int pcbc_make_docfrag(zval *doc, zapval *value, zapval *cas TSRMLS_DC)
{
    object_init_ex(doc, docfrag_ce);

    if (value) {
        zend_update_property(docfrag_ce, doc,
                             "value", sizeof("value") - 1, zapval_zvalptr_p(value) TSRMLS_CC);
    }
    if (cas) {
        zend_update_property(docfrag_ce, doc,
                             "cas", sizeof("cas") - 1, zapval_zvalptr_p(cas) TSRMLS_CC);
    }

    return SUCCESS;
}

int pcbc_make_docfrag_error(zval *doc, lcb_error_t err, zapval *value TSRMLS_DC)
{
    zapval zerror;

    object_init_ex(doc, docfrag_ce);
    make_lcb_exception(&zerror, err, NULL TSRMLS_CC);
    zend_update_property(docfrag_ce, doc, "error", sizeof("error") - 1,
        zapval_zvalptr(zerror) TSRMLS_CC);
    if (value) {
        zend_update_property(docfrag_ce, doc,
                             "value", sizeof("value") - 1, zapval_zvalptr_p(value) TSRMLS_CC);
    }

    zapval_destroy(zerror);
    return SUCCESS;
}
