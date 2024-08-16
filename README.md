# Bioreactor Project

## Overview

This project aims to develop a comprehensive system for monitoring and controlling a bioreactor. The system is built using a combination of hardware and software components, including Arduino Mega, ESP32, Raspberry Pi, FastAPI, Vue.js, and various sensors and actuators. 

## Architecture

### Hardware Components

1. **Arduino Mega**: 
   - Controls the actuators and reads data from the sensors.
   - Communicates with the ESP32 via serial communication.

2. **ESP32**:
   - Acts as a bridge between the Arduino Mega and the Raspberry Pi.
   - Receives data from the Arduino Mega and sends it to the Raspberry Pi via HTTP POST requests.

3. **Raspberry Pi**:
   - Hosts the FastAPI backend and the Vue.js frontend.
   - Processes and stores the data received from the ESP32.
   - Provides a user interface for monitoring and controlling the bioreactor.

### Software Components

1. **FastAPI (Backend)**:
   - Handles HTTP requests from the ESP32.
   - Processes and stores sensor data in a CSV file.
   - Provides endpoints for the frontend to retrieve data.

2. **Vue.js (Frontend)**:
   - Provides a user interface for visualizing sensor data and controlling the bioreactor.
   - Communicates with the FastAPI backend to fetch data and send control commands.

### Sensors and Actuators

1. **Sensors**:
   - **PT100 Sensor**: Measures water temperature.
   - **DS18B20 Sensor**: Measures air temperature.
   - **PHSensor**: Measures pH levels.
   - **TurbiditySensor**: Measures turbidity.
   - **OxygenSensor**: Measures dissolved oxygen levels.
   - **AirFlowSensor**: Measures airflow.

2. **Actuators**:
   - **DCPump**: Controls air and drain pumps.
   - **PeristalticPump**: Controls nutrient and base pumps.
   - **StirringMotor**: Controls the stirring motor.
   - **HeatingPlate**: Controls the heating plate.
   - **LEDGrowLight**: Controls the LED grow light.

### Communication Flow

1. **Data Collection**:
   - The Arduino Mega reads data from the sensors and controls the actuators.
   - The ESP32 receives this data from the Arduino Mega and sends it to the Raspberry Pi via HTTP POST requests.

2. **Data Processing**:
   - The FastAPI backend on the Raspberry Pi receives the data, processes it, and stores it in a CSV file.
   - The backend also provides endpoints for the frontend to fetch this data.

3. **User Interface**:
   #- The Vue.js frontend retrieves data from the FastAPI backend and displays it to the user.
   #- Users can also send control commands to the backend via the frontend to adjust the bioreactor's operation.

## Installation

### Prerequisites

- Arduino IDE
- Python 3.x
- Node.js and npm
- FastAPI
- Vue CLI

### Setup

1. **Arduino Mega**:
   - Upload the Arduino code to the Arduino Mega using the Arduino IDE.

2. **ESP32**:
   - Upload the ESP32 code to the ESP32 using the Arduino IDE.

3. **Raspberry Pi**:
   - Set up the Python virtual environment and install FastAPI:
     ```bash
     python3 -m venv env_bioreactor
     source env_bioreactor/bin/activate
     pip install fastapi uvicorn
     ```

   - Set up the Vue.js frontend:
     ```bash
     cd /Raspberry/Bioreactor/ServerFastAPI/frontend
     npm install
     ```

   - Create systemd services for FastAPI and Vue.js to run them at startup.

## Usage

1. **Start the backend**:
   ```bash
   sudo systemctl start fastapi.service
   ```

2. **Start the frontend**:

   ```bash
   sudo systemctl start vuejs.service
   ```

3. **Access the User Interface**:

Open a web browser and navigate to "http://192.168.1.25:8080/" to access the frontend.

## Troubleshooting
Ensure all connections between the Arduino Mega, ESP32, and Raspberry Pi are secure.
Check the systemd service logs if the backend or frontend does not start:
   ```bash
   sudo journalctl -u fastapi.service
   sudo journalctl -u vuejs.service
   ```
