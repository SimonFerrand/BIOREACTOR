// WatchdogManager.cpp
#include "WatchdogManager.h"

bool WatchdogManager::initialized = false;

bool WatchdogManager::initialize(uint32_t timeoutMs) {
    if (initialized) {
        return true;
    }

    esp_task_wdt_config_t twdt_config = {
        .timeout_ms = timeoutMs,
        .idle_core_mask = (1 << portNUM_PROCESSORS) - 1,  // Surveille tous les cores disponibles
        .trigger_panic = true
    };

    esp_err_t err = esp_task_wdt_init(&twdt_config);
    if (err != ESP_OK) {
        Logger::log(Logger::LogLevel::ERROR, "Failed to initialize TWDT: " + String(esp_err_to_name(err)));
        return false;
    }

    initialized = true;
    Logger::log(Logger::LogLevel::INFO, F("TWDT initialized successfully"));
    return true;
}

bool WatchdogManager::addTask() {
    if (!initialized) {
        Logger::log(Logger::LogLevel::ERROR, F("TWDT not initialized"));
        return false;
    }

    TaskHandle_t handle = xTaskGetCurrentTaskHandle();
    esp_err_t err = esp_task_wdt_add(handle);
    if (err != ESP_OK) {
        Logger::log(Logger::LogLevel::ERROR, "Failed to add task to TWDT: " + String(esp_err_to_name(err)));
        return false;
    }

    Logger::log(Logger::LogLevel::INFO, "Task added to TWDT: " + String(pcTaskGetName(handle)));
    return true;
}

bool WatchdogManager::resetTimer() {
    if (!initialized) {
        return false;
    }

    esp_err_t err = esp_task_wdt_reset();
    if (err != ESP_OK) {
        Logger::log(Logger::LogLevel::ERROR, "Failed to reset TWDT: " + String(esp_err_to_name(err)));
        return false;
    }
    return true;
}

bool WatchdogManager::deleteTask(TaskHandle_t task) {
    if (!initialized) {
        return false;
    }

    esp_err_t err = esp_task_wdt_delete(task);  // on passe directement le handle
    if (err != ESP_OK) {
        Logger::log(Logger::LogLevel::ERROR, "Failed to delete task from TWDT: " + String(esp_err_to_name(err)));
        return false;
    }
    
    Logger::log(Logger::LogLevel::INFO, "Task deleted from TWDT: " + String(pcTaskGetName(task)));
    return true;
}

void WatchdogManager::deinitialize() {
    if (initialized) {
        esp_task_wdt_deinit();
        initialized = false;
        Logger::log(Logger::LogLevel::INFO, F("TWDT deinitialized"));
    }
}