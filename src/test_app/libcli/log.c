#include <log.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct logpriv_s* logpriv;

static void log_reinit(void) {
    log_init(NULL, 0, 0);
}

void lprintf(int level, const char* format, ...) {
    static char logmsg[LOG_MSG_LENGTH];
    va_list vptr;

    if (!logpriv) {
        log_reinit();
    }

    if (logpriv->level < level) {
        return;
    }

    va_start(vptr, format);
    vsnprintf(logmsg, LOG_MSG_LENGTH, format, vptr);
    va_end(vptr);

    if (logpriv->daemon) {
        syslog(level, "%s", logmsg);
    } else {
        fprintf(stderr, "%s\n", logmsg);
    }
    return;
}

/*
 * open connection to syslog if daemon
 */
void log_init(const char* name, int is_daemon, int verbose) {
    if (logpriv) {
        return;
    }

    logpriv = malloc(sizeof(struct logpriv_s));
    if (!logpriv) {
        return;
    }

    if (name) {
        logpriv->name = strdup(name);
    } else {
        logpriv->name = strdup(LOG_NAME_DEFAULT);
    }

    if (!logpriv->name) {
        fprintf(stderr, "%s: malloc failure\n", LOG_NAME_DEFAULT);
    }

    logpriv->daemon = is_daemon;
    logpriv->level = verbose + LOG_NOTICE;

    if (logpriv->daemon) {
        openlog(logpriv->name, LOG_CONS, LOG_LOCAL4);
    }
}

void log_level_set(int verbose) {
    logpriv->level = verbose + LOG_NOTICE;
}
