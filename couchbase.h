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

#ifndef COUCHBASE_H_
#define COUCHBASE_H_

// clang-format off
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <php.h>

#include <libcouchbase/couchbase.h>
#include <libcouchbase/ixmgmt.h>
#include <libcouchbase/crypto.h>

#include <ext/standard/base64.h>
#include <ext/standard/php_var.h>
#include <zend_smart_str.h>
#include <zend_interfaces.h>
#include <zend_exceptions.h>

#include <ext/json/php_json.h>

#include "php_couchbase.h"
#include "log.h"
#include "contrib/php_array.h"
#include <string.h>
// clang-format on

#define PCBC_ARG_VARIADIC_INFO(__pcbc_pass_by_ref, __pcbc_name)                                                        \
    ZEND_ARG_VARIADIC_INFO((__pcbc_pass_by_ref), (__pcbc_name))

enum pcbc_constants {
    PERSISTTO_ONE = 1,
    PERSISTTO_TWO = 2,
    PERSISTTO_THREE = 4,
    PERSISTTO_MASTER = PERSISTTO_ONE,
    REPLICATETO_ONE = 1 << 4,
    REPLICATETO_TWO = 2 << 4,
    REPLICATETO_THREE = 4 << 4
};

struct pcbc_connection {
    lcb_type_t type;
    char *connstr;
    char *bucketname;
    char *auth_hash;
    lcb_INSTANCE *lcb;
    int refs;
    time_t idle_at;
};
typedef struct pcbc_connection pcbc_connection_t;
 lcb_STATUS pcbc_connection_get(pcbc_connection_t **result, lcb_type_t type, const char *connstr,
                                const char *bucketname, lcb_AUTHENTICATOR *auth, char *auth_hash TSRMLS_DC);
void pcbc_connection_addref(pcbc_connection_t *conn TSRMLS_DC);
void pcbc_connection_delref(pcbc_connection_t *conn TSRMLS_DC);
void pcbc_connection_cleanup();

ZEND_BEGIN_MODULE_GLOBALS(couchbase)
char *log_level;

char *enc_format;
char *enc_cmpr;
int enc_format_i;
int enc_cmpr_i;
long enc_cmpr_threshold;
long pool_max_idle_time;
double enc_cmpr_factor;
zend_bool dec_json_array;
ZEND_END_MODULE_GLOBALS(couchbase)
ZEND_EXTERN_MODULE_GLOBALS(couchbase)

#ifdef ZTS
#define PCBCG(v) TSRMG(couchbase_globals_id, zend_couchbase_globals *, v)
#else
#define PCBCG(v) (couchbase_globals.v)
#endif

#if PHP_VERSION_ID < 70200

#undef ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO
#define ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO(name, type, allow_null) \
	ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(name, 0, -1, type, NULL, allow_null)

#define zend_parse_parameters_none_throw zend_parse_parameters_none

#define ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO(name, class_name, allow_null) \
	ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(name, 0, -1, IS_OBJECT, NULL, allow_null)

#define ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(name, return_reference, required_num_args, class_name, allow_null) \
	ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(name, return_reference, required_num_args, IS_OBJECT, NULL, allow_null)
#endif

#define PCBC_DATE_FORMAT_RFC3339 "Y-m-d\\TH:i:sP"

extern char *pcbc_client_string;
extern zend_class_entry *pcbc_analytics_query_ce;
extern zend_class_entry *pcbc_n1ql_query_ce;
extern zend_class_entry *pcbc_cluster_ce;
extern zend_class_entry *pcbc_bucket_ce;
extern zend_class_entry *pcbc_document_fragment_ce;
extern zend_class_entry *pcbc_mutation_token_ce;
extern zend_class_entry *pcbc_mutation_state_ce;
extern zend_class_entry *pcbc_exception_ce;
extern zend_class_entry *pcbc_search_query_ce;
extern zend_class_entry *pcbc_search_query_part_ce;
extern zend_class_entry *pcbc_search_sort_ce;
extern zend_class_entry *pcbc_search_facet_ce;
extern zend_class_entry *pcbc_view_query_encodable_ce;
extern zend_class_entry *pcbc_json_serializable_ce;
extern zend_class_entry *pcbc_user_settings_ce;
extern zend_class_entry *pcbc_crypto_provider_ce;

extern zend_class_entry *pcbc_collection_ce;

#define PCBC_RESOLVE_COLLECTION \
    const char *collection_str = NULL, *scope_str = NULL; \
    size_t collection_len = 0, scope_len = 0; \
    pcbc_bucket_t *bucket = NULL; \
    { \
        zval *prop, rv; \
        zval *self = getThis(); \
        prop = zend_read_property(pcbc_collection_ce, self, ZEND_STRL("bucket"), 0, &rv); \
        bucket = Z_BUCKET_OBJ_P(prop); \
        prop = zend_read_property(pcbc_collection_ce, self, ZEND_STRL("scope"), 0, &rv); \
        if (Z_TYPE_P(prop) == IS_STRING) { \
            scope_str = Z_STRVAL_P(prop); \
            scope_len = Z_STRLEN_P(prop); \
        } \
        prop = zend_read_property(pcbc_collection_ce, self, ZEND_STRL("name"), 0, &rv); \
        if (Z_TYPE_P(prop) == IS_STRING) { \
            collection_str = Z_STRVAL_P(prop); \
            collection_len = Z_STRLEN_P(prop); \
        } \
    }

void pcbc_create_lcb_exception(zval *return_value, long code, zend_string *context, zend_string *ref TSRMLS_DC);

void pcbc_exception_init(zval *return_value, long code, const char *message TSRMLS_DC);
#define throw_pcbc_exception(__pcbc_message, __pcbc_code)                                                              \
    do {                                                                                                               \
        zval __pcbc_error;                                                                                             \
        ZVAL_UNDEF(&__pcbc_error);                                                                                     \
        pcbc_exception_init(&__pcbc_error, __pcbc_code, __pcbc_message TSRMLS_CC);                                     \
        zend_throw_exception_object(&__pcbc_error TSRMLS_CC);                                                          \
    } while (0)

#define throw_lcb_exception(code, result_ce)                                                                           \
    do {                                                                                                               \
        zend_string *ctx = NULL, *ref = NULL;                                                                          \
        zval *zref, __rv1, *zctx, __rv2;                                                                               \
        if (result_ce) {                                                                                               \
            zref = zend_read_property(result_ce, return_value, ZEND_STRL("err_ref"), 0, &__rv1);                       \
            if (Z_TYPE_P(zref) == IS_STRING) {                                                                         \
                ref = Z_STR_P(zref);                                                                                   \
            }                                                                                                          \
            zctx = zend_read_property(result_ce, return_value, ZEND_STRL("err_ctx"), 0, &__rv2);                       \
            if (Z_TYPE_P(zctx) == IS_STRING) {                                                                         \
                ctx = Z_STR_P(zctx);                                                                                   \
            }                                                                                                          \
        }                                                                                                              \
        zval __pcbc_error;                                                                                             \
        ZVAL_UNDEF(&__pcbc_error);                                                                                     \
        pcbc_create_lcb_exception(&__pcbc_error, code, ctx, ref TSRMLS_CC);                                            \
        zend_throw_exception_object(&__pcbc_error TSRMLS_CC);                                                          \
    } while (0)

#define PCBC_CHECK_ZVAL(__pcbc_val, __pcbc_type, __pcbc_message)                                                       \
    if (__pcbc_val && Z_TYPE_P(__pcbc_val) != __pcbc_type) {                                                           \
        throw_pcbc_exception(__pcbc_message, LCB_EINVAL);                                                              \
        RETURN_NULL();                                                                                                 \
    }
#define PCBC_CHECK_ZVAL_ARRAY(__pcbc_val, __pcbc_message) PCBC_CHECK_ZVAL((__pcbc_val), IS_ARRAY, (__pcbc_message))
#define PCBC_CHECK_ZVAL_STRING(__pcbc_val, __pcbc_message) PCBC_CHECK_ZVAL((__pcbc_val), IS_STRING, (__pcbc_message))
#define PCBC_CHECK_ZVAL_LONG(__pcbc_val, __pcbc_message) PCBC_CHECK_ZVAL((__pcbc_val), IS_LONG, (__pcbc_message))
#define PCBC_CHECK_ZVAL_BOOL(__pcbc_val, __pcbc_message)                                                               \
    if (__pcbc_val && !PCBC_ISBOOL(__pcbc_val)) {                                                                      \
        throw_pcbc_exception(__pcbc_message, LCB_EINVAL);                                                              \
        RETURN_NULL();                                                                                                 \
    }

#define PCBC_CONTENT_TYPE_FORM "application/x-www-form-urlencoded"
#define PCBC_CONTENT_TYPE_JSON "application/json"

#define PCBC_ISBOOL(__pcbc_val) (Z_TYPE_P(__pcbc_val) == IS_TRUE || Z_TYPE_P(__pcbc_val) == IS_FALSE)

#define PCBC_ARRAY_MERGE(__pcbc_dest, __pcbc_src) php_array_merge((__pcbc_dest), (__pcbc_src)TSRMLS_CC)

#define ADD_ASSOC_STRING(zv, key, value) add_assoc_string_ex(zv, ZEND_STRL(key), (char *)(value));
#define ADD_ASSOC_STRINGL(zv, key, value, len) add_assoc_stringl_ex(zv, ZEND_STRL(key), (char *)(value), len);
#define ADD_ASSOC_LONG_EX(zv, key, value) add_assoc_long_ex(zv, ZEND_STRL(key), value);
#define ADD_ASSOC_DOUBLE_EX(zv, key, value) add_assoc_double_ex(zv, ZEND_STRL(key), value);
#define ADD_ASSOC_BOOL_EX(zv, key, value) add_assoc_bool_ex(zv, ZEND_STRL(key), value);
#define ADD_ASSOC_ZVAL_EX(zv, key, value) add_assoc_zval_ex(zv, ZEND_STRL(key), value);
#define ADD_ASSOC_NULL_EX(zv, key) add_assoc_null_ex(zv, ZEND_STRL(key));
#define ADD_NEXT_INDEX_STRING(zv, value) add_next_index_string(zv, value);
#define ADD_NEXT_INDEX_STRINGL(zv, value, len) add_next_index_stringl(zv, value, len);

#define PCBC_ALLOC_OBJECT_T(obj_t, class_type)                                                                         \
    (obj_t *)ecalloc(1, sizeof(obj_t) + zend_object_properties_size(class_type))

#define PCBC_CE_DISABLE_SERIALIZATION(ce)                                                                              \
    do {                                                                                                               \
        ce->serialize = zend_class_serialize_deny;                                                                     \
        ce->unserialize = zend_class_unserialize_deny;                                                                 \
    } while (0);

#define PCBC_STRLEN_P(__pcbc_zval) Z_STRLEN((__pcbc_zval))
#define PCBC_STRVAL_P(__pcbc_zval) Z_STRVAL((__pcbc_zval))
#define PCBC_STRLEN_ZP(__pcbc_zval) Z_STRLEN_P((__pcbc_zval))
#define PCBC_STRVAL_ZP(__pcbc_zval) Z_STRVAL_P((__pcbc_zval))
#define PCBC_STRINGL(__pcbc_zval, __pcbc_str, __pcbc_len) ZVAL_STRINGL(&(__pcbc_zval), (__pcbc_str), (__pcbc_len))
#define PCBC_STRINGS(__pcbc_zval, __pcbc_smart_str)                                                                    \
    ZVAL_STRINGL(&(__pcbc_zval), ZSTR_VAL((__pcbc_smart_str).s), ZSTR_LEN((__pcbc_smart_str).s))
#define PCBC_STRING(__pcbc_zval, __pcbc_str) ZVAL_STRING(&(__pcbc_zval), (__pcbc_str))
#define PCBC_PSTRING(__pcbc_zval, __pcbc_str) ZVAL_STRING(&(__pcbc_zval), (__pcbc_str))

#define pcbc_make_printable_zval(__pcbc_expr, __pcbc_expr_copy, __pcbc_use_copy)                                       \
    do {                                                                                                               \
        *(__pcbc_use_copy) = zend_make_printable_zval((__pcbc_expr), (__pcbc_expr_copy));                              \
    } while (0)

/* classes */
#define PCBC_JSON_RESET_STATE                                                                                          \
    do {                                                                                                               \
        JSON_G(error_code) = 0;                                                                                        \
        JSON_G(encode_max_depth) = PHP_JSON_PARSER_DEFAULT_DEPTH;                                                      \
    } while (0)

#define PCBC_JSON_ENCODE(__pcbc_buf, __pcbc_value, __pcbc_flags, __pcbc_error_code)                                    \
    do {                                                                                                               \
        PCBC_JSON_RESET_STATE;                                                                                         \
        php_json_encode((__pcbc_buf), (__pcbc_value), (__pcbc_flags)TSRMLS_CC);                                        \
        (__pcbc_error_code) = JSON_G(error_code);                                                                      \
    } while (0)

#define PCBC_JSON_COPY_DECODE(__pcbc_zval, __pcbc_src, __pcbc_len, __options, __pcbc_error_code)                       \
    do {                                                                                                               \
        char *__copy = estrndup((__pcbc_src), (__pcbc_len));                                                           \
        PCBC_JSON_RESET_STATE;                                                                                         \
        php_json_decode_ex((__pcbc_zval), (__copy), (__pcbc_len), (__options),                                         \
                           PHP_JSON_PARSER_DEFAULT_DEPTH TSRMLS_CC);                                                   \
        efree(__copy);                                                                                                 \
        (__pcbc_error_code) = JSON_G(error_code);                                                                      \
    } while (0)

#define PCBC_READ_PROPERTY(__pcbc_receiver, __pcbc_scope, __pcbc_object, __pcbc_name, __pcbc_silent)                   \
    do {                                                                                                               \
        zval __pcbc_rv;                                                                                                \
        __pcbc_receiver = zend_read_property((__pcbc_scope), (__pcbc_object), ZEND_STRL(__pcbc_name), (__pcbc_silent), \
                                             &__pcbc_rv TSRMLS_CC);                                                    \
    } while (0)

#define PCBC_SMARTSTR_DUP(__pcbc_smart_str, __pcbc_receiver_buf)                                                       \
    do {                                                                                                               \
        (__pcbc_receiver_buf) = estrndup(ZSTR_VAL((__pcbc_smart_str).s), ZSTR_LEN((__pcbc_smart_str).s));              \
    } while (0)
#define PCBC_SMARTSTR_SET(__pcbc_smart_str, __pcbc_receiver_buf, __pcbc_receiver_length)                               \
    do {                                                                                                               \
        (__pcbc_receiver_buf) = ZSTR_VAL((__pcbc_smart_str).s);                                                        \
        (__pcbc_receiver_length) = ZSTR_LEN((__pcbc_smart_str).s);                                                     \
    } while (0)
#define PCBC_SMARTSTR_VAL(__pcbc_smart_str) (char *)ZSTR_VAL((__pcbc_smart_str).s)
#define PCBC_SMARTSTR_LEN(__pcbc_smart_str) ((__pcbc_smart_str).s ? (int)(ZSTR_LEN((__pcbc_smart_str).s)) : 0)
#define PCBC_SMARTSTR_EMPTY(__pcbc_smart_str) ((__pcbc_smart_str).s == NULL || PCBC_SMARTSTR_LEN(__pcbc_smart_str) == 0)

#define PCBC_SMARTSTR_TRACE(__pcbc_smart_str)                                                                          \
    PCBC_SMARTSTR_LEN((__pcbc_smart_str)), PCBC_SMARTSTR_VAL((__pcbc_smart_str))

struct pcbc_credential {
    char *username;
    int username_len;
    char *password;
    int password_len;

    struct pcbc_credential *next;
};
typedef struct pcbc_credential pcbc_credential_t;

typedef struct {
    pcbc_credential_t cluster;
    pcbc_credential_t *buckets;
    pcbc_credential_t *tail;
    int nbuckets;
    zend_object std;
} pcbc_classic_authenticator_t;

typedef struct {
    char *username;
    int username_len;
    char *password;
    int password_len;
    zend_object std;
} pcbc_password_authenticator_t;

typedef struct {
    zend_object std;
} pcbc_cert_authenticator_t;

typedef struct {
    char *connstr;
    zval auth;
    zend_object std;
} pcbc_cluster_t;

typedef struct {
    pcbc_connection_t *conn;
    zend_object std;
} pcbc_cluster_manager_t;

typedef struct {
    pcbc_connection_t *conn;
    zend_object std;
} pcbc_search_index_manager_t;

struct pcbc_crypto_id {
    char *name;
    int name_len;
    struct pcbc_crypto_id *next;
};
typedef struct pcbc_crypto_id pcbc_crypto_id_t;

typedef struct {
    pcbc_connection_t *conn;
    zval encoder;
    zval decoder;
    lcb_BTYPE type;
    pcbc_crypto_id_t *crypto_head; /* registered crypto providers */
    pcbc_crypto_id_t *crypto_tail; /* registered crypto providers */
    zend_object std;
} pcbc_bucket_t;

typedef struct {
    pcbc_connection_t *conn;
    zend_object std;
} pcbc_bucket_manager_t;

struct pcbc_mutation_token {
    char *bucket;
    lcb_MUTATION_TOKEN mt;
    struct pcbc_mutation_token *next;
    zend_object std;
};
typedef struct pcbc_mutation_token pcbc_mutation_token_t;
#define PCBC_MUTATION_TOKEN_ID(p) ((p)->mt.uuid_)
#define PCBC_MUTATION_TOKEN_SEQ(p) ((p)->mt.seqno_)
#define PCBC_MUTATION_TOKEN_VB(p) ((p)->mt.vbid_)

typedef struct {
    int ntokens;
    pcbc_mutation_token_t *head;
    pcbc_mutation_token_t *tail;
    zend_object std;
} pcbc_mutation_state_t;

typedef struct {
    pcbc_bucket_t *bucket;
    zval bucket_zval;
    char *id;
    int id_len;
    lcb_SUBDOCOPS *operations;
    zend_object std;
} pcbc_lookup_in_builder_t;

#define PCBC_SUBDOC_FULLDOC_REPLACE 1
#define PCBC_SUBDOC_FULLDOC_INSERT 2
#define PCBC_SUBDOC_FULLDOC_UPSERT 3

typedef struct {
    zend_object std;
} pcbc_MUTATE_IN_SPEC;

typedef struct {
    zend_object std;
} pcbc_analytics_query_t;

typedef struct {
    int adhoc;
    int cross_bucket;
    zend_object std;
} pcbc_n1ql_query_t;

typedef struct {
    char *index_name;
    int limit;
    int skip;
    zend_bool explain;
    int server_side_timeout;

    zval query_part;
    zval consistency;
    zval fields;
    zval sort;
    zval facets;

    char *highlight_style;
    zval highlight_fields;
    zend_object std;
} pcbc_search_query_t;

#define UPDATE_BEFORE 1
#define UPDATE_NONE 2
#define UPDATE_AFTER 3

#define ORDER_ASCENDING 1
#define ORDER_DESCENDING 2

typedef struct {
    char *design_document;
    char *view_name;
    char *keys;
    int keys_len;
    zend_bool include_docs;
    zval options;
    zend_object std;
} pcbc_view_query_t;

typedef struct {
    char *full_name;
    char *password;
    int full_name_len;
    int password_len;
    smart_str roles;
    zend_object std;
} pcbc_user_settings_t;

/* param parser */
#define PCBC_PP_MAX_ARGS 10

typedef struct {
    char *str;
    uint len;
} pcbc_pp_id;

typedef struct {
    char name[16];
    zval **ptr;
    zval val;
} pcbc_pp_state_arg;

typedef struct {
    pcbc_pp_state_arg args[PCBC_PP_MAX_ARGS];
    int arg_req;
    int arg_opt;
    int arg_named;
    int cur_idx;
    zval zids;
    HashPosition hash_pos;
} pcbc_pp_state;

typedef struct {
    void *next;
    lcb_STATUS err;
    char *err_ctx;
    size_t err_ctx_len;
    char *err_ref;
    size_t err_reflen;
} opcookie_res;

#define PCBC_RESP_ERR_COPY(opcookie, cbtype, respbase)                                                                 \
    do {                                                                                                               \
        const char *ctx, *ref;                                                                                         \
        opcookie.err = respbase->rc;                                                                                   \
        ctx = lcb_resp_get_error_context(cbtype, respbase);                                                            \
        if (ctx) {                                                                                                     \
            opcookie.err_ctx = strdup(ctx);                                                                            \
        }                                                                                                              \
        ref = lcb_resp_get_error_ref(cbtype, respbase);                                                                \
        if (ref) {                                                                                                     \
            opcookie.err_ref = strdup(ref);                                                                            \
        }                                                                                                              \
    } while (0);

#define PCBC_RESP_ERR_FREE(opcookie)                                                                                   \
    do {                                                                                                               \
        free(opcookie.err_ctx);                                                                                        \
        free(opcookie.err_ref);                                                                                        \
    } while (0);

int pcbc_decode_value(zval *return_value, pcbc_bucket_t *bucket, const char *bytes, int bytes_len, lcb_U32 flags,
                      lcb_datatype_t datatype TSRMLS_DC);
int pcbc_encode_value(pcbc_bucket_t *bucket, zval *value, void **bytes, lcb_size_t *nbytes, lcb_uint32_t *flags,
                      lcb_uint8_t *datatype TSRMLS_DC);

lcb_U64 pcbc_base36_decode_str(const char *str, int len);
char *pcbc_base36_encode_str(lcb_U64 num);
lcb_cas_t pcbc_cas_decode(zval *cas TSRMLS_DC);
void pcbc_cas_encode(zval *return_value, lcb_cas_t cas TSRMLS_DC);

void pcbc_bucket_subdoc_request(pcbc_bucket_t *data, void *builder, int is_lookup, zval *return_value TSRMLS_DC);
void pcbc_http_request(zval *return_value, lcb_INSTANCE *conn, lcb_CMDHTTP *cmd, int json_response TSRMLS_DC);

void pcbc_bucket_init(zval *return_value, pcbc_cluster_t *cluster, const char *bucketname,
                      const char *password TSRMLS_DC);
void pcbc_bucket_manager_init(zval *return_value, zval *bucket TSRMLS_DC);
void pcbc_bucket_get(pcbc_bucket_t *obj, pcbc_pp_state *pp_state, pcbc_pp_id *id, zval **lock, zval **expiry,
                     zval **groupid, zval *return_value TSRMLS_DC);

lcb_U32 pcbc_subdoc_options_to_flags(int is_path, int is_lookup, zval *options TSRMLS_DC);
int pcbc_lookup_in_builder_get(pcbc_lookup_in_builder_t *builder, char *path, int path_len, zval *options TSRMLS_DC);
void pcbc_mutate_in_builder_init(zval *return_value, zval *bucket, const char *id, int id_len, lcb_cas_t cas TSRMLS_DC);
int pcbc_mutate_in_builder_upsert(pcbc_MUTATE_IN_SPEC *builder, char *path, int path_len, zval *value,
                                  lcb_U32 flags TSRMLS_DC);
int pcbc_mutate_in_builder_remove(pcbc_MUTATE_IN_SPEC *builder, char *path, int path_len, lcb_U32 flags TSRMLS_DC);
int pcbc_mutate_in_builder_array_append(pcbc_MUTATE_IN_SPEC *builder, char *path, int path_len, zval *value,
                                        lcb_U32 flags TSRMLS_DC);
int pcbc_mutate_in_builder_array_prepend(pcbc_MUTATE_IN_SPEC *builder, char *path, int path_len, zval *value,
                                         lcb_U32 flags TSRMLS_DC);
int pcbc_mutate_in_builder_array_add_unique(pcbc_MUTATE_IN_SPEC *builder, char *path, int path_len, zval *value,
                                            lcb_U32 flags TSRMLS_DC);
int pcbc_mutate_in_builder_replace(pcbc_MUTATE_IN_SPEC *builder, char *path, int path_len, zval *value,
                                   lcb_U32 flags TSRMLS_DC);

void pcbc_cluster_manager_init(zval *return_value, pcbc_cluster_t *cluster, const char *name,
                               const char *password TSRMLS_DC);

int pcbc_n1ix_init(zval *return_value, zval *json TSRMLS_DC);
int pcbc_n1ix_list(pcbc_bucket_manager_t *manager, zval *return_value TSRMLS_DC);
void pcbc_n1ix_create(pcbc_bucket_manager_t *manager, lcb_CMDN1XMGMT *cmd, zend_bool ignore_if_exist,
                      zval *return_value TSRMLS_DC);
void pcbc_n1ix_drop(pcbc_bucket_manager_t *manager, lcb_CMDN1XMGMT *cmd, zend_bool ignore_if_not_exist,
                    zval *return_value TSRMLS_DC);

void pcbc_analytics_query_init(zval *return_value, const char *statement, int statement_len TSRMLS_DC);
void pcbc_n1ql_query_init(zval *return_value, const char *statement, int statement_len TSRMLS_DC);
void pcbc_mutation_state_init(zval *return_value, zval *source TSRMLS_DC);
void pcbc_mutation_token_init(zval *return_value, const char *bucket, const lcb_MUTATION_TOKEN *mt TSRMLS_DC);
void pcbc_mutation_token_init_php(zval *return_value, char *bucket, int bucket_len, long vbid, char *vbuuid,
                                  int vbuuid_len, char *seqno, int seqno_len TSRMLS_DC);
void pcbc_mutation_state_export_for_n1ql(pcbc_mutation_state_t *obj, zval *scan_vectors TSRMLS_DC);
void pcbc_mutation_state_export_for_search(pcbc_mutation_state_t *obj, zval *scan_vectors TSRMLS_DC);

void pcbc_document_init_decode(zval *return_value, pcbc_bucket_t *bucket, const char *bytes, int bytes_len,
                               lcb_U32 flags, lcb_datatype_t datatype, lcb_cas_t cas,
                               const lcb_MUTATION_TOKEN *token TSRMLS_DC);
void pcbc_document_init_counter(zval *return_value, pcbc_bucket_t *bucket, lcb_U64 value, lcb_cas_t cas,
                                const lcb_MUTATION_TOKEN *token TSRMLS_DC);
void pcbc_document_init_error(zval *return_value, opcookie_res *header TSRMLS_DC);
void pcbc_document_init(zval *return_value, pcbc_bucket_t *bucket, const char *bytes, int bytes_len, lcb_U32 flags,
                        lcb_cas_t cas, const lcb_MUTATION_TOKEN *token TSRMLS_DC);

int pcbc_document_fragment_init(zval *return_value, zval *value, zval *cas, zval *token TSRMLS_DC);
int pcbc_document_fragment_init_error(zval *return_value, opcookie_res *header, zval *value TSRMLS_DC);
void pcbc_bucket_n1ql_request(pcbc_bucket_t *bucket, lcb_CMDN1QL *cmd, int json_response, int json_options, int is_cbas,
                              zval *return_value TSRMLS_DC);
void pcbc_bucket_cbft_request(pcbc_bucket_t *bucket, lcb_CMDFTS *cmd, int json_response, int json_options,
                              zval *return_value TSRMLS_DC);
void pcbc_search_index_manager_init(zval *return_value, pcbc_bucket_manager_t *bucket_manager TSRMLS_DC);

void pcbc_boolean_field_search_query_init(zval *return_value, zend_bool value TSRMLS_DC);
void pcbc_boolean_search_query_init(zval *return_value TSRMLS_DC);
void pcbc_date_range_search_query_init(zval *return_value TSRMLS_DC);
void pcbc_match_all_search_query_init(zval *return_value TSRMLS_DC);
void pcbc_match_none_search_query_init(zval *return_value TSRMLS_DC);
void pcbc_match_phrase_search_query_init(zval *return_value, char *match_phrase, int match_phrase_len TSRMLS_DC);
void pcbc_match_search_query_init(zval *return_value, char *match, int match_len TSRMLS_DC);
void pcbc_numeric_range_search_query_init(zval *return_value TSRMLS_DC);
void pcbc_prefix_search_query_init(zval *return_value, char *prefix, int prefix_len TSRMLS_DC);
void pcbc_query_string_search_query_init(zval *return_value, char *query, int query_len TSRMLS_DC);
void pcbc_regexp_search_query_init(zval *return_value, char *regexp, int regexp_len TSRMLS_DC);
void pcbc_term_search_query_init(zval *return_value, char *term, int term_len TSRMLS_DC);
void pcbc_term_range_search_query_init(zval *return_value TSRMLS_DC);
void pcbc_wildcard_search_query_init(zval *return_value, char *wildcard, int wildcard_len TSRMLS_DC);
void pcbc_geo_bounding_box_search_query_init(zval *return_value, double top_left_longitude, double top_left_latitude,
                                             double bottom_right_longitude, double bottom_right_latitude TSRMLS_DC);
void pcbc_geo_distance_search_query_init(zval *return_value, double longitude, double latitude, char *distance,
                                         int distance_len TSRMLS_DC);
void pcbc_term_search_facet_init(zval *return_value, char *field, int field_len, int limit TSRMLS_DC);
void pcbc_date_range_search_facet_init(zval *return_value, char *field, int field_len, int limit TSRMLS_DC);
void pcbc_numeric_range_search_facet_init(zval *return_value, char *field, int field_len, int limit TSRMLS_DC);

void pcbc_search_sort_id_init(zval *return_value TSRMLS_DC);
void pcbc_search_sort_score_init(zval *return_value TSRMLS_DC);
void pcbc_search_sort_field_init(zval *return_value, const char *field, int field_len TSRMLS_DC);
void pcbc_search_sort_geo_distance_init(zval *return_value, const char *field, int field_len, double lon,
                                        double lat TSRMLS_DC);

void pcbc_lookup_in_builder_init(zval *return_value, zval *bucket, const char *id, int id_len, zval *args,
                                 int num_args TSRMLS_DC);
void pcbc_disjunction_search_query_init(zval *return_value, zval *args, int num_args TSRMLS_DC);
void pcbc_conjunction_search_query_init(zval *return_value, zval *args, int num_args TSRMLS_DC);
void pcbc_doc_id_search_query_init(zval *return_value, zval *args, int num_args TSRMLS_DC);
void pcbc_phrase_search_query_init(zval *return_value, zval *args, int num_args TSRMLS_DC);
void pcbc_generate_classic_lcb_auth(pcbc_classic_authenticator_t *auth, lcb_AUTHENTICATOR **result, lcb_type_t type,
                                    const char *name, const char *password, char **hash TSRMLS_DC);
void pcbc_generate_password_lcb_auth(pcbc_password_authenticator_t *auth, lcb_AUTHENTICATOR **result, lcb_type_t type,
                                     const char *name, const char *password, char **hash TSRMLS_DC);
void pcbc_password_authenticator_init(zval *return_value, char *username, int username_len, char *password,
                                      int password_len TSRMLS_DC);

void pcbc_crypto_register(pcbc_bucket_t *obj, const char *name, int name_len, zval *provider TSRMLS_DC);
void pcbc_crypto_unregister(pcbc_bucket_t *obj, const char *name, int name_len TSRMLS_DC);
void pcbc_crypto_encrypt_fields(pcbc_bucket_t *obj, zval *document, zval *options, const char *prefix,
                                zval *return_value TSRMLS_DC);
void pcbc_crypto_decrypt_fields(pcbc_bucket_t *obj, zval *document, zval *options, const char *prefix,
                                zval *return_value TSRMLS_DC);

static inline pcbc_cluster_t *pcbc_cluster_fetch_object(zend_object *obj)
{
    return (pcbc_cluster_t *)((char *)obj - XtOffsetOf(pcbc_cluster_t, std));
}
static inline pcbc_cluster_manager_t *pcbc_cluster_manager_fetch_object(zend_object *obj)
{
    return (pcbc_cluster_manager_t *)((char *)obj - XtOffsetOf(pcbc_cluster_manager_t, std));
}
static inline pcbc_bucket_t *pcbc_bucket_fetch_object(zend_object *obj)
{
    return (pcbc_bucket_t *)((char *)obj - XtOffsetOf(pcbc_bucket_t, std));
}
static inline pcbc_analytics_query_t *pcbc_analytics_query_fetch_object(zend_object *obj)
{
    return (pcbc_analytics_query_t *)((char *)obj - XtOffsetOf(pcbc_analytics_query_t, std));
}
static inline pcbc_n1ql_query_t *pcbc_n1ql_query_fetch_object(zend_object *obj)
{
    return (pcbc_n1ql_query_t *)((char *)obj - XtOffsetOf(pcbc_n1ql_query_t, std));
}
static inline pcbc_lookup_in_builder_t *pcbc_lookup_in_builder_fetch_object(zend_object *obj)
{
    return (pcbc_lookup_in_builder_t *)((char *)obj - XtOffsetOf(pcbc_lookup_in_builder_t, std));
}
static inline pcbc_MUTATE_IN_SPEC *pcbc_mutate_in_builder_fetch_object(zend_object *obj)
{
    return (pcbc_MUTATE_IN_SPEC *)((char *)obj - XtOffsetOf(pcbc_MUTATE_IN_SPEC, std));
}
static inline pcbc_mutation_state_t *pcbc_mutation_state_fetch_object(zend_object *obj)
{
    return (pcbc_mutation_state_t *)((char *)obj - XtOffsetOf(pcbc_mutation_state_t, std));
}
static inline pcbc_mutation_token_t *pcbc_mutation_token_fetch_object(zend_object *obj)
{
    return (pcbc_mutation_token_t *)((char *)obj - XtOffsetOf(pcbc_mutation_token_t, std));
}
static inline pcbc_classic_authenticator_t *pcbc_classic_authenticator_fetch_object(zend_object *obj)
{
    return (pcbc_classic_authenticator_t *)((char *)obj - XtOffsetOf(pcbc_classic_authenticator_t, std));
}
static inline pcbc_password_authenticator_t *pcbc_password_authenticator_fetch_object(zend_object *obj)
{
    return (pcbc_password_authenticator_t *)((char *)obj - XtOffsetOf(pcbc_password_authenticator_t, std));
}
static inline pcbc_user_settings_t *pcbc_user_settings_fetch_object(zend_object *obj)
{
    return (pcbc_user_settings_t *)((char *)obj - XtOffsetOf(pcbc_user_settings_t, std));
}
#define Z_CLUSTER_OBJ(zo) (pcbc_cluster_fetch_object(zo))
#define Z_CLUSTER_OBJ_P(zv) (pcbc_cluster_fetch_object(Z_OBJ_P(zv)))
#define Z_CLUSTER_MANAGER_OBJ(zo) (pcbc_cluster_manager_fetch_object(zo))
#define Z_CLUSTER_MANAGER_OBJ_P(zv) (pcbc_cluster_manager_fetch_object(Z_OBJ_P(zv)))
#define Z_BUCKET_OBJ(zo) (pcbc_bucket_fetch_object(zo))
#define Z_BUCKET_OBJ_P(zv) (pcbc_bucket_fetch_object(Z_OBJ_P(zv)))
#define Z_ANALYTICS_QUERY_OBJ(zo) (pcbc_analytics_query_fetch_object(zo))
#define Z_ANALYTICS_QUERY_OBJ_P(zv) (pcbc_analytics_query_fetch_object(Z_OBJ_P(zv)))
#define Z_N1QL_QUERY_OBJ(zo) (pcbc_n1ql_query_fetch_object(zo))
#define Z_N1QL_QUERY_OBJ_P(zv) (pcbc_n1ql_query_fetch_object(Z_OBJ_P(zv)))
#define Z_LOOKUP_IN_BUILDER_OBJ(zo) (pcbc_lookup_in_builder_fetch_object(zo))
#define Z_LOOKUP_IN_BUILDER_OBJ_P(zv) (pcbc_lookup_in_builder_fetch_object(Z_OBJ_P(zv)))
#define Z_MUTATE_IN_BUILDER_OBJ(zo) (pcbc_mutate_in_builder_fetch_object(zo))
#define Z_MUTATE_IN_BUILDER_OBJ_P(zv) (pcbc_mutate_in_builder_fetch_object(Z_OBJ_P(zv)))
#define Z_MUTATION_TOKEN_OBJ(zo) (pcbc_mutation_token_fetch_object(zo))
#define Z_MUTATION_TOKEN_OBJ_P(zv) (pcbc_mutation_token_fetch_object(Z_OBJ_P(zv)))
#define Z_MUTATION_STATE_OBJ(zo) (pcbc_mutation_state_fetch_object(zo))
#define Z_MUTATION_STATE_OBJ_P(zv) (pcbc_mutation_state_fetch_object(Z_OBJ_P(zv)))
#define Z_CLASSIC_AUTHENTICATOR_OBJ(zo) (pcbc_classic_authenticator_fetch_object(zo))
#define Z_CLASSIC_AUTHENTICATOR_OBJ_P(zv) (pcbc_classic_authenticator_fetch_object(Z_OBJ_P(zv)))
#define Z_PASSWORD_AUTHENTICATOR_OBJ(zo) (pcbc_password_authenticator_fetch_object(zo))
#define Z_PASSWORD_AUTHENTICATOR_OBJ_P(zv) (pcbc_password_authenticator_fetch_object(Z_OBJ_P(zv)))
#define Z_USER_SETTINGS_OBJ(zo) (pcbc_user_settings_fetch_object(zo))
#define Z_USER_SETTINGS_OBJ_P(zv) (pcbc_user_settings_fetch_object(Z_OBJ_P(zv)))

typedef struct {
    opcookie_res *res_head;
    opcookie_res *res_tail;
    lcb_STATUS first_error;
    int json_response;
    int json_options;
    int is_cbas; // FIXME: convert to bit-flags
    zval exc;
    lcbtrace_SPAN *span;
} opcookie;

opcookie *opcookie_init();
void opcookie_destroy(opcookie *cookie);
void opcookie_push(opcookie *cookie, opcookie_res *res); lcb_STATUS opcookie_get_first_error(opcookie *cookie);
opcookie_res *opcookie_next_res(opcookie *cookie, opcookie_res *cur);

#define FOREACH_OPCOOKIE_RES(Type, Res, cookie)                                                                        \
    Res = NULL;                                                                                                        \
    while ((Res = (Type *)opcookie_next_res(cookie, (opcookie_res *)Res)) != NULL)

typedef struct {
    opcookie_res header;
    char *key;
    int key_len;
    lcb_cas_t cas;
    lcb_MUTATION_TOKEN token;
} opcookie_store_res;
 lcb_STATUS proc_store_results(pcbc_bucket_t *bucket, zval *return_value, opcookie *cookie TSRMLS_DC);

#define proc_remove_results proc_store_results
#define proc_touch_results proc_store_results

#define pcbc_assert_number_of_commands(lcb, cmd, nscheduled, ntotal, err)                                              \
    if (nscheduled != ntotal) {                                                                                        \
        pcbc_log(LOGARGS(lcb, ERROR), "Failed to schedule %s commands (%d out of %d sent). Last error: %s.", cmd,      \
                 nscheduled, ntotal, lcb_strerror_short(err));                                                         \
    }

#define PCBC_ADDREF_P(__pcbc_zval) Z_TRY_ADDREF_P((__pcbc_zval))

#define PCBC_DELREF_P(__pcbc_zval) Z_TRY_DELREF_P((__pcbc_zval))



#define set_property_str(getter, class_entry, prop) \
    do { \
        const char *data = NULL; \
        size_t ndata = 0; \
        getter(resp, &data, &ndata); \
        if (ndata && data) { \
            zend_update_property_stringl(class_entry, return_value, ZEND_STRL(prop), data, ndata TSRMLS_CC); \
        } \
    } while (0);

#define set_property_num(type, getter, class_entry, prop) \
    do { \
        type data = 0; \
        getter(resp, &data); \
        zend_update_property_long(class_entry, return_value, ZEND_STRL(prop), data TSRMLS_CC); \
    } while (0);


#endif /* COUCHBASE_H_ */
