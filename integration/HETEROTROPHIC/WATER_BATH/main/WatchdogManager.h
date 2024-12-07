// WatchdogManager.h
#ifndef WATCHDOG_MANAGER_H
#define WATCHDOG_MANAGER_H

#include <esp_task_wdt.h>
#include <esp_system.h>
#include "Logger.h"

class WatchdogManager {
public:
    static bool initialize(uint32_t timeoutMs = 3000);  // 3 secondes par défaut
    static bool addTask();
    static bool resetTimer();
    static bool deleteTask(TaskHandle_t task); 
    static void deinitialize();
    static bool isInitialized() { return initialized; }

private:
    static bool initialized;
};

#endif // WATCHDOG_MANAGER_H