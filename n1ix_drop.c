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
#include "n1ix_spec.h"

typedef struct {
    opcookie_res header;
} opcookie_n1ix_drop_res;

static void n1ix_drop_callback(lcb_t instance, int cbtype, const lcb_RESPN1XMGMT *resp)
{
    opcookie_n1ix_drop_res *result = ecalloc(1, sizeof(opcookie_n1ix_drop_res));
    TSRMLS_FETCH();

    result->header.err = resp->rc;
    if (result->header.err == LCB_QUERY_ERROR) {
        php_error_docref(NULL TSRMLS_CC, E_WARNING,
                         "failed to drop index. %d: %.*s",
                         (int)resp->inner->htresp->htstatus,
                         (int)resp->inner->nrow,
                         (char *)resp->inner->row);
    }

    opcookie_push((opcookie*)resp->cookie, &result->header);
}

PHP_METHOD(Bucket, n1ix_drop)
{
    bucket_object *data = PCBC_PHP_THISOBJ();
    lcb_CMDN1XMGMT cmd = { 0 };
    opcookie *cookie;
    lcb_error_t err;
    zval *name;
    zend_bool ignore_if_not_exist = 0, is_primary = 0;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zbb",
                              &name, &ignore_if_not_exist, &is_primary) == FAILURE) {
        throw_pcbc_exception("Invalid arguments.", LCB_EINVAL);
        RETURN_NULL();
    }

    PCBC_CHECK_ZVAL_STRING(name, "name must be a string");

    cmd.spec.name = Z_STRVAL_P(name);
    cmd.spec.nname = Z_STRLEN_P(name);

    cmd.spec.keyspace = data->conn->bucket;
    cmd.spec.nkeyspace = strlen(data->conn->bucket);

    cmd.spec.ixtype = LCB_N1XSPEC_T_GSI;
    cmd.spec.flags = is_primary ? LCB_N1XSPEC_F_PRIMARY : 0;
    cmd.callback = n1ix_drop_callback;

    cookie = opcookie_init();

    err = lcb_n1x_drop(data->conn->lcb, cookie, &cmd);

    if (err == LCB_SUCCESS) {
        lcb_wait(data->conn->lcb);

        err = opcookie_get_first_error(cookie);
        if (err == LCB_KEY_ENOENT && ignore_if_not_exist) {
            err = LCB_SUCCESS;
        }
    }

    opcookie_destroy(cookie);

    if (err != LCB_SUCCESS) {
        throw_lcb_exception(err);
    }
}
