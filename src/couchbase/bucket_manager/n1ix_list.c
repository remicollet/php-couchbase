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

#define LOGARGS(instance, lvl) LCB_LOG_##lvl, instance, "pcbc/n1ix_list", __FILE__, __LINE__

typedef struct {
    opcookie_res header;
    PCBC_ZVAL *specs;
    int nspecs;
} opcookie_n1ix_list_res;

static void n1ix_list_callback(lcb_t instance, int cbtype, const lcb_RESPN1XMGMT *resp)
{
    opcookie_n1ix_list_res *result = ecalloc(1, sizeof(opcookie_n1ix_list_res));
    int i;
    TSRMLS_FETCH();

    result->header.err = resp->rc;
    if (result->header.err != LCB_SUCCESS) {
        pcbc_log(LOGARGS(instance, ERROR), "Failed to list indexes. %d: %.*s", (int)resp->inner->htresp->htstatus,
                 (int)resp->inner->nrow, (char *)resp->inner->row);
    }
    result->nspecs = resp->nspecs;
    result->specs = ecalloc(result->nspecs, sizeof(PCBC_ZVAL));
    for (i = 0; i < result->nspecs; ++i) {
        const lcb_N1XSPEC *spec = resp->specs[i];
        int last_error;
        PCBC_ZVAL value, json;

        PCBC_ZVAL_ALLOC(value);
        PCBC_ZVAL_ALLOC(json);
        PCBC_JSON_COPY_DECODE(PCBC_P(json), spec->rawjson, spec->nrawjson, PHP_JSON_OBJECT_AS_ARRAY, last_error);
        if (last_error != 0) {
            pcbc_log(LOGARGS(instance, WARN), "Failed to decode value as JSON: json_last_error=%d", last_error);
            ZVAL_NULL(PCBC_P(value));
        } else {
            pcbc_n1ix_init(PCBC_P(value), PCBC_P(json) TSRMLS_CC);
        }

        zval_ptr_dtor(&json);
        result->specs[i] = value;
    }

    opcookie_push((opcookie *)resp->cookie, &result->header);
}

static lcb_error_t proc_n1ix_list_results(zval *return_value, opcookie *cookie TSRMLS_DC)
{
    opcookie_n1ix_list_res *result = (opcookie_n1ix_list_res *)opcookie_next_res(cookie, NULL);
    lcb_error_t err = opcookie_get_first_error(cookie);
    int i;

    if (result) {
        if (err == LCB_SUCCESS) {
            array_init(return_value);
            for (i = 0; i < result->nspecs; ++i) {
                add_index_zval(return_value, i, PCBC_P(result->specs[i]));
            }
        }
        efree(result->specs);
    }

    return err;
}

int pcbc_n1ix_list(pcbc_bucket_manager_t *manager, zval *return_value TSRMLS_DC)
{
    lcb_CMDN1XMGMT cmd = {0};
    opcookie *cookie;
    lcb_error_t err;

    cmd.callback = n1ix_list_callback;
    cookie = opcookie_init();

    cmd.spec.nkeyspace = strlen(manager->conn->bucketname);
    cmd.spec.keyspace = manager->conn->bucketname;
    err = lcb_n1x_list(manager->conn->lcb, cookie, &cmd);

    if (err == LCB_SUCCESS) {
        lcb_wait(manager->conn->lcb);
        err = proc_n1ix_list_results(return_value, cookie TSRMLS_CC);
    }

    opcookie_destroy(cookie);

    if (err != LCB_SUCCESS) {
        throw_lcb_exception(err);
        return FAILURE;
    }
    return SUCCESS;
}
