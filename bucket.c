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
#include "ext/standard/php_var.h"
#include "ext/json/php_json.h"
#include "exception.h"
#include "datainfo.h"
#include "paramparser.h"
#include "zap.h"
#include "bucket.h"
#include "cas.h"
#include "metadoc.h"
#include "docfrag.h"
#include "transcoding.h"
#include "opcookie.h"

extern struct pcbc_logger_st pcbc_logger;
#define LOGARGS(instance, lvl) LCB_LOG_##lvl, instance, "pcbc/bucket", __FILE__, __LINE__

zap_class_entry bucket_class;
zend_class_entry *bucket_ce;
char *pcbc_client_string = "PCBC/"PHP_COUCHBASE_VERSION;

zap_FREEOBJ_FUNC(bucket_free_storage)
{
    bucket_object *obj = zap_get_object(bucket_object, object);

    zapval_destroy(obj->encoder);
    zapval_destroy(obj->decoder);
    zapval_destroy(obj->prefix);

    zend_object_std_dtor(&obj->std TSRMLS_CC);
    zap_free_object_storage(obj);
}

zap_CREATEOBJ_FUNC(bucket_create_handler)
{
    bucket_object *obj = zap_alloc_object_storage(bucket_object, type);

    zend_object_std_init(&obj->std, type TSRMLS_CC);
    zap_object_properties_init(&obj->std, type);

    zapval_alloc_empty_string(obj->encoder);
    zapval_alloc_empty_string(obj->decoder);
    zapval_alloc_empty_string(obj->prefix);

    obj->conn = NULL;

    return zap_finalize_object(obj, &bucket_class);
}

zval * bop_get_return_doc(zval *return_value, zapval *key, int is_mapped)
{
    zval *doc = return_value;
    if (is_mapped) {
        if (!zap_zval_is_array(return_value)) {
            array_init(return_value);
        }
        {
            char tmpstr[251];
            HashTable *htretval = Z_ARRVAL_P(return_value);
            uint key_len = zapval_strlen_p(key);
            zapval new_doc;
            zapval_alloc_null(new_doc);

            memcpy(tmpstr, zapval_strval_p(key), key_len);
            tmpstr[key_len] = '\0';

            doc = zap_hash_str_update(
                    htretval, tmpstr, key_len, zapval_zvalptr(new_doc));
        }
    }
    return doc;
}

PHP_METHOD(Bucket, __construct)
{
    bucket_object *data = PCBC_PHP_THISOBJ();
    zval *zdsn = NULL;
    zval *zname = NULL;
    zval *zpassword = NULL;
    char *dsn = NULL;
    char *name = NULL;
    char *password = NULL;
    lcb_error_t err;
    lcb_t instance;
    struct lcb_create_st create_options;
    char *connkey = NULL;
    pcbc_lcb *conn_iter, *conn;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|zzz",
                              &zdsn, &zname, &zpassword) == FAILURE) {
        RETURN_NULL();
    }

    if (zap_zval_is_undef(zdsn)) {
        zdsn = NULL;
    }
    if (zap_zval_is_undef(zname)) {
        zname = NULL;
    }
    if (zap_zval_is_undef(zpassword)) {
        zpassword = NULL;
    }

    if (zdsn) {
        if (Z_TYPE_P(zdsn) == IS_STRING) {
            dsn = estrndup(Z_STRVAL_P(zdsn), Z_STRLEN_P(zdsn));
        } else {
            throw_pcbc_exception("Expected dsn as string", LCB_EINVAL);
            RETURN_NULL();
        }
    }

    if (zname) {
        if (Z_TYPE_P(zname) == IS_STRING) {
            name = estrndup(Z_STRVAL_P(zname), Z_STRLEN_P(zname));
        } else {
            throw_pcbc_exception("Expected bucket name as string", LCB_EINVAL);
            if (dsn) efree(dsn);
            RETURN_NULL();
        }
    }

    if (zpassword) {
        if (Z_TYPE_P(zpassword) == IS_STRING) {
            password = estrndup(Z_STRVAL_P(zpassword), Z_STRLEN_P(zpassword));
        } else {
            throw_pcbc_exception("Expected bucket password as string", LCB_EINVAL);
            if (dsn) efree(dsn);
            if (name) efree(name);
            RETURN_NULL();
        }
    }

    spprintf(&connkey, 512, "%s|%s|%s",
             dsn ? dsn : "",
             name ? name : "",
             password ? password : "");

    conn_iter = PCBCG(first_bconn);
    while (conn_iter) {
        if (strcmp(conn_iter->key, connkey) == 0) {
            break;
        }
        conn_iter = conn_iter->next;
    }

    if (!conn_iter)
    {
        memset(&create_options, 0, sizeof(create_options));
        create_options.version = 3;
        create_options.v.v3.connstr = dsn;
        create_options.v.v3.username = name;
        create_options.v.v3.passwd = password;
        create_options.v.v3.type = LCB_TYPE_BUCKET;
        err = lcb_create(&instance, &create_options);

        if (dsn) efree(dsn);
        if (password) efree(password);

        if (err != LCB_SUCCESS) {
            efree(connkey);
            efree(name);
            throw_lcb_exception(err);
            RETURN_NULL();
        }
        pcbc_log(LOGARGS(instance, INFO), "New lcb_t instance has been initialized");
        err = lcb_cntl(instance, LCB_CNTL_SET, LCB_CNTL_LOGGER, &pcbc_logger);
        if (err != LCB_SUCCESS) {
            efree(connkey);
            efree(name);
            throw_lcb_exception(err);
            RETURN_NULL();
        }

        lcb_cntl(instance, LCB_CNTL_SET, LCB_CNTL_CLIENT_STRING, pcbc_client_string);

        lcb_install_callback3(instance, LCB_CALLBACK_GET, get_callback);
        lcb_install_callback3(instance, LCB_CALLBACK_GETREPLICA, get_callback);
        lcb_install_callback3(instance, LCB_CALLBACK_UNLOCK, unlock_callback);
        lcb_install_callback3(instance, LCB_CALLBACK_STORE, store_callback);
        lcb_install_callback3(instance, LCB_CALLBACK_STOREDUR, store_callback);
        lcb_install_callback3(instance, LCB_CALLBACK_REMOVE, remove_callback);
        lcb_install_callback3(instance, LCB_CALLBACK_TOUCH, touch_callback);
        lcb_install_callback3(instance, LCB_CALLBACK_COUNTER, counter_callback);
        lcb_install_callback3(instance, LCB_CALLBACK_SDLOOKUP, subdoc_callback);
        lcb_install_callback3(instance, LCB_CALLBACK_SDMUTATE, subdoc_callback);
        lcb_install_callback3(instance, LCB_CALLBACK_HTTP, http_callback);

        lcb_set_durability_callback(instance, durability_callback);


        err = lcb_connect(instance);
        if (err != LCB_SUCCESS) {
            efree(connkey);
            efree(name);
            lcb_destroy(instance);
            throw_lcb_exception(err);
            RETURN_NULL();
        }

        // We use lcb_wait here as no callbacks are invoked by connect.
        lcb_wait(instance);

        err = lcb_get_bootstrap_status(instance);
        if (err != LCB_SUCCESS) {
            efree(connkey);
            efree(name);
            lcb_destroy(instance);
            throw_lcb_exception(err);
            RETURN_NULL();
        }

        conn = pemalloc(sizeof(pcbc_lcb), 1);
        conn->bucket = pestrdup(name, 1);
        efree(name);
        conn->key = pestrdup(connkey, 1);
        conn->lcb = instance;
        conn->next = NULL;
        data->conn = conn;

        if (PCBCG(last_bconn)) {
            PCBCG(last_bconn)->next = conn;
            PCBCG(last_bconn) = conn;
        } else {
            PCBCG(first_bconn) = conn;
            PCBCG(last_bconn) = conn;
        }
        pcbc_log(LOGARGS(instance, INFO), "lcb_t instance has been connected");
    } else {
        if (dsn) efree(dsn);
        if (name) efree(name);
        if (password) efree(password);

        data->conn = conn_iter;
        pcbc_log(LOGARGS(data->conn->lcb, INFO), "lcb_t instance has been fetched from cache");
    }

    efree(connkey);
}

PHP_METHOD(Bucket, setTranscoder)
{
    bucket_object *data = PCBC_PHP_THISOBJ();
    zval *zencoder, *zdecoder;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zz", &zencoder, &zdecoder) == FAILURE) {
        RETURN_NULL();
    }

    zapval_destroy(data->encoder);
    zapval_alloc_zval(data->encoder, zencoder, 1, 0);

    zapval_destroy(data->decoder);
    zapval_alloc_zval(data->decoder, zdecoder, 1, 0);

    RETURN_NULL();
}

PHP_METHOD(Bucket, setOption)
{
    bucket_object *data = PCBC_PHP_THISOBJ();
    long type, val;
    lcb_uint32_t lcbval;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ll", &type, &val) == FAILURE) {
        RETURN_NULL();
    }

    lcbval = val;
    lcb_cntl(data->conn->lcb, LCB_CNTL_SET, type, &lcbval);

    RETURN_LONG(val);
}

PHP_METHOD(Bucket, getOption)
{
    bucket_object *data = PCBC_PHP_THISOBJ();
    long type;
    lcb_uint32_t lcbval;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &type) == FAILURE) {
        RETURN_NULL();
    }

    lcb_cntl(data->conn->lcb, LCB_CNTL_GET, type, &lcbval);

    RETURN_LONG(lcbval);
}

zend_function_entry bucket_methods[] = {
    PHP_ME(Bucket,  __construct,     NULL, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)

    PHP_ME(Bucket,  insert,          NULL, ZEND_ACC_PUBLIC)
    PHP_ME(Bucket,  upsert,          NULL, ZEND_ACC_PUBLIC)
    PHP_ME(Bucket,  replace,         NULL, ZEND_ACC_PUBLIC)
    PHP_ME(Bucket,  append,          NULL, ZEND_ACC_PUBLIC)
    PHP_ME(Bucket,  prepend,         NULL, ZEND_ACC_PUBLIC)
    PHP_ME(Bucket,  remove,          NULL, ZEND_ACC_PUBLIC)
    PHP_ME(Bucket,  get,             NULL, ZEND_ACC_PUBLIC)
    PHP_ME(Bucket,  getFromReplica,  NULL, ZEND_ACC_PUBLIC)
    PHP_ME(Bucket,  touch,           NULL, ZEND_ACC_PUBLIC)
    PHP_ME(Bucket,  counter,         NULL, ZEND_ACC_PUBLIC)
    PHP_ME(Bucket,  unlock,          NULL, ZEND_ACC_PUBLIC)
    PHP_ME(Bucket,  n1ql_request,    NULL, ZEND_ACC_PUBLIC)
    PHP_ME(Bucket,  http_request,    NULL, ZEND_ACC_PUBLIC)
    PHP_ME(Bucket,  fts_request,     NULL, ZEND_ACC_PUBLIC)
    PHP_ME(Bucket,  subdoc_request,  NULL, ZEND_ACC_PUBLIC)
    PHP_ME(Bucket,  durability,      NULL, ZEND_ACC_PUBLIC)
    PHP_ME(Bucket,  n1ix_list,       NULL, ZEND_ACC_PUBLIC)
    PHP_ME(Bucket,  n1ix_create,     NULL, ZEND_ACC_PUBLIC)
    PHP_ME(Bucket,  n1ix_drop,       NULL, ZEND_ACC_PUBLIC)

    PHP_ME(Bucket,  setTranscoder,   NULL, ZEND_ACC_PUBLIC)
    PHP_ME(Bucket,  setOption,       NULL, ZEND_ACC_PUBLIC)
    PHP_ME(Bucket,  getOption,       NULL, ZEND_ACC_PUBLIC)
    {NULL, NULL, NULL}
};

void couchbase_init_bucket(INIT_FUNC_ARGS) {
    zap_init_class_entry(&bucket_class, "_CouchbaseBucket", bucket_methods);
    bucket_class.create_obj = bucket_create_handler;
    bucket_class.free_obj = bucket_free_storage;
    bucket_ce = zap_register_internal_class(&bucket_class, bucket_object);
}

void couchbase_shutdown_bucket(SHUTDOWN_FUNC_ARGS) {
    pcbc_lcb *cur, *next;
    next = PCBCG(first_bconn);
    while (next) {
        cur = next;
        next = cur->next;
        lcb_destroy(cur->lcb);
        free(cur->key);
        free(cur->bucket);
        free(cur);
    }
    PCBCG(first_bconn) = NULL;
    PCBCG(last_bconn) = NULL;
}
