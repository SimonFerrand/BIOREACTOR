// ===== TaskManager.h =====
#ifndef TASK_MANAGER_H
#define TASK_MANAGER_H

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include "Logger.h"
#include <Arduino.h>

class Logger;

class TaskManager {
public:
    static TaskHandle_t createTask(TaskFunction_t taskFunction,
                                 const char* taskName,
                                 uint32_t stackSize,
                                 void* parameter,
                                 UBaseType_t priority,
                                 BaseType_t coreID);
                                 
    static QueueHandle_t createQueue(size_t queueLength, size_t itemSize);
    static void deleteTask(TaskHandle_t& taskHandle);
    static void deleteQueue(QueueHandle_t& queueHandle);
    
    static void suspendTask(TaskHandle_t taskHandle);
    static void resumeTask(TaskHandle_t taskHandle);
    
private:
    static void checkStackUsage(TaskHandle_t taskHandle);
};

#endif