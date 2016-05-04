#ifndef CAS_H_
#define CAS_H_

#include <php.h>
#include <libcouchbase/couchbase.h>
#include "zap.h"

lcb_cas_t cas_decode(zval *zcas TSRMLS_DC);
void cas_encode(zapval *casout, lcb_cas_t value TSRMLS_DC);

#endif // CAS_H_
