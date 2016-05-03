#include <libcouchbase/couchbase.h>
#include <php.h>
#include "zap.h"
#include "cas.h"

#define PCBC_CAS_DIGITS "0123456789abcdefghijklmnopqrstuvwxyz"
#define PCBC_CAS_BASE   36

lcb_cas_t cas_decode(zval *zcas TSRMLS_DC) {
    char *str, c;
    int i;
    lcb_cas_t cas = 0;

    if (!zapval_is_string(*zcas)) {
        return 0;
    }

    str = zapval_strval_p(zcas);
    for (i = zapval_strlen_p(zcas); i > 0; i--) {
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

void cas_encode(zapval *casout, lcb_cas_t value TSRMLS_DC) {
    static char digits[] = PCBC_CAS_DIGITS;
    char buf[14];
    char *ptr, *end;
    lcb_cas_t orig = value;

    end = ptr = buf + sizeof(buf) - 1;
    *ptr = '\0';

    do {
        --ptr;
        *ptr = digits[value % PCBC_CAS_BASE];
        value /= PCBC_CAS_BASE;
    } while (ptr > buf && value);

    zapval_alloc_stringl(*casout, ptr, end - ptr);
}
