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

#include <libcouchbase/couchbase.h>
#include <php.h>
#include "zap.h"
#include "cas.h"

#define PCBC_CAS_DIGITS "0123456789abcdefghijklmnopqrstuvwxyz"
#define PCBC_CAS_BASE   36

lcb_cas_t cas_decode(zval *zcasp TSRMLS_DC)
{
    char *str, c;
    int i;
    lcb_cas_t cas = 0;
    zapval zcas = zapval_from_zvalptr(zcasp);

    if (!zapval_is_string(zcas)) {
        return 0;
    }

    str = zapval_strval_p(&zcas);
    for (i = zapval_strlen_p(&zcas); i > 0; i--) {
        c = *str++;

        if (c >= '0' && c <= '9') {
            c -= '0';
        } else if (c >= 'A' && c <= 'Z') {
            c -= 'A' - 10;
        } else if (c >= 'a' && c <= 'z') {
            c -= 'a' - 10;
        } else {
            continue;
        }
        cas = cas * PCBC_CAS_BASE + c;
    }
    return cas;
}

void cas_encode(zapval *casout, lcb_cas_t value TSRMLS_DC)
{
    static char digits[] = PCBC_CAS_DIGITS;
    char buf[14];
    char *ptr, *end;

    end = ptr = buf + sizeof(buf) - 1;
    *ptr = '\0';

    do {
        --ptr;
        *ptr = digits[value % PCBC_CAS_BASE];
        value /= PCBC_CAS_BASE;
    } while (ptr > buf && value);

    zapval_alloc_stringl(*casout, ptr, end - ptr);
}
