#ifndef LOGGER_H
#define LOGGER_H

#include <Arduino.h>
#include "config.h"

class Logger {
public:
    enum LogLevel {
        DEBUG,
        INFO,
        WARNING,
        ERROR
    };

    static void log(LogLevel level, const String& message);
    static void logWithHeap(LogLevel level, const String& message);
    static const char* getLogLevelString(LogLevel level);
    static String getHeapInfo();
    static void formatMessage(String& output, LogLevel level, const String& message);
};

#endif // LOGGER_H