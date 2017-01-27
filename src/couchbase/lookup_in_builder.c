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
#include <Zend/zend_alloc.h>

#define LOGARGS(builder, lvl) LCB_LOG_##lvl, builder->bucket->conn->lcb, "pcbc/lookup_in_builder", __FILE__, __LINE__

zend_class_entry *pcbc_lookup_in_builder_ce;

/* {{{ proto void LookupInBuilder::__construct() Should not be called directly */
PHP_METHOD(LookupInBuilder, __construct) { throw_pcbc_exception("Accessing private constructor.", LCB_EINVAL); }
/* }}} */

int pcbc_lookup_in_builder_get(pcbc_lookup_in_builder_t *builder, char *path, int path_len TSRMLS_DC)
{
    pcbc_sd_spec_t *spec;

    spec = ecalloc(1, sizeof(pcbc_sd_spec_t));
    spec->next = NULL;
    spec->s.sdcmd = LCB_SDCMD_GET;
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

/* {{{ proto \Couchbase\LookupInBuilder LookupInBuilder::get(string $path) */
PHP_METHOD(LookupInBuilder, get)
{
    pcbc_lookup_in_builder_t *obj;
    char *path = NULL;
    pcbc_str_arg_size path_len = 0;
    int rv;

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &path, &path_len);
    if (rv == FAILURE) {
        RETURN_NULL();
    }

    obj = Z_LOOKUP_IN_BUILDER_OBJ_P(getThis());
    pcbc_lookup_in_builder_get(obj, path, path_len TSRMLS_CC);

    RETURN_ZVAL(getThis(), 1, 0);
} /* }}} */

/* {{{ proto \Couchbase\LookupInBuilder LookupInBuilder::exists(string $path) */
PHP_METHOD(LookupInBuilder, exists)
{
    pcbc_lookup_in_builder_t *obj;
    const char *path = NULL;
    pcbc_str_arg_size path_len = 0;
    int rv;
    pcbc_sd_spec_t *spec;

    obj = Z_LOOKUP_IN_BUILDER_OBJ_P(getThis());

    rv = zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &path, &path_len);
    if (rv == FAILURE) {
        RETURN_NULL();
    }

    spec = ecalloc(1, sizeof(pcbc_sd_spec_t));
    spec->next = NULL;
    spec->s.sdcmd = LCB_SDCMD_EXISTS;
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
ZEND_END_ARG_INFO()

// clang-format off
zend_function_entry lookup_in_builder_methods[] = {
    PHP_ME(LookupInBuilder, __construct, ai_LookupInBuilder_none, ZEND_ACC_PRIVATE | ZEND_ACC_FINAL | ZEND_ACC_CTOR)
    PHP_ME(LookupInBuilder, get, ai_LookupInBuilder_get, ZEND_ACC_PUBLIC | ZEND_ACC_FINAL)
    PHP_ME(LookupInBuilder, exists, ai_LookupInBuilder_get, ZEND_ACC_PUBLIC | ZEND_ACC_FINAL)
    PHP_ME(LookupInBuilder, execute, ai_LookupInBuilder_none, ZEND_ACC_PUBLIC | ZEND_ACC_FINAL)
    PHP_FE_END
};
// clang-format on

zend_object_handlers pcbc_lookup_in_builder_handlers;

void pcbc_lookup_in_builder_init(zval *return_value, zval *bucket, const char *id, int id_len,
#if PHP_VERSION_ID >= 70000
                                 zval *args,
#else
                                 zval ***args,
#endif
                                 int num_args TSRMLS_DC)
{
    pcbc_lookup_in_builder_t *builder;

    object_init_ex(return_value, pcbc_lookup_in_builder_ce);
    builder = Z_LOOKUP_IN_BUILDER_OBJ_P(return_value);
    builder->bucket_zval = bucket;
    Z_ADDREF_P(builder->bucket_zval);
    builder->bucket = Z_BUCKET_OBJ_P(bucket);
    builder->id_len = id_len;
    builder->id = estrndup(id, id_len);
    builder->nspecs = 0;
    builder->head = NULL;
    builder->tail = NULL;
    if (num_args && args) {
        int i;
        for (i = 0; i < num_args; ++i) {
            PCBC_ZVAL *path;
#if PHP_VERSION_ID >= 70000
            path = &args[i];
#else
            path = args[i];
#endif
            if (PCBC_P(*path) && Z_TYPE_P(PCBC_P(*path)) != IS_STRING) {
                pcbc_log(LOGARGS(builder, WARN), "path has to be a string (skipping argument #%d)", i);
                continue;
            }
            pcbc_lookup_in_builder_get(builder, Z_STRVAL_P(PCBC_P(*path)), Z_STRLEN_P(PCBC_P(*path)) TSRMLS_CC);
        }
    }
}

static void lookup_in_builder_free_object(pcbc_free_object_arg *object TSRMLS_DC) /* {{{ */
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
    Z_DELREF_P(obj->bucket_zval);
    obj->bucket = NULL;

    zend_object_std_dtor(&obj->std TSRMLS_CC);
#if PHP_VERSION_ID < 70000
    efree(obj);
#endif
} /* }}} */

static pcbc_create_object_retval lookup_in_builder_create_object(zend_class_entry *class_type TSRMLS_DC)
{
    pcbc_lookup_in_builder_t *obj = NULL;

    obj = PCBC_ALLOC_OBJECT_T(pcbc_lookup_in_builder_t, class_type);

    zend_object_std_init(&obj->std, class_type TSRMLS_CC);
    object_properties_init(&obj->std, class_type);

#if PHP_VERSION_ID >= 70000
    obj->std.handlers = &pcbc_lookup_in_builder_handlers;
    return &obj->std;
#else
    {
        zend_object_value ret;
        ret.handle = zend_objects_store_put(obj, (zend_objects_store_dtor_t)zend_objects_destroy_object,
                                            lookup_in_builder_free_object, NULL TSRMLS_CC);
        ret.handlers = &pcbc_lookup_in_builder_handlers;
        return ret;
    }
#endif
}

static HashTable *lookup_in_builder_get_debug_info(zval *object, int *is_temp TSRMLS_DC) /* {{{ */
{
    pcbc_lookup_in_builder_t *obj = NULL;
#if PHP_VERSION_ID >= 70000
    zval retval;
#else
    zval retval = zval_used_for_init;
#endif
    PCBC_ZVAL specs;
    pcbc_sd_spec_t *spec;

    *is_temp = 1;
    obj = Z_LOOKUP_IN_BUILDER_OBJ_P(object);

    array_init(&retval);
    ADD_ASSOC_STRING(&retval, "id", obj->id);

    PCBC_ZVAL_ALLOC(specs);
    array_init_size(PCBC_P(specs), obj->nspecs);

    spec = obj->head;
    while (spec) {
        PCBC_ZVAL s;
        char *path = NULL;
        int path_len = 0;
#if PHP_VERSION_ID < 70000
        MAKE_STD_ZVAL(s);
#endif
        array_init(PCBC_P(s));
        switch (spec->s.sdcmd) {
        case LCB_SDCMD_GET:
            ADD_ASSOC_STRING(PCBC_P(s), "cmd", "get");
            PCBC_SDSPEC_GET_PATH(spec, path, path_len);
            ADD_ASSOC_STRINGL(PCBC_P(s), "path", path, path_len);
            break;
        case LCB_SDCMD_EXISTS:
            ADD_ASSOC_STRING(PCBC_P(s), "cmd", "exists");
            PCBC_SDSPEC_GET_PATH(spec, path, path_len);
            ADD_ASSOC_STRINGL(PCBC_P(s), "path", path, path_len);
            break;
        }
        add_next_index_zval(PCBC_P(specs), PCBC_P(s));
        spec = spec->next;
    }
    add_assoc_zval(&retval, "specs", PCBC_P(specs));

    return Z_ARRVAL(retval);
} /* }}} */

PHP_MINIT_FUNCTION(LookupInBuilder)
{
    zend_class_entry ce;

    INIT_NS_CLASS_ENTRY(ce, "Couchbase", "LookupInBuilder", lookup_in_builder_methods);
    pcbc_lookup_in_builder_ce = zend_register_internal_class(&ce TSRMLS_CC);
    pcbc_lookup_in_builder_ce->create_object = lookup_in_builder_create_object;
    PCBC_CE_FLAGS_FINAL(pcbc_lookup_in_builder_ce);
    PCBC_CE_DISABLE_SERIALIZATION(pcbc_lookup_in_builder_ce);

    memcpy(&pcbc_lookup_in_builder_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    pcbc_lookup_in_builder_handlers.get_debug_info = lookup_in_builder_get_debug_info;
#if PHP_VERSION_ID >= 70000
    pcbc_lookup_in_builder_handlers.free_obj = lookup_in_builder_free_object;
    pcbc_lookup_in_builder_handlers.offset = XtOffsetOf(pcbc_lookup_in_builder_t, std);
#endif

    zend_register_class_alias("\\CouchbaseLookupInBuilder", pcbc_lookup_in_builder_ce);
    return SUCCESS;
}
