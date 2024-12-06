// ===== MQTTClient.h =====
#ifndef MQTT_CLIENT_H
#define MQTT_CLIENT_H

#include <AsyncMqttClient.h>
#include <functional>
#include "Logger.h"
#include "MessageFormatter.h"
#include "TaskManager.h"
#include "SystemMonitor.h"
#include "WiFiManager.h"
#include "config.h"

class MQTTClient {
public:
    MQTTClient();
    ~MQTTClient();

    void begin();
    void disconnect();
    bool isConnected() const;
    
    // Publishing methods
    bool publishSensorData(const SensorData& data);
    bool publishStatus(const String& status);
    bool publishHeartbeat();

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
    
    // State
    bool connected;
    int retryCount;

    // Task handlers
    static void reconnectTask(void* parameter);
    static void heartbeatTask(void* parameter);
    static void messageHandlerTask(void* parameter);

    // MQTT handlers
    void setupMQTT();
    void handleConnect(bool sessionPresent);
    void handleDisconnect(AsyncMqttClientDisconnectReason reason);
    void handleMessage(char* topic, char* payload, 
                      AsyncMqttClientMessageProperties properties,
                      size_t len, size_t index, size_t total);

    // Callbacks
    std::function<void()> connectionEstablishedCallback;
    std::function<void()> connectionLostCallback;
    std::function<void(const char* topic, const char* message)> messageCallback;
    std::function<void(const char* error)> errorCallback;
};

#endif // MQTT_CLIENT_H