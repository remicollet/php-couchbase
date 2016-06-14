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

#include "couchbase.h"
#include "ext/standard/php_var.h"
#include "ext/json/php_json.h"
#include "exception.h"
#include "paramparser.h"
#include "zap.h"
#include "bucket.h"
#include "cas.h"
#include "metadoc.h"
#include "docfrag.h"
#include "transcoding.h"
#include "opcookie.h"

void store_callback(lcb_t instance, int cbtype, const lcb_RESPBASE *rb)
{
    opcookie_store_res *result = ecalloc(1, sizeof(opcookie_store_res));
    const lcb_RESPSTORE *resp = (const lcb_RESPSTORE *)rb;
    TSRMLS_FETCH();

    result->header.err = resp->rc;
    zapval_alloc_stringl(result->key, resp->key, resp->nkey);
    cas_encode(&result->cas, resp->cas TSRMLS_CC);

    opcookie_push((opcookie*)resp->cookie, &result->header);
}

lcb_error_t proc_store_results(bucket_object *bucket, zval *return_value,
        opcookie *cookie, int is_mapped TSRMLS_DC)
{
    opcookie_store_res *res;
    lcb_error_t err = LCB_SUCCESS;

    // If we are not mapped, we need to throw any op errors
    if (is_mapped == 0) {
        err = opcookie_get_first_error(cookie);
    }

    if (err == LCB_SUCCESS) {
        FOREACH_OPCOOKIE_RES(opcookie_store_res, res, cookie) {
            zval *doc = bop_get_return_doc(
                    return_value, &res->key, is_mapped);

            if (res->header.err == LCB_SUCCESS) {
                make_metadoc(doc, NULL, NULL, &res->cas TSRMLS_CC);
            } else {
                make_metadoc_error(doc, res->header.err TSRMLS_CC);
            }
        }
    }

    FOREACH_OPCOOKIE_RES(opcookie_store_res, res, cookie) {
        zapval_destroy(res->key);
        zapval_destroy(res->cas);
    }

    return err;
}

// insert($id, $doc {, $expiry, $groupid}) : MetaDoc
PHP_METHOD(Bucket, insert)
{
    bucket_object *data = PCBC_PHP_THISOBJ();
    int ii, ncmds, nscheduled;
    pcbc_pp_state pp_state;
    pcbc_pp_id id;
    zval *zvalue, *zexpiry, *zflags, *zgroupid;
    opcookie *cookie;
    lcb_error_t err;

    // Note that groupid is experimental here and should not be used.
    if (pcbc_pp_begin(ZEND_NUM_ARGS() TSRMLS_CC, &pp_state,
                      "id|value|expiry,flags,groupid",
                      &id, &zvalue, &zexpiry, &zflags, &zgroupid) != SUCCESS)
    {
        throw_pcbc_exception("Invalid arguments.", LCB_EINVAL);
        RETURN_NULL();
    }

    ncmds = pcbc_pp_keycount(&pp_state);
    cookie = opcookie_init();

    nscheduled = 0;
    for (ii = 0; pcbc_pp_next(&pp_state); ++ii) {
        lcb_CMDSTORE cmd = {0};
        void *bytes;
        lcb_size_t nbytes;

        PCBC_CHECK_ZVAL_LONG(zexpiry, "expiry must be an integer");
        PCBC_CHECK_ZVAL_LONG(zflags, "flags must be an integer");
        PCBC_CHECK_ZVAL_STRING(zgroupid, "groupid must be a string");

        cmd.operation = LCB_ADD;
        LCB_CMD_SET_KEY(&cmd, id.str, id.len);

        if (pcbc_encode_value(data, zvalue, &bytes, &nbytes,
                              &cmd.flags, &cmd.datatype TSRMLS_CC) != SUCCESS) {
            php_error_docref(NULL TSRMLS_CC, E_WARNING, "Failed to encode value for before storing");
            err = LCB_ERROR;
            break;
        }
        LCB_CMD_SET_VALUE(&cmd, bytes, nbytes);

        if (zexpiry) {
            cmd.exptime = Z_LVAL_P(zexpiry);
        }
        if (zflags) {
            cmd.flags = Z_LVAL_P(zflags);
        }
        if (zgroupid) {
            LCB_KREQ_SIMPLE(&cmd._hashkey, Z_STRVAL_P(zgroupid), Z_STRLEN_P(zgroupid));
        }

        err = lcb_store3(data->conn->lcb, cookie, &cmd);
        efree(bytes);
        if (err != LCB_SUCCESS) {
            break;
        }
        nscheduled++;
    }
    pcbc_assert_number_of_commands("insert", nscheduled, ncmds);

    if (nscheduled) {
        lcb_wait(data->conn->lcb);

        err = proc_store_results(data, return_value,
                                 cookie, pcbc_pp_ismapped(&pp_state) TSRMLS_CC);
    }

    opcookie_destroy(cookie);

    if (err != LCB_SUCCESS) {
        throw_lcb_exception(err);
    }
}

// upsert($id, $doc {, $expiry, $groupid}) : MetaDoc
PHP_METHOD(Bucket, upsert)
{
    bucket_object *data = PCBC_PHP_THISOBJ();
    int ii, ncmds, nscheduled;
    pcbc_pp_state pp_state;
    zval *zvalue, *zexpiry, *zflags, *zgroupid;
    pcbc_pp_id id;
    opcookie *cookie;
    lcb_error_t err;

    // Note that groupid is experimental here and should not be used.
    if (pcbc_pp_begin(ZEND_NUM_ARGS() TSRMLS_CC, &pp_state,
                      "id|value|expiry,flags,groupid",
                      &id, &zvalue, &zexpiry, &zflags, &zgroupid) != SUCCESS)
    {
        throw_pcbc_exception("Invalid arguments.", LCB_EINVAL);
        RETURN_NULL();
    }

    ncmds = pcbc_pp_keycount(&pp_state);
    cookie = opcookie_init();

    nscheduled = 0;
    for (ii = 0; pcbc_pp_next(&pp_state); ++ii) {
        lcb_CMDSTORE cmd = {0};
        void *bytes;
        lcb_size_t nbytes;

        PCBC_CHECK_ZVAL_LONG(zexpiry, "expiry must be an integer");
        PCBC_CHECK_ZVAL_LONG(zflags, "flags must be an integer");
        PCBC_CHECK_ZVAL_STRING(zgroupid, "groupid must be a string");

        cmd.operation = LCB_SET;
        LCB_CMD_SET_KEY(&cmd, id.str, id.len);

        if (pcbc_encode_value(data, zvalue, &bytes, &nbytes,
                              &cmd.flags, &cmd.datatype TSRMLS_CC) != SUCCESS) {
            php_error_docref(NULL TSRMLS_CC, E_WARNING, "Failed to encode value for before storing");
            err = LCB_ERROR;
            break;
        }
        LCB_CMD_SET_VALUE(&cmd, bytes, nbytes);

        if (zexpiry) {
            cmd.exptime = Z_LVAL_P(zexpiry);
        }
        if (zflags) {
            cmd.flags = Z_LVAL_P(zflags);
        }
        if (zgroupid) {
            LCB_KREQ_SIMPLE(&cmd._hashkey, Z_STRVAL_P(zgroupid), Z_STRLEN_P(zgroupid));
        }

        err = lcb_store3(data->conn->lcb, cookie, &cmd);
        efree(bytes);
        if (err != LCB_SUCCESS) {
            break;
        }
        nscheduled++;
    }
    pcbc_assert_number_of_commands("upsert", nscheduled, ncmds);

    if (nscheduled) {
        lcb_wait(data->conn->lcb);

        err = proc_store_results(data, return_value,
                                 cookie, pcbc_pp_ismapped(&pp_state) TSRMLS_CC);
    }

    opcookie_destroy(cookie);

    if (err != LCB_SUCCESS) {
        throw_lcb_exception(err);
    }
}

// replace($id, $doc {, $cas, $expiry, $groupid}) : MetaDoc
PHP_METHOD(Bucket, replace)
{
    bucket_object *data = PCBC_PHP_THISOBJ();
    int ii, ncmds, nscheduled;
    pcbc_pp_state pp_state;
    pcbc_pp_id id;
    zval *zvalue, *zcas, *zexpiry, *zflags, *zgroupid;
    opcookie *cookie;
    lcb_error_t err;

    // Note that groupid is experimental here and should not be used.
    if (pcbc_pp_begin(ZEND_NUM_ARGS() TSRMLS_CC, &pp_state,
                      "id|value|cas,expiry,flags,groupid",
                      &id, &zvalue, &zcas, &zexpiry, &zflags, &zgroupid) != SUCCESS)
    {
        throw_pcbc_exception("Invalid arguments.", LCB_EINVAL);
        RETURN_NULL();
    }

    ncmds = pcbc_pp_keycount(&pp_state);
    cookie = opcookie_init();

    nscheduled = 0;
    for (ii = 0; pcbc_pp_next(&pp_state); ++ii) {
        lcb_CMDSTORE cmd = {0};
        void *bytes;
        lcb_size_t nbytes;

        PCBC_CHECK_ZVAL_STRING(zcas, "cas must be a string");
        PCBC_CHECK_ZVAL_LONG(zexpiry, "expiry must be an integer");
        PCBC_CHECK_ZVAL_LONG(zflags, "flags must be an integer");
        PCBC_CHECK_ZVAL_STRING(zgroupid, "groupid must be a string");

        cmd.operation = LCB_REPLACE;
        LCB_CMD_SET_KEY(&cmd, id.str, id.len);

        if (pcbc_encode_value(data, zvalue, &bytes, &nbytes,
                              &cmd.flags, &cmd.datatype TSRMLS_CC) != SUCCESS) {
            php_error_docref(NULL TSRMLS_CC, E_WARNING, "Failed to encode value for before storing");
            err = LCB_ERROR;
            break;
        }
        LCB_CMD_SET_VALUE(&cmd, bytes, nbytes);

        if (zcas) {
            cmd.cas = cas_decode(zcas TSRMLS_CC);
        }
        if (zexpiry) {
            cmd.exptime = Z_LVAL_P(zexpiry);
        }
        if (zflags) {
            cmd.flags = Z_LVAL_P(zflags);
        }
        if (zgroupid) {
            LCB_KREQ_SIMPLE(&cmd._hashkey, Z_STRVAL_P(zgroupid), Z_STRLEN_P(zgroupid));
        }

        err = lcb_store3(data->conn->lcb, cookie, &cmd);
        efree(bytes);
        if (err != LCB_SUCCESS) {
            break;
        }
        nscheduled++;
    }
    pcbc_assert_number_of_commands("replace", nscheduled, ncmds);

    if (nscheduled) {
        lcb_wait(data->conn->lcb);

        err = proc_store_results(data, return_value,
                                 cookie, pcbc_pp_ismapped(&pp_state) TSRMLS_CC);
    }

    opcookie_destroy(cookie);

    if (err != LCB_SUCCESS) {
        throw_lcb_exception(err);
    }
}

// append($id, $doc {, $cas, $groupid}) : MetaDoc
PHP_METHOD(Bucket, append)
{
    bucket_object *data = PCBC_PHP_THISOBJ();
    int ii, ncmds, nscheduled;
    pcbc_pp_state pp_state;
    pcbc_pp_id id;
    zval *zvalue, *zcas, *zgroupid;
    opcookie *cookie;
    lcb_error_t err;

    // Note that groupid is experimental here and should not be used.
    if (pcbc_pp_begin(ZEND_NUM_ARGS() TSRMLS_CC, &pp_state, "id|value|cas,groupid",
                      &id, &zvalue, &zcas, &zgroupid) != SUCCESS)
    {
        throw_pcbc_exception("Invalid arguments.", LCB_EINVAL);
        RETURN_NULL();
    }

    ncmds = pcbc_pp_keycount(&pp_state);
    cookie = opcookie_init();

    nscheduled = 0;
    for (ii = 0; pcbc_pp_next(&pp_state); ++ii) {
        lcb_CMDSTORE cmd = {0};
        void *bytes;
        lcb_size_t nbytes;

        PCBC_CHECK_ZVAL_STRING(zcas, "cas must be a string");
        PCBC_CHECK_ZVAL_STRING(zgroupid, "groupid must be a string");

        cmd.operation = LCB_APPEND;
        LCB_CMD_SET_KEY(&cmd, id.str, id.len);

        if (pcbc_encode_value(data, zvalue, &bytes, &nbytes,
                              &cmd.flags, &cmd.datatype TSRMLS_CC) != SUCCESS) {
            php_error_docref(NULL TSRMLS_CC, E_WARNING, "Failed to encode value for before storing");
            err = LCB_ERROR;
            break;
        }
        LCB_CMD_SET_VALUE(&cmd, bytes, nbytes);

        if (zcas) {
            cmd.cas = cas_decode(zcas TSRMLS_CC);
        }
        if (zgroupid) {
            LCB_KREQ_SIMPLE(&cmd._hashkey, Z_STRVAL_P(zgroupid), Z_STRLEN_P(zgroupid));
        }

        // Flags ignored for this op, enforced by libcouchbase
        cmd.flags = 0;

        err = lcb_store3(data->conn->lcb, cookie, &cmd);
        efree(bytes);
        if (err != LCB_SUCCESS) {
            break;
        }
        nscheduled++;
    }
    pcbc_assert_number_of_commands("append", nscheduled, ncmds);

    if (nscheduled) {
        lcb_wait(data->conn->lcb);

        err = proc_store_results(data, return_value,
                                 cookie, pcbc_pp_ismapped(&pp_state) TSRMLS_CC);
    }

    opcookie_destroy(cookie);

    if (err != LCB_SUCCESS) {
        throw_lcb_exception(err);
    }
}

// prepend($id, $doc {, $cas, $groupid}) : MetaDoc
PHP_METHOD(Bucket, prepend)
{
    bucket_object *data = PCBC_PHP_THISOBJ();
    int ii, ncmds, nscheduled;
    pcbc_pp_state pp_state;
    pcbc_pp_id id;
    zval *zvalue, *zcas, *zgroupid;
    opcookie *cookie;
    lcb_error_t err = LCB_SUCCESS;

    // Note that groupid is experimental here and should not be used.
    if (pcbc_pp_begin(ZEND_NUM_ARGS() TSRMLS_CC, &pp_state,
                      "id|value|cas,groupid",
                      &id, &zvalue, &zcas, &zgroupid) != SUCCESS)
    {
        throw_pcbc_exception("Invalid arguments.", LCB_EINVAL);
        RETURN_NULL();
    }

    ncmds = pcbc_pp_keycount(&pp_state);
    cookie = opcookie_init();

    nscheduled = 0;
    for (ii = 0; pcbc_pp_next(&pp_state); ++ii) {
        lcb_CMDSTORE cmd = {0};
        void *bytes;
        lcb_size_t nbytes;

        PCBC_CHECK_ZVAL_STRING(zcas, "cas must be a string");
        PCBC_CHECK_ZVAL_STRING(zgroupid, "groupid must be a string");

        cmd.operation = LCB_PREPEND;
        LCB_CMD_SET_KEY(&cmd, id.str, id.len);

        if (pcbc_encode_value(data, zvalue, &bytes, &nbytes,
                              &cmd.flags, &cmd.datatype TSRMLS_CC) != SUCCESS) {
            php_error_docref(NULL TSRMLS_CC, E_WARNING, "Failed to encode value for before storing");
            err = LCB_ERROR;
            break;
        }
        LCB_CMD_SET_VALUE(&cmd, bytes, nbytes);

        if (zcas) {
            cmd.cas = cas_decode(zcas TSRMLS_CC);
        }
        if (zgroupid) {
            LCB_KREQ_SIMPLE(&cmd._hashkey, Z_STRVAL_P(zgroupid), Z_STRLEN_P(zgroupid));
        }

        // Flags ignored for this op, enforced by libcouchbase
        cmd.flags = 0;

        err = lcb_store3(data->conn->lcb, cookie, &cmd);
        efree(bytes);
        if (err != LCB_SUCCESS) {
            break;
        }
        nscheduled++;
    }
    pcbc_assert_number_of_commands("prepend", nscheduled, ncmds);

    if (nscheduled) {
        lcb_wait(data->conn->lcb);

        err = proc_store_results(data, return_value,
                                 cookie, pcbc_pp_ismapped(&pp_state) TSRMLS_CC);
    }

    opcookie_destroy(cookie);

    if (err != LCB_SUCCESS) {
        throw_lcb_exception(err);
    }
}
