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

#ifndef OPCOOKIE_H_
#define OPCOOKIE_H_

#include <php.h>
#include "couchbase.h"
#include "zap.h"

typedef struct {
    void *next;
    lcb_error_t err;
} opcookie_res;

typedef struct {
    opcookie_res *res_head;
    opcookie_res *res_tail;
    lcb_error_t first_error;
} opcookie;

opcookie * opcookie_init();
void opcookie_destroy(opcookie *cookie);
void opcookie_push(opcookie *cookie, opcookie_res *res);
lcb_error_t opcookie_get_first_error(opcookie *cookie);
opcookie_res * opcookie_next_res(opcookie *cookie, opcookie_res *cur);

#define FOREACH_OPCOOKIE_RES(Type, Res, cookie) \
    Res = NULL; \
    while ((Res = (Type*)opcookie_next_res(cookie, (opcookie_res*)Res)) != NULL)

#endif /* OPCOOKIE_H_ */
