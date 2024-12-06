// ===== SystemMonitor.h =====
#ifndef SYSTEM_MONITOR_H
#define SYSTEM_MONITOR_H

#include "Logger.h"
#include "WiFiManager.h"
#include <esp_system.h>
#include "TaskManager.h"

class SystemMonitor {
public:
    static void initialize();
    static void checkHeap();
    static void checkWiFiStrength();
    static void logSystemStats();
    static void startMonitoring();
    static void stopMonitoring();

private:
    static void monitorTask(void* parameter);
    static TaskHandle_t monitorTaskHandle;
    static uint32_t lastHeapSize;
    static int8_t lastRSSI;
};

#endif