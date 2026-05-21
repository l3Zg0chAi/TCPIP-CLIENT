#ifndef LOGGER_H
#define LOGGER_H
#include <pthread.h>
#include <string>
#include <cstring>
inline const char* getCurrentThreadName() {
    thread_local char name[16] = {};
    pthread_getname_np(pthread_self(), name, sizeof(name));
    return name;
}

inline void setCurrentThreadName(const std::string& name){
    pthread_setname_np(pthread_self(), name.substr(0, 15).c_str());
}

#define ENABLE_DEBUG_LOG
#ifdef ENABLE_DEBUG_LOG
#include <cstdio>
#include <iostream>
#include <sstream>
#include <thread>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>

#ifndef gettid
#define gettid() syscall(SYS_gettid)
#endif

#define DLT_DEBUG_LOG
#ifdef DLT_DEBUG_LOG
#include <dlt/dlt.h>

extern DltContext main_dltCxt;
#define DEBUG_LOG(fmt, ...)                                                   \
    do {                                                                      \
        char logBuffer[1024];                                                 \
        std::snprintf(logBuffer, sizeof(logBuffer),                           \
            "[%d][%s][%s:%d][%s()] " fmt,                                     \
            static_cast<int>(gettid()),                                       \
            getCurrentThreadName(),                                           \
            __FILE__, __LINE__, __func__, ##__VA_ARGS__);                     \
                                                                              \
        std::fprintf(stderr, "%s\n", logBuffer);                              \
        DLT_LOG(main_dltCxt, DLT_LOG_INFO, DLT_CSTRING(logBuffer));          \
    } while (0)
    /* write the log to dlt level info, belongs to main_dltCxt context */
#else
#define DEBUG_LOG(fmt, ...) \
    do { \
        std::fprintf(stderr, "[%d][%s][%s:%d][%s()] " fmt "\n", \
            static_cast<int>(gettid()), \
            getCurrentThreadName(), \
            __FILE__, __LINE__, __func__, ##__VA_ARGS__); \
    } while (0)
#endif // DLT_DEBUG_LOG
#else
#define DEBUG_LOG(fmt, ...) do {} while (0)
#endif // ENABLE_DEBUG_LOG
#endif // LOGGER_H