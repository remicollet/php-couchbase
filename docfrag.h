#ifndef DOCFRAG_H_
#define DOCFRAG_H_

#include <php.h>
#include <libcouchbase/couchbase.h>
#include "bucket.h"

void couchbase_init_docfrag(INIT_FUNC_ARGS);

int pcbc_make_docfrag(zval *doc, zapval *value, zapval *cas TSRMLS_DC);
int pcbc_make_docfrag_error(zval *doc, lcb_error_t err, zapval *value TSRMLS_DC);

#endif // DOCFRAG_H_
