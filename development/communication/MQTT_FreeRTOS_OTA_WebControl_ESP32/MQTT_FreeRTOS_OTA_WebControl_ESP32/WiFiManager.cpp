#include "WiFiManager.h"

bool WiFiManager::initialized = false;
bool WiFiManager::connected = false;

void WiFiManager::initialize() {
    if (initialized) return;
    
    Logger::log(Logger::INFO, "Initializing WiFi Manager...");
    WiFi.mode(WIFI_STA);
    
    // Configuration des événements WiFi
    WiFi.onEvent(onWiFiEvent);
    
    // Configuration supplémentaire pour améliorer la stabilité
    WiFi.setAutoReconnect(true);
    WiFi.persistent(true);
    
    initialized = true;
    Logger::log(Logger::INFO, "WiFi Manager initialized");
}

void WiFiManager::connect() {
    if (!initialized) {
        initialize();
    }
    
    Logger::log(Logger::INFO, "Connecting to WiFi: " + String(WIFI_SSID));
    
    // Déconnexion préalable pour un état propre
    WiFi.disconnect(true);
    delay(1000);
    
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    // Attendre la connexion avec timeout
    int attempts = 0;
    const int maxAttempts = 20; // 10 secondes maximum
    
    while (WiFi.status() != WL_CONNECTED && attempts < maxAttempts) {
        delay(500);
        Logger::log(Logger::INFO, ".");
        attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        connected = true;
        Logger::log(Logger::INFO, "\nWiFi connected successfully!");
        Logger::log(Logger::INFO, "IP: " + WiFi.localIP().toString());
        Logger::log(Logger::INFO, "MAC: " + WiFi.macAddress());
        Logger::log(Logger::INFO, "RSSI: " + String(WiFi.RSSI()) + " dBm");
    } else {
        Logger::log(Logger::ERROR, "Failed to connect to WiFi");
        // Réessayer après un délai
        delay(3000);
        connect(); // Récursion avec précaution
    }
}

void WiFiManager::onWiFiEvent(WiFiEvent_t event, WiFiEventInfo_t info) {
    switch (event) {
        case WIFI_EVENT_STA_START:
            Logger::log(Logger::INFO, "WiFi client started");
            break;
            
        case WIFI_EVENT_STA_CONNECTED:
            Logger::log(Logger::INFO, "Connected to WiFi network");
            break;
            
        case ARDUINO_EVENT_WIFI_STA_GOT_IP:
            connected = true;
            Logger::log(Logger::INFO, "WiFi connected with IP: " + WiFi.localIP().toString());
            break;
            
        case WIFI_EVENT_STA_DISCONNECTED:
            connected = false;
            Logger::log(Logger::WARNING, "WiFi connection lost");
            // Tentative de reconnexion automatique
            WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
            break;
            
        default:
            break;
    }
}

void WiFiManager::disconnect() {
    WiFi.disconnect(true);
    connected = false;
    Logger::log(Logger::INFO, "WiFi disconnected");
}

bool WiFiManager::isConnected() {
    return WiFi.status() == WL_CONNECTED;
}

int8_t WiFiManager::getSignalStrength() {
    return WiFi.RSSI();
}