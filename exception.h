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

#ifndef EXCEPTION_H_
#define EXCEPTION_H_

#include "zap.h"

void make_exception(zapval *ex, zend_class_entry *exception_ce, const char *message, long code TSRMLS_DC);
void make_pcbc_exception(zapval *ex, const char *message, long code TSRMLS_DC);
void make_lcb_exception(zapval *ex, long code, const char *msg TSRMLS_DC);

#define throw_pcbc_exception(message, code) { \
    zapval zerror; \
    make_pcbc_exception(&zerror, message, code TSRMLS_CC); \
    zap_throw_exception_object(zerror); }
#define throw_lcb_exception(code) { \
    zapval zerror; \
    make_lcb_exception(&zerror, code, NULL TSRMLS_CC); \
    zap_throw_exception_object(zerror); }

extern zend_class_entry *default_exception_ce;
extern zend_class_entry *cb_exception_ce;

#endif
