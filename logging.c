#include <stdio.h>
#include <stdarg.h>

#define LOG(fmt, ...)                                                   \
    fprintf(stderr, "[%s:%d] " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)

LOG("user=%s", username);

void l() {
  int x = 1;
  char *user = "charles";
fprintf(stderr, "login failed for user %s\n", user);
printf("DEBUG: x=%d\n", x);
}


enum {
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR
};


void log_msg(int level, const char *fmt, ...)
{
    static const char *names[] = {
        "DEBUG",
        "INFO",
        "WARN",
        "ERROR"
    };

    fprintf(stderr, "[%s] ", names[level]);

    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);

    fprintf(stderr, "\n");
}

/* usage */
void messagetr() {
    char *user = "jose";
log_msg(LOG_INFO, "user %s logged in", user);
log_msg(LOG_ERROR, "db open failed");
}



#define LOG_INFO(fmt, ...) \
    fprintf(stderr, "[INFO] " fmt "\n", ##__VA_ARGS__)

#define LOG_WARN(fmt, ...) \
    fprintf(stderr, "[WARN] " fmt "\n", ##__VA_ARGS__)

#define LOG_ERROR(fmt, ...) \
    fprintf(stderr, "[ERROR] " fmt "\n", ##__VA_ARGS__)

LOG_INFO("conn=%llu login success", conn_id);
LOG_WARN("conn=%llu bad password", conn_id);
LOG_ERROR("sqlite open failed");


// can add hex_dump pointer and len you can get from salvatore
// he has hex dump logger in videos
