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

#include "log.h"
#include "zap.h"

static const char *level_to_string(int severity)
{
    switch (severity) {
    case LCB_LOG_TRACE:
        return "TRAC";
    case LCB_LOG_DEBUG:
        return "DEBG";
    case LCB_LOG_INFO:
        return "INFO";
    case LCB_LOG_WARN:
        return "WARN";
    case LCB_LOG_ERROR:
        return "EROR";
    case LCB_LOG_FATAL:
        return "FATL";
    default:
        return "";
    }
}

static void level_to_psr3(int severity, zapval *result)
{
    switch (severity) {
    case LCB_LOG_TRACE:
        zapval_alloc_stringl(*result, "debug", strlen("debug"));
        return;
    case LCB_LOG_DEBUG:
        zapval_alloc_stringl(*result, "debug", strlen("debug"));
        return;
    case LCB_LOG_INFO:
        zapval_alloc_stringl(*result, "info", strlen("info"));
        return;
    case LCB_LOG_WARN:
        zapval_alloc_stringl(*result, "warning", strlen("warning"));
        return;
    case LCB_LOG_ERROR:
        zapval_alloc_stringl(*result, "error", strlen("error"));
        return;
    case LCB_LOG_FATAL:
        zapval_alloc_stringl(*result, "emergency", strlen("emergency"));
        return;
    default:
        return;
    }
}

static void invoke_psr3_logger(int severity, const char *msg TSRMLS_DC);

static void log_handler(struct lcb_logprocs_st *procs, unsigned int iid,
                        const char *subsys, int severity, const char *srcfile,
                        int srcline, const char *fmt, va_list ap)
{
    struct pcbc_logger_st *logger = (struct pcbc_logger_st *)procs;
    char *buf = NULL;
    char *msg = NULL;
    TSRMLS_FETCH();

    if (severity < logger->minlevel) {
        return;
    }

    vspprintf(&msg, 0, fmt, ap);
    spprintf(&buf, 0, "[cb,%s] (%s L:%d I:%d) %s", level_to_string(severity),
             subsys, srcline, iid, msg);
    efree(msg);
    if (zap_zval_is_null(zapval_zvalptr(logger->psr3_logger))) {
        php_log_err(buf TSRMLS_CC);
    } else {
        invoke_psr3_logger(severity, buf TSRMLS_CC);
    }
    efree(buf);
}

struct pcbc_logger_st pcbc_logger = {
    {0 /* version */, {{log_handler} /* v1 */} /*v*/},
    /** Minimum severity */
    LCB_LOG_INFO};

void pcbc_log(int severity, lcb_t instance, const char *subsys,
              const char *srcfile, int srcline, const char *fmt, ...)
{
    va_list ap;
    char *msg = NULL;
    char *buf = NULL;
    TSRMLS_FETCH();

    if (severity < pcbc_logger.minlevel) {
        return;
    }

    va_start(ap, fmt);
    vspprintf(&msg, 0, fmt, ap);
    va_end(ap);
    if (instance) {
        spprintf(&buf, 0, "[cb,%s] (%s L:%d) %s. I=%p",
                 level_to_string(severity), subsys, srcline, msg,
                 (void *)instance);
    } else {
        spprintf(&buf, 0, "[cb,%s] (%s L:%d) %s", level_to_string(severity),
                 subsys, srcline, msg);
    }
    efree(msg);

    if (!zapval_zvalptr(pcbc_logger.psr3_logger) || zap_zval_is_null(zapval_zvalptr(pcbc_logger.psr3_logger))) {
        php_log_err(buf TSRMLS_CC);
    } else {
        invoke_psr3_logger(severity, buf TSRMLS_CC);
    }
    efree(buf);
}

zapval psr3_levels[LCB_LOG_MAX];
zapval psr3_method_name;

static void invoke_psr3_logger(int severity, const char *msg TSRMLS_DC)
{
    zapval message, rv;

    zapval_alloc_stringl(message, msg, strlen(msg));
    zapval_alloc_null(rv);

    if (!zapval_zvalptr(psr3_method_name) || zap_zval_is_null(zapval_zvalptr(psr3_method_name))) {
        zapval_alloc_stringl(psr3_method_name, "log", strlen("log"));
    }
    if (!zapval_zvalptr(psr3_levels[severity]) || zap_zval_is_null(zapval_zvalptr(psr3_levels[severity]))) {
        level_to_psr3(severity, &psr3_levels[severity]);
    }

    {
        zapval params[] = {psr3_levels[severity], message};
        call_user_function(CG(function_table), &pcbc_logger.psr3_logger,
                           zapval_zvalptr(psr3_method_name), zapval_zvalptr(rv),
                           2, params TSRMLS_CC);
    }
}
