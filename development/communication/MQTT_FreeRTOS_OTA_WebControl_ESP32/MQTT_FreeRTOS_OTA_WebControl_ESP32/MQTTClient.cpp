// ===== MQTTClient.cpp =====
#include "MQTTClient.h"

MQTTClient::MQTTClient() :
    reconnectTaskHandle(nullptr),
    heartbeatTaskHandle(nullptr),
    messageQueue(nullptr),
    connected(false),
    retryCount(0),
    connectionEstablishedCallback(nullptr),
    connectionLostCallback(nullptr),
    messageCallback(nullptr),
    errorCallback(nullptr) {
}

MQTTClient::~MQTTClient() {
    if (reconnectTaskHandle) TaskManager::deleteTask(reconnectTaskHandle);
    if (heartbeatTaskHandle) TaskManager::deleteTask(heartbeatTaskHandle);
    if (messageQueue) TaskManager::deleteQueue(messageQueue);
    disconnect();
}

void MQTTClient::begin() {
    Logger::log(Logger::INFO, "Initializing MQTT Client");
    
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
            Logger::log(Logger::INFO, "Attempting MQTT reconnection...");
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
    Logger::log(Logger::INFO, "Connected to MQTT broker");
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
    Logger::log(Logger::ERROR, "Disconnected from MQTT broker");
    connected = false;

    if (connectionLostCallback) {
        connectionLostCallback();
    }
}

void MQTTClient::handleMessage(char* topic, char* payload,
                             AsyncMqttClientMessageProperties properties,
                             size_t len, size_t index, size_t total) {
    if (len >= sizeof(MQTTMessage::payload)) {
        Logger::log(Logger::ERROR, "Message too large");
        return;
    }

    MQTTMessage message;
    strlcpy(message.topic, topic, sizeof(message.topic));
    strlcpy(message.payload, payload, sizeof(message.payload));

    if (xQueueSend(messageQueue, &message, 0) != pdTRUE) {
        Logger::log(Logger::ERROR, "Message queue full");
    }
}

bool MQTTClient::publishSensorData(const SensorData& data) {
    if (!isConnected()) return false;

    JsonDocument doc = MessageFormatter::createSensorMessage(data);
    String payload;
    serializeJson(doc, payload);
    
    if (payload.length() >= sizeof(MQTTMessage::payload)) {
        Logger::log(Logger::ERROR, "Generated message too large: " + String(payload.length()) + " bytes");
        return false;
    }
    
    return mqttClient.publish(MQTT_TOPIC_SENSORS, 0, true, payload.c_str()) != 0;
}

bool MQTTClient::publishStatus(const String& status) {
    if (!isConnected()) return false;

    JsonDocument doc = MessageFormatter::createStatusMessage(status);
    String payload;
    serializeJson(doc, payload);
    
    return mqttClient.publish(MQTT_TOPIC_STATUS, 0, true, payload.c_str()) != 0;
}

bool MQTTClient::publishHeartbeat() {
    if (!isConnected()) return false;

    JsonDocument doc = MessageFormatter::createHeartbeatMessage();
    String payload;
    serializeJson(doc, payload);
    
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