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

#include "couchbase.h"
#include <libcouchbase/couchbase.h>
#include "zap.h"

zend_class_entry *default_exception_ce;
zend_class_entry *cb_exception_ce;

void make_exception(zapval *ex, zend_class_entry *exception_ce, const char *message, long code TSRMLS_DC)
{
    zapval_alloc(*ex);
    object_init_ex(zapval_zvalptr_p(ex), cb_exception_ce);

    if (message) {
        zend_update_property_string(
            cb_exception_ce,
            zapval_zvalptr_p(ex),
            "message", sizeof("message")-1,
            message TSRMLS_CC);
    }
    if (code) {
        zend_update_property_long(
            cb_exception_ce,
            zapval_zvalptr_p(ex),
            "code", sizeof("code")-1,
            code TSRMLS_CC);
    }
}

void make_pcbc_exception(zapval *ex, const char *message, long code TSRMLS_DC)
{
    make_exception(ex, cb_exception_ce, message, code TSRMLS_CC);
}


static const char *cb_strerror(lcb_error_t error)
{
#define X(c, v, t, s) if (error == c) { return #c ": " s; }
    LCB_XERR(X)
#undef X

        return lcb_strerror(NULL, error);
}

void make_lcb_exception(zapval *ex, long code, const char *msg TSRMLS_DC)
{
    if (msg) {
        make_exception(ex, cb_exception_ce, msg, code TSRMLS_CC);
    } else {
        const char *str = cb_strerror((lcb_error_t)code);
        make_exception(ex, cb_exception_ce, str, code TSRMLS_CC);
    }
}

#define setup(var, name, parent)                                        \
    do {                                                                \
        zend_class_entry cbe;                                           \
        INIT_CLASS_ENTRY(cbe, name, NULL);                              \
        var = zap_zend_register_internal_class_ex(&cbe, parent);        \
    } while(0)

void couchbase_init_exceptions(INIT_FUNC_ARGS)
{
    default_exception_ce = (zend_class_entry *)zap_zend_exception_get_default();

    setup(cb_exception_ce, "CouchbaseException", default_exception_ce);
}
