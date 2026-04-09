#include "../include/common.h"
#include "../include/logger.h"

void log_action(const char *username, const char *action, const char *detail) {
    FILE *fp = fopen(LOG_FILE, "a");
    if (fp == NULL) {
        return;
    }

    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);

    char time_buf[64];
    strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", tm_info);

    fprintf(fp, "[%s] USER=%s ACTION=%s DETAIL=%s\n",
            time_buf,
            username ? username : "UNKNOWN",
            action ? action : "NONE",
            detail ? detail : "NONE");

    fclose(fp);
}