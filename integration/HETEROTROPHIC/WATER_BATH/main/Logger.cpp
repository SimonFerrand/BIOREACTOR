// Logger.cpp
#include "Logger.h"

void Logger::log(LogLevel level, const String& message) {
    // Vérifie si le message doit être affiché selon DEBUG_LEVEL
    if ((level == LogLevel::ERROR && DEBUG_LEVEL >= 1) ||
        (level == LogLevel::INFO && DEBUG_LEVEL >= 2) ||
        (level == LogLevel::DEBUG && DEBUG_LEVEL >= 3)) {
        
        String levelStr = getLogLevelString(level);
        uint32_t timestamp = xTaskGetTickCount() * portTICK_PERIOD_MS;
        
        String formattedMessage;
        formatMessage(formattedMessage, level, message);
        
        // Format: [TIME][LEVEL] Message
        Serial.printf("[%lu][%s] %s\n", timestamp, levelStr.c_str(), formattedMessage.c_str());
    }
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