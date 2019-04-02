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

#define LOGARGS(builder, lvl) LCB_LOG_##lvl, builder->bucket->conn->lcb, "pcbc/lookup_in_builder", __FILE__, __LINE__

zend_class_entry *pcbc_lookup_in_builder_ce;

/* {{{ proto void LookupInBuilder::__construct() Should not be called directly */
PHP_METHOD(LookupInBuilder, __construct)
{
    throw_pcbc_exception("Accessing private constructor.", LCB_EINVAL);
}
/* }}} */

int pcbc_lookup_in_builder_get(pcbc_lookup_in_builder_t *builder, char *path, int path_len, zval *options TSRMLS_DC)
{
    pcbc_sd_spec_t *spec;

    spec = ecalloc(1, sizeof(pcbc_sd_spec_t));
    spec->next = NULL;
    if (path) {
        spec->s.sdcmd = LCB_SDCMD_GET;
        spec->s.options = pcbc_subdoc_options_to_flags(1, 1, options TSRMLS_CC);
        PCBC_SDSPEC_COPY_PATH(spec, path, path_len);
    } else {
        spec->s.sdcmd = LCB_SDCMD_GET_FULLDOC;
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

/* {{{ proto \Couchbase\LookupInBuilder LookupInBuilder::get(string $path, array $options = []) */
PHP_METHOD(LookupInBuilder, get)
{
    pcbc_lookup_in_builder_t *obj;
    char *path = NULL;
    size_t path_len = 0;
    zval *options = NULL;
    int rv;

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|sz", &path, &path_len, &options);
    if (rv == FAILURE) {
        RETURN_NULL();
    }

    obj = Z_LOOKUP_IN_BUILDER_OBJ_P(getThis());
    pcbc_lookup_in_builder_get(obj, path, path_len, options TSRMLS_CC);

    RETURN_ZVAL(getThis(), 1, 0);
} /* }}} */

/* {{{ proto \Couchbase\LookupInBuilder LookupInBuilder::exists(string $path, array $options = []) */
PHP_METHOD(LookupInBuilder, exists)
{
    pcbc_lookup_in_builder_t *obj;
    const char *path = NULL;
    size_t path_len = 0;
    int rv;
    zval *options = NULL;
    pcbc_sd_spec_t *spec;

    obj = Z_LOOKUP_IN_BUILDER_OBJ_P(getThis());

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|z", &path, &path_len, &options);
    if (rv == FAILURE) {
        RETURN_NULL();
    }

    spec = ecalloc(1, sizeof(pcbc_sd_spec_t));
    spec->next = NULL;
    spec->s.sdcmd = LCB_SDCMD_EXISTS;
    spec->s.options = pcbc_subdoc_options_to_flags(1, 1, options TSRMLS_CC);
    PCBC_SDSPEC_COPY_PATH(spec, path, path_len);
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

/* {{{ proto \Couchbase\LookupInBuilder LookupInBuilder::getCount(string $path, array $options = []) */
PHP_METHOD(LookupInBuilder, getCount)
{
    pcbc_lookup_in_builder_t *obj;
    const char *path = NULL;
    size_t path_len = 0;
    int rv;
    zval *options = NULL;
    pcbc_sd_spec_t *spec;

    obj = Z_LOOKUP_IN_BUILDER_OBJ_P(getThis());

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|z", &path, &path_len, &options);
    if (rv == FAILURE) {
        RETURN_NULL();
    }

    spec = ecalloc(1, sizeof(pcbc_sd_spec_t));
    spec->next = NULL;
    spec->s.sdcmd = LCB_SDCMD_GET_COUNT;
    spec->s.options = pcbc_subdoc_options_to_flags(1, 1, options TSRMLS_CC);
    PCBC_SDSPEC_COPY_PATH(spec, path, path_len);
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

/* {{{ proto \Couchbase\LookupInBuilder LookupInBuilder::execute() */
PHP_METHOD(LookupInBuilder, execute)
{
    pcbc_lookup_in_builder_t *obj;
    int rv;

    obj = Z_LOOKUP_IN_BUILDER_OBJ_P(getThis());

    rv = zend_parse_parameters_none();
    if (rv == FAILURE) {
        RETURN_NULL();
    }

    pcbc_bucket_subdoc_request(obj->bucket, obj, 1, return_value TSRMLS_CC);
} /* }}} */

ZEND_BEGIN_ARG_INFO_EX(ai_LookupInBuilder_none, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(ai_LookupInBuilder_get, 0, 0, 1)
ZEND_ARG_INFO(0, path)
ZEND_ARG_INFO(0, options)
ZEND_END_ARG_INFO()

// clang-format off
zend_function_entry lookup_in_builder_methods[] = {
    PHP_ME(LookupInBuilder, __construct, ai_LookupInBuilder_none, ZEND_ACC_PRIVATE | ZEND_ACC_FINAL | ZEND_ACC_CTOR)
    PHP_ME(LookupInBuilder, get, ai_LookupInBuilder_get, ZEND_ACC_PUBLIC)
    PHP_ME(LookupInBuilder, getCount, ai_LookupInBuilder_get, ZEND_ACC_PUBLIC)
    PHP_ME(LookupInBuilder, exists, ai_LookupInBuilder_get, ZEND_ACC_PUBLIC)
    PHP_ME(LookupInBuilder, execute, ai_LookupInBuilder_none, ZEND_ACC_PUBLIC)
    PHP_FE_END
};
// clang-format on

zend_object_handlers pcbc_lookup_in_builder_handlers;

void pcbc_lookup_in_builder_init(zval *return_value, zval *bucket, const char *id, int id_len, zval *args,
                                 int num_args TSRMLS_DC)
{
    pcbc_lookup_in_builder_t *builder;

    object_init_ex(return_value, pcbc_lookup_in_builder_ce);
    builder = Z_LOOKUP_IN_BUILDER_OBJ_P(return_value);
    ZVAL_COPY(&builder->bucket_zval, bucket);
    builder->bucket = Z_BUCKET_OBJ_P(bucket);
    builder->id_len = id_len;
    builder->id = estrndup(id, id_len);
    builder->nspecs = 0;
    builder->head = NULL;
    builder->tail = NULL;
    if (num_args && args) {
        int i;
        for (i = 0; i < num_args; ++i) {
            zval *path;
            path = &args[i];
            if (&*path && Z_TYPE_P(&*path) != IS_STRING) {
                pcbc_log(LOGARGS(builder, WARN), "path has to be a string (skipping argument #%d)", i);
                continue;
            }
            pcbc_lookup_in_builder_get(builder, Z_STRVAL_P(&*path), Z_STRLEN_P(&*path), NULL TSRMLS_CC);
        }
    }
}

static void lookup_in_builder_free_object(zend_object *object TSRMLS_DC) /* {{{ */
{
    pcbc_lookup_in_builder_t *obj = Z_LOOKUP_IN_BUILDER_OBJ(object);
    pcbc_sd_spec_t *spec;

    if (obj->id != NULL) {
        efree(obj->id);
    }
    spec = obj->head;
    while (spec) {
        pcbc_sd_spec_t *tmp = spec;
        spec = spec->next;
        PCBC_SDSPEC_FREE_PATH(tmp);
        efree(tmp);
    }
    obj->head = obj->tail = NULL;
    Z_DELREF_P(&obj->bucket_zval);
    ZVAL_UNDEF(&obj->bucket_zval);
    obj->bucket = NULL;
    zend_object_std_dtor(&obj->std TSRMLS_CC);
} /* }}} */

static zend_object *lookup_in_builder_create_object(zend_class_entry *class_type TSRMLS_DC)
{
    pcbc_lookup_in_builder_t *obj = NULL;

    obj = PCBC_ALLOC_OBJECT_T(pcbc_lookup_in_builder_t, class_type);

    zend_object_std_init(&obj->std, class_type TSRMLS_CC);
    object_properties_init(&obj->std, class_type);

    obj->std.handlers = &pcbc_lookup_in_builder_handlers;
    return &obj->std;
}

static HashTable *lookup_in_builder_get_debug_info(zval *object, int *is_temp TSRMLS_DC) /* {{{ */
{
    pcbc_lookup_in_builder_t *obj = NULL;
    zval retval;
    zval specs;
    pcbc_sd_spec_t *spec;

    *is_temp = 1;
    obj = Z_LOOKUP_IN_BUILDER_OBJ_P(object);

    array_init(&retval);
    ADD_ASSOC_STRING(&retval, "id", obj->id);

    ZVAL_UNDEF(&specs);
    array_init_size(&specs, obj->nspecs);

    spec = obj->head;
    while (spec) {
        zval s;
        char *path = NULL;
        int path_len = 0;
        array_init(&s);
        switch (spec->s.sdcmd) {
        case LCB_SDCMD_GET:
            ADD_ASSOC_STRING(&s, "cmd", "get");
            PCBC_SDSPEC_GET_PATH(spec, path, path_len);
            ADD_ASSOC_STRINGL(&s, "path", path, path_len);
            ADD_ASSOC_LONG_EX(&s, "options", spec->s.options);
            break;
        case LCB_SDCMD_EXISTS:
            ADD_ASSOC_STRING(&s, "cmd", "exists");
            PCBC_SDSPEC_GET_PATH(spec, path, path_len);
            ADD_ASSOC_STRINGL(&s, "path", path, path_len);
            ADD_ASSOC_LONG_EX(&s, "options", spec->s.options);
            break;
        }
        add_next_index_zval(&specs, &s);
        spec = spec->next;
    }
    add_assoc_zval(&retval, "specs", &specs);

    return Z_ARRVAL(retval);
} /* }}} */

PHP_MINIT_FUNCTION(LookupInBuilder)
{
    zend_class_entry ce;

    INIT_NS_CLASS_ENTRY(ce, "Couchbase", "LookupInBuilder", lookup_in_builder_methods);
    pcbc_lookup_in_builder_ce = zend_register_internal_class(&ce TSRMLS_CC);
    pcbc_lookup_in_builder_ce->create_object = lookup_in_builder_create_object;
    PCBC_CE_DISABLE_SERIALIZATION(pcbc_lookup_in_builder_ce);

    memcpy(&pcbc_lookup_in_builder_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    pcbc_lookup_in_builder_handlers.get_debug_info = lookup_in_builder_get_debug_info;
    pcbc_lookup_in_builder_handlers.free_obj = lookup_in_builder_free_object;
    pcbc_lookup_in_builder_handlers.offset = XtOffsetOf(pcbc_lookup_in_builder_t, std);

    zend_register_class_alias("\\CouchbaseLookupInBuilder", pcbc_lookup_in_builder_ce);
    return SUCCESS;
}
