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

#define LOGARGS(builder, lvl) LCB_LOG_##lvl, builder->bucket->conn->lcb, "pcbc/mutate_in_builder", __FILE__, __LINE__

zend_class_entry *pcbc_mutate_in_builder_ce;

/* {{{ proto void Bucket::__construct()
   Should not be called directly */
PHP_METHOD(MutateInBuilder, __construct)
{
    throw_pcbc_exception("Accessing private constructor.", LCB_EINVAL);
}
/* }}} */

int pcbc_mutate_in_builder_remove(pcbc_mutate_in_builder_t *builder, char *path, int path_len, lcb_U32 flags TSRMLS_DC)
{
    pcbc_sd_spec_t *spec;

    spec = ecalloc(1, sizeof(pcbc_sd_spec_t));
    spec->next = NULL;
    spec->s.sdcmd = LCB_SDCMD_REMOVE;
    spec->s.options = flags;
    PCBC_SDSPEC_COPY_PATH(spec, path, path_len);
    if (builder->tail) {
        builder->tail->next = spec;
    }
    builder->tail = spec;
    if (builder->head == NULL) {
        builder->head = builder->tail;
    }
    builder->nspecs++;

    return SUCCESS;
}
/* {{{ proto \Couchbase\MutateInBuilder MutateInBuilder::remove(string $path, array $options = [])
 */
PHP_METHOD(MutateInBuilder, remove)
{
    pcbc_mutate_in_builder_t *obj;
    char *path = NULL;
    size_t path_len = 0;
    int rv;
    zval *options = NULL;

    obj = Z_MUTATE_IN_BUILDER_OBJ_P(getThis());

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|z", &path, &path_len, &options);
    if (rv == FAILURE) {
        RETURN_NULL();
    }

    pcbc_mutate_in_builder_remove(obj, path, path_len, pcbc_subdoc_options_to_flags(1, 0, options TSRMLS_CC) TSRMLS_CC);

    RETURN_ZVAL(getThis(), 1, 0);
} /* }}} */

/* {{{ proto \Couchbase\MutateInBuilder MutateInBuilder::counter(string $path, int $delta, array $options = [])
 */
PHP_METHOD(MutateInBuilder, counter)
{
    pcbc_mutate_in_builder_t *obj;
    char *path = NULL;
    size_t path_len = 0;
    zval *options = NULL;
    int rv;
    pcbc_sd_spec_t *spec;
    long delta;

    obj = Z_MUTATE_IN_BUILDER_OBJ_P(getThis());

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sl|z", &path, &path_len, &delta, &options);
    if (rv == FAILURE) {
        RETURN_NULL();
    }

    spec = ecalloc(1, sizeof(pcbc_sd_spec_t));
    spec->next = NULL;
    spec->s.sdcmd = LCB_SDCMD_COUNTER;
    spec->s.options = pcbc_subdoc_options_to_flags(1, 0, options TSRMLS_CC);
    PCBC_SDSPEC_COPY_PATH(spec, path, path_len);
    {
        char *delta_str = NULL;
        int delta_str_len = 0;

        delta_str_len = spprintf(&delta_str, 0, "%ld", delta);
        PCBC_SDSPEC_SET_VALUE(spec, delta_str, delta_str_len);
    }
    if (obj->tail) {
        obj->tail->next = spec;
    }
    obj->tail = spec;
    if (obj->head == NULL) {
        obj->head = obj->tail;
    }
    obj->nspecs++;

    RETURN_ZVAL(getThis(), 1, 0);
} /* }}} */

/* {{{ proto \Couchbase\MutateInBuilder MutateInBuilder::insert(string $path, mixed $value, array $options = [])
 */
PHP_METHOD(MutateInBuilder, insert)
{
    pcbc_mutate_in_builder_t *obj;
    char *path = NULL;
    size_t path_len = 0;
    zval *options = NULL;
    int rv;
    pcbc_sd_spec_t *spec;
    zval *value;

    obj = Z_MUTATE_IN_BUILDER_OBJ_P(getThis());

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sz|z", &path, &path_len, &value, &options);
    if (rv == FAILURE) {
        RETURN_NULL();
    }

    spec = ecalloc(1, sizeof(pcbc_sd_spec_t));
    spec->next = NULL;
    spec->s.sdcmd = LCB_SDCMD_DICT_ADD;
    spec->s.options = pcbc_subdoc_options_to_flags(1, 0, options TSRMLS_CC);
    PCBC_SDSPEC_COPY_PATH(spec, path, path_len);
    {
        smart_str buf = {0};
        int last_error;

        PCBC_JSON_ENCODE(&buf, value, 0, last_error);
        if (last_error != 0) {
            pcbc_log(LOGARGS(obj, WARN), "Failed to encode value as JSON: json_last_error=%d", last_error);
            smart_str_free(&buf);
            efree(spec);
            RETURN_NULL(); // TODO: throw exception?
        } else {
            smart_str_0(&buf);
            PCBC_SDSPEC_SET_VALUE_SMARTSTR(spec, buf);
            smart_str_free(&buf);
        }
    }
    if (obj->tail) {
        obj->tail->next = spec;
    }
    obj->tail = spec;
    if (obj->head == NULL) {
        obj->head = obj->tail;
    }
    obj->nspecs++;

    RETURN_ZVAL(getThis(), 1, 0);
} /* }}} */

int pcbc_mutate_in_builder_upsert(pcbc_mutate_in_builder_t *builder, char *path, int path_len, zval *value,
                                  lcb_U32 flags TSRMLS_DC)
{
    pcbc_sd_spec_t *spec;

    spec = ecalloc(1, sizeof(pcbc_sd_spec_t));
    spec->next = NULL;
    if (path) {
        spec->s.sdcmd = LCB_SDCMD_DICT_UPSERT;
        spec->s.options = flags;
        PCBC_SDSPEC_COPY_PATH(spec, path, path_len);
    } else {
        spec->s.sdcmd = LCB_SDCMD_SET_FULLDOC;
    }
    {
        smart_str buf = {0};
        int last_error;

        PCBC_JSON_ENCODE(&buf, value, 0, last_error);
        if (last_error != 0) {
            pcbc_log(LOGARGS(builder, WARN), "Failed to encode value as JSON: json_last_error=%d", last_error);
            smart_str_free(&buf);
            efree(spec);
            return FAILURE;
        } else {
            smart_str_0(&buf);
            PCBC_SDSPEC_SET_VALUE_SMARTSTR(spec, buf);
            smart_str_free(&buf);
        }
    }
    if (builder->tail) {
        builder->tail->next = spec;
    }
    builder->tail = spec;
    if (builder->head == NULL) {
        builder->head = builder->tail;
    }
    builder->nspecs++;
    return SUCCESS;
}

/* {{{ proto \Couchbase\MutateInBuilder MutateInBuilder::modeDocument(long $mode)
 */
PHP_METHOD(MutateInBuilder, modeDocument)
{
    pcbc_mutate_in_builder_t *obj;
    int rv;
    long mode;

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &mode);
    if (rv == FAILURE) {
        RETURN_NULL();
    }
    obj = Z_MUTATE_IN_BUILDER_OBJ_P(getThis());
    switch (mode) {
    case PCBC_SUBDOC_FULLDOC_REPLACE:
    case PCBC_SUBDOC_FULLDOC_INSERT:
    case PCBC_SUBDOC_FULLDOC_UPSERT:
        obj->fulldoc = mode;
        break;
    }

    RETURN_ZVAL(getThis(), 1, 0);
} /* }}} */

/* {{{ proto \Couchbase\MutateInBuilder MutateInBuilder::upsert(string $path, mixed $value, array $options = [])
 */
PHP_METHOD(MutateInBuilder, upsert)
{
    pcbc_mutate_in_builder_t *obj;
    char *path_str = NULL;
    size_t path_len = 0;
    zval *options = NULL;
    int rv;
    zval *value = NULL, *path = NULL;

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z|zz", &path, &value, &options);
    if (rv == FAILURE) {
        RETURN_NULL();
    }
    if (value == NULL && options == NULL) {
        // consider call as full-doc upsert
        value = path;
    } else {
        // regular subdoc with path
        path_str = PCBC_STRVAL_ZP(path);
        path_len = PCBC_STRLEN_ZP(path);
    }

    obj = Z_MUTATE_IN_BUILDER_OBJ_P(getThis());

    rv = pcbc_mutate_in_builder_upsert(obj, path_str, path_len, value,
                                       pcbc_subdoc_options_to_flags(1, 0, options TSRMLS_CC) TSRMLS_CC);
    if (rv == FAILURE) {
        RETURN_NULL();
    }

    RETURN_ZVAL(getThis(), 1, 0);
} /* }}} */

int pcbc_mutate_in_builder_replace(pcbc_mutate_in_builder_t *builder, char *path, int path_len, zval *value,
                                   lcb_U32 flags TSRMLS_DC)
{
    pcbc_sd_spec_t *spec;

    spec = ecalloc(1, sizeof(pcbc_sd_spec_t));
    spec->next = NULL;
    spec->s.sdcmd = LCB_SDCMD_REPLACE;
    spec->s.options = flags;
    PCBC_SDSPEC_COPY_PATH(spec, path, path_len);
    {
        smart_str buf = {0};
        int last_error;

        PCBC_JSON_ENCODE(&buf, value, 0, last_error);
        if (last_error != 0) {
            pcbc_log(LOGARGS(builder, WARN), "Failed to encode value as JSON: json_last_error=%d", last_error);
            smart_str_free(&buf);
            efree(spec);
            return FAILURE;
        } else {
            smart_str_0(&buf);
            PCBC_SDSPEC_SET_VALUE_SMARTSTR(spec, buf);
            smart_str_free(&buf);
        }
    }
    if (builder->tail) {
        builder->tail->next = spec;
    }
    builder->tail = spec;
    if (builder->head == NULL) {
        builder->head = builder->tail;
    }
    builder->nspecs++;

    return SUCCESS;
}

/* {{{ proto \Couchbase\MutateInBuilder MutateInBuilder::replace(string $path, mixed $value, array $options = [])
 */
PHP_METHOD(MutateInBuilder, replace)
{
    pcbc_mutate_in_builder_t *obj;
    char *path = NULL;
    size_t path_len = 0;
    zval *options = NULL;
    int rv;
    zval *value;

    obj = Z_MUTATE_IN_BUILDER_OBJ_P(getThis());

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sz|z", &path, &path_len, &value, &options);
    if (rv == FAILURE) {
        RETURN_NULL();
    }

    rv = pcbc_mutate_in_builder_replace(obj, path, path_len, value,
                                        pcbc_subdoc_options_to_flags(1, 0, options TSRMLS_CC) TSRMLS_CC);
    if (rv == FAILURE) {
        RETURN_NULL();
    }

    RETURN_ZVAL(getThis(), 1, 0);
} /* }}} */

/* {{{ proto \Couchbase\MutateInBuilder MutateInBuilder::arrayInsert(string $path, mixed $value. array $options = [])
 */
PHP_METHOD(MutateInBuilder, arrayInsert)
{
    pcbc_mutate_in_builder_t *obj;
    const char *path = NULL;
    size_t path_len = 0;
    int rv;
    pcbc_sd_spec_t *spec;
    zval *value;
    zval *options = NULL;

    obj = Z_MUTATE_IN_BUILDER_OBJ_P(getThis());

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sz|z", &path, &path_len, &value, &options);
    if (rv == FAILURE) {
        RETURN_NULL();
    }

    spec = ecalloc(1, sizeof(pcbc_sd_spec_t));
    spec->next = NULL;
    spec->s.sdcmd = LCB_SDCMD_ARRAY_INSERT;
    spec->s.options = pcbc_subdoc_options_to_flags(1, 0, options TSRMLS_CC);
    PCBC_SDSPEC_COPY_PATH(spec, path, path_len);
    {
        smart_str buf = {0};
        int last_error;

        PCBC_JSON_ENCODE(&buf, value, 0, last_error);
        if (last_error != 0) {
            pcbc_log(LOGARGS(obj, WARN), "Failed to encode value as JSON: json_last_error=%d", last_error);
            smart_str_free(&buf);
            efree(spec);
            RETURN_NULL(); // TODO: throw exception?
        } else {
            smart_str_0(&buf);
            PCBC_SDSPEC_SET_VALUE_SMARTSTR(spec, buf);
            smart_str_free(&buf);
        }
    }
    if (obj->tail) {
        obj->tail->next = spec;
    }
    obj->tail = spec;
    if (obj->head == NULL) {
        obj->head = obj->tail;
    }
    obj->nspecs++;

    RETURN_ZVAL(getThis(), 1, 0);
} /* }}} */

/* {{{ proto \Couchbase\MutateInBuilder MutateInBuilder::arrayInsertAll(string $path, array $value, array $options = [])
 */
PHP_METHOD(MutateInBuilder, arrayInsertAll)
{
    pcbc_mutate_in_builder_t *obj;
    char *path = NULL;
    size_t path_len = 0;
    int rv;
    pcbc_sd_spec_t *spec;
    zval *value;
    zval *options = NULL;

    obj = Z_MUTATE_IN_BUILDER_OBJ_P(getThis());

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sz|z", &path, &path_len, &value, &options);
    if (rv == FAILURE) {
        RETURN_NULL();
    }

    spec = ecalloc(1, sizeof(pcbc_sd_spec_t));
    spec->next = NULL;
    spec->s.sdcmd = LCB_SDCMD_ARRAY_INSERT;
    spec->s.options = pcbc_subdoc_options_to_flags(1, 0, options TSRMLS_CC);
    PCBC_SDSPEC_COPY_PATH(spec, path, path_len);
    {
        smart_str buf = {0};
        int last_error;

        PCBC_JSON_ENCODE(&buf, value, 0, last_error);
        if (last_error != 0) {
            pcbc_log(LOGARGS(obj, WARN), "Failed to encode value as JSON: json_last_error=%d", last_error);
            smart_str_free(&buf);
            efree(spec);
            RETURN_NULL(); // TODO: throw exception?
        } else {
            char *p, *stripped = NULL;
            int n;
            smart_str_0(&buf);
            p = PCBC_SMARTSTR_VAL(buf);
            n = PCBC_SMARTSTR_LEN(buf);
            for (; isspace(*p) && n; n--, p++) {
            }
            for (; n && isspace(p[n - 1]); n--) {
            }
            if (n < 3 || p[0] != '[' || p[n - 1] != ']') {
                pcbc_log(LOGARGS(obj, ERROR), "multivalue operation expects non-empty array");
                efree(spec);
                RETURN_NULL();
            }
            p++;
            n -= 2;
            stripped = estrndup(p, n);
            smart_str_free(&buf);
            PCBC_SDSPEC_SET_VALUE(spec, stripped, n);
        }
    }
    if (obj->tail) {
        obj->tail->next = spec;
    }
    obj->tail = spec;
    if (obj->head == NULL) {
        obj->head = obj->tail;
    }
    obj->nspecs++;

    RETURN_ZVAL(getThis(), 1, 0);
} /* }}} */

int pcbc_mutate_in_builder_array_add_unique(pcbc_mutate_in_builder_t *builder, char *path, int path_len, zval *value,
                                            lcb_U32 flags TSRMLS_DC)
{
    pcbc_sd_spec_t *spec;

    spec = ecalloc(1, sizeof(pcbc_sd_spec_t));
    spec->next = NULL;
    spec->s.sdcmd = LCB_SDCMD_ARRAY_ADD_UNIQUE;
    spec->s.options = flags;
    PCBC_SDSPEC_COPY_PATH(spec, path, path_len);
    {
        smart_str buf = {0};
        int last_error;

        PCBC_JSON_ENCODE(&buf, value, 0, last_error);
        if (last_error != 0) {
            pcbc_log(LOGARGS(builder, WARN), "Failed to encode value as JSON: json_last_error=%d", last_error);
            smart_str_free(&buf);
            efree(spec);
            return FAILURE;
        } else {
            smart_str_0(&buf);
            PCBC_SDSPEC_SET_VALUE_SMARTSTR(spec, buf);
            smart_str_free(&buf);
        }
    }
    if (builder->tail) {
        builder->tail->next = spec;
    }
    builder->tail = spec;
    if (builder->head == NULL) {
        builder->head = builder->tail;
    }
    builder->nspecs++;

    return SUCCESS;
}

/* {{{ proto \Couchbase\MutateInBuilder MutateInBuilder::arrayAddUnique(string $path, mixed $value,
 *                                                                      array $options = [])
 */
PHP_METHOD(MutateInBuilder, arrayAddUnique)
{
    pcbc_mutate_in_builder_t *obj;
    char *path = NULL;
    size_t path_len = 0;
    zval *options = NULL;
    int rv;
    zval *value;

    obj = Z_MUTATE_IN_BUILDER_OBJ_P(getThis());

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sz|z", &path, &path_len, &value, &options);
    if (rv == FAILURE) {
        RETURN_NULL();
    }

    rv = pcbc_mutate_in_builder_array_add_unique(obj, path, path_len, value,
                                                 pcbc_subdoc_options_to_flags(1, 0, options TSRMLS_CC) TSRMLS_CC);
    if (rv == FAILURE) {
        RETURN_NULL();
    }

    RETURN_ZVAL(getThis(), 1, 0);
} /* }}} */

int pcbc_mutate_in_builder_array_prepend(pcbc_mutate_in_builder_t *builder, char *path, int path_len, zval *value,
                                         lcb_U32 flags TSRMLS_DC)
{
    pcbc_sd_spec_t *spec;

    spec = ecalloc(1, sizeof(pcbc_sd_spec_t));
    spec->next = NULL;
    spec->s.sdcmd = LCB_SDCMD_ARRAY_ADD_FIRST;
    spec->s.options = flags;
    PCBC_SDSPEC_COPY_PATH(spec, path, path_len);
    {
        smart_str buf = {0};
        int last_error;

        PCBC_JSON_ENCODE(&buf, value, 0, last_error);
        if (last_error != 0) {
            pcbc_log(LOGARGS(builder, WARN), "Failed to encode value as JSON: json_last_error=%d", last_error);
            smart_str_free(&buf);
            efree(spec);
            return FAILURE;
        } else {
            smart_str_0(&buf);
            PCBC_SDSPEC_SET_VALUE_SMARTSTR(spec, buf);
            smart_str_free(&buf);
        }
    }
    if (builder->tail) {
        builder->tail->next = spec;
    }
    builder->tail = spec;
    if (builder->head == NULL) {
        builder->head = builder->tail;
    }
    builder->nspecs++;

    return SUCCESS;
}

/* {{{ proto \Couchbase\MutateInBuilder MutateInBuilder::arrayPrepend(string $path, mixed $value,
                                                                      array $options = [])
 */
PHP_METHOD(MutateInBuilder, arrayPrepend)
{
    pcbc_mutate_in_builder_t *obj;
    char *path = NULL;
    size_t path_len = 0;
    zval *options = NULL;
    int rv;
    zval *value;

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sz|z", &path, &path_len, &value, &options);
    if (rv == FAILURE) {
        RETURN_NULL();
    }

    obj = Z_MUTATE_IN_BUILDER_OBJ_P(getThis());

    pcbc_mutate_in_builder_array_prepend(obj, path, path_len, value,
                                         pcbc_subdoc_options_to_flags(1, 0, options TSRMLS_CC) TSRMLS_CC);
    RETURN_ZVAL(getThis(), 1, 0);
} /* }}} */

/* {{{ proto \Couchbase\MutateInBuilder MutateInBuilder::arrayPrependAll(string $path, array $values,
 *                                                                       array $options = [])
 */
PHP_METHOD(MutateInBuilder, arrayPrependAll)
{
    pcbc_mutate_in_builder_t *obj;
    char *path = NULL;
    size_t path_len = 0;
    zval *options = NULL;
    int rv;
    pcbc_sd_spec_t *spec;
    zval *value;

    obj = Z_MUTATE_IN_BUILDER_OBJ_P(getThis());

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sz|z", &path, &path_len, &value, &options);
    if (rv == FAILURE) {
        RETURN_NULL();
    }

    spec = ecalloc(1, sizeof(pcbc_sd_spec_t));
    spec->next = NULL;
    spec->s.sdcmd = LCB_SDCMD_ARRAY_ADD_FIRST;
    spec->s.options = pcbc_subdoc_options_to_flags(1, 0, options TSRMLS_CC);
    PCBC_SDSPEC_COPY_PATH(spec, path, path_len);
    {
        smart_str buf = {0};
        int last_error;

        PCBC_JSON_ENCODE(&buf, value, 0, last_error);
        if (last_error != 0) {
            pcbc_log(LOGARGS(obj, WARN), "Failed to encode value as JSON: json_last_error=%d", last_error);
            smart_str_free(&buf);
            efree(spec);
            RETURN_NULL(); // TODO: throw exception?
        } else {
            char *p, *stripped = NULL;
            int n;
            smart_str_0(&buf);
            p = PCBC_SMARTSTR_VAL(buf);
            n = PCBC_SMARTSTR_LEN(buf);
            for (; isspace(*p) && n; n--, p++) {
            }
            for (; n && isspace(p[n - 1]); n--) {
            }
            if (n < 3 || p[0] != '[' || p[n - 1] != ']') {
                pcbc_log(LOGARGS(obj, ERROR), "multivalue operation expects non-empty array");
                efree(spec);
                RETURN_NULL();
            }
            p++;
            n -= 2;
            stripped = estrndup(p, n);
            smart_str_free(&buf);
            PCBC_SDSPEC_SET_VALUE(spec, stripped, n);
        }
    }
    if (obj->tail) {
        obj->tail->next = spec;
    }
    obj->tail = spec;
    if (obj->head == NULL) {
        obj->head = obj->tail;
    }
    obj->nspecs++;

    RETURN_ZVAL(getThis(), 1, 0);
} /* }}} */

int pcbc_mutate_in_builder_array_append(pcbc_mutate_in_builder_t *builder, char *path, int path_len, zval *value,
                                        lcb_U32 flags TSRMLS_DC)
{
    pcbc_sd_spec_t *spec;

    spec = ecalloc(1, sizeof(pcbc_sd_spec_t));
    spec->next = NULL;
    spec->s.sdcmd = LCB_SDCMD_ARRAY_ADD_LAST;
    spec->s.options = flags;
    PCBC_SDSPEC_COPY_PATH(spec, path, path_len);
    {
        smart_str buf = {0};
        int last_error;

        PCBC_JSON_ENCODE(&buf, value, 0, last_error);
        if (last_error != 0) {
            pcbc_log(LOGARGS(builder, WARN), "Failed to encode value as JSON: json_last_error=%d", last_error);
            smart_str_free(&buf);
            efree(spec);
            return FAILURE;
        } else {
            smart_str_0(&buf);
            PCBC_SDSPEC_SET_VALUE_SMARTSTR(spec, buf);
            smart_str_free(&buf);
        }
    }
    if (builder->tail) {
        builder->tail->next = spec;
    }
    builder->tail = spec;
    if (builder->head == NULL) {
        builder->head = builder->tail;
    }
    builder->nspecs++;

    return SUCCESS;
}

/* {{{ proto \Couchbase\MutateInBuilder MutateInBuilder::arrayAppend(string $path, mixed $value, array $options = [])
 */
PHP_METHOD(MutateInBuilder, arrayAppend)
{
    pcbc_mutate_in_builder_t *obj;
    char *path = NULL;
    size_t path_len = 0;
    zval *options = NULL;
    int rv;
    zval *value;

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sz|z", &path, &path_len, &value, &options);
    if (rv == FAILURE) {
        RETURN_NULL();
    }

    obj = Z_MUTATE_IN_BUILDER_OBJ_P(getThis());

    rv = pcbc_mutate_in_builder_array_append(obj, path, path_len, value,
                                             pcbc_subdoc_options_to_flags(1, 0, options TSRMLS_CC) TSRMLS_CC);
    if (rv == FAILURE) {
        RETURN_NULL();
    }

    RETURN_ZVAL(getThis(), 1, 0);
} /* }}} */

/* {{{ proto \Couchbase\MutateInBuilder MutateInBuilder::arrayAppendAll(string $path, array $values, array $options =
 * [])
 */
PHP_METHOD(MutateInBuilder, arrayAppendAll)
{
    pcbc_mutate_in_builder_t *obj;
    char *path = NULL;
    size_t path_len = 0;
    zval *options = NULL;
    int rv;
    pcbc_sd_spec_t *spec;
    zval *value;

    obj = Z_MUTATE_IN_BUILDER_OBJ_P(getThis());

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sz|z", &path, &path_len, &value, &options);
    if (rv == FAILURE) {
        RETURN_NULL();
    }

    spec = ecalloc(1, sizeof(pcbc_sd_spec_t));
    spec->next = NULL;
    spec->s.sdcmd = LCB_SDCMD_ARRAY_ADD_LAST;
    spec->s.options = pcbc_subdoc_options_to_flags(1, 0, options TSRMLS_CC);
    PCBC_SDSPEC_COPY_PATH(spec, path, path_len);
    {
        smart_str buf = {0};
        int last_error;

        PCBC_JSON_ENCODE(&buf, value, 0, last_error);
        if (last_error != 0) {
            pcbc_log(LOGARGS(obj, WARN), "Failed to encode value as JSON: json_last_error=%d", last_error);
            smart_str_free(&buf);
            efree(spec);
            RETURN_NULL(); // TODO: throw exception?
        } else {
            char *p, *stripped = NULL;
            int n;
            smart_str_0(&buf);
            p = PCBC_SMARTSTR_VAL(buf);
            n = PCBC_SMARTSTR_LEN(buf);
            for (; isspace(*p) && n; n--, p++) {
            }
            for (; n && isspace(p[n - 1]); n--) {
            }
            if (n < 3 || p[0] != '[' || p[n - 1] != ']') {
                pcbc_log(LOGARGS(obj, ERROR), "multivalue operation expects non-empty array");
                efree(spec);
                RETURN_NULL();
            }
            p++;
            n -= 2;
            stripped = estrndup(p, n);
            smart_str_free(&buf);
            PCBC_SDSPEC_SET_VALUE(spec, stripped, n);
        }
    }
    if (obj->tail) {
        obj->tail->next = spec;
    }
    obj->tail = spec;
    if (obj->head == NULL) {
        obj->head = obj->tail;
    }
    obj->nspecs++;

    RETURN_ZVAL(getThis(), 1, 0);
} /* }}} */

/* {{{ proto \Couchbase\MutateInBuilder MutateInBuilder::withExpiry(int $expiry)
 */
PHP_METHOD(MutateInBuilder, withExpiry)
{
    pcbc_mutate_in_builder_t *obj;
    char *path = NULL;
    size_t path_len = 0;
    zval *options = NULL;
    int rv;
    pcbc_sd_spec_t *spec;
    zval *value;
    long expiry = 0;

    obj = Z_MUTATE_IN_BUILDER_OBJ_P(getThis());

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &expiry);
    if (rv == FAILURE) {
        RETURN_NULL();
    }
    obj->expiry = expiry;

    RETURN_ZVAL(getThis(), 1, 0);
} /* }}} */

/* {{{ proto \Couchbase\MutateInBuilder MutateInBuilder::execute() */
PHP_METHOD(MutateInBuilder, execute)
{
    pcbc_mutate_in_builder_t *obj;
    int rv;

    obj = Z_MUTATE_IN_BUILDER_OBJ_P(getThis());

    rv = zend_parse_parameters_none();
    if (rv == FAILURE) {
        RETURN_NULL();
    }

    pcbc_bucket_subdoc_request(obj->bucket, obj, 0, return_value TSRMLS_CC);
} /* }}} */

ZEND_BEGIN_ARG_INFO_EX(ai_MutateInBuilder_none, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_MutateInBuilder_mutatePathValue, 0, 0, 2)
ZEND_ARG_INFO(0, path)
ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_MutateInBuilder_mutatePathValues, 0, 0, 2)
ZEND_ARG_INFO(0, path)
ZEND_ARG_INFO(0, values)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_MutateInBuilder_mutatePathValueParents, 0, 0, 3)
ZEND_ARG_INFO(0, path)
ZEND_ARG_INFO(0, value)
ZEND_ARG_INFO(0, createParents)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_MutateInBuilder_mutatePathValuesParents, 0, 0, 3)
ZEND_ARG_INFO(0, path)
ZEND_ARG_INFO(0, values)
ZEND_ARG_INFO(0, createParents)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_MutateInBuilder_remove, 0, 0, 1)
ZEND_ARG_INFO(0, path)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_MutateInBuilder_counter, 0, 0, 3)
ZEND_ARG_INFO(0, path)
ZEND_ARG_INFO(0, delta)
ZEND_ARG_INFO(0, createParents)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_MutateInBuilder_withExpiry, 0, 0, 1)
ZEND_ARG_INFO(0, expiry)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_MutateInBuilder_modeDocument, 0, 0, 1)
ZEND_ARG_INFO(0, mode)
ZEND_END_ARG_INFO()

// clang-format off
zend_function_entry mutate_in_builder_methods[] = {
    PHP_ME(MutateInBuilder, __construct, ai_MutateInBuilder_none, ZEND_ACC_PRIVATE | ZEND_ACC_FINAL | ZEND_ACC_CTOR)
    PHP_ME(MutateInBuilder, insert, ai_MutateInBuilder_mutatePathValueParents, ZEND_ACC_PUBLIC)
    PHP_ME(MutateInBuilder, upsert, ai_MutateInBuilder_mutatePathValueParents, ZEND_ACC_PUBLIC)
    PHP_ME(MutateInBuilder, modeDocument, ai_MutateInBuilder_modeDocument, ZEND_ACC_PUBLIC)
    PHP_ME(MutateInBuilder, replace, ai_MutateInBuilder_mutatePathValue, ZEND_ACC_PUBLIC)
    PHP_ME(MutateInBuilder, remove, ai_MutateInBuilder_remove, ZEND_ACC_PUBLIC)
    PHP_ME(MutateInBuilder, arrayPrepend, ai_MutateInBuilder_mutatePathValueParents, ZEND_ACC_PUBLIC)
    PHP_ME(MutateInBuilder, arrayAppend, ai_MutateInBuilder_mutatePathValueParents, ZEND_ACC_PUBLIC)
    PHP_ME(MutateInBuilder, arrayInsert, ai_MutateInBuilder_mutatePathValue, ZEND_ACC_PUBLIC)
    PHP_ME(MutateInBuilder, arrayPrependAll, ai_MutateInBuilder_mutatePathValuesParents, ZEND_ACC_PUBLIC)
    PHP_ME(MutateInBuilder, arrayAppendAll, ai_MutateInBuilder_mutatePathValuesParents, ZEND_ACC_PUBLIC)
    PHP_ME(MutateInBuilder, arrayInsertAll, ai_MutateInBuilder_mutatePathValues, ZEND_ACC_PUBLIC)
    PHP_ME(MutateInBuilder, arrayAddUnique, ai_MutateInBuilder_mutatePathValueParents, ZEND_ACC_PUBLIC)
    PHP_ME(MutateInBuilder, counter, ai_MutateInBuilder_counter, ZEND_ACC_PUBLIC)
    PHP_ME(MutateInBuilder, withExpiry, ai_MutateInBuilder_withExpiry, ZEND_ACC_PUBLIC)
    PHP_ME(MutateInBuilder, execute, ai_MutateInBuilder_none, ZEND_ACC_PUBLIC)
    PHP_FE_END
};
// clang-format on

zend_object_handlers pcbc_mutate_in_builder_handlers;

void pcbc_mutate_in_builder_init(zval *return_value, zval *bucket, const char *id, int id_len, lcb_cas_t cas TSRMLS_DC)
{
    pcbc_mutate_in_builder_t *builder;

    object_init_ex(return_value, pcbc_mutate_in_builder_ce);
    builder = Z_MUTATE_IN_BUILDER_OBJ_P(return_value);
    ZVAL_COPY(&builder->bucket_zval, bucket);
    builder->bucket = Z_BUCKET_OBJ_P(bucket);
    builder->id_len = id_len;
    builder->id = estrndup(id, id_len);
    builder->cas = cas;
    builder->nspecs = 0;
    builder->head = NULL;
    builder->tail = NULL;
}

static void mutate_in_builder_free_object(zend_object *object TSRMLS_DC) /* {{{ */
{
    pcbc_mutate_in_builder_t *obj = Z_MUTATE_IN_BUILDER_OBJ(object);
    pcbc_sd_spec_t *spec;

    if (obj->id != NULL) {
        efree(obj->id);
    }
    spec = obj->head;
    while (spec) {
        pcbc_sd_spec_t *tmp = spec;
        spec = spec->next;
        PCBC_SDSPEC_FREE_PATH(tmp);
        PCBC_SDSPEC_FREE_VALUE(tmp);
        efree(tmp);
    }
    obj->head = obj->tail = NULL;
    Z_DELREF_P(&obj->bucket_zval);
    ZVAL_UNDEF(&obj->bucket_zval);
    obj->bucket = NULL;
    zend_object_std_dtor(&obj->std TSRMLS_CC);
} /* }}} */

static zend_object *mutate_in_builder_create_object(zend_class_entry *class_type TSRMLS_DC)
{
    pcbc_mutate_in_builder_t *obj = NULL;

    obj = PCBC_ALLOC_OBJECT_T(pcbc_mutate_in_builder_t, class_type);

    zend_object_std_init(&obj->std, class_type TSRMLS_CC);
    object_properties_init(&obj->std, class_type);

    obj->std.handlers = &pcbc_mutate_in_builder_handlers;
    return &obj->std;
}

static HashTable *mutate_in_builder_get_debug_info(zval *object, int *is_temp TSRMLS_DC) /* {{{ */
{
    pcbc_mutate_in_builder_t *obj = NULL;
    zval retval;
    zval specs;
    zval cas;
    pcbc_sd_spec_t *spec;

    *is_temp = 1;
    obj = Z_MUTATE_IN_BUILDER_OBJ_P(object);

    array_init(&retval);
    ADD_ASSOC_STRING(&retval, "id", obj->id);
    ZVAL_UNDEF(&cas);
    pcbc_cas_encode(&cas, obj->cas TSRMLS_CC);
    ADD_ASSOC_ZVAL_EX(&retval, "cas", &cas);
    if (obj->expiry) {
        ADD_ASSOC_LONG_EX(&retval, "expiry", obj->expiry);
    }

    ZVAL_UNDEF(&specs);
    array_init_size(&specs, obj->nspecs);

    spec = obj->head;
    while (spec) {
        zval s;
        char *path = NULL, *value = NULL;
        int path_len = 0, value_len = 0;

        ZVAL_UNDEF(&s);
        array_init(&s);
        switch (spec->s.sdcmd) {
        case LCB_SDCMD_REPLACE:
            ADD_ASSOC_STRING(&s, "cmd", "replace");
            PCBC_SDSPEC_GET_PATH(spec, path, path_len);
            ADD_ASSOC_STRINGL(&s, "path", path, path_len);
            PCBC_SDSPEC_GET_VALUE(spec, value, value_len);
            ADD_ASSOC_STRINGL(&s, "value", value, value_len);
            ADD_ASSOC_LONG_EX(&s, "options", spec->s.options);
            break;
        case LCB_SDCMD_DICT_ADD:
            ADD_ASSOC_STRING(&s, "cmd", "insert");
            PCBC_SDSPEC_GET_PATH(spec, path, path_len);
            ADD_ASSOC_STRINGL(&s, "path", path, path_len);
            PCBC_SDSPEC_GET_VALUE(spec, value, value_len);
            ADD_ASSOC_STRINGL(&s, "value", value, value_len);
            ADD_ASSOC_LONG_EX(&s, "options", spec->s.options);
            break;
        case LCB_SDCMD_DICT_UPSERT:
            ADD_ASSOC_STRING(&s, "cmd", "upsert");
            PCBC_SDSPEC_GET_PATH(spec, path, path_len);
            ADD_ASSOC_STRINGL(&s, "path", path, path_len);
            PCBC_SDSPEC_GET_VALUE(spec, value, value_len);
            ADD_ASSOC_STRINGL(&s, "value", value, value_len);
            ADD_ASSOC_LONG_EX(&s, "options", spec->s.options);
            break;

        case LCB_SDCMD_ARRAY_ADD_FIRST:
            ADD_ASSOC_STRING(&s, "cmd", "arrayPrepend");
            PCBC_SDSPEC_GET_PATH(spec, path, path_len);
            ADD_ASSOC_STRINGL(&s, "path", path, path_len);
            PCBC_SDSPEC_GET_VALUE(spec, value, value_len);
            ADD_ASSOC_STRINGL(&s, "value", value, value_len);
            ADD_ASSOC_LONG_EX(&s, "options", spec->s.options);
            break;
        case LCB_SDCMD_ARRAY_ADD_LAST:
            ADD_ASSOC_STRING(&s, "cmd", "arrayAppend");
            PCBC_SDSPEC_GET_PATH(spec, path, path_len);
            ADD_ASSOC_STRINGL(&s, "path", path, path_len);
            PCBC_SDSPEC_GET_VALUE(spec, value, value_len);
            ADD_ASSOC_STRINGL(&s, "value", value, value_len);
            ADD_ASSOC_LONG_EX(&s, "options", spec->s.options);
            break;
        case LCB_SDCMD_ARRAY_INSERT:
            ADD_ASSOC_STRING(&s, "cmd", "arrayInsert");
            PCBC_SDSPEC_GET_PATH(spec, path, path_len);
            ADD_ASSOC_STRINGL(&s, "path", path, path_len);
            PCBC_SDSPEC_GET_VALUE(spec, value, value_len);
            ADD_ASSOC_STRINGL(&s, "value", value, value_len);
            ADD_ASSOC_LONG_EX(&s, "options", spec->s.options);
            break;
        case LCB_SDCMD_ARRAY_ADD_UNIQUE:
            ADD_ASSOC_STRING(&s, "cmd", "arrayAddUnique");
            PCBC_SDSPEC_GET_PATH(spec, path, path_len);
            ADD_ASSOC_STRINGL(&s, "path", path, path_len);
            PCBC_SDSPEC_GET_VALUE(spec, value, value_len);
            ADD_ASSOC_STRINGL(&s, "value", value, value_len);
            ADD_ASSOC_LONG_EX(&s, "options", spec->s.options);
            break;
        case LCB_SDCMD_COUNTER:
            ADD_ASSOC_STRING(&s, "cmd", "counter");
            PCBC_SDSPEC_GET_PATH(spec, path, path_len);
            ADD_ASSOC_STRINGL(&s, "path", path, path_len);
            PCBC_SDSPEC_GET_VALUE(spec, value, value_len);
            ADD_ASSOC_STRINGL(&s, "value", value, value_len);
            ADD_ASSOC_LONG_EX(&s, "options", spec->s.options);
            break;
        case LCB_SDCMD_REMOVE:
            ADD_ASSOC_STRING(&s, "cmd", "remove");
            PCBC_SDSPEC_GET_PATH(spec, path, path_len);
            ADD_ASSOC_STRINGL(&s, "path", path, path_len);
            ADD_ASSOC_LONG_EX(&s, "options", spec->s.options);
            break;
        }
        add_next_index_zval(&specs, &s);
        spec = spec->next;
    }
    ADD_ASSOC_ZVAL_EX(&retval, "specs", &specs);

    return Z_ARRVAL(retval);
} /* }}} */

PHP_MINIT_FUNCTION(MutateInBuilder)
{
    zend_class_entry ce;

    INIT_NS_CLASS_ENTRY(ce, "Couchbase", "MutateInBuilder", mutate_in_builder_methods);
    pcbc_mutate_in_builder_ce = zend_register_internal_class(&ce TSRMLS_CC);
    pcbc_mutate_in_builder_ce->create_object = mutate_in_builder_create_object;
    PCBC_CE_DISABLE_SERIALIZATION(pcbc_mutate_in_builder_ce);

    zend_declare_class_constant_long(pcbc_mutate_in_builder_ce, ZEND_STRL("FULLDOC_REPLACE"),
                                     PCBC_SUBDOC_FULLDOC_REPLACE TSRMLS_CC);
    zend_declare_class_constant_long(pcbc_mutate_in_builder_ce, ZEND_STRL("FULLDOC_INSERT"),
                                     PCBC_SUBDOC_FULLDOC_INSERT TSRMLS_CC);
    zend_declare_class_constant_long(pcbc_mutate_in_builder_ce, ZEND_STRL("FULLDOC_UPSERT"),
                                     PCBC_SUBDOC_FULLDOC_UPSERT TSRMLS_CC);

    memcpy(&pcbc_mutate_in_builder_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    pcbc_mutate_in_builder_handlers.get_debug_info = mutate_in_builder_get_debug_info;
    pcbc_mutate_in_builder_handlers.free_obj = mutate_in_builder_free_object;
    pcbc_mutate_in_builder_handlers.offset = XtOffsetOf(pcbc_mutate_in_builder_t, std);

    zend_register_class_alias("\\CouchbaseMutateInBuilder", pcbc_mutate_in_builder_ce);
    return SUCCESS;
}
