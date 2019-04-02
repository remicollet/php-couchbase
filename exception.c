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
#include <libcouchbase/couchbase.h>

zend_class_entry *pcbc_exception_ce;

static void pcbc_exception_make(zval *return_value, zend_class_entry *exception_ce, long code,
                                const char *message TSRMLS_DC)
{
    object_init_ex(return_value, pcbc_exception_ce);

    if (message) {
        zend_update_property_string(pcbc_exception_ce, return_value, ZEND_STRL("message"), message TSRMLS_CC);
    }
    if (code) {
        zend_update_property_long(pcbc_exception_ce, return_value, ZEND_STRL("code"), code TSRMLS_CC);
    }
}

const char *pcbc_lcb_strerror(lcb_error_t error)
{
#define X(c, v, t, s)                                                                                                  \
    if (error == c) {                                                                                                  \
        return #c ": " s;                                                                                              \
    }
    LCB_XERR(X)
#undef X

    return lcb_strerror(NULL, error);
}

void pcbc_exception_init(zval *return_value, long code, const char *message TSRMLS_DC)
{
    pcbc_exception_make(return_value, pcbc_exception_ce, code, message TSRMLS_CC);
}

void pcbc_exception_init_lcb(zval *return_value, long code, const char *message, const char *ctx,
                             const char *ref TSRMLS_DC)
{
    if (!message) {
        message = pcbc_lcb_strerror((lcb_error_t)code);
    }
    pcbc_exception_make(return_value, pcbc_exception_ce, code, message TSRMLS_CC);
    if (ctx) {
        zend_update_property_string(pcbc_exception_ce, return_value, ZEND_STRL("context"), ctx TSRMLS_CC);
    }
    if (ref) {
        zend_update_property_string(pcbc_exception_ce, return_value, ZEND_STRL("ref"), ref TSRMLS_CC);
    }
}

PHP_MINIT_FUNCTION(CouchbaseException)
{
    zend_class_entry ce;
    zend_class_entry *default_exception_ce;

    INIT_NS_CLASS_ENTRY(ce, "Couchbase", "Exception", NULL);

    default_exception_ce = (zend_class_entry *)zend_exception_get_default(TSRMLS_C);

    pcbc_exception_ce = zend_register_internal_class_ex(&ce, default_exception_ce TSRMLS_CC);

    zend_register_class_alias("\\CouchbaseException", pcbc_exception_ce);
    return SUCCESS;
}
