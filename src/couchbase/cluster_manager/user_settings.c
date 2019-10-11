/**
 *     Copyright 2017-2019 Couchbase, Inc.
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

#define LOGARGS(lvl) LCB_LOG_##lvl, NULL, "pcbc/user_settings", __FILE__, __LINE__

zend_class_entry *pcbc_user_settings_ce;

PHP_METHOD(UserSettings, __construct)
{
    pcbc_user_settings_t *obj;
    int rv;

    rv = zend_parse_parameters_none();
    if (rv == FAILURE) {
        throw_pcbc_exception("Invalid arguments.", LCB_EINVAL);
        RETURN_NULL();
    }
    obj = Z_USER_SETTINGS_OBJ_P(getThis());
    obj->full_name = NULL;
    obj->full_name_len = 0;
    obj->password = NULL;
    obj->password_len = 0;
    memset(&obj->roles, 0, sizeof(obj->roles));
}

PHP_METHOD(UserSettings, fullName)
{
    pcbc_user_settings_t *obj;
    char *full_name = NULL;
    int rv;
    size_t full_name_len;

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &full_name, &full_name_len);
    if (rv == FAILURE) {
        RETURN_NULL();
    }

    obj = Z_USER_SETTINGS_OBJ_P(getThis());
    if (obj->full_name) {
        efree(obj->full_name);
    }
    obj->full_name = estrndup(full_name, full_name_len);
    obj->full_name_len = full_name_len;

    RETURN_ZVAL(getThis(), 1, 0);
}

PHP_METHOD(UserSettings, password)
{
    pcbc_user_settings_t *obj;
    char *password = NULL;
    int rv;
    size_t password_len;

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &password, &password_len);
    if (rv == FAILURE) {
        RETURN_NULL();
    }

    obj = Z_USER_SETTINGS_OBJ_P(getThis());
    if (obj->password) {
        efree(obj->password);
    }
    obj->password = estrndup(password, password_len);
    obj->password_len = password_len;

    RETURN_ZVAL(getThis(), 1, 0);
}

PHP_METHOD(UserSettings, role)
{
    pcbc_user_settings_t *obj;
    char *role = NULL, *bucket = NULL;
    int rv;
    size_t role_len = 0, bucket_len = 0;

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|s", &role, &role_len, &bucket, &bucket_len);
    if (rv == FAILURE) {
        RETURN_NULL();
    }

    obj = Z_USER_SETTINGS_OBJ_P(getThis());
    if (PCBC_SMARTSTR_LEN(obj->roles)) {
        smart_str_appendc(&obj->roles, ',');
    }
    smart_str_appendl(&obj->roles, role, role_len);
    if (bucket_len) {
        smart_str_appendc(&obj->roles, '[');
        smart_str_appendl(&obj->roles, bucket, bucket_len);
        smart_str_appendc(&obj->roles, ']');
    }

    RETURN_ZVAL(getThis(), 1, 0);
}

ZEND_BEGIN_ARG_INFO_EX(ai_UserSettings_none, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_UserSettings_fullName, 0, 0, 1)
ZEND_ARG_INFO(0, fullName)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_UserSettings_password, 0, 0, 1)
ZEND_ARG_INFO(0, password)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_UserSettings_role, 0, 0, 2)
ZEND_ARG_INFO(0, role)
ZEND_ARG_INFO(0, bucket)
ZEND_END_ARG_INFO()

// clang-format off
zend_function_entry user_settings_methods[] = {
    PHP_ME(UserSettings, __construct, ai_UserSettings_none, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
    PHP_ME(UserSettings, fullName, ai_UserSettings_fullName, ZEND_ACC_PUBLIC)
    PHP_ME(UserSettings, password, ai_UserSettings_password, ZEND_ACC_PUBLIC)
    PHP_ME(UserSettings, role, ai_UserSettings_role, ZEND_ACC_PUBLIC)
    PHP_FE_END
};
// clang-format on

zend_object_handlers user_settings_handlers;

static void user_settings_free_object(zend_object *object TSRMLS_DC)
{
    pcbc_user_settings_t *obj = Z_USER_SETTINGS_OBJ(object);

    if (obj->full_name) {
        efree(obj->full_name);
    }
    if (obj->password) {
        efree(obj->password);
    }
    if (PCBC_SMARTSTR_LEN(obj->roles)) {
        smart_str_free(&obj->roles);
    }

    zend_object_std_dtor(&obj->std TSRMLS_CC);
}

static zend_object *user_settings_create_object(zend_class_entry *class_type TSRMLS_DC)
{
    pcbc_user_settings_t *obj = NULL;

    obj = PCBC_ALLOC_OBJECT_T(pcbc_user_settings_t, class_type);

    zend_object_std_init(&obj->std, class_type TSRMLS_CC);
    object_properties_init(&obj->std, class_type);

    obj->std.handlers = &user_settings_handlers;
    return &obj->std;
}

static HashTable *pcbc_user_settings_get_debug_info(zval *object, int *is_temp TSRMLS_DC)
{
    pcbc_user_settings_t *obj = NULL;
    zval retval;

    *is_temp = 1;
    obj = Z_USER_SETTINGS_OBJ_P(object);

    array_init(&retval);
    if (obj->full_name) {
        add_assoc_stringl(&retval, "fullName", obj->full_name, obj->full_name_len);
    }
    if (PCBC_SMARTSTR_LEN(obj->roles)) {
        add_assoc_stringl(&retval, "roles", PCBC_SMARTSTR_VAL(obj->roles), PCBC_SMARTSTR_LEN(obj->roles));
    }
    return Z_ARRVAL(retval);
}

PHP_MINIT_FUNCTION(UserSettings)
{
    zend_class_entry ce;

    INIT_NS_CLASS_ENTRY(ce, "Couchbase", "UserSettings", user_settings_methods);
    pcbc_user_settings_ce = zend_register_internal_class(&ce TSRMLS_CC);
    pcbc_user_settings_ce->create_object = user_settings_create_object;
    PCBC_CE_DISABLE_SERIALIZATION(pcbc_user_settings_ce);

    memcpy(&user_settings_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    user_settings_handlers.get_debug_info = pcbc_user_settings_get_debug_info;
    user_settings_handlers.free_obj = user_settings_free_object;
    user_settings_handlers.offset = XtOffsetOf(pcbc_user_settings_t, std);

    return SUCCESS;
}
