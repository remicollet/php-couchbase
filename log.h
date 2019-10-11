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

#ifndef LOG_H_
#define LOG_H_

#include <libcouchbase/couchbase.h>

struct pcbc_logger_st {
    int minlevel;
    lcb_LOGGER_CALLBACK callback;
};

void pcbc_log_formatter(char *buf, int buf_size, const char *severity, const char *subsystem, int srcline,
                        int instance_id, void *instance_ptr, int is_lcb, const char *fmt, va_list ap);
void pcbc_log(int severity, lcb_INSTANCE *instance, const char *subsys, const char *srcfile, int srcline, const char *fmt, ...);

#endif // LOG_H_
