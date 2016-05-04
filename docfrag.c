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
