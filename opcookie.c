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

opcookie *opcookie_init() { return ecalloc(1, sizeof(opcookie)); }

void opcookie_destroy(opcookie *cookie)
{
    opcookie_res *iter = cookie->res_head;
    while (iter != NULL) {
        opcookie_res *cur = iter;
        iter = cur->next;
        efree(cur);
    }
    efree(cookie);
}

lcb_error_t opcookie_get_first_error(opcookie *cookie) { return cookie->first_error; }

void opcookie_push(opcookie *cookie, opcookie_res *res)
{
    if (cookie->res_head == NULL) {
        cookie->res_head = res;
        cookie->res_tail = res;
    } else {
        cookie->res_tail->next = res;
        cookie->res_tail = res;
    }
    res->next = NULL;

    if (res->err != LCB_SUCCESS && cookie->first_error == LCB_SUCCESS) {
        cookie->first_error = res->err;
    }
}

opcookie_res *opcookie_next_res(opcookie *cookie, opcookie_res *cur)
{
    if (cur == NULL) {
        return cookie->res_head;
    } else {
        return cur->next;
    }
}
