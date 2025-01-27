// ===== MQTTClient.cpp =====
#include "MQTTClient.h"

MQTTClient::MQTTClient()
    : reconnectTaskHandle(nullptr)
    , heartbeatTaskHandle(nullptr)
    , messageQueue(nullptr)
    , stateMachine(nullptr)
    , connected(false)
    , retryCount(0)
    , connectionEstablishedCallback(nullptr)
    , connectionLostCallback(nullptr)
    , messageCallback(nullptr)
    , errorCallback(nullptr)
{
}

MQTTClient::~MQTTClient() {
    if (reconnectTaskHandle) TaskManager::deleteTask(reconnectTaskHandle);
    if (heartbeatTaskHandle) TaskManager::deleteTask(heartbeatTaskHandle);
    if (messageQueue) TaskManager::deleteQueue(messageQueue);
    disconnect();
}


bool MQTTClient::begin() {
    Logger::log(Logger::LogLevel::INFO, F("Initializing MQTT Client"));
    
    // S'assurer que le WiFi est connecté
    if (!WiFiManager::isConnected()) {
        WiFiManager::connect();
    }
    
    // Créer la queue de messages
    messageQueue = TaskManager::createQueue(MQTT_QUEUE_SIZE, sizeof(MQTTMessage));
    
    // Configuration MQTT
    setupMQTT();
    
    // Créer les tâches
    reconnectTaskHandle = TaskManager::createTask(
        reconnectTask,
        "MQTTReconnect",
        STACK_SIZE_MQTT,
        this,
        TASK_PRIORITY_MEDIUM,
        MQTT_CORE
    );
    
    heartbeatTaskHandle = TaskManager::createTask(
        heartbeatTask,
        "MQTTHeartbeat",
        STACK_SIZE_MQTT,
        this,
        TASK_PRIORITY_LOW,
        MQTT_CORE
    );
    
    TaskManager::createTask(
        messageHandlerTask,
        "MQTTMsgHandler",
        STACK_SIZE_MQTT,
        this,
        TASK_PRIORITY_HIGH,
        MQTT_CORE
    );

    TaskManager::createTask(
        dataSenderTask,
        "DataSender",
        STACK_SIZE_SENSORS,
        this,
        TASK_PRIORITY_LOW,
        SENSOR_CORE
    );

  return true;

}

void MQTTClient::dataSenderTask(void* parameter) {
    MQTTClient* client = static_cast<MQTTClient*>(parameter);
    TickType_t xLastWakeTime = xTaskGetTickCount();
    
    while (true) {
        if (client->isConnected()) {
            SensorData data = DataManager::collectSensorData();
            String jsonData = DataManager::collectAllData(*client->stateMachine);
            Logger::log(Logger::LogLevel::INFO, "Data collected: " + jsonData);
            
            if (client->publishSensorData(data)) {
                Logger::log(Logger::LogLevel::INFO, "Data sent successfully");
            } else {
                Logger::log(Logger::LogLevel::ERROR, "Failed to send data");
            }
        }
        vTaskDelayUntil(&xLastWakeTime, client->senderFrequency);
    }
}

void MQTTClient::disconnect() {
    if (isConnected()) {
        publishStatus("offline");
        mqttClient.disconnect();
    }
    connected = false;
}

void MQTTClient::setupMQTT() {
    mqttClient.onConnect([this](bool sessionPresent) { 
        handleConnect(sessionPresent); 
    });
    
    mqttClient.onDisconnect([this](AsyncMqttClientDisconnectReason reason) { 
        handleDisconnect(reason); 
    });
    
    mqttClient.onMessage([this](char* topic, char* payload,
                               AsyncMqttClientMessageProperties properties,
                               size_t len, size_t index, size_t total) {
        handleMessage(topic, payload, properties, len, index, total);
    });

    mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
    mqttClient.setClientId(MQTT_CLIENT_ID);
    if (strlen(MQTT_USERNAME) > 0) {
        mqttClient.setCredentials(MQTT_USERNAME, MQTT_PASSWORD);
    }
    mqttClient.setKeepAlive(60);
}

void MQTTClient::reconnectTask(void* parameter) {
    MQTTClient* client = static_cast<MQTTClient*>(parameter);
    const TickType_t xDelay = pdMS_TO_TICKS(MQTT_RECONNECT_INTERVAL);

    while (true) {
        if (!client->isConnected() && client->retryCount < MQTT_MAX_RETRIES) {
            Logger::log(Logger::LogLevel::INFO, F("Attempting MQTT reconnection..."));
            client->mqttClient.connect();
            client->retryCount++;
        }
        vTaskDelay(xDelay);
    }
}

void MQTTClient::heartbeatTask(void* parameter) {
    MQTTClient* client = static_cast<MQTTClient*>(parameter);
    const TickType_t xDelay = pdMS_TO_TICKS(MQTT_HEARTBEAT_INTERVAL);

    while (true) {
        if (client->isConnected()) {
            client->publishHeartbeat();
        }
        vTaskDelay(xDelay);
    }
}

void MQTTClient::messageHandlerTask(void* parameter) {
    MQTTClient* client = static_cast<MQTTClient*>(parameter);
    MQTTMessage message;

    while (true) {
        if (xQueueReceive(client->messageQueue, &message, portMAX_DELAY) == pdTRUE) {
            if (client->messageCallback) {
                client->messageCallback(message.topic, message.payload);
            }
        }
    }
}

void MQTTClient::handleConnect(bool sessionPresent) {
    Logger::log(Logger::LogLevel::INFO, F("Connected to MQTT broker"));
    connected = true;
    retryCount = 0;

    // Souscrire aux topics
    mqttClient.subscribe(MQTT_TOPIC_STATUS, 1);
    mqttClient.subscribe(MQTT_TOPIC_SENSORS, 1);
    mqttClient.subscribe(MQTT_TOPIC_COMMANDS, 1);

    // Publier le status initial
    publishStatus("online");

    if (connectionEstablishedCallback) {
        connectionEstablishedCallback();
    }
}

void MQTTClient::handleDisconnect(AsyncMqttClientDisconnectReason reason) {
    Logger::log(Logger::LogLevel::ERROR, F("Disconnected from MQTT broker"));
    connected = false;

    if (connectionLostCallback) {
        connectionLostCallback();
    }
}

void MQTTClient::handleMessage(char* topic, char* payload,
                             AsyncMqttClientMessageProperties properties,
                             size_t len, size_t index, size_t total) {
    static unsigned long lastMsgTime = 0;
    if (millis() - lastMsgTime < 100) return;
    lastMsgTime = millis();

    if (len >= sizeof(MQTTMessage::payload)) {
        Logger::log(Logger::LogLevel::ERROR, F("Message too large"));
        return;
    }

    MQTTMessage message;
    strlcpy(message.topic, topic, sizeof(message.topic));
    strlcpy(message.payload, payload, sizeof(message.payload));

    if (xQueueSend(messageQueue, &message, 0) != pdTRUE) {
        Logger::log(Logger::LogLevel::ERROR, F("Message queue full"));
    }
}

bool MQTTClient::publishSensorData(const SensorData& data) {
    if (!isConnected() || !stateMachine) return false;

    String payload = DataManager::collectAllData(*stateMachine);
    
    if (payload.length() >= sizeof(MQTTMessage::payload)) {     //if (payload.length() >= JSON_BUFFER_SIZE) { 
        Logger::log(Logger::LogLevel::ERROR, "Generated message too large: " + String(payload.length()) + " bytes");
        return false;
    }
    
    return mqttClient.publish(MQTT_TOPIC_SENSORS, 0, true, payload.c_str()) != 0;
}

bool MQTTClient::publishStatus(const String& status) {
    if (!isConnected()) return false;

    JsonDocument doc;
    doc["status"] = status;
    doc["timestamp"] = millis();
    String payload;
    serializeJson(doc, payload);
    
    return mqttClient.publish(MQTT_TOPIC_STATUS, 0, true, payload.c_str()) != 0;
}

bool MQTTClient::publishHeartbeat() {
    if (!isConnected()) return false;

    String payload = DataManager::createHeartbeatMessage();
    
    return mqttClient.publish(MQTT_TOPIC_STATUS, 0, false, payload.c_str()) != 0;
}

// Getters & Setters
bool MQTTClient::isConnected() const {
    return connected && WiFiManager::isConnected();
}

// Callback setters
void MQTTClient::onConnectionEstablished(std::function<void()> callback) {
    connectionEstablishedCallback = callback;
}

void MQTTClient::onConnectionLost(std::function<void()> callback) {
    connectionLostCallback = callback;
}

void MQTTClient::onMessageReceived(std::function<void(const char* topic, const char* message)> callback) {
    messageCallback = callback;
}

void MQTTClient::onError(std::function<void(const char* error)> callback) {
    errorCallback = callback;
}

void MQTTClient::publishStateChange(const StateMachine& stateMachine) {
   if (!isConnected()) return;
   String jsonData = DataManager::collectAllData(stateMachine);
   mqttClient.publish(MQTT_TOPIC_SENSORS, 0, true, jsonData.c_str());  // Utiliser mqttClient au lieu de publish
}