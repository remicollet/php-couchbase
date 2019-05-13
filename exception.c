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

zend_class_entry *pcbc_default_exception_ce;
zend_class_entry *pcbc_base_exception_ce;

static void pcbc_exception_make(zval *return_value, zend_class_entry *exception_ce, long code,
                                const char *message TSRMLS_DC)
{
    object_init_ex(return_value, pcbc_base_exception_ce);

    if (message) {
        zend_update_property_string(pcbc_base_exception_ce, return_value, ZEND_STRL("message"), message TSRMLS_CC);
    }
    if (code) {
        zend_update_property_long(pcbc_base_exception_ce, return_value, ZEND_STRL("code"), code TSRMLS_CC);
    }
}

void pcbc_exception_init(zval *return_value, long code, const char *message TSRMLS_DC)
{
    pcbc_exception_make(return_value, pcbc_base_exception_ce, code, message TSRMLS_CC);
}

zend_class_entry *pcbc_http_exception_ce;
zend_class_entry *pcbc_query_exception_ce;
zend_class_entry *pcbc_query_error_exception_ce;
zend_class_entry *pcbc_query_service_exception_ce;
zend_class_entry *pcbc_search_exception_ce;
zend_class_entry *pcbc_analytics_exception_ce;
zend_class_entry *pcbc_view_exception_ce;
zend_class_entry *pcbc_partial_view_exception_ce;
zend_class_entry *pcbc_bindings_exception_ce;
zend_class_entry *pcbc_invalid_state_exception_ce;
zend_class_entry *pcbc_key_value_exception_ce;
zend_class_entry *pcbc_key_not_found_exception_ce;
zend_class_entry *pcbc_key_exists_exception_ce;
zend_class_entry *pcbc_value_too_big_exception_ce;
zend_class_entry *pcbc_key_locked_exception_ce;
zend_class_entry *pcbc_temp_fail_exception_ce;
zend_class_entry *pcbc_path_not_found_exception_ce;
zend_class_entry *pcbc_path_exists_exception_ce;
zend_class_entry *pcbc_invalid_range_exception_ce;
zend_class_entry *pcbc_key_deleted_exception_ce;
zend_class_entry *pcbc_cas_mismatch_exception_ce;
zend_class_entry *pcbc_invalid_configuration_exception_ce;
zend_class_entry *pcbc_service_missing_exception_ce;
zend_class_entry *pcbc_network_exception_ce;
zend_class_entry *pcbc_timeout_exception_ce;
zend_class_entry *pcbc_bucket_missing_exception_ce;
zend_class_entry *pcbc_scope_missing_exception_ce;
zend_class_entry *pcbc_collection_missing_exception_ce;
zend_class_entry *pcbc_authentication_exception_ce;
zend_class_entry *pcbc_bad_input_exception_ce;
zend_class_entry *pcbc_durability_exception_ce;
zend_class_entry *pcbc_subdocument_exception_ce;

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO(ai_BaseException_ref, IS_STRING, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO(ai_BaseException_context, IS_OBJECT, 1)
ZEND_END_ARG_INFO()

PHP_METHOD(BaseException, ref);
PHP_METHOD(BaseException, context);

static const zend_function_entry pcbc_base_exception_methods[] = {
    PHP_ME(BaseException, ref, ai_BaseException_ref, ZEND_ACC_PUBLIC)
    PHP_ME(BaseException, context, ai_BaseException_context, ZEND_ACC_PUBLIC)
    PHP_FE_END
};

PHP_MINIT_FUNCTION(CouchbaseException)
{
    zend_class_entry ce;
    pcbc_default_exception_ce = (zend_class_entry *)zend_exception_get_default(TSRMLS_C);

    INIT_NS_CLASS_ENTRY(ce, "Couchbase", "BaseException", pcbc_base_exception_methods);
    pcbc_base_exception_ce = zend_register_internal_class_ex(&ce, pcbc_default_exception_ce TSRMLS_CC);
    zend_declare_property_null(pcbc_base_exception_ce, ZEND_STRL("ref"), ZEND_ACC_PROTECTED TSRMLS_CC);
    zend_declare_property_null(pcbc_base_exception_ce, ZEND_STRL("context"), ZEND_ACC_PROTECTED TSRMLS_CC);
    zend_declare_property_null(pcbc_base_exception_ce, ZEND_STRL("is_input"), ZEND_ACC_PROTECTED TSRMLS_CC);
    zend_declare_property_null(pcbc_base_exception_ce, ZEND_STRL("is_network"), ZEND_ACC_PROTECTED TSRMLS_CC);
    zend_declare_property_null(pcbc_base_exception_ce, ZEND_STRL("is_fatal"), ZEND_ACC_PROTECTED TSRMLS_CC);
    zend_declare_property_null(pcbc_base_exception_ce, ZEND_STRL("is_transient"), ZEND_ACC_PROTECTED TSRMLS_CC);
    zend_declare_property_null(pcbc_base_exception_ce, ZEND_STRL("is_data_operation"), ZEND_ACC_PROTECTED TSRMLS_CC);
    zend_declare_property_null(pcbc_base_exception_ce, ZEND_STRL("is_internal"), ZEND_ACC_PROTECTED TSRMLS_CC);
    zend_declare_property_null(pcbc_base_exception_ce, ZEND_STRL("is_plugin"), ZEND_ACC_PROTECTED TSRMLS_CC);
    zend_declare_property_null(pcbc_base_exception_ce, ZEND_STRL("is_server_under_load"), ZEND_ACC_PROTECTED TSRMLS_CC);
    zend_declare_property_null(pcbc_base_exception_ce, ZEND_STRL("is_server_generated"), ZEND_ACC_PROTECTED TSRMLS_CC);
    zend_declare_property_null(pcbc_base_exception_ce, ZEND_STRL("is_subdoc"), ZEND_ACC_PROTECTED TSRMLS_CC);
    zend_declare_property_null(pcbc_base_exception_ce, ZEND_STRL("is_durability"), ZEND_ACC_PROTECTED TSRMLS_CC);

    INIT_NS_CLASS_ENTRY(ce, "Couchbase", "HttpException", NULL);
    pcbc_http_exception_ce = zend_register_internal_class_ex(&ce, pcbc_base_exception_ce TSRMLS_CC);

    INIT_NS_CLASS_ENTRY(ce, "Couchbase", "QueryException", NULL);
    pcbc_query_exception_ce = zend_register_internal_class_ex(&ce, pcbc_http_exception_ce TSRMLS_CC);
    INIT_NS_CLASS_ENTRY(ce, "Couchbase", "QueryErrorException", NULL);
    pcbc_query_error_exception_ce = zend_register_internal_class_ex(&ce, pcbc_query_exception_ce TSRMLS_CC);
    INIT_NS_CLASS_ENTRY(ce, "Couchbase", "QueryServiceException", NULL);
    pcbc_query_service_exception_ce = zend_register_internal_class_ex(&ce, pcbc_query_exception_ce TSRMLS_CC);


    INIT_NS_CLASS_ENTRY(ce, "Couchbase", "SearchException", NULL);
    pcbc_search_exception_ce = zend_register_internal_class_ex(&ce, pcbc_http_exception_ce TSRMLS_CC);

    INIT_NS_CLASS_ENTRY(ce, "Couchbase", "AnalyticsException", NULL);
    pcbc_analytics_exception_ce = zend_register_internal_class_ex(&ce, pcbc_http_exception_ce TSRMLS_CC);

    INIT_NS_CLASS_ENTRY(ce, "Couchbase", "ViewException", NULL);
    pcbc_view_exception_ce = zend_register_internal_class_ex(&ce, pcbc_http_exception_ce TSRMLS_CC);
    INIT_NS_CLASS_ENTRY(ce, "Couchbase", "PartialViewException", NULL);
    pcbc_partial_view_exception_ce = zend_register_internal_class_ex(&ce, pcbc_view_exception_ce TSRMLS_CC);

    INIT_NS_CLASS_ENTRY(ce, "Couchbase", "BindingsException", NULL);
    pcbc_bindings_exception_ce = zend_register_internal_class_ex(&ce, pcbc_base_exception_ce TSRMLS_CC);

    INIT_NS_CLASS_ENTRY(ce, "Couchbase", "InvalidStateException", NULL);
    pcbc_invalid_state_exception_ce = zend_register_internal_class_ex(&ce, pcbc_base_exception_ce TSRMLS_CC);

    INIT_NS_CLASS_ENTRY(ce, "Couchbase", "KeyValueException", NULL);
    pcbc_key_value_exception_ce = zend_register_internal_class_ex(&ce, pcbc_base_exception_ce TSRMLS_CC);
    INIT_NS_CLASS_ENTRY(ce, "Couchbase", "KeyNotFoundException", NULL);
    pcbc_key_not_found_exception_ce = zend_register_internal_class_ex(&ce, pcbc_key_value_exception_ce TSRMLS_CC);
    INIT_NS_CLASS_ENTRY(ce, "Couchbase", "KeyExistsException", NULL);
    pcbc_key_exists_exception_ce = zend_register_internal_class_ex(&ce, pcbc_key_value_exception_ce TSRMLS_CC);
    INIT_NS_CLASS_ENTRY(ce, "Couchbase", "ValueTooBigException", NULL);
    pcbc_value_too_big_exception_ce = zend_register_internal_class_ex(&ce, pcbc_key_value_exception_ce TSRMLS_CC);
    INIT_NS_CLASS_ENTRY(ce, "Couchbase", "KeyLockedException", NULL);
    pcbc_key_locked_exception_ce = zend_register_internal_class_ex(&ce, pcbc_key_value_exception_ce TSRMLS_CC);
    INIT_NS_CLASS_ENTRY(ce, "Couchbase", "TempFailException", NULL);
    pcbc_temp_fail_exception_ce = zend_register_internal_class_ex(&ce, pcbc_key_value_exception_ce TSRMLS_CC);
    INIT_NS_CLASS_ENTRY(ce, "Couchbase", "PathNotFoundException", NULL);
    pcbc_path_not_found_exception_ce = zend_register_internal_class_ex(&ce, pcbc_key_value_exception_ce TSRMLS_CC);
    INIT_NS_CLASS_ENTRY(ce, "Couchbase", "PathExistsException", NULL);
    pcbc_path_exists_exception_ce = zend_register_internal_class_ex(&ce, pcbc_key_value_exception_ce TSRMLS_CC);
    INIT_NS_CLASS_ENTRY(ce, "Couchbase", "InvalidRangeException", NULL);
    pcbc_invalid_range_exception_ce = zend_register_internal_class_ex(&ce, pcbc_key_value_exception_ce TSRMLS_CC);
    INIT_NS_CLASS_ENTRY(ce, "Couchbase", "KeyDeletedException", NULL);
    pcbc_key_deleted_exception_ce = zend_register_internal_class_ex(&ce, pcbc_key_value_exception_ce TSRMLS_CC);
    INIT_NS_CLASS_ENTRY(ce, "Couchbase", "CasMismatchException", NULL);
    pcbc_cas_mismatch_exception_ce = zend_register_internal_class_ex(&ce, pcbc_key_value_exception_ce TSRMLS_CC);

    INIT_NS_CLASS_ENTRY(ce, "Couchbase", "InvalidConfigurationException", NULL);
    pcbc_invalid_configuration_exception_ce = zend_register_internal_class_ex(&ce, pcbc_base_exception_ce TSRMLS_CC);

    INIT_NS_CLASS_ENTRY(ce, "Couchbase", "ServiceMissingException", NULL);
    pcbc_service_missing_exception_ce = zend_register_internal_class_ex(&ce, pcbc_base_exception_ce TSRMLS_CC);

    INIT_NS_CLASS_ENTRY(ce, "Couchbase", "NetworkException", NULL);
    pcbc_network_exception_ce = zend_register_internal_class_ex(&ce, pcbc_base_exception_ce TSRMLS_CC);

    INIT_NS_CLASS_ENTRY(ce, "Couchbase", "TimeoutException", NULL);
    pcbc_timeout_exception_ce = zend_register_internal_class_ex(&ce, pcbc_base_exception_ce TSRMLS_CC);

    INIT_NS_CLASS_ENTRY(ce, "Couchbase", "BucketMissingException", NULL);
    pcbc_bucket_missing_exception_ce = zend_register_internal_class_ex(&ce, pcbc_base_exception_ce TSRMLS_CC);

    INIT_NS_CLASS_ENTRY(ce, "Couchbase", "ScopeMissingException", NULL);
    pcbc_scope_missing_exception_ce = zend_register_internal_class_ex(&ce, pcbc_base_exception_ce TSRMLS_CC);

    INIT_NS_CLASS_ENTRY(ce, "Couchbase", "CollectionMissingException", NULL);
    pcbc_collection_missing_exception_ce = zend_register_internal_class_ex(&ce, pcbc_base_exception_ce TSRMLS_CC);

    INIT_NS_CLASS_ENTRY(ce, "Couchbase", "AuthenticationException", NULL);
    pcbc_authentication_exception_ce = zend_register_internal_class_ex(&ce, pcbc_base_exception_ce TSRMLS_CC);

    INIT_NS_CLASS_ENTRY(ce, "Couchbase", "BadInputException", NULL);
    pcbc_bad_input_exception_ce = zend_register_internal_class_ex(&ce, pcbc_base_exception_ce TSRMLS_CC);

    INIT_NS_CLASS_ENTRY(ce, "Couchbase", "DurabilityException", NULL);
    pcbc_durability_exception_ce = zend_register_internal_class_ex(&ce, pcbc_base_exception_ce TSRMLS_CC);

    INIT_NS_CLASS_ENTRY(ce, "Couchbase", "SubdocumentException", NULL);
    pcbc_subdocument_exception_ce = zend_register_internal_class_ex(&ce, pcbc_base_exception_ce TSRMLS_CC);

    return SUCCESS;
}

void pcbc_create_lcb_exception(zval *return_value, long code, zend_string *context, zend_string *ref TSRMLS_DC)
{
    zend_class_entry *exc_ce = NULL;

    switch (code) {
        case LCB_SUCCESS:
            return;

        case LCB_AUTH_CONTINUE:
        case LCB_AUTH_ERROR:
        case LCB_NOT_AUTHORIZED:
        case LCB_SASLMECH_UNAVAILABLE:
        case LCB_INVALID_USERNAME:
            exc_ce = pcbc_authentication_exception_ce;
            break;

        case LCB_KEY_EEXISTS:
            exc_ce = pcbc_key_exists_exception_ce;
            break;

        case LCB_KEY_ENOENT:
            exc_ce = pcbc_key_not_found_exception_ce;
            break;

        case LCB_E2BIG:
            exc_ce = pcbc_value_too_big_exception_ce;
            break;

        case LCB_ETMPFAIL:
        case LCB_EBUSY:
        case LCB_GENERIC_TMPERR:
        case LCB_BUSY:
        case LCB_MAP_CHANGED:
            exc_ce = pcbc_temp_fail_exception_ce;
            break;

        case LCB_SUBDOC_PATH_ENOENT:
            exc_ce = pcbc_path_not_found_exception_ce;
            break;

        case LCB_SUBDOC_PATH_EEXISTS:
            exc_ce = pcbc_path_exists_exception_ce;
            break;

        case LCB_SUBDOC_PATH_MISMATCH:
        case LCB_SUBDOC_PATH_EINVAL:
        case LCB_SUBDOC_PATH_E2BIG:
        case LCB_SUBDOC_DOC_E2DEEP:
        case LCB_SUBDOC_VALUE_CANTINSERT:
        case LCB_SUBDOC_DOC_NOTJSON:
        case LCB_SUBDOC_MULTI_FAILURE:
        case LCB_SUBDOC_VALUE_E2DEEP:
        case LCB_GENERIC_SUBDOCERR:
        case LCB_SUBDOC_INVALID_COMBO:
        case LCB_SUBDOC_MULTI_PATH_FAILURE:
        case LCB_SUBDOC_SUCCESS_DELETED:
        case LCB_SUBDOC_XATTR_INVALID_FLAG_COMBO:
        case LCB_SUBDOC_XATTR_INVALID_KEY_COMBO:
        case LCB_SUBDOC_XATTR_UNKNOWN_MACRO:
        case LCB_SUBDOC_XATTR_UNKNOWN_VATTR:
        case LCB_SUBDOC_XATTR_CANT_MODIFY_VATTR:
        case LCB_SUBDOC_MULTI_PATH_FAILURE_DELETED:
        case LCB_SUBDOC_INVALID_XATTR_ORDER:
            exc_ce = pcbc_subdocument_exception_ce;
            break;

        case LCB_GENERIC_CONSTRAINT_ERR:
        case LCB_INVALID_HOST_FORMAT:
        case LCB_INVALID_CHAR:
        case LCB_UNKNOWN_SDCMD:
        case LCB_EINVAL:
        case LCB_EMPTY_PATH:
        case LCB_OPTIONS_CONFLICT:
        case LCB_EMPTY_KEY:
            exc_ce = pcbc_bad_input_exception_ce;
            break;

        case LCB_EINTERNAL:
        case LCB_NOT_SUPPORTED:
        case LCB_UNKNOWN_COMMAND:
        case LCB_PROTOCOL_ERROR:
        case LCB_ERROR:
        case LCB_SERVER_BUG:
        case LCB_EINVAL_MCD:
        case LCB_ECTL_UNKNOWN:
        case LCB_ECTL_UNSUPPMODE:
        case LCB_ECTL_BADARG:
        case LCB_NOT_STORED:
            exc_ce = pcbc_invalid_state_exception_ce;
            break;

        case LCB_DLOPEN_FAILED:
        case LCB_DLSYM_FAILED:
        case LCB_CLIENT_ENOMEM:
        case LCB_DUPLICATE_COMMANDS:
        case LCB_PLUGIN_VERSION_MISMATCH:
        case LCB_NOT_MY_VBUCKET:
        case LCB_BAD_ENVIRONMENT:
        case LCB_INCOMPLETE_PACKET:
        case LCB_EBADHANDLE:
        case LCB_ENOMEM:
        case LCB_SCHEDFAIL_INTERNAL:
        case LCB_UNKNOWN_MEMCACHED_ERROR:
        case LCB_ENO_COMMANDS:
            exc_ce = pcbc_bindings_exception_ce;
            break;

        case LCB_BUCKET_ENOENT:
            exc_ce = pcbc_bucket_missing_exception_ce;
            break;

        case LCB_DELTA_BADVAL:
        case LCB_ERANGE:
        case LCB_SUBDOC_NUM_ERANGE:
        case LCB_SUBDOC_BAD_DELTA:
            exc_ce = pcbc_invalid_range_exception_ce;
            break;

        case LCB_ECONNREFUSED:
        case LCB_ESOCKSHUTDOWN:
        case LCB_ECONNRESET:
        case LCB_ECANTGETPORT:
        case LCB_EFDLIMITREACHED:
        case LCB_ENETUNREACH:
        case LCB_NETWORK_ERROR:
        case LCB_CONNECT_ERROR:
        case LCB_UNKNOWN_HOST:
        case LCB_SSL_ERROR:
        case LCB_SSL_CANTVERIFY:
        case LCB_NAMESERVER_ERROR:
        case LCB_TOO_MANY_REDIRECTS:
            exc_ce = pcbc_network_exception_ce;
            break;

        case LCB_COLLECTION_UNKNOWN:
            exc_ce = pcbc_collection_missing_exception_ce;
            break;

        case LCB_CLIENT_ENOCONF:
        case LCB_CONFIG_CACHE_INVALID:
        case LCB_CLIENT_FEATURE_UNAVAILABLE:
        case LCB_COLLECTION_NO_MANIFEST:
        case LCB_COLLECTION_CANNOT_APPLY_MANIFEST:
        case LCB_COLLECTION_MANIFEST_IS_AHEAD:
            exc_ce = pcbc_invalid_configuration_exception_ce;
            break;

        case LCB_DURABILITY_ETOOMANY:
        case LCB_NO_MATCHING_SERVER:
        case LCB_MUTATION_LOST:
        case LCB_DURABILITY_NO_MUTATION_TOKENS:
        case LCB_DURABILITY_INVALID_LEVEL:
        case LCB_DURABILITY_IMPOSSIBLE:
        case LCB_DURABILITY_SYNC_WRITE_IN_PROGRESS:
        case LCB_DURABILITY_SYNC_WRITE_AMBIGUOUS:
            exc_ce = pcbc_durability_exception_ce;
            break;

        case LCB_ETIMEDOUT:
            exc_ce = pcbc_timeout_exception_ce;
            break;

        case LCB_HTTP_ERROR:
            exc_ce = pcbc_http_exception_ce;
            break;

        case LCB_QUERY_ERROR:
            exc_ce = pcbc_http_exception_ce;
            break;

        default:
            exc_ce = pcbc_base_exception_ce;
            break;
    }
    object_init_ex(return_value, exc_ce);
    zend_update_property_long(pcbc_default_exception_ce, return_value, ZEND_STRL("code"), code TSRMLS_CC);
    zend_update_property_string(pcbc_default_exception_ce, return_value, ZEND_STRL("message"), lcb_strerror_short(code) TSRMLS_CC);

    if (ref) {
        zend_update_property_str(pcbc_base_exception_ce, return_value, ZEND_STRL("ref"), ref TSRMLS_CC);
    }
    if (context) {
        zend_update_property_str(pcbc_base_exception_ce, return_value, ZEND_STRL("context"), context TSRMLS_CC);
    }

    uint32_t errtype = lcb_get_errtype(code);
    zend_update_property_bool(pcbc_base_exception_ce, return_value, ZEND_STRL("is_input"), (errtype & LCB_ERRTYPE_INPUT) TSRMLS_CC);
    zend_update_property_bool(pcbc_base_exception_ce, return_value, ZEND_STRL("is_network"), (errtype & LCB_ERRTYPE_NETWORK) TSRMLS_CC);
    zend_update_property_bool(pcbc_base_exception_ce, return_value, ZEND_STRL("is_fatal"), (errtype & LCB_ERRTYPE_FATAL) TSRMLS_CC);
    zend_update_property_bool(pcbc_base_exception_ce, return_value, ZEND_STRL("is_transient"), (errtype & LCB_ERRTYPE_TRANSIENT) TSRMLS_CC);
    zend_update_property_bool(pcbc_base_exception_ce, return_value, ZEND_STRL("is_data_operation"), (errtype & LCB_ERRTYPE_DATAOP) TSRMLS_CC);
    zend_update_property_bool(pcbc_base_exception_ce, return_value, ZEND_STRL("is_internal"), (errtype & LCB_ERRTYPE_INTERNAL) TSRMLS_CC);
    zend_update_property_bool(pcbc_base_exception_ce, return_value, ZEND_STRL("is_plugin"), (errtype & LCB_ERRTYPE_PLUGIN) TSRMLS_CC);
    zend_update_property_bool(pcbc_base_exception_ce, return_value, ZEND_STRL("is_server_under_load"), (errtype & LCB_ERRTYPE_SRVLOAD) TSRMLS_CC);
    zend_update_property_bool(pcbc_base_exception_ce, return_value, ZEND_STRL("is_server_generated"), (errtype & LCB_ERRTYPE_SRVGEN) TSRMLS_CC);
    zend_update_property_bool(pcbc_base_exception_ce, return_value, ZEND_STRL("is_subdoc"), (errtype & LCB_ERRTYPE_SUBDOC) TSRMLS_CC);
    zend_update_property_bool(pcbc_base_exception_ce, return_value, ZEND_STRL("is_durability"), (errtype & LCB_ERRTYPE_DURABILITY) TSRMLS_CC);
}

PHP_METHOD(BaseException, context)
{
    if (zend_parse_parameters_none_throw() == FAILURE) {
        return;
    }

    zval *prop, rv;
    prop = zend_read_property(pcbc_base_exception_ce, getThis(), ZEND_STRL("context"), 0, &rv);
    ZVAL_DEREF(prop);
    ZVAL_COPY(return_value, prop);
}

PHP_METHOD(BaseException, code)
{
    if (zend_parse_parameters_none_throw() == FAILURE) {
        return;
    }

    zval *prop, rv;
    prop = zend_read_property(pcbc_base_exception_ce, getThis(), ZEND_STRL("code"), 0, &rv);
    ZVAL_DEREF(prop);
    ZVAL_COPY(return_value, prop);
}

PHP_METHOD(BaseException, message)
{
    if (zend_parse_parameters_none_throw() == FAILURE) {
        return;
    }

    zval *prop, rv;
    prop = zend_read_property(pcbc_base_exception_ce, getThis(), ZEND_STRL("message"), 0, &rv);
    ZVAL_DEREF(prop);
    ZVAL_COPY(return_value, prop);
}

PHP_METHOD(BaseException, ref)
{
    if (zend_parse_parameters_none_throw() == FAILURE) {
        return;
    }

    zval *prop, rv;
    prop = zend_read_property(pcbc_base_exception_ce, getThis(), ZEND_STRL("ref"), 0, &rv);
    ZVAL_DEREF(prop);
    ZVAL_COPY(return_value, prop);
}

/*
 * vim: et ts=4 sw=4 sts=4
 */
