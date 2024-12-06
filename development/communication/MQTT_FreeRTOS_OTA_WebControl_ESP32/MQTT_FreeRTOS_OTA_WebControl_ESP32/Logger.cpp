// Logger.cpp
#include "Logger.h"

void Logger::log(LogLevel level, const String& message) {
    String levelStr = getLogLevelString(level);
    uint32_t timestamp = xTaskGetTickCount() * portTICK_PERIOD_MS;
    
    String formattedMessage;
    formatMessage(formattedMessage, level, message);
    
    // Format: [TIME][LEVEL] Message
    Serial.printf("[%lu][%s] %s\n", timestamp, levelStr, formattedMessage.c_str());
}

void Logger::logWithHeap(LogLevel level, const String& message) {
    String heapInfo = getHeapInfo();
    log(level, message + " [" + heapInfo + "]");
}

const char* Logger::getLogLevelString(LogLevel level) {
    switch (level) {
        case DEBUG:   return "DEBUG";
        case INFO:    return "INFO";
        case WARNING: return "WARN";
        case ERROR:   return "ERROR";
        default:      return "UNKNOWN";
    }
}

String Logger::getHeapInfo() {
    return "Heap: " + String(ESP.getFreeHeap()) + " bytes";
}

void Logger::formatMessage(String& output, LogLevel level, const String& message) {
    if (level == ERROR) {
        output = "!!! " + message + " !!!";
    } else {
        output = message;
    }
}