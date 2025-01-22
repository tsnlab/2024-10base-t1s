#ifndef _LOG_H_
#define _LOG_H_

#include <syslog.h>
/* sys/syslog.h:
 * LOG_EMERG       0       system is unusable
 * LOG_ALERT       1       action must be taken immediately
 * LOG_CRIT        2       critical conditions
 * LOG_ERR         3       error conditions
 * LOG_WARNING     4       warning conditions
 * LOG_NOTICE      5       normal but significant condition
 * LOG_INFO        6       informational
 * LOG_DEBUG       7       debug-level messages
 */

#define LOG_NAME_DEFAULT "xbase-t1s"
#define LOG_MSG_LENGTH 1024

struct logpriv_s {
    char* name;
    int daemon;
    int level;
};

void log_init(const char* name, int isdaemon, int verbose);
void log_level_set(int verbose);
void lprintf(int level, const char* format, ...);
#endif // _LOG_H_
