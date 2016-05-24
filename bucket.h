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

#ifndef BUCKET_H_
#define BUCKET_H_

#include <php.h>
#include "couchbase.h"
#include "zap.h"
#include "opcookie.h"

typedef struct bucket_object {
    zap_ZEND_OBJECT_START

    zapval encoder;
    zapval decoder;
    zapval prefix;
    pcbc_lcb *conn;

    zap_ZEND_OBJECT_END
} bucket_object;

#define _PCBC_CHECK_ZVAL(v,t,m) \
	if (v && Z_TYPE_P(v) != t) { \
		throw_pcbc_exception(m, LCB_EINVAL); \
		RETURN_NULL(); \
	}
#define PCBC_CHECK_ZVAL_STRING(v, m) \
    _PCBC_CHECK_ZVAL(v, IS_STRING, m)
#define PCBC_CHECK_ZVAL_LONG(v, m) \
    _PCBC_CHECK_ZVAL(v, IS_LONG, m)
#define PCBC_CHECK_ZVAL_BOOL(v, m) \
    if (v && !zap_zval_is_bool(v)) {      \
        throw_pcbc_exception(m, LCB_EINVAL); \
        RETURN_NULL(); \
    }
#define PCBC_CHECK_ZVAL_ARRAY(v, m) \
    _PCBC_CHECK_ZVAL(v, IS_ARRAY, m)

#define PCBC_PHP_THISOBJ() zap_fetch_this(bucket_object)

#define pcbc_assert_number_of_commands(cmd, nscheduled, ntotal)         \
    if (nscheduled != ntotal) {                                         \
        php_error_docref(NULL TSRMLS_CC, E_WARNING,                     \
                         "Failed to schedule %s commands (%d out of %d sent)", \
                         cmd, nscheduled, ntotal);                      \
    }

zval* bop_get_return_doc(zval *return_value, zapval *key, int is_mapped);

PHP_METHOD(Bucket, get);
PHP_METHOD(Bucket, getFromReplica);
void get_callback(lcb_t instance, int cbtype, const lcb_RESPBASE *rb);

PHP_METHOD(Bucket, unlock);
void unlock_callback(lcb_t instance, int cbtype, const lcb_RESPBASE *rb);

PHP_METHOD(Bucket, store);
void store_callback(lcb_t instance, int cbtype, const lcb_RESPBASE *rb);
typedef struct {
    opcookie_res header;
    zapval key;
    zapval cas;
} opcookie_store_res;
lcb_error_t proc_store_results(bucket_object *bucket, zval *return_value, opcookie *cookie, int is_mapped TSRMLS_DC);

PHP_METHOD(Bucket, remove);
void remove_callback(lcb_t instance, int cbtype, const lcb_RESPBASE *rb);
#define proc_touch_results proc_store_results

PHP_METHOD(Bucket, touch);
void touch_callback(lcb_t instance, int cbtype, const lcb_RESPBASE *rb);
#define proc_remove_results proc_store_results

PHP_METHOD(Bucket, counter);
void counter_callback(lcb_t instance, int cbtype, const lcb_RESPBASE *rb);

PHP_METHOD(Bucket, http_request);
void http_callback(lcb_t instance, int cbtype, const lcb_RESPBASE *rb);

PHP_METHOD(Bucket, subdoc_request);
void subdoc_callback(lcb_t instance, int cbtype, const lcb_RESPBASE *rb);

PHP_METHOD(Bucket, insert);
PHP_METHOD(Bucket, upsert);
PHP_METHOD(Bucket, replace);
PHP_METHOD(Bucket, append);
PHP_METHOD(Bucket, prepend);
void store_callback(lcb_t instance, int cbtype, const lcb_RESPBASE *rb);

PHP_METHOD(Bucket, n1ql_request);

PHP_METHOD(Bucket, durability);
void durability_callback(lcb_t instance, const void *cookie, lcb_error_t error, const lcb_durability_resp_t *resp);

PHP_METHOD(Bucket, n1ix_list);
PHP_METHOD(Bucket, n1ix_create);
PHP_METHOD(Bucket, n1ix_drop);

#endif // BUCKET_H_
