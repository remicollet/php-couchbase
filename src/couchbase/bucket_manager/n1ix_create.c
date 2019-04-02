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

#define LOGARGS(instance, lvl) LCB_LOG_##lvl, instance, "pcbc/n1ix", __FILE__, __LINE__

typedef struct {
    opcookie_res header;
} opcookie_n1ix_create_res;

static void n1ix_create_callback(lcb_t instance, int cbtype, const lcb_RESPN1XMGMT *resp)
{
    opcookie_n1ix_create_res *result = ecalloc(1, sizeof(opcookie_n1ix_create_res));
    TSRMLS_FETCH();

    result->header.err = resp->rc;
    if (result->header.err != LCB_SUCCESS) {
        pcbc_log(LOGARGS(instance, ERROR), "Failed to create index. %d: %.*s", (int)resp->inner->htresp->htstatus,
                 (int)resp->inner->nrow, (char *)resp->inner->row);
    }
    opcookie_push((opcookie *)resp->cookie, &result->header);
}

void pcbc_n1ix_create(pcbc_bucket_manager_t *manager, lcb_CMDN1XMGMT *cmd, zend_bool ignore_if_exist,
                      zval *return_value TSRMLS_DC)
{
    opcookie *cookie;
    lcb_error_t err;

    cmd->callback = n1ix_create_callback;
    cookie = opcookie_init();
    err = lcb_n1x_create(manager->conn->lcb, cookie, cmd);
    if (err == LCB_SUCCESS) {
        lcb_wait(manager->conn->lcb);

        err = opcookie_get_first_error(cookie);
        if (err == LCB_KEY_EEXISTS && ignore_if_exist) {
            err = LCB_SUCCESS;
        }
    }
    opcookie_destroy(cookie);
    if (err != LCB_SUCCESS) {
        throw_lcb_exception(err);
    }
}
