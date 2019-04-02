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
#include <Zend/zend_alloc.h>

#define LOGARGS(lvl) LCB_LOG_##lvl, NULL, "pcbc/mutation_token", __FILE__, __LINE__

zend_class_entry *pcbc_mutation_token_ce;

/* {{{ proto void MutationToken::__construct() Should not be called directly */
PHP_METHOD(MutationToken, __construct)
{
    throw_pcbc_exception("Accessing private constructor.", LCB_EINVAL);
}
/* }}} */

/* {{{ proto \Couchbase\MutationToken MutationToken::from(string $bucketName, int $vbucketID, string $vbucketUUID,
                                                          string $sequeceNumber) */
PHP_METHOD(MutationToken, from)
{
    long vbid = 0;
    char *bucket = NULL, *vbuuid = NULL, *seqno = NULL;
    size_t bucket_len = 0, vbuuid_len = 0, seqno_len = 0;
    int rv;

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "slss", &bucket, &bucket_len, &vbid, &vbuuid, &vbuuid_len,
                               &seqno, &seqno_len);
    if (rv == FAILURE) {
        RETURN_NULL();
    }

    pcbc_mutation_token_init_php(return_value, bucket, bucket_len, vbid, vbuuid, vbuuid_len, seqno,
                                 seqno_len TSRMLS_CC);
} /* }}} */

/* {{{ proto string MutationToken::bucketName() */
PHP_METHOD(MutationToken, bucketName)
{
    pcbc_mutation_token_t *obj;
    int rv;

    rv = zend_parse_parameters_none();
    if (rv == FAILURE) {
        RETURN_NULL();
    }
    obj = Z_MUTATION_TOKEN_OBJ_P(getThis());

    ZVAL_STRING(return_value, obj->bucket);
} /* }}} */

/* {{{ proto int MutationToken::vbucketId() */
PHP_METHOD(MutationToken, vbucketId)
{
    pcbc_mutation_token_t *obj;
    int rv;

    rv = zend_parse_parameters_none();
    if (rv == FAILURE) {
        RETURN_NULL();
    }
    obj = Z_MUTATION_TOKEN_OBJ_P(getThis());

    RETURN_LONG(PCBC_MUTATION_TOKEN_VB(obj));
} /* }}} */

/* {{{ proto int MutationToken::vbucketUuid() */
PHP_METHOD(MutationToken, vbucketUuid)
{
    pcbc_mutation_token_t *obj;
    int rv;
    char *str;

    rv = zend_parse_parameters_none();
    if (rv == FAILURE) {
        RETURN_NULL();
    }
    obj = Z_MUTATION_TOKEN_OBJ_P(getThis());

    str = pcbc_base36_encode_str(PCBC_MUTATION_TOKEN_ID(obj));
    ZVAL_STRING(return_value, str);
    efree(str);
} /* }}} */

/* {{{ proto int MutationToken::sequenceNumber() */
PHP_METHOD(MutationToken, sequenceNumber)
{
    pcbc_mutation_token_t *obj;
    int rv;
    char *str;

    rv = zend_parse_parameters_none();
    if (rv == FAILURE) {
        RETURN_NULL();
    }
    obj = Z_MUTATION_TOKEN_OBJ_P(getThis());

    str = pcbc_base36_encode_str(PCBC_MUTATION_TOKEN_SEQ(obj));
    ZVAL_STRING(return_value, str);
    efree(str);
} /* }}} */

ZEND_BEGIN_ARG_INFO_EX(ai_MutationToken_none, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_MutationToken_from, 0, 0, 4)
ZEND_ARG_INFO(0, bucketName)
ZEND_ARG_INFO(0, vbucketId)
ZEND_ARG_INFO(0, vbucketUuid)
ZEND_ARG_INFO(0, sequenceNumber)
ZEND_END_ARG_INFO()

// clang-format off
zend_function_entry mutation_token_methods[] = {
    PHP_ME(MutationToken, __construct, ai_MutationToken_none, ZEND_ACC_PRIVATE | ZEND_ACC_FINAL | ZEND_ACC_CTOR)
    PHP_ME(MutationToken, from, ai_MutationToken_from, ZEND_ACC_STATIC | ZEND_ACC_PUBLIC)
    PHP_ME(MutationToken, bucketName, ai_MutationToken_none, ZEND_ACC_PUBLIC)
    PHP_ME(MutationToken, vbucketId, ai_MutationToken_none, ZEND_ACC_PUBLIC)
    PHP_ME(MutationToken, vbucketUuid, ai_MutationToken_none, ZEND_ACC_PUBLIC)
    PHP_ME(MutationToken, sequenceNumber, ai_MutationToken_none, ZEND_ACC_PUBLIC)
    PHP_FE_END
};
// clang-format on

zend_object_handlers pcbc_mutation_token_handlers;

void pcbc_mutation_token_init(zval *return_value, const char *bucket, const lcb_MUTATION_TOKEN *mt TSRMLS_DC)
{
    pcbc_mutation_token_t *token;

    object_init_ex(return_value, pcbc_mutation_token_ce);
    token = Z_MUTATION_TOKEN_OBJ_P(return_value);
    token->bucket = estrdup(bucket);
    token->mt = *mt;
}

void pcbc_mutation_token_init_php(zval *return_value, char *bucket, int bucket_len, long vbid, char *vbuuid,
                                  int vbuuid_len, char *seqno, int seqno_len TSRMLS_DC)
{
    lcb_MUTATION_TOKEN mt;
    LCB_MUTATION_TOKEN_VB(&mt) = (lcb_U16)vbid;
    LCB_MUTATION_TOKEN_ID(&mt) = pcbc_base36_decode_str(vbuuid, vbuuid_len);
    LCB_MUTATION_TOKEN_SEQ(&mt) = pcbc_base36_decode_str(seqno, seqno_len);
    pcbc_mutation_token_init(return_value, bucket, &mt TSRMLS_CC);
}

static void mutation_token_free_object(zend_object *object TSRMLS_DC) /* {{{ */
{
    pcbc_mutation_token_t *obj = Z_MUTATION_TOKEN_OBJ(object);

    efree(obj->bucket);

    zend_object_std_dtor(&obj->std TSRMLS_CC);
} /* }}} */

static zend_object *mutation_token_create_object(zend_class_entry *class_type TSRMLS_DC)
{
    pcbc_mutation_token_t *obj = NULL;

    obj = PCBC_ALLOC_OBJECT_T(pcbc_mutation_token_t, class_type);

    zend_object_std_init(&obj->std, class_type TSRMLS_CC);
    object_properties_init(&obj->std, class_type);

    obj->std.handlers = &pcbc_mutation_token_handlers;
    return &obj->std;
}

static HashTable *mutation_token_get_debug_info(zval *object, int *is_temp TSRMLS_DC) /* {{{ */
{
    pcbc_mutation_token_t *obj = NULL;
    zval retval;
    char *num36;

    *is_temp = 1;
    obj = Z_MUTATION_TOKEN_OBJ_P(object);

    array_init(&retval);
    ADD_ASSOC_STRING(&retval, "bucket", obj->bucket);
    ADD_ASSOC_LONG_EX(&retval, "vbucketId", PCBC_MUTATION_TOKEN_VB(obj));
    num36 = pcbc_base36_encode_str(PCBC_MUTATION_TOKEN_ID(obj));
    ADD_ASSOC_STRING(&retval, "vbucketUuid", num36);
    efree(num36);
    num36 = pcbc_base36_encode_str(PCBC_MUTATION_TOKEN_SEQ(obj));
    ADD_ASSOC_STRING(&retval, "sequenceNumber", num36);
    efree(num36);

    return Z_ARRVAL(retval);
} /* }}} */

PHP_MINIT_FUNCTION(MutationToken)
{
    zend_class_entry ce;

    INIT_NS_CLASS_ENTRY(ce, "Couchbase", "MutationToken", mutation_token_methods);
    pcbc_mutation_token_ce = zend_register_internal_class(&ce TSRMLS_CC);
    pcbc_mutation_token_ce->create_object = mutation_token_create_object;
    PCBC_CE_DISABLE_SERIALIZATION(pcbc_mutation_token_ce);

    memcpy(&pcbc_mutation_token_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    pcbc_mutation_token_handlers.get_debug_info = mutation_token_get_debug_info;
    pcbc_mutation_token_handlers.free_obj = mutation_token_free_object;
    pcbc_mutation_token_handlers.offset = XtOffsetOf(pcbc_mutation_token_t, std);

    zend_register_class_alias("\\CouchbaseMutationToken", pcbc_mutation_token_ce);
    return SUCCESS;
}
