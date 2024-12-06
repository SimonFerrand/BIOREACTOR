// ===== SystemMonitor.cpp =====
#include "SystemMonitor.h"

uint32_t SystemMonitor::lastHeapSize = 0;
int8_t SystemMonitor::lastRSSI = 0;
TaskHandle_t SystemMonitor::monitorTaskHandle = nullptr;

void SystemMonitor::initialize() {
    lastHeapSize = ESP.getFreeHeap();
    lastRSSI = WiFiManager::getSignalStrength();
    Logger::log(Logger::INFO, "System Monitor initialized");
}

void SystemMonitor::checkHeap() {
    uint32_t currentHeap = ESP.getFreeHeap();
    
    // Alerte si moins de 20% de mémoire libre par rapport à la dernière vérification
    if (currentHeap < lastHeapSize * 0.8) {
        Logger::log(Logger::WARNING, 
            "Low memory: " + String(currentHeap) + " bytes (was " + 
            String(lastHeapSize) + " bytes)");
    }
    
    // Alerte critique si moins de 10% de la mémoire totale
    if (currentHeap < ESP.getHeapSize() * 0.1) {
        Logger::log(Logger::ERROR, 
            "Critical memory level: " + String(currentHeap) + " bytes");
    }
    
    lastHeapSize = currentHeap;
}

void SystemMonitor::checkWiFiStrength() {
    int8_t rssi = WiFiManager::getSignalStrength();
    
    if (rssi < -80) {
        Logger::log(Logger::WARNING, 
            "Poor WiFi signal strength: " + String(rssi) + " dBm");
    }
    
    // Si le signal s'est dégradé de plus de 10dB
    if (rssi < lastRSSI - 10) {
        Logger::log(Logger::WARNING, 
            "WiFi signal degraded: " + String(rssi) + " dBm");
    }
    
    lastRSSI = rssi;
}

void SystemMonitor::logSystemStats() {
    uint32_t totalHeap = ESP.getHeapSize();
    uint32_t freeHeap = ESP.getFreeHeap();
    uint8_t cpuFreq = ESP.getCpuFreqMHz();
    
    String stats = "System Stats:\n";
    stats += "- CPU Frequency: " + String(cpuFreq) + " MHz\n";
    stats += "- Free Heap: " + String(freeHeap) + "/" + String(totalHeap) + " bytes\n";
    stats += "- WiFi Signal: " + String(WiFiManager::getSignalStrength()) + " dBm\n";
    stats += "- Uptime: " + String(millis() / 1000) + " seconds";
    
    Logger::log(Logger::INFO, stats);
}

void SystemMonitor::startMonitoring() {
    if (monitorTaskHandle != nullptr) {
        Logger::log(Logger::WARNING, "Monitoring already started");
        return;
    }

    monitorTaskHandle = TaskManager::createTask(
        monitorTask,
        "SysMonitor",
        STACK_SIZE_MONITOR,
        nullptr,
        TASK_PRIORITY_LOW,
        MQTT_CORE
    );
}

void SystemMonitor::stopMonitoring() {
    TaskManager::deleteTask(monitorTaskHandle);
}

void SystemMonitor::monitorTask(void* parameter) {
    const TickType_t xFrequency = pdMS_TO_TICKS(MONITOR_CHECK_INTERVAL);
    
    while (true) {
        checkHeap();
        checkWiFiStrength();
        logSystemStats();
        vTaskDelay(xFrequency);
    }
}