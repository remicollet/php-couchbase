/**
 *     Copyright 2016 Couchbase, Inc.
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

#include "bucket.h"
#include "cas.h"
#include "transcoding.h"
#include "exception.h"

zend_class_entry *n1ix_spec_ce;

zend_function_entry n1ix_spec_methods[] = {
    { NULL, NULL, NULL }
};

#define PROPERTIES(X)                           \
    X("name", name)                             \
    X("isPrimary", is_primary)                  \
    X("type", type)                             \
    X("state", state)                           \
    X("keyspace", keyspace)                     \
    X("namespace", namespace)                   \
    X("fields", fields)                         \
    X("condition", condition)

void couchbase_init_n1ix_spec(INIT_FUNC_ARGS) {
    zend_class_entry ce;

    INIT_CLASS_ENTRY(ce, "CouchbaseN1qlIndex", n1ix_spec_methods);
    n1ix_spec_ce = zend_register_internal_class(&ce TSRMLS_CC);

#define X(name, var) \
    zend_declare_property_null(n1ix_spec_ce, name, strlen(name), ZEND_ACC_PUBLIC TSRMLS_CC);

    PROPERTIES(X)

#undef X
}

int pcbc_make_n1ix_spec(zval *spec, zapval json TSRMLS_DC)
{
    HashTable *hjson;
    zval *val;

    object_init_ex(spec, n1ix_spec_ce);

    hjson = zapval_arrval(json);

    val = zap_hash_str_find_s(hjson, "name");
    if (val) {
        zend_update_property(n1ix_spec_ce, spec, "name", strlen("name"), val TSRMLS_CC);
    }

    val = zap_hash_str_find_s(hjson, "is_primary");
    if (val) {
        zend_update_property(n1ix_spec_ce, spec, "is_primary", strlen("is_primary"), val TSRMLS_CC);
    }

    {
        zapval type;

        val = zap_hash_str_find_s(hjson, "using");
        if (val && strncmp(Z_STRVAL_P(val), "view", sizeof("view")) == 0) {
            zapval_alloc_long(type, LCB_N1XSPEC_T_VIEW);
        } else if (val && strncmp(Z_STRVAL_P(val), "gsi", sizeof("gsi")) == 0) {
            zapval_alloc_long(type, LCB_N1XSPEC_T_GSI);
        } else {
            zapval_alloc_long(type, LCB_N1XSPEC_T_DEFAULT);
        }
        zend_update_property(n1ix_spec_ce, spec, "type", strlen("type"), zapval_zvalptr(type) TSRMLS_CC);
    }

    val = zap_hash_str_find_s(hjson, "state");
    if (val) {
        zend_update_property(n1ix_spec_ce, spec, "state", strlen("state"), val TSRMLS_CC);
    }

    val = zap_hash_str_find_s(hjson, "keyspace_id");
    if (val) {
        zend_update_property(n1ix_spec_ce, spec, "keyspace", strlen("keyspace"), val TSRMLS_CC);
    }

    val = zap_hash_str_find_s(hjson, "namespace_id");
    if (val) {
        zend_update_property(n1ix_spec_ce, spec, "namespace", strlen("namespace"), val TSRMLS_CC);
    }

    val = zap_hash_str_find_s(hjson, "index_key");
    if (val) {
        zend_update_property(n1ix_spec_ce, spec, "fields", strlen("fields"), val TSRMLS_CC);
    }

    val = zap_hash_str_find_s(hjson, "condition");
    if (val) {
        zend_update_property(n1ix_spec_ce, spec, "condition", strlen("condition"), val TSRMLS_CC);
    }

    return SUCCESS;
}
