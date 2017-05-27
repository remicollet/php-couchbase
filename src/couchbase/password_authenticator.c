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

#include <ext/standard/md5.h>

#define LOGARGS(lvl) LCB_LOG_##lvl, NULL, "pcbc/password_authenticator", __FILE__, __LINE__

zend_class_entry *pcbc_password_authenticator_ce;
extern zend_class_entry *pcbc_authenticator_ce;

/* {{{ proto void PasswordAuthenticator::__construct() */
PHP_METHOD(PasswordAuthenticator, __construct)
{
    pcbc_password_authenticator_t *obj;
    int rv;

    rv = zend_parse_parameters_none();
    if (rv == FAILURE) {
        throw_pcbc_exception("Invalid arguments.", LCB_EINVAL);
        RETURN_NULL();
    }
    obj = Z_PASSWORD_AUTHENTICATOR_OBJ_P(getThis());
    obj->username = NULL;
    obj->username_len = 0;
    obj->password = NULL;
    obj->password_len = 0;
}
/* }}} */

void pcbc_password_authenticator_init(zval *return_value, char *username, int username_len, char *password,
                                      int password_len TSRMLS_DC)
{
    pcbc_password_authenticator_t *obj;

    object_init_ex(return_value, pcbc_password_authenticator_ce);
    obj = Z_PASSWORD_AUTHENTICATOR_OBJ_P(return_value);
    obj->username = estrndup(username, username_len);
    obj->username_len = username_len;
    obj->password = estrndup(password, password_len);
    obj->password_len = password_len;
}

/* {{{ proto \Couchbase\PasswordAuthenticator PasswordAuthenticator::username(string $username)
 */
PHP_METHOD(PasswordAuthenticator, username)
{
    pcbc_password_authenticator_t *obj;
    char *username = NULL;
    pcbc_str_arg_size username_len;
    int rv;

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &username, &username_len);
    if (rv == FAILURE) {
        RETURN_NULL();
    }

    obj = Z_PASSWORD_AUTHENTICATOR_OBJ_P(getThis());

    if (obj->username) {
        efree(obj->username);
    }
    obj->username_len = username_len;
    obj->username = estrndup(username, username_len);

    RETURN_ZVAL(getThis(), 1, 0);
} /* }}} */

/* {{{ proto \Couchbase\PasswordAuthenticator PasswordAuthenticator::password(string $password)
 */
PHP_METHOD(PasswordAuthenticator, password)
{
    pcbc_password_authenticator_t *obj;
    char *password = NULL;
    pcbc_str_arg_size password_len;
    int rv;

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &password, &password_len);
    if (rv == FAILURE) {
        RETURN_NULL();
    }

    obj = Z_PASSWORD_AUTHENTICATOR_OBJ_P(getThis());

    if (obj->password) {
        efree(obj->password);
    }
    obj->password_len = password_len;
    obj->password = estrndup(password, password_len);

    RETURN_ZVAL(getThis(), 1, 0);
} /* }}} */

void pcbc_generate_password_lcb_auth(pcbc_password_authenticator_t *auth, lcb_AUTHENTICATOR **result, lcb_type_t type,
                                     const char *name, const char *password, char **hash TSRMLS_DC)
{
    PHP_MD5_CTX md5;
    unsigned char digest[16];

    *result = lcbauth_new();
    lcbauth_set_mode(*result, LCBAUTH_MODE_RBAC);
    PHP_MD5Init(&md5);

    if (name && password) {
        lcbauth_add_pass(*result, name, password, LCBAUTH_F_CLUSTER);
        PHP_MD5Update(&md5, "rbac-extra", sizeof("rbac-extra"));
        PHP_MD5Update(&md5, name, strlen(name));
        PHP_MD5Update(&md5, password, strlen(password));
    } else {
        lcbauth_add_pass(*result, auth->username, auth->password, LCBAUTH_F_CLUSTER);
        PHP_MD5Update(&md5, "rbac", sizeof("rbac"));
        PHP_MD5Update(&md5, auth->username, auth->username_len);
        PHP_MD5Update(&md5, auth->password, auth->password_len);
    }
    PHP_MD5Final(digest, &md5);
    *hash = ecalloc(sizeof(char), 33);
    make_digest(*hash, digest);
}

ZEND_BEGIN_ARG_INFO_EX(ai_PasswordAuthenticator_none, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_PasswordAuthenticator_username, 0, 0, 1)
ZEND_ARG_INFO(0, username)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_PasswordAuthenticator_password, 0, 0, 2)
ZEND_ARG_INFO(0, password)
ZEND_END_ARG_INFO()

// clang-format off
zend_function_entry password_authenticator_methods[] = {
    PHP_ME(PasswordAuthenticator, __construct, ai_PasswordAuthenticator_none, ZEND_ACC_PUBLIC | ZEND_ACC_FINAL | ZEND_ACC_CTOR)
    PHP_ME(PasswordAuthenticator, username, ai_PasswordAuthenticator_username, ZEND_ACC_PUBLIC | ZEND_ACC_FINAL)
    PHP_ME(PasswordAuthenticator, password, ai_PasswordAuthenticator_password, ZEND_ACC_PUBLIC | ZEND_ACC_FINAL)
    PHP_FE_END
};
// clang-format on

zend_object_handlers password_authenticator_handlers;

static void password_authenticator_free_object(pcbc_free_object_arg *object TSRMLS_DC) /* {{{ */
{
    pcbc_password_authenticator_t *obj = Z_PASSWORD_AUTHENTICATOR_OBJ(object);
    pcbc_credential_t *ptr;

    if (obj->username != NULL) {
        efree(obj->username);
    }
    if (obj->password != NULL) {
        efree(obj->password);
    }

    zend_object_std_dtor(&obj->std TSRMLS_CC);
#if PHP_VERSION_ID < 70000
    efree(obj);
#endif
} /* }}} */

static pcbc_create_object_retval authenticator_create_object(zend_class_entry *class_type TSRMLS_DC)
{
    pcbc_password_authenticator_t *obj = NULL;

    obj = PCBC_ALLOC_OBJECT_T(pcbc_password_authenticator_t, class_type);

    zend_object_std_init(&obj->std, class_type TSRMLS_CC);
    object_properties_init(&obj->std, class_type);

#if PHP_VERSION_ID >= 70000
    obj->std.handlers = &password_authenticator_handlers;
    return &obj->std;
#else
    {
        zend_object_value ret;
        ret.handle = zend_objects_store_put(obj, (zend_objects_store_dtor_t)zend_objects_destroy_object,
                                            password_authenticator_free_object, NULL TSRMLS_CC);
        ret.handlers = &password_authenticator_handlers;
        return ret;
    }
#endif
}

static HashTable *pcbc_password_authenticator_get_debug_info(zval *object, int *is_temp TSRMLS_DC) /* {{{ */
{
    pcbc_password_authenticator_t *obj = NULL;
#if PHP_VERSION_ID >= 70000
    zval retval;
#else
    zval retval = zval_used_for_init;
#endif
    PCBC_ZVAL buckets;
    pcbc_credential_t *ptr;

    *is_temp = 1;
    obj = Z_PASSWORD_AUTHENTICATOR_OBJ_P(object);

    array_init(&retval);
    if (obj->username) {
        ADD_ASSOC_STRING(&retval, "username", obj->username);
    }
    return Z_ARRVAL(retval);
} /* }}} */

PHP_MINIT_FUNCTION(PasswordAuthenticator)
{
    zend_class_entry ce;

    INIT_NS_CLASS_ENTRY(ce, "Couchbase", "PasswordAuthenticator", password_authenticator_methods);
    pcbc_password_authenticator_ce = zend_register_internal_class(&ce TSRMLS_CC);
    pcbc_password_authenticator_ce->create_object = authenticator_create_object;
    PCBC_CE_FLAGS_FINAL(pcbc_password_authenticator_ce);
    PCBC_CE_DISABLE_SERIALIZATION(pcbc_password_authenticator_ce);

    zend_class_implements(pcbc_password_authenticator_ce TSRMLS_CC, 1, pcbc_authenticator_ce);

    memcpy(&password_authenticator_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    password_authenticator_handlers.get_debug_info = pcbc_password_authenticator_get_debug_info;
#if PHP_VERSION_ID >= 70000
    password_authenticator_handlers.free_obj = password_authenticator_free_object;
    password_authenticator_handlers.offset = XtOffsetOf(pcbc_password_authenticator_t, std);
#endif

    zend_register_class_alias("\\CouchbaseAuthenticator", pcbc_password_authenticator_ce);
    return SUCCESS;
}
