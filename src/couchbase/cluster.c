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

#define LOGARGS(lvl) LCB_LOG_##lvl, NULL, "pcbc/cluster", __FILE__, __LINE__
#define DEFAULT_CONNECTION_STRING "couchbase://127.0.0.1"
#define DEFAULT_BUCKET_NAME "default"

zend_class_entry *pcbc_cluster_ce;
extern zend_class_entry *pcbc_authenticator_ce;
extern zend_class_entry *pcbc_classic_authenticator_ce;
extern zend_class_entry *pcbc_password_authenticator_ce;
extern zend_class_entry *pcbc_cert_authenticator_ce;
extern zend_class_entry *pcbc_cluster_manager_ce;

PHP_METHOD(Cluster, query);
PHP_METHOD(Cluster, analyticsQuery);
PHP_METHOD(Cluster, searchQuery);

static lcb_AUTHENTICATOR *pcbc_gen_auth(zval *return_value, pcbc_cluster_t *cluster, char **auth_hash TSRMLS_DC)
{
    if (Z_ISUNDEF(cluster->auth)) {
        throw_pcbc_exception("Missing authenticator", LCB_EINVAL);
        return NULL;
    }
    if (instanceof_function(Z_OBJCE_P(&cluster->auth), pcbc_cert_authenticator_ce TSRMLS_CC)) {
        if (!cluster->connstr) {
            throw_pcbc_exception("mixed-auth: connection string is not set with CertAuthenticator", LCB_EINVAL);
            return NULL;
        }
        if (strstr(cluster->connstr, "keypath") == NULL) {
            throw_pcbc_exception("mixed-auth: keypath must be in connection string with CertAuthenticator", LCB_EINVAL);
            return NULL;
        }
        if (strstr(cluster->connstr, "certpath") == NULL) {
            throw_pcbc_exception("mixed-auth: certpath must be in connection string with CertAuthenticator",
                                 LCB_EINVAL);
            return NULL;
        }
    } else if (cluster->connstr) {
        if (strstr(cluster->connstr, "keypath") != NULL) {
            throw_pcbc_exception("mixed-auth: keypath in connection string requires CertAuthenticator", LCB_EINVAL);
            return NULL;
        }
    }

    lcb_AUTHENTICATOR *auth = NULL;
    if (instanceof_function(Z_OBJCE_P(&cluster->auth), pcbc_classic_authenticator_ce TSRMLS_CC)) {
        pcbc_generate_classic_lcb_auth(Z_CLASSIC_AUTHENTICATOR_OBJ_P(&cluster->auth), &auth, LCB_TYPE_BUCKET,
                                       auth_hash TSRMLS_CC);
    } else if (instanceof_function(Z_OBJCE_P(&cluster->auth), pcbc_password_authenticator_ce TSRMLS_CC)) {
        pcbc_generate_password_lcb_auth(Z_PASSWORD_AUTHENTICATOR_OBJ_P(&cluster->auth), &auth, LCB_TYPE_BUCKET,
                                        auth_hash TSRMLS_CC);
    } else {
        throw_pcbc_exception("Unknown type of the authenticator. Unable to open bucket", LCB_EINVAL);
        return NULL;
    }
    return auth;
}

static void pcbc_bucket_init(zval *return_value, pcbc_cluster_t *cluster, const char *bucketname TSRMLS_DC)
{
    char *auth_hash = NULL;
    lcb_AUTHENTICATOR *auth = pcbc_gen_auth(return_value, cluster, &auth_hash TSRMLS_CC);
    if (auth == NULL) {
        return;
    }

    pcbc_bucket_t *bucket;
    pcbc_connection_t *conn;
    lcb_STATUS err;

    err = pcbc_connection_get(&conn, LCB_TYPE_BUCKET, cluster->connstr, bucketname, auth, auth_hash TSRMLS_CC);
    if (auth_hash) {
        efree(auth_hash);
    }
    if (err) {
        throw_lcb_exception(err, NULL);
        return;
    }
    object_init_ex(return_value, pcbc_bucket_ce);
    bucket = Z_BUCKET_OBJ_P(return_value);
    bucket->conn = conn;
    lcb_cntl(conn->lcb, LCB_CNTL_GET, LCB_CNTL_BUCKETTYPE, &bucket->type);
    ZVAL_UNDEF(&bucket->encoder);
    ZVAL_UNDEF(&bucket->decoder);
    PCBC_STRING(bucket->encoder, "\\Couchbase\\defaultEncoder");
    PCBC_STRING(bucket->decoder, "\\Couchbase\\defaultDecoder");
}

static void pcbc_cluster_connection_init(zval *return_value, pcbc_cluster_t *cluster TSRMLS_DC)
{
    char *auth_hash = NULL;
    lcb_AUTHENTICATOR *auth = pcbc_gen_auth(return_value, cluster, &auth_hash TSRMLS_CC);
    if (auth == NULL) {
        return;
    }

    pcbc_bucket_t *bucket;
    pcbc_connection_t *conn;
    lcb_STATUS err;

    err = pcbc_connection_get(&conn, LCB_TYPE_CLUSTER, cluster->connstr, NULL, auth, auth_hash TSRMLS_CC);
    if (auth_hash) {
        efree(auth_hash);
    }
    if (err) {
        throw_lcb_exception(err, NULL);
        return;
    }
    cluster->conn = conn;
}

static void pcbc_cluster_manager_init(zval *return_value, pcbc_cluster_t *cluster TSRMLS_DC)
{
    char *auth_hash = NULL;
    lcb_AUTHENTICATOR *auth = pcbc_gen_auth(return_value, cluster, &auth_hash TSRMLS_CC);
    if (auth == NULL) {
        return;
    }

    lcb_STATUS err;
    pcbc_connection_t *conn = NULL;
    err = pcbc_connection_get(&conn, LCB_TYPE_CLUSTER, cluster->connstr, NULL, auth, auth_hash TSRMLS_CC);
    efree(auth_hash);
    if (err != LCB_SUCCESS) {
        throw_lcb_exception(err, NULL);
        return;
    }

    pcbc_cluster_manager_t *manager;
    object_init_ex(return_value, pcbc_cluster_manager_ce);
    manager = Z_CLUSTER_MANAGER_OBJ_P(return_value);
    manager->conn = conn;
}

PHP_METHOD(Cluster, __construct)
{
    pcbc_cluster_t *obj;
    char *connstr;
    size_t connstr_len = 0;
    int rv;

    obj = Z_CLUSTER_OBJ_P(getThis());

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &connstr, &connstr_len);
    if (rv == FAILURE) {
        throw_pcbc_exception("Invalid arguments.", LCB_EINVAL);
        RETURN_NULL();
    }
    if (connstr_len == 0) {
        connstr = DEFAULT_CONNECTION_STRING;
    }
    obj->connstr = estrdup(connstr);
    obj->conn = NULL;
    ZVAL_UNDEF(&obj->auth);
    pcbc_log(LOGARGS(DEBUG), "Initialize Cluster. C=%p connstr=\"%s\"", (void *)obj, obj->connstr);
}

PHP_METHOD(Cluster, bucket)
{
    pcbc_cluster_t *obj;
    const char *bucketname = NULL, *password = NULL;
    size_t bucketname_len = 0, password_len = 0;
    int rv;

    obj = Z_CLUSTER_OBJ_P(getThis());

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|s", &bucketname, &bucketname_len);
    if (rv == FAILURE) {
        return;
    }
    if (bucketname_len == 0) {
        bucketname = DEFAULT_BUCKET_NAME;
        pcbc_log(LOGARGS(DEBUG), "Fallback to default bucket bucketname. C=%p", (void *)obj);
    }
    pcbc_bucket_init(return_value, obj, bucketname TSRMLS_CC);
}

PHP_METHOD(Cluster, manager)
{
    pcbc_cluster_t *obj;
    int rv;

    if (zend_parse_parameters_none_throw() == FAILURE) {
        return;
    }
    obj = Z_CLUSTER_OBJ_P(getThis());
    pcbc_cluster_manager_init(return_value, obj TSRMLS_CC);
}

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
        ZVAL_UNDEF(&obj->auth);
    }
    ZVAL_ZVAL(&obj->auth, authenticator, 1, 0);
    pcbc_cluster_connection_init(return_value, obj TSRMLS_CC);

    RETURN_NULL();
}

PHP_METHOD(Cluster, authenticateAs)
{
    pcbc_cluster_t *obj;
    zval authenticator;
    char *username = NULL, *password = NULL;
    size_t username_len = 0, password_len = 0;
    int rv;

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss", &username, &username_len, &password, &password_len);
    if (rv == FAILURE) {
        RETURN_NULL();
    }
    ZVAL_UNDEF(&authenticator);
    pcbc_password_authenticator_init(&authenticator, username, username_len, password, password_len TSRMLS_CC);
    obj = Z_CLUSTER_OBJ_P(getThis());
    if (!Z_ISUNDEF(obj->auth)) {
        zval_ptr_dtor(&obj->auth);
        ZVAL_UNDEF(&obj->auth);
    }
    ZVAL_ZVAL(&obj->auth, &authenticator, 0, 0);
    pcbc_cluster_connection_init(return_value, obj TSRMLS_CC);

    RETURN_NULL();
}

ZEND_BEGIN_ARG_INFO_EX(ai_Cluster_constructor, 0, 0, 1)
ZEND_ARG_TYPE_INFO(0, connstr, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(ai_Cluster_bucket, 0, 1, \\Couchbase\\Bucket, 0)
ZEND_ARG_TYPE_INFO(0, name, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(ai_Cluster_manager, 0, 1, \\Couchbase\\ClusterManager, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_Cluster_authenticate, 0, 0, 1)
ZEND_ARG_OBJ_INFO(0, authenticator, \\Couchbase\\Authenticator, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_Cluster_authenticateAs, 0, 0, 2)
ZEND_ARG_TYPE_INFO(0, username, IS_STRING, 0)
ZEND_ARG_TYPE_INFO(0, password, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(ai_Cluster_query, 0, 1, \\Couchbase\\QueryResult, 0)
ZEND_ARG_TYPE_INFO(0, statement, IS_STRING, 0)
ZEND_ARG_OBJ_INFO(0, queryOptions, \\Couchbase\\QueryOptions, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(ai_Cluster_analyticsQuery, 0, 1, \\Couchbase\\AnalyticsResult, 0)
ZEND_ARG_TYPE_INFO(0, statement, IS_STRING, 0)
ZEND_ARG_OBJ_INFO(0, queryOptions, \\Couchbase\\AnalyticsQueryOptions, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(ai_Cluster_searchQuery, 0, 2, \\Couchbase\\SearchResult, 0)
ZEND_ARG_TYPE_INFO(0, indexName, IS_STRING, 0)
ZEND_ARG_OBJ_INFO(0, query, \\Couchbase\\SearchQuery, 1)
ZEND_ARG_OBJ_INFO(0, queryOptions, \\Couchbase\\SearchQueryOptions, 1)
ZEND_END_ARG_INFO()

// clang-format off
zend_function_entry cluster_methods[] = {
    PHP_ME(Cluster, __construct, ai_Cluster_constructor, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
    PHP_ME(Cluster, bucket, ai_Cluster_bucket, ZEND_ACC_PUBLIC)
    PHP_ME(Cluster, manager, ai_Cluster_manager, ZEND_ACC_PUBLIC)
    PHP_ME(Cluster, authenticate, ai_Cluster_authenticate, ZEND_ACC_PUBLIC)
    PHP_ME(Cluster, authenticateAs, ai_Cluster_authenticateAs, ZEND_ACC_PUBLIC)
    PHP_ME(Cluster, query, ai_Cluster_query, ZEND_ACC_PUBLIC)
    PHP_ME(Cluster, analyticsQuery, ai_Cluster_analyticsQuery, ZEND_ACC_PUBLIC)
    PHP_ME(Cluster, searchQuery, ai_Cluster_searchQuery, ZEND_ACC_PUBLIC)
    PHP_FE_END
};
// clang-format on

zend_object_handlers pcbc_cluster_handlers;

static void pcbc_cluster_free_object(zend_object *object TSRMLS_DC) /* {{{ */
{
    pcbc_cluster_t *obj = Z_CLUSTER_OBJ(object);

    if (!Z_ISUNDEF(obj->auth)) {
        zval_ptr_dtor(&obj->auth);
        ZVAL_UNDEF(&obj->auth);
    }
    if (obj->connstr != NULL) {
        efree(obj->connstr);
    }

    zend_object_std_dtor(&obj->std TSRMLS_CC);
}

static zend_object *pcbc_cluster_create_object(zend_class_entry *class_type TSRMLS_DC)
{
    pcbc_cluster_t *obj = NULL;

    obj = PCBC_ALLOC_OBJECT_T(pcbc_cluster_t, class_type);

    zend_object_std_init(&obj->std, class_type TSRMLS_CC);
    object_properties_init(&obj->std, class_type);

    obj->std.handlers = &pcbc_cluster_handlers;
    return &obj->std;
}

static HashTable *pcbc_cluster_get_debug_info(zval *object, int *is_temp TSRMLS_DC) /* {{{ */
{
    pcbc_cluster_t *obj = NULL;
    zval retval;

    *is_temp = 1;
    obj = Z_CLUSTER_OBJ_P(object);

    array_init(&retval);
    ADD_ASSOC_STRING(&retval, "connstr", obj->connstr);
    if (!Z_ISUNDEF(obj->auth)) {
        ADD_ASSOC_ZVAL_EX(&retval, "authenticator", &obj->auth);
        PCBC_ADDREF_P(&obj->auth);
    } else {
        ADD_ASSOC_NULL_EX(&retval, "authenticator");
    }

    return Z_ARRVAL(retval);
}

PHP_MINIT_FUNCTION(Cluster)
{
    zend_class_entry ce;

    INIT_NS_CLASS_ENTRY(ce, "Couchbase", "Cluster", cluster_methods);
    pcbc_cluster_ce = zend_register_internal_class(&ce TSRMLS_CC);
    pcbc_cluster_ce->create_object = pcbc_cluster_create_object;
    PCBC_CE_DISABLE_SERIALIZATION(pcbc_cluster_ce);

    memcpy(&pcbc_cluster_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    pcbc_cluster_handlers.get_debug_info = pcbc_cluster_get_debug_info;
    pcbc_cluster_handlers.free_obj = pcbc_cluster_free_object;
    pcbc_cluster_handlers.offset = XtOffsetOf(pcbc_cluster_t, std);


    return SUCCESS;
}

/*
 * vim: et ts=4 sw=4 sts=4
 */
