// ===== MQTTClient.h =====
#ifndef MQTT_CLIENT_H
#define MQTT_CLIENT_H

#include "StateMachine.h"
#include <AsyncMqttClient.h>
#include <functional>
#include "Logger.h"
#include "TaskManager.h"
#include "DataManager.h"
#include "SystemMonitor.h"
#include "WiFiManager.h"
#include "config.h"


class MQTTClient {
public:
    MQTTClient();
    ~MQTTClient();

    bool begin();
    void disconnect();
    bool isConnected() const;

    void setStateMachine(StateMachine* machine) { stateMachine = machine; }
    
    // Publishing methods
    bool publishSensorData(const SensorData& data);
    bool publishStatus(const String& status);
    bool publishHeartbeat();
    void publishStateChange(const StateMachine& stateMachine);

    // Callback setters
    void onConnectionEstablished(std::function<void()> callback);
    void onConnectionLost(std::function<void()> callback);
    void onMessageReceived(std::function<void(const char* topic, const char* message)> callback);
    void onError(std::function<void(const char* error)> callback);

private: 
    struct MQTTMessage {
        char topic[128];
        char payload[1024];
    };

    // MQTT client instance
    AsyncMqttClient mqttClient;
    
    // FreeRTOS resources
    TaskHandle_t reconnectTaskHandle;
    TaskHandle_t heartbeatTaskHandle;
    QueueHandle_t messageQueue;

    // Composants système
    StateMachine* stateMachine;
    
    // State
    bool connected;
    int retryCount;

    // Callbacks
    std::function<void()> connectionEstablishedCallback;
    std::function<void()> connectionLostCallback;
    std::function<void(const char* topic, const char* message)> messageCallback;
    std::function<void(const char* error)> errorCallback;

    // Task handlers
    static void reconnectTask(void* parameter);
    static void heartbeatTask(void* parameter);
    static void messageHandlerTask(void* parameter);
    static void dataSenderTask(void* parameter);
    const TickType_t senderFrequency = pdMS_TO_TICKS(TASK_INTERVAL_DATASENDER);

    // MQTT handlers
    void setupMQTT();
    void handleConnect(bool sessionPresent);
    void handleDisconnect(AsyncMqttClientDisconnectReason reason);
    void handleMessage(char* topic, char* payload, 
                      AsyncMqttClientMessageProperties properties,
                      size_t len, size_t index, size_t total);
    




};

#endif // MQTT_CLIENT_H