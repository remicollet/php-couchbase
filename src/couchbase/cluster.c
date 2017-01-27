/**
 *     Copyright 2016-2017 Couchbase, Inc.
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

#define LOGARGS(lvl) LCB_LOG_##lvl, NULL, "pcbc/cluster", __FILE__, __LINE__
#define DEFAULT_CONNECTION_STRING "couchbase://127.0.0.1"
#define DEFAULT_BUCKET_NAME "default"

zend_class_entry *pcbc_cluster_ce;
extern zend_class_entry *pcbc_authenticator_ce;

/* {{{ proto void Cluster::__construct($connstr = 'couchbase://127.0.0.1/')
   Creates a connection to a cluster */
PHP_METHOD(Cluster, __construct)
{
    pcbc_cluster_t *obj;
    char *connstr;
    pcbc_str_arg_size connstr_len = 0;
    int rv;

    obj = Z_CLUSTER_OBJ_P(getThis());

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &connstr, &connstr_len);
    if (rv == FAILURE) {
        return;
    }
    if (connstr_len == 0) {
        connstr = DEFAULT_CONNECTION_STRING;
    }
    obj->connstr = estrdup(connstr);
    ZVAL_UNDEF(PCBC_P(obj->auth));
    pcbc_log(LOGARGS(DEBUG), "Initialize Cluster. C=%p connstr=\"%s\"", (void *)obj, obj->connstr);
}
/* }}} */

/* {{{ proto \Couchbase\Bucket Cluster::openBucket(string $bucketname = "default", string $password = "")
   Constructs a connection to a bucket. */
PHP_METHOD(Cluster, openBucket)
{
    pcbc_cluster_t *obj;
    const char *bucketname = NULL, *password = NULL;
    pcbc_str_arg_size bucketname_len = 0, password_len = 0;
    int rv;

    obj = Z_CLUSTER_OBJ_P(getThis());

    rv =
        zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|ss", &bucketname, &bucketname_len, &password, &password_len);
    if (rv == FAILURE) {
        return;
    }
    if (bucketname_len == 0) {
        bucketname = DEFAULT_BUCKET_NAME;
        pcbc_log(LOGARGS(DEBUG), "Fallback to default bucket bucketname. C=%p", (void *)obj);
    }
    pcbc_bucket_init(return_value, obj, bucketname, password TSRMLS_CC);
} /* }}} */

/* {{{ proto \Couchbase\ClusterManager Cluster::manager(string $username = NULL, string $password = NULL) */
PHP_METHOD(Cluster, manager)
{
    pcbc_cluster_t *obj;
    const char *name = NULL, *password = NULL;
    pcbc_str_arg_size name_len = 0, password_len = 0;
    int rv;

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|ss", &name, &name_len, &password, &password_len);
    if (rv == FAILURE) {
        return;
    }
    obj = Z_CLUSTER_OBJ_P(getThis());
    pcbc_cluster_manager_init(return_value, obj, name, password TSRMLS_CC);
} /* }}} */

/* {{{ proto void Cluster::authenticate(\Couchbase\Authenticator $authenticator) */
PHP_METHOD(Cluster, authenticate)
{
    pcbc_cluster_t *obj;
    zval *authenticator = NULL;
    int rv;

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &authenticator, pcbc_authenticator_ce);
    if (rv == FAILURE) {
        RETURN_NULL();
    }
    obj = Z_CLUSTER_OBJ_P(getThis());
    if (!Z_ISUNDEF(obj->auth)) {
        zval_ptr_dtor(&obj->auth);
        ZVAL_UNDEF(PCBC_P(obj->auth));
    }
#if PHP_VERSION_ID >= 70000
    ZVAL_ZVAL(&obj->auth, authenticator, 1, 0);
#else
    PCBC_ADDREF_P(authenticator);
    obj->auth = authenticator;
#endif

    RETURN_NULL();
} /* }}} */

ZEND_BEGIN_ARG_INFO_EX(ai_Cluster_constructor, 0, 0, 1)
ZEND_ARG_INFO(0, connstr)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_Cluster_openBucket, 0, 0, 2)
ZEND_ARG_INFO(0, name)
ZEND_ARG_INFO(0, password)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_Cluster_manager, 0, 0, 2)
ZEND_ARG_INFO(0, username)
ZEND_ARG_INFO(0, password)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_Cluster_authenticate, 0, 0, 1)
ZEND_ARG_INFO(0, authenticator)
ZEND_END_ARG_INFO()

// clang-format off
zend_function_entry cluster_methods[] = {
    PHP_ME(Cluster, __construct, ai_Cluster_constructor, ZEND_ACC_PUBLIC | ZEND_ACC_FINAL | ZEND_ACC_CTOR)
    PHP_ME(Cluster, openBucket, ai_Cluster_openBucket, ZEND_ACC_PUBLIC | ZEND_ACC_FINAL)
    PHP_ME(Cluster, manager, ai_Cluster_manager, ZEND_ACC_PUBLIC | ZEND_ACC_FINAL)
    PHP_ME(Cluster, authenticate, ai_Cluster_authenticate, ZEND_ACC_PUBLIC | ZEND_ACC_FINAL)
    PHP_FE_END
};
// clang-format on

zend_object_handlers pcbc_cluster_handlers;

static void pcbc_cluster_free_object(pcbc_free_object_arg *object TSRMLS_DC) /* {{{ */
{
    pcbc_cluster_t *obj = Z_CLUSTER_OBJ(object);

    if (!Z_ISUNDEF(obj->auth)) {
        zval_ptr_dtor(&obj->auth);
        ZVAL_UNDEF(PCBC_P(obj->auth));
    }
    if (obj->connstr != NULL) {
        efree(obj->connstr);
    }

    zend_object_std_dtor(&obj->std TSRMLS_CC);
#if PHP_VERSION_ID < 70000
    efree(obj);
#endif
} /* }}} */

static pcbc_create_object_retval pcbc_cluster_create_object(zend_class_entry *class_type TSRMLS_DC)
{
    pcbc_cluster_t *obj = NULL;

    obj = PCBC_ALLOC_OBJECT_T(pcbc_cluster_t, class_type);

    zend_object_std_init(&obj->std, class_type TSRMLS_CC);
    object_properties_init(&obj->std, class_type);

#if PHP_VERSION_ID >= 70000
    obj->std.handlers = &pcbc_cluster_handlers;
    return &obj->std;
#else
    {
        zend_object_value ret;
        ret.handle = zend_objects_store_put(obj, (zend_objects_store_dtor_t)zend_objects_destroy_object,
                                            pcbc_cluster_free_object, NULL TSRMLS_CC);
        ret.handlers = &pcbc_cluster_handlers;
        return ret;
    }
#endif
}

static HashTable *pcbc_cluster_get_debug_info(zval *object, int *is_temp TSRMLS_DC) /* {{{ */
{
    pcbc_cluster_t *obj = NULL;
#if PHP_VERSION_ID >= 70000
    zval retval;
#else
    zval retval = zval_used_for_init;
#endif

    *is_temp = 1;
    obj = Z_CLUSTER_OBJ_P(object);

    array_init(&retval);
    ADD_ASSOC_STRING(&retval, "connstr", obj->connstr);
    if (!Z_ISUNDEF(obj->auth)) {
        ADD_ASSOC_ZVAL_EX(&retval, "authenticator", PCBC_P(obj->auth));
        PCBC_ADDREF_P(PCBC_P(obj->auth));
    } else {
        ADD_ASSOC_NULL_EX(&retval, "authenticator");
    }

    return Z_ARRVAL(retval);
} /* }}} */

PHP_MINIT_FUNCTION(Cluster)
{
    zend_class_entry ce;

    INIT_NS_CLASS_ENTRY(ce, "Couchbase", "Cluster", cluster_methods);
    pcbc_cluster_ce = zend_register_internal_class(&ce TSRMLS_CC);
    pcbc_cluster_ce->create_object = pcbc_cluster_create_object;
    PCBC_CE_FLAGS_FINAL(pcbc_cluster_ce);
    PCBC_CE_DISABLE_SERIALIZATION(pcbc_cluster_ce);

    memcpy(&pcbc_cluster_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    pcbc_cluster_handlers.get_debug_info = pcbc_cluster_get_debug_info;
#if PHP_VERSION_ID >= 70000
    pcbc_cluster_handlers.free_obj = pcbc_cluster_free_object;
    pcbc_cluster_handlers.offset = XtOffsetOf(pcbc_cluster_t, std);
#endif

    zend_register_class_alias("\\CouchbaseCluster", pcbc_cluster_ce);
    return SUCCESS;
}
