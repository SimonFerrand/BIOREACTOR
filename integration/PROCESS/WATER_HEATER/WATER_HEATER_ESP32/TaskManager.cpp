// ===== TaskManager.cpp =====
#include "TaskManager.h"
#include "TaskManager.h"
#include "Logger.h"
#include <Arduino.h>

TaskHandle_t TaskManager::createTask(TaskFunction_t taskFunction,
                                   const char* taskName,
                                   uint32_t stackSize,
                                   void* parameter,
                                   UBaseType_t priority,
                                   BaseType_t coreID) {
    TaskHandle_t taskHandle = nullptr;
    BaseType_t result = xTaskCreatePinnedToCore(
        taskFunction,
        taskName,
        stackSize,
        parameter,
        priority,
        &taskHandle,
        coreID
    );

    if (result != pdPASS) {
        Logger::log(Logger::LogLevel::ERROR, String("Failed to create task: ") + taskName);
        return nullptr;
    }

    Logger::log(Logger::LogLevel::INFO, String("Task created: ") + taskName);
    return taskHandle;
}

QueueHandle_t TaskManager::createQueue(size_t queueLength, size_t itemSize) {
    QueueHandle_t queue = xQueueCreate(queueLength, itemSize);
    
    if (queue == nullptr) {
        Logger::log(Logger::LogLevel::ERROR, "Failed to create queue");
    } else {
        Logger::log(Logger::LogLevel::DEBUG, "Queue created successfully");
    }
    
    return queue;
}

void TaskManager::deleteTask(TaskHandle_t& taskHandle) {
    if (taskHandle != nullptr) {
        vTaskDelete(taskHandle);
        taskHandle = nullptr;
        Logger::log(Logger::LogLevel::DEBUG, "Task deleted");
    }
}

void TaskManager::deleteQueue(QueueHandle_t& queueHandle) {
    if (queueHandle != nullptr) {
        vQueueDelete(queueHandle);
        queueHandle = nullptr;
        Logger::log(Logger::LogLevel::DEBUG, "Queue deleted");
    }
}

void TaskManager::suspendTask(TaskHandle_t taskHandle) {
    if (taskHandle != nullptr) {
        vTaskSuspend(taskHandle);
        Logger::log(Logger::LogLevel::DEBUG, "Task suspended");
    }
}

void TaskManager::resumeTask(TaskHandle_t taskHandle) {
    if (taskHandle != nullptr) {
        vTaskResume(taskHandle);
        Logger::log(Logger::LogLevel::DEBUG, "Task resumed");
    }
}

void TaskManager::checkStackUsage(TaskHandle_t taskHandle) {
    if (taskHandle != nullptr) {
        UBaseType_t stackHighWaterMark = uxTaskGetStackHighWaterMark(taskHandle);
        if (stackHighWaterMark < 512) {  // Seuil d'avertissement
            Logger::logWithHeap(Logger::LogLevel::WARNING, 
                String("Low stack space for task: ") + pcTaskGetName(taskHandle) +
                " (remaining: " + String(stackHighWaterMark) + " bytes)");
        }
    }
}