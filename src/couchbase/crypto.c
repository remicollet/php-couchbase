/**
 *     Copyright 2018 Couchbase, Inc.
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

#define LOGARGS(obj, lvl) LCB_LOG_##lvl, obj->conn->lcb, "pcbc/crypto", __FILE__, __LINE__

zend_class_entry *pcbc_crypto_provider_ce;

static void pcbc_crypto_destructor(lcbcrypto_PROVIDER *provider)
{
    if (provider) {
        if (provider->cookie) {
            zval *zprovider = (zval *)provider->cookie;
            if (!Z_ISUNDEF_P(zprovider)) {
                PCBC_DELREF_P(zprovider);
            }
#if PHP_VERSION_ID >= 70000
            efree(provider->cookie);
#endif
        }
        provider->cookie = NULL;
        efree(provider);
    }
}

static void pcbc_crypto_release_bytes(lcbcrypto_PROVIDER *provider, void *bytes)
{
    if (bytes) {
        efree(bytes);
    }
    (void)provider;
}

static lcb_error_t pcbc_crypto_load_key(struct lcbcrypto_PROVIDER *provider, lcbcrypto_KEYTYPE type, const char *keyid,
                                        uint8_t **key, size_t *key_len)
{
    zval *zprovider = (zval *)provider->cookie;
    int rv;
    PCBC_ZVAL params[2];
    PCBC_ZVAL fname;
    PCBC_ZVAL retval;
    TSRMLS_FETCH();

    PCBC_ZVAL_ALLOC(fname);
    PCBC_ZVAL_ALLOC(params[0]);
    PCBC_ZVAL_ALLOC(params[1]);

    ZVAL_LONG(PCBC_P(params[0]), type);
    PCBC_STRINGL(params[1], keyid, strlen(keyid));
    PCBC_STRING(fname, "loadKey");

    rv = call_user_function_ex(EG(function_table), PCBC_CP(zprovider), PCBC_P(fname), &retval, 2, params, 1,
                               NULL TSRMLS_CC);

    zval_ptr_dtor(&params[0]);
    zval_ptr_dtor(&params[1]);
    if (rv == FAILURE || EG(exception) || Z_ISUNDEF(retval)) {
        return LCB_EINVAL;
    }

    if (Z_TYPE_P(PCBC_P(retval)) == IS_STRING && PCBC_STRLEN_P(retval)) {
        *key_len = PCBC_STRLEN_P(retval);
        *key = (uint8_t *)(estrndup(PCBC_STRVAL_P(retval), *key_len));
        return LCB_SUCCESS;
    }

    return LCB_EINVAL;
}

static lcb_error_t pcbc_crypto_generate_iv(struct lcbcrypto_PROVIDER *provider, uint8_t **iv, size_t *iv_len)
{
    zval *zprovider = (zval *)provider->cookie;
    int rv;
    PCBC_ZVAL fname;
    PCBC_ZVAL retval;
    TSRMLS_FETCH();

    PCBC_ZVAL_ALLOC(fname);
    PCBC_STRING(fname, "generateIV");

    rv = call_user_function_ex(EG(function_table), PCBC_CP(zprovider), PCBC_P(fname), &retval, 0, NULL, 1,
                               NULL TSRMLS_CC);
    if (rv == FAILURE || EG(exception) || Z_ISUNDEF(retval)) {
        return LCB_EINVAL;
    }
    if (Z_TYPE_P(PCBC_P(retval)) == IS_STRING && PCBC_STRLEN_P(retval)) {
        *iv_len = PCBC_STRLEN_P(retval);
        *iv = (uint8_t *)(estrndup(PCBC_STRVAL_P(retval), *iv_len));
        return LCB_SUCCESS;
    }

    return LCB_EINVAL;
}

static lcb_error_t pcbc_crypto_sign(struct lcbcrypto_PROVIDER *provider, const lcbcrypto_SIGV *inputs,
                                    size_t inputs_num, uint8_t **sig, size_t *sig_len)
{
    zval *zprovider = (zval *)provider->cookie;
    int rv;
    size_t ii;
    PCBC_ZVAL param;
    PCBC_ZVAL fname;
    PCBC_ZVAL retval;
    TSRMLS_FETCH();

    PCBC_ZVAL_ALLOC(fname);
    PCBC_ZVAL_ALLOC(param);

    array_init_size(PCBC_P(param), inputs_num);
    for (ii = 0; ii < inputs_num; ii++) {
        ADD_NEXT_INDEX_STRINGL(PCBC_P(param), inputs[ii].data, inputs[ii].len);
    }
    PCBC_STRING(fname, "sign");

    rv = call_user_function_ex(EG(function_table), PCBC_CP(zprovider), PCBC_P(fname), &retval, 1, &param, 1,
                               NULL TSRMLS_CC);

    zval_ptr_dtor(&param);
    if (rv == FAILURE || EG(exception) || Z_ISUNDEF(retval)) {
        return LCB_EINVAL;
    }

    if (Z_TYPE_P(PCBC_P(retval)) == IS_STRING && PCBC_STRLEN_P(retval)) {
        *sig_len = PCBC_STRLEN_P(retval);
        *sig = (uint8_t *)(estrndup(PCBC_STRVAL_P(retval), *sig_len));
        return LCB_SUCCESS;
    }

    return LCB_EINVAL;
}

static lcb_error_t pcbc_crypto_verify_signature(struct lcbcrypto_PROVIDER *provider, const lcbcrypto_SIGV *inputs,
                                                size_t inputs_num, uint8_t *sig, size_t sig_len)
{
    zval *zprovider = (zval *)provider->cookie;
    int rv;
    size_t ii;
    PCBC_ZVAL params[2];
    PCBC_ZVAL fname;
    PCBC_ZVAL retval;
    TSRMLS_FETCH();

    PCBC_ZVAL_ALLOC(fname);
    PCBC_ZVAL_ALLOC(params[0]);
    PCBC_ZVAL_ALLOC(params[1]);

    array_init_size(PCBC_P(params[0]), inputs_num);
    for (ii = 0; ii < inputs_num; ii++) {
        ADD_NEXT_INDEX_STRINGL(PCBC_P(params[0]), inputs[ii].data, inputs[ii].len);
    }
    PCBC_STRINGL(params[1], sig, sig_len);
    PCBC_STRING(fname, "verifySignature");

    rv = call_user_function_ex(EG(function_table), PCBC_CP(zprovider), PCBC_P(fname), &retval, 2, params, 1,
                               NULL TSRMLS_CC);

    zval_ptr_dtor(&params[0]);
    zval_ptr_dtor(&params[1]);
    if (rv == FAILURE || EG(exception) || Z_ISUNDEF(retval)) {
        return LCB_EINVAL;
    }

    switch (Z_TYPE_P(PCBC_P(retval))) {
#if PHP_VERSION_ID >= 70000
    case IS_TRUE:
        return LCB_SUCCESS;
    case IS_FALSE:
        return LCB_EINVAL;
#else
    case IS_BOOL:
        if (Z_BVAL_P(retval)) {
            return LCB_SUCCESS;
        } else {
            return LCB_EINVAL;
        }
#endif
    }
    return LCB_EINVAL;
}

static lcb_error_t pcbc_crypto_encrypt(struct lcbcrypto_PROVIDER *provider, const uint8_t *input, size_t input_len,
                                       const uint8_t *key, size_t key_len, const uint8_t *iv, size_t iv_len,
                                       uint8_t **output, size_t *output_len)
{
    zval *zprovider = (zval *)provider->cookie;
    int rv;
    PCBC_ZVAL params[3];
    PCBC_ZVAL fname;
    PCBC_ZVAL retval;
    TSRMLS_FETCH();

    PCBC_ZVAL_ALLOC(fname);
    PCBC_ZVAL_ALLOC(params[0]);
    PCBC_ZVAL_ALLOC(params[1]);
    PCBC_ZVAL_ALLOC(params[2]);

    PCBC_STRINGL(params[0], input, input_len);
    PCBC_STRINGL(params[1], key, key_len);
    if (iv) {
        PCBC_STRINGL(params[2], iv, iv_len);
    } else {
        ZVAL_NULL(PCBC_P(params[2]));
    }
    PCBC_STRING(fname, "encrypt");

    rv = call_user_function_ex(EG(function_table), PCBC_CP(zprovider), PCBC_P(fname), &retval, 3, params, 1,
                               NULL TSRMLS_CC);

    zval_ptr_dtor(&params[0]);
    zval_ptr_dtor(&params[1]);
    zval_ptr_dtor(&params[2]);
    if (rv == FAILURE || EG(exception) || Z_ISUNDEF(retval)) {
        return LCB_EINVAL;
    }

    if (Z_TYPE_P(PCBC_P(retval)) == IS_STRING && PCBC_STRLEN_P(retval)) {
        *output_len = PCBC_STRLEN_P(retval);
        *output = (uint8_t *)(estrndup(PCBC_STRVAL_P(retval), *output_len));
        return LCB_SUCCESS;
    }

    return LCB_EINVAL;
}

static lcb_error_t pcbc_crypto_decrypt(struct lcbcrypto_PROVIDER *provider, const uint8_t *input, size_t input_len,
                                       const uint8_t *key, size_t key_len, const uint8_t *iv, size_t iv_len,
                                       uint8_t **output, size_t *output_len)
{
    zval *zprovider = (zval *)provider->cookie;
    int rv;
    PCBC_ZVAL params[3];
    PCBC_ZVAL fname;
    PCBC_ZVAL retval;
    TSRMLS_FETCH();

    PCBC_ZVAL_ALLOC(fname);
    PCBC_ZVAL_ALLOC(params[0]);
    PCBC_ZVAL_ALLOC(params[1]);
    PCBC_ZVAL_ALLOC(params[2]);

    PCBC_STRINGL(params[0], input, input_len);
    PCBC_STRINGL(params[1], key, key_len);
    if (iv) {
        PCBC_STRINGL(params[2], iv, iv_len);
    } else {
        ZVAL_NULL(PCBC_P(params[2]));
    }
    PCBC_STRING(fname, "decrypt");

    rv = call_user_function_ex(EG(function_table), PCBC_CP(zprovider), PCBC_P(fname), &retval, 3, params, 1,
                               NULL TSRMLS_CC);

    zval_ptr_dtor(&params[0]);
    zval_ptr_dtor(&params[1]);
    zval_ptr_dtor(&params[2]);
    if (rv == FAILURE || EG(exception) || Z_ISUNDEF(retval)) {
        return LCB_EINVAL;
    }

    if (Z_TYPE_P(PCBC_P(retval)) == IS_STRING && PCBC_STRLEN_P(retval)) {
        *output_len = PCBC_STRLEN_P(retval);
        *output = (uint8_t *)(estrndup(PCBC_STRVAL_P(retval), *output_len));
        return LCB_SUCCESS;
    }

    return LCB_EINVAL;
}

void pcbc_crypto_register(pcbc_bucket_t *obj, const char *name, int name_len, zval *zprovider TSRMLS_DC)
{
    lcbcrypto_PROVIDER *provider = ecalloc(1, sizeof(lcbcrypto_PROVIDER));

    provider->version = 0;
    provider->destructor = pcbc_crypto_destructor;
    provider->v.v0.release_bytes = pcbc_crypto_release_bytes;
    provider->v.v0.load_key = pcbc_crypto_load_key;
    provider->v.v0.generate_iv = pcbc_crypto_generate_iv;
    provider->v.v0.sign = pcbc_crypto_sign;
    provider->v.v0.verify_signature = pcbc_crypto_verify_signature;
    provider->v.v0.encrypt = pcbc_crypto_encrypt;
    provider->v.v0.decrypt = pcbc_crypto_decrypt;

#if PHP_VERSION_ID >= 70000
    {
        zval *tmp = ecalloc(1, sizeof(zval));
        ZVAL_ZVAL(tmp, zprovider, 1, 0);
        provider->cookie = tmp;
    }
#else
    PCBC_ADDREF_P(zprovider);
    provider->cookie = zprovider;
#endif

    lcbcrypto_register(obj->conn->lcb, name, provider);
}

void pcbc_crypto_unregister(pcbc_bucket_t *obj, const char *name, int name_len TSRMLS_DC)
{
    lcbcrypto_unregister(obj->conn->lcb, name);
}

void pcbc_crypto_encrypt_document(pcbc_bucket_t *obj, zval *document, zval *options, const char *prefix,
                                  zval *return_value TSRMLS_DC)
{
    smart_str buf = {0};
    int last_error;
    long ii;
    size_t nfields;
    lcbcrypto_CMDENCRYPT ecmd = {0};
    lcb_error_t err;

    PCBC_JSON_ENCODE(&buf, document, 0, last_error);
    if (last_error != 0) {
        pcbc_log(LOGARGS(obj, WARN), "Failed to encode document as JSON: json_last_error=%d", last_error);
        smart_str_free(&buf);
        return;
    }
    smart_str_0(&buf);

    ecmd.version = 0;
    ecmd.prefix = prefix;
    ecmd.doc = PCBC_SMARTSTR_VAL(buf);
    ecmd.ndoc = PCBC_SMARTSTR_LEN(buf);
    ecmd.out = NULL;
    ecmd.nout = 0;

    nfields = php_array_count(options);
    ecmd.nfields = 0;
    ecmd.fields = ecalloc(nfields, sizeof(lcbcrypto_FIELDSPEC));
    for (ii = 0; ii < nfields; ii++) {
        zval *val;
        zval *field = php_array_fetchn(options, ii);

        if (field == NULL) {
            continue;
        }
        val = php_array_fetch(field, "alg");
        if (val == NULL || Z_TYPE_P(val) != IS_STRING) {
            continue;
        }
        ecmd.fields[ecmd.nfields].alg = Z_STRVAL_P(val);

        val = php_array_fetch(field, "name");
        if (val == NULL || Z_TYPE_P(val) != IS_STRING) {
            continue;
        }
        ecmd.fields[ecmd.nfields].name = Z_STRVAL_P(val);

        val = php_array_fetch(field, "kid");
        if (val == NULL || Z_TYPE_P(val) != IS_STRING) {
            continue;
        }
        ecmd.fields[ecmd.nfields].kid = Z_STRVAL_P(val);
        ecmd.nfields++;
    }

    err = lcbcrypto_encrypt_document(obj->conn->lcb, &ecmd);
    smart_str_free(&buf);
    efree(ecmd.fields);
    if (err != LCB_SUCCESS) {
        pcbc_log(LOGARGS(obj, WARN), "Failed to encrypt document");
        return;
    }
    if (ecmd.out) {
        PCBC_ZVAL_ALLOC(PCBC_D(return_value));
        PCBC_JSON_COPY_DECODE(return_value, ecmd.out, ecmd.nout, PHP_JSON_OBJECT_AS_ARRAY, last_error);
        free(ecmd.out);
        ecmd.out = NULL;
        ecmd.nout = 0;
        if (last_error != 0) {
            pcbc_log(LOGARGS(obj, WARN), "Failed to decode value as JSON: json_last_error=%d", last_error);
            ZVAL_NULL(return_value);
        }
    }
}

void pcbc_crypto_decrypt_document(pcbc_bucket_t *obj, zval *document, const char *prefix, zval *return_value TSRMLS_DC)
{
    smart_str buf = {0};
    int last_error;
    long ii;
    size_t nfields;
    lcbcrypto_CMDDECRYPT dcmd = {0};
    lcb_error_t err;

    PCBC_JSON_ENCODE(&buf, document, 0, last_error);
    if (last_error != 0) {
        pcbc_log(LOGARGS(obj, WARN), "Failed to encode document as JSON: json_last_error=%d", last_error);
        smart_str_free(&buf);
        return;
    }
    smart_str_0(&buf);

    dcmd.version = 0;
    dcmd.prefix = prefix;
    dcmd.doc = PCBC_SMARTSTR_VAL(buf);
    dcmd.ndoc = PCBC_SMARTSTR_LEN(buf);
    dcmd.out = NULL;
    dcmd.nout = 0;

    err = lcbcrypto_decrypt_document(obj->conn->lcb, &dcmd);
    smart_str_free(&buf);
    if (err != LCB_SUCCESS) {
        pcbc_log(LOGARGS(obj, WARN), "Failed to decrypt document");
        return;
    }
    if (dcmd.out) {
        PCBC_ZVAL_ALLOC(PCBC_D(return_value));
        PCBC_JSON_COPY_DECODE(return_value, dcmd.out, dcmd.nout, PHP_JSON_OBJECT_AS_ARRAY, last_error);
        free(dcmd.out);
        dcmd.out = NULL;
        dcmd.nout = 0;
        if (last_error != 0) {
            pcbc_log(LOGARGS(obj, WARN), "Failed to decode value as JSON: json_last_error=%d", last_error);
            ZVAL_NULL(return_value);
        }
    }
}

PHP_METHOD(CryptoProvider, loadKey)
{
    RETURN_NULL();
}

PHP_METHOD(CryptoProvider, encrypt)
{
    RETURN_NULL();
}

PHP_METHOD(CryptoProvider, decrypt)
{
    RETURN_NULL();
}

PHP_METHOD(CryptoProvider, generateIV)
{
    RETURN_NULL();
}

PHP_METHOD(CryptoProvider, sign)
{
    RETURN_NULL();
}

PHP_METHOD(CryptoProvider, verifySignature)
{
    RETURN_NULL();
}

ZEND_BEGIN_ARG_INFO_EX(ai_CryptoProvider_loadKey, 0, 0, 2)
ZEND_ARG_TYPE_INFO(0, keyType, IS_LONG, 0)
ZEND_ARG_TYPE_INFO(0, keyId, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_CryptoProvider_encrypt, 0, 0, 2)
ZEND_ARG_TYPE_INFO(0, bytes, IS_STRING, 0)
ZEND_ARG_TYPE_INFO(0, key, IS_STRING, 0)
ZEND_ARG_TYPE_INFO(0, iv, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_CryptoProvider_decrypt, 0, 0, 2)
ZEND_ARG_TYPE_INFO(0, bytes, IS_STRING, 0)
ZEND_ARG_TYPE_INFO(0, key, IS_STRING, 0)
ZEND_ARG_TYPE_INFO(0, iv, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_CryptoProvider_generateIV, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_CryptoProvider_sign, 0, 0, 1)
ZEND_ARG_TYPE_INFO(0, bytes, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_CryptoProvider_verifySignature, 0, 0, 2)
ZEND_ARG_TYPE_INFO(0, bytes, IS_ARRAY, 0)
ZEND_ARG_TYPE_INFO(0, signature, IS_STRING, 0)
ZEND_END_ARG_INFO()

// clang-format off
static const zend_function_entry crypto_provider_methods[] = {
    /* mandatory interface */
    PHP_ME(CryptoProvider, loadKey, ai_CryptoProvider_loadKey, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT)
    PHP_ME(CryptoProvider, encrypt, ai_CryptoProvider_encrypt, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT)
    PHP_ME(CryptoProvider, decrypt, ai_CryptoProvider_decrypt, ZEND_ACC_PUBLIC|ZEND_ACC_ABSTRACT)
    /* optional interface */
    PHP_ME(CryptoProvider, generateIV, ai_CryptoProvider_generateIV, ZEND_ACC_PUBLIC)
    PHP_ME(CryptoProvider, sign, ai_CryptoProvider_sign, ZEND_ACC_PUBLIC)
    PHP_ME(CryptoProvider, verifySignature, ai_CryptoProvider_verifySignature, ZEND_ACC_PUBLIC)

    PHP_FE_END
};
// clang-format on

PHP_MINIT_FUNCTION(CryptoProvider)
{
    zend_class_entry ce;
    INIT_NS_CLASS_ENTRY(ce, "Couchbase", "CryptoProvider", crypto_provider_methods);
    pcbc_crypto_provider_ce = zend_register_internal_class(&ce TSRMLS_CC);

    zend_declare_class_constant_long(pcbc_crypto_provider_ce, ZEND_STRL("KEY_TYPE_ENCRYPT"),
                                     LCBCRYPTO_KEY_ENCRYPT TSRMLS_CC);
    zend_declare_class_constant_long(pcbc_crypto_provider_ce, ZEND_STRL("KEY_TYPE_DECRYPT"),
                                     LCBCRYPTO_KEY_DECRYPT TSRMLS_CC);
    return SUCCESS;
}
