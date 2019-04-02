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

lcb_U64 pcbc_base36_decode_str(const char *str, int len)
{
    lcb_U64 num = 0;
    const char *s;
    int i;

    s = str;
    for (i = len; i > 0; i--) {
        char c = *s++;

        if (c >= '0' && c <= '9') {
            c -= '0';
        } else if (c >= 'A' && c <= 'Z') {
            c -= 'A' - 10;
        } else if (c >= 'a' && c <= 'z') {
            c -= 'a' - 10;
        } else {
            continue;
        }
        num = num * 36 + c;
    }
    return num;
}

char *pcbc_base36_encode_str(lcb_U64 num)
{
    static char digits[36] = "0123456789abcdefghijklmnopqrstuvwxyz";
    char buf[14];
    char *ptr, *end;

    end = ptr = buf + sizeof(buf) - 1;
    *ptr = '\0';

    do {
        --ptr;
        *ptr = digits[num % 36];
        num /= 36;
    } while (ptr > buf && num);
    return estrndup(ptr, end - ptr);
}

lcb_cas_t pcbc_cas_decode(zval *cas TSRMLS_DC)
{
    if (Z_TYPE_P(cas) != IS_STRING) {
        return 0;
    }

    return pcbc_base36_decode_str(Z_STRVAL_P(cas), Z_STRLEN_P(cas));
}

void pcbc_cas_encode(zval *return_value, lcb_cas_t cas TSRMLS_DC)
{
    char *str = pcbc_base36_encode_str(cas);

    ZVAL_STRING(return_value, str);
    efree(str);
}
