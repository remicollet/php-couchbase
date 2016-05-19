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

#ifndef PARAMPARSER_H_
#define PARAMPARSER_H_

#include <php.h>
#include "zap.h"

#define PCBC_PP_MAX_ARGS 10

typedef struct {
    char *str;
    uint len;
} pcbc_pp_id;

typedef struct {
    char name[16];
    zval **ptr;
    zapval val;
} pcbc_pp_state_arg;

typedef struct {
    pcbc_pp_state_arg args[PCBC_PP_MAX_ARGS];
    int arg_req;
    int arg_opt;
    int arg_named;
    int cur_idx;
    zapval zids;
    HashPosition hash_pos;
} pcbc_pp_state;

// assumes first parameter in the spec is the ids (`id|`).
int pcbc_pp_begin(int param_count TSRMLS_DC, pcbc_pp_state *state, const char *spec, ...);
int pcbc_pp_ismapped(pcbc_pp_state *state);
int pcbc_pp_keycount(pcbc_pp_state *state) ;
int pcbc_pp_next(pcbc_pp_state *state) ;

#endif
