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

#include <ext/standard/md5.h>

#define LOGARGS(lvl) LCB_LOG_##lvl, NULL, "pcbc/classic_authenticator", __FILE__, __LINE__

zend_class_entry *pcbc_classic_authenticator_ce;
extern zend_class_entry *pcbc_authenticator_ce;

PHP_METHOD(ClassicAuthenticator, __construct)
{
    pcbc_classic_authenticator_t *obj;
    int rv;

    rv = zend_parse_parameters_none();
    if (rv == FAILURE) {
        throw_pcbc_exception("Invalid arguments.", LCB_EINVAL);
        RETURN_NULL();
    }
    obj = Z_CLASSIC_AUTHENTICATOR_OBJ_P(getThis());
    obj->cluster.username = NULL;
    obj->cluster.username_len = 0;
    obj->cluster.password = NULL;
    obj->cluster.password_len = 0;
    obj->buckets = NULL;
    obj->tail = NULL;
    obj->nbuckets = 0;
}

PHP_METHOD(ClassicAuthenticator, cluster)
{
    pcbc_classic_authenticator_t *obj;
    char *username = NULL, *password = NULL;
    size_t username_len, password_len;
    int rv;

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss", &username, &username_len, &password, &password_len);
    if (rv == FAILURE) {
        RETURN_NULL();
    }

    obj = Z_CLASSIC_AUTHENTICATOR_OBJ_P(getThis());

    if (obj->cluster.username) {
        efree(obj->cluster.username);
    }
    obj->cluster.username_len = username_len;
    obj->cluster.username = estrndup(username, username_len);

    if (obj->cluster.password) {
        efree(obj->cluster.password);
    }
    obj->cluster.password_len = password_len;
    obj->cluster.password = estrndup(password, password_len);

    RETURN_ZVAL(getThis(), 1, 0);
}

PHP_METHOD(ClassicAuthenticator, bucket)
{
    pcbc_classic_authenticator_t *obj;
    char *name = NULL, *password = NULL;
    size_t name_len, password_len;
    pcbc_credential_t *ptr;
    int rv;

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss", &name, &name_len, &password, &password_len);
    if (rv == FAILURE) {
        RETURN_NULL();
    }

    obj = Z_CLASSIC_AUTHENTICATOR_OBJ_P(getThis());

    ptr = obj->buckets;
    while (ptr) {
        if (ptr->username_len == name_len && strncmp(ptr->username, name, name_len) == 0) {
            break;
        }
        ptr = ptr->next;
    }
    if (ptr == NULL) {
        pcbc_credential_t *cred;
        cred = emalloc(sizeof(pcbc_credential_t));
        cred->username_len = name_len;
        cred->username = estrndup(name, name_len);
        cred->password_len = password_len;
        cred->password = estrndup(password, password_len);
        cred->next = NULL;

        if (obj->buckets == NULL) {
            obj->buckets = cred;
        }
        if (obj->tail) {
            obj->tail->next = cred;
        }
        obj->tail = cred;
        obj->nbuckets++;
    } else {
        if (ptr->password) {
            efree(ptr->password);
        }
        ptr->password_len = password_len;
        ptr->password = estrndup(password, password_len);
    }

    RETURN_ZVAL(getThis(), 1, 0);
}

/* TODO: update to refactored lcb_AUTHENTICATOR after libcouchbase 2.7.4 */
void pcbc_generate_classic_lcb_auth(pcbc_classic_authenticator_t *auth, lcb_AUTHENTICATOR **result,
                                    lcb_INSTANCE_TYPE type, char **hash TSRMLS_DC)
{
    PHP_MD5_CTX md5;
    unsigned char digest[16];
    const char *empty_pass = "";
    int empty_pass_len = strlen(empty_pass);
    const char *pass;
    int pass_len;

    *result = lcbauth_new();
    lcbauth_set_mode(*result, LCBAUTH_MODE_CLASSIC);
    PHP_MD5Init(&md5);

    if (auth && (auth->cluster.username || auth->nbuckets)) {
        pcbc_credential_t *ptr;

        if (auth->cluster.username) {
            pass = empty_pass;
            pass_len = empty_pass_len;
            if (auth->cluster.password) {
                pass = auth->cluster.password;
                pass_len = auth->cluster.password_len;
            }
            lcbauth_add_pass(*result, auth->cluster.username, pass, LCBAUTH_F_CLUSTER);
            PHP_MD5Update(&md5, "cluster", sizeof("cluster"));
            PHP_MD5Update(&md5, auth->cluster.username, auth->cluster.username_len);
            PHP_MD5Update(&md5, pass, pass_len);
        }
        ptr = auth->buckets;
        while (ptr) {
            pass = empty_pass;
            pass_len = empty_pass_len;
            if (ptr->password) {
                pass = ptr->password;
                pass_len = ptr->password_len;
            }
            lcbauth_add_pass(*result, ptr->username, pass, LCBAUTH_F_BUCKET);
            PHP_MD5Update(&md5, "bucket", sizeof("bucket"));
            PHP_MD5Update(&md5, ptr->username, ptr->username_len);
            PHP_MD5Update(&md5, pass, pass_len);
            ptr = ptr->next;
        }
    }
    PHP_MD5Final(digest, &md5);
    *hash = ecalloc(sizeof(char), 33);
    make_digest(*hash, digest);
}

ZEND_BEGIN_ARG_INFO_EX(ai_ClassicAuthenticator_none, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_ClassicAuthenticator_cluster, 0, 0, 2)
ZEND_ARG_INFO(0, username)
ZEND_ARG_INFO(0, password)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_ClassicAuthenticator_bucket, 0, 0, 2)
ZEND_ARG_INFO(0, name)
ZEND_ARG_INFO(0, password)
ZEND_END_ARG_INFO()

// clang-format off
zend_function_entry classic_authenticator_methods[] = {
    PHP_ME(ClassicAuthenticator, __construct, ai_ClassicAuthenticator_none, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
    PHP_ME(ClassicAuthenticator, cluster, ai_ClassicAuthenticator_cluster, ZEND_ACC_PUBLIC)
    PHP_ME(ClassicAuthenticator, bucket, ai_ClassicAuthenticator_bucket, ZEND_ACC_PUBLIC)
    PHP_FE_END
};
// clang-format on

zend_object_handlers classic_authenticator_handlers;

static void classic_authenticator_free_object(zend_object *object TSRMLS_DC)
{
    pcbc_classic_authenticator_t *obj = Z_CLASSIC_AUTHENTICATOR_OBJ(object);
    pcbc_credential_t *ptr;

    if (obj->cluster.username != NULL) {
        efree(obj->cluster.username);
    }
    if (obj->cluster.password != NULL) {
        efree(obj->cluster.password);
    }
    ptr = obj->buckets;
    while (ptr) {
        pcbc_credential_t *tmp = ptr;
        ptr = ptr->next;
        if (tmp->username != NULL) {
            efree(tmp->username);
        }
        if (tmp->password != NULL) {
            efree(tmp->password);
        }
        efree(tmp);
    }
    obj->buckets = obj->tail = NULL;

    zend_object_std_dtor(&obj->std TSRMLS_CC);
}

static zend_object *authenticator_create_object(zend_class_entry *class_type TSRMLS_DC)
{
    pcbc_classic_authenticator_t *obj = NULL;

    obj = PCBC_ALLOC_OBJECT_T(pcbc_classic_authenticator_t, class_type);

    zend_object_std_init(&obj->std, class_type TSRMLS_CC);
    object_properties_init(&obj->std, class_type);

    obj->std.handlers = &classic_authenticator_handlers;
    return &obj->std;
}

static HashTable *pcbc_classic_authenticator_get_debug_info(zval *object, int *is_temp TSRMLS_DC)
{
    pcbc_classic_authenticator_t *obj = NULL;
    zval retval;
    zval buckets;
    pcbc_credential_t *ptr;

    *is_temp = 1;
    obj = Z_CLASSIC_AUTHENTICATOR_OBJ_P(object);

    array_init(&retval);
    if (obj->cluster.username) {
        add_assoc_string(&retval, "cluster", obj->cluster.username);
    }
    ZVAL_UNDEF(&buckets);
    array_init_size(&buckets, obj->nbuckets);

    ptr = obj->buckets;
    while (ptr) {
        ADD_NEXT_INDEX_STRING(&buckets, ptr->username)
        ptr = ptr->next;
    }
    add_assoc_zval(&retval, "buckets", &buckets);
    return Z_ARRVAL(retval);
}

PHP_MINIT_FUNCTION(ClassicAuthenticator)
{
    zend_class_entry ce;

    INIT_NS_CLASS_ENTRY(ce, "Couchbase", "ClassicAuthenticator", classic_authenticator_methods);
    pcbc_classic_authenticator_ce = zend_register_internal_class(&ce TSRMLS_CC);
    pcbc_classic_authenticator_ce->create_object = authenticator_create_object;
    PCBC_CE_DISABLE_SERIALIZATION(pcbc_classic_authenticator_ce);

    zend_class_implements(pcbc_classic_authenticator_ce TSRMLS_CC, 1, pcbc_authenticator_ce);

    memcpy(&classic_authenticator_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    classic_authenticator_handlers.get_debug_info = pcbc_classic_authenticator_get_debug_info;
    classic_authenticator_handlers.free_obj = classic_authenticator_free_object;
    classic_authenticator_handlers.offset = XtOffsetOf(pcbc_classic_authenticator_t, std);

    return SUCCESS;
}
