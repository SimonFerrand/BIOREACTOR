from fastapi import FastAPI, HTTPException, WebSocket, WebSocketDisconnect, status
from fastapi.middleware.cors import CORSMiddleware
from fastapi.responses import JSONResponse
from pydantic import BaseModel, Field, ValidationError
import csv
from datetime import datetime
import os
import pandas as pd
from typing import List
import json
import asyncio
import logging
from logging.handlers import RotatingFileHandler
import paho.mqtt.client as mqtt
from threading import Thread

def flatten_dict(d, parent_key='', sep='_'):
    items = []
    for k, v in d.items():
        new_key = f"{parent_key}{sep}{k}" if parent_key else k
        if isinstance(v, dict):
            items.extend(flatten_dict(v, new_key, sep=sep).items())
        else:
            items.append((new_key, v))
    return dict(items)

class BioreactorData(BaseModel):
    arduino_value: dict
    timestamp: str

# Configuration du logging
log_formatter = logging.Formatter('%(asctime)s - %(name)s - %(levelname)s - %(message)s')
log_file = '/Raspberry/Bioreactor/ServerFastAPI/backend.log'
log_handler = RotatingFileHandler(log_file, maxBytes=1024 * 1024, backupCount=5)
log_handler.setFormatter(log_formatter)

logger = logging.getLogger(__name__)
logger.setLevel(logging.DEBUG)
logger.addHandler(log_handler)

# Ajoutez également un handler pour afficher les logs dans la console
console_handler = logging.StreamHandler()
console_handler.setFormatter(log_formatter)
logger.addHandler(console_handler)

# MQTT Setup
mqtt_client = mqtt.Client()
mqtt_connected = False

def on_mqtt_connect(client, userdata, flags, rc):
    global mqtt_connected
    mqtt_connected = True
    logger.info("Connected to MQTT broker")
    client.subscribe("bioreactor/status")
    client.subscribe("bioreactor/sensors")

def on_mqtt_message(client, userdata, msg):
    try:
        payload = json.loads(msg.payload.decode())
        logger.debug(f"MQTT message received on {msg.topic}: {payload}")
    except Exception as e:
        logger.error(f"Error processing MQTT message: {e}")

mqtt_client.on_connect = on_mqtt_connect
mqtt_client.on_message = on_mqtt_message

# Start MQTT in a separate thread
def start_mqtt():
    try:
        mqtt_client.connect("localhost", 1883, 60)
        mqtt_client.loop_start()
    except Exception as e:
        logger.error(f"Failed to connect to MQTT broker: {e}")

Thread(target=start_mqtt, daemon=True).start()

app = FastAPI()

# Define the directory and file path
data_dir = "/Raspberry/Bioreactor/ServerFastAPI/data"
filename = os.path.join(data_dir, "data.csv")

# Ensure the data directory exists
os.makedirs(data_dir, exist_ok=True)

# Add the CORS middleware to allow requests from the frontend
app.add_middleware(
    CORSMiddleware,
    allow_origins=["http://192.168.1.25:8080", "http://localhost:8080"],
    allow_credentials=True,
    allow_methods=["GET", "POST", "PUT", "DELETE"],
    allow_headers=["*"],
)

def ensure_csv_header():
    """Ensure that the CSV file has the correct header"""
    file_exists = os.path.isfile(filename)
    if not file_exists:
        with open(filename, 'w', newline='') as file:
            writer = csv.writer(file)
            writer.writerow([
                "Backend_Time", "ESP_Time", "Event_Type",
                "currentProgram", "programState",
                # sensorData
                "waterTemp", "airTemp", "elecTemp", "pH",
                "turbidity", "oxygen", "airFlow",
                # actuatorData 
                "airPump", "drainPump", "samplePump",
                "nutrientPump", "basePump", "fillPump",  
                "stirringMotor", "heatingPlate", "ledGrowLight",
                # actuatorSetpoints 
                "airPumpSetpoint", "drainPumpSetpoint", "samplePumpSetpoint",
                "nutrientPumpSetpoint", "basePumpSetpoint", "fillPumpSetpoint",
                "stirringMotorSetpoint", "heatingPlateSetpoint", "ledGrowLightSetpoint",
                # volumeData
                "currentVolume", "availableVolume", "addedNaOH",
                "addedNutrient", "addedMicroalgae", "removedVolume",
                # program data
                "program",
                "tempSetpoint",  # temperature
                "pHSetpoint",    # pH
                "DOSetpoint",    # dissolvedOxygen
                "nutrientConc",  # nutrientConcentration
                "baseConc",      # baseConcentration
                "duration",      # duration
                "nutrientDelay", # delay
                "experimentName",
                "comment"
            ])

class MixCommand(BaseModel):
    speed: int

class DrainCommand(BaseModel):
    rate: int
    duration: int

class FermentationCommand(BaseModel):
    temperature: float
    pH: float
    dissolvedOxygen: float
    nutrientConcentration: float
    baseConcentration: float
    duration: int
    nutrientDelay: float 
    experimentName: str
    comment: str

@app.post("/execute/mix")
async def execute_mix(command: MixCommand):
    try:
        logger.info(f"MQTT Connected: {mqtt_client.is_connected()}")
        if not mqtt_client.is_connected():
            mqtt_client.reconnect()
        
        message = {"program": "mix", "speed": command.speed}
        mqtt_client.publish("bioreactor/commands", json.dumps(message))
        return {"message": f"Mix program started with speed {command.speed}"}
    except Exception as e:
        logger.error(f"MQTT Error: {e}")
        raise HTTPException(status_code=500, detail=str(e))

@app.post("/execute/drain")
async def execute_drain(command: DrainCommand):
    logger.info(f"Received drain command: rate={command.rate}, duration={command.duration}")
    
    message = {"program": "drain", "rate": command.rate, "duration": command.duration}
    mqtt_client.publish("bioreactor/commands", json.dumps(message))
    return {"message": f"Drain program started with rate {command.rate} and duration {command.duration}"}

@app.post("/execute/fermentation")
async def execute_fermentation(command: FermentationCommand):
    logger.info(f"Received fermentation command: {command}")
    
    message = {
        "program": "fermentation",
        "temperature": command.temperature,
        "pH": command.pH,
        "dissolvedOxygen": command.dissolvedOxygen,
        "nutrientConcentration": command.nutrientConcentration,
        "baseConcentration": command.baseConcentration,
        "duration": command.duration,
        "nutrientDelay": command.nutrientDelay,
        "experimentName": command.experimentName,
        "comment": command.comment
    }
    mqtt_client.publish("bioreactor/commands", json.dumps(message))
    return {"message": "Fermentation program started"}

@app.post("/execute/stop")
async def execute_stop():
    logger.info("Received stop command")
    
    message = {"program": "stop"}
    mqtt_client.publish("bioreactor/commands", json.dumps(message))
    return {"message": "All programs stopped"}

@app.post("/sensor_data")
async def receive_data(data: BioreactorData):
    logger.info("Received bioreactor data")
    logger.debug(f"Raw received data: {data.dict()}")
    ensure_csv_header()

    backend_time = datetime.now().strftime("%Y-%m-%d %H:%M:%S")

    try:
        bioreactor_data = data.arduino_value
        logger.debug(f"Parsed arduino_value: {bioreactor_data}")

        if not isinstance(bioreactor_data, dict):
            raise ValueError("arduino_value must be a dictionary")
        
        # Determine event type and process program parameters
        event_type = "unknown"
        program_parameters = {}
        
        if "sensorData" in bioreactor_data:
            event_type = "periodic"
        elif "program" in bioreactor_data:
            event_type = "program_event"
            if bioreactor_data.get("program") == "Fermentation":
                # Map fermentation parameters correctly
                program_parameters = {
                    "program": "Fermentation",
                    "tempSetpoint": bioreactor_data.get("temperatureSetpoint", ""),
                    "pHSetpoint": bioreactor_data.get("phSetpoint", ""),
                    "DOSetpoint": bioreactor_data.get("O2Setpoint", ""),
                    "nutrientConc": bioreactor_data.get("nutrientConcentration", ""),
                    "baseConc": bioreactor_data.get("baseConcentration", ""),
                    "duration": bioreactor_data.get("duration", ""),
                    "nutrientDelay": bioreactor_data.get("nutrientDelay", ""),
                    "experimentName": bioreactor_data.get("experimentName", ""),
                    "comment": bioreactor_data.get("comment", "")
                }

        # Flatten the data structure
        flat_data = flatten_dict(bioreactor_data)
        logger.debug(f"Flattened data: {flat_data}")

        # Prepare row with correct field order
        row = [backend_time, data.timestamp, event_type]
        
        # Add non-program fields
        standard_fields = [
            # State fields
            "currentProgram", "programState",
            # Sensor data
            "sensorData_waterTemp", "sensorData_airTemp", "sensorData_elecTemp", 
            "sensorData_pH", "sensorData_turbidity", "sensorData_oxygen", "sensorData_airFlow",
            # Actuator states
            "actuatorData_airPump", "actuatorData_drainPump", "actuatorData_samplePump",
            "actuatorData_nutrientPump", "actuatorData_basePump", "actuatorData_fillPump",
            "actuatorData_stirringMotor", "actuatorData_heatingPlate", "actuatorData_ledGrowLight",
            # Actuator setpoints
            "actuatorSetpoints_airPumpValue", "actuatorSetpoints_drainPumpValue", 
            "actuatorSetpoints_samplePumpValue", "actuatorSetpoints_nutrientPumpValue",
            "actuatorSetpoints_basePumpValue", "actuatorSetpoints_fillPumpValue",
            "actuatorSetpoints_stirringMotorValue", "actuatorSetpoints_heatingPlateValue",
            "actuatorSetpoints_ledGrowLightValue"
        ]
        # Add standard fields
        for field in standard_fields:
            row.append(str(flat_data.get(field, "")))

        # Add program-specific fields
        row.extend([
            str(program_parameters.get("program", "")),
            str(program_parameters.get("tempSetpoint", "")),
            str(program_parameters.get("pHSetpoint", "")),
            str(program_parameters.get("DOSetpoint", "")),
            str(program_parameters.get("nutrientConc", "")),
            str(program_parameters.get("baseConc", "")),
            str(program_parameters.get("duration", "")),
            str(program_parameters.get("nutrientDelay", "")),
            str(program_parameters.get("experimentName", "")),
            str(program_parameters.get("comment", ""))
        ])

        # Write to CSV
        with open(filename, 'a', newline='') as file:
            writer = csv.writer(file)
            writer.writerow(row)

        logger.info("Bioreactor data successfully saved")
        return {"status": "success", "message": "Data received and processed"}

    except Exception as e:
        logger.error(f"Error processing data: {str(e)}")
        raise HTTPException(status_code=500, detail=str(e))

@app.get("/sensor_data")
async def get_sensor_data():
    logger.info("Fetching sensor data")
    try:
        data = []
        with open(filename, 'r') as file:
            reader = csv.DictReader(file)
            for row in reader:
                data.append(row)
        logger.info(f"Returning {len(data)} data points")
        return data
    except Exception as e:
        logger.error(f"Error fetching sensor data: {str(e)}")
        raise HTTPException(status_code=500, detail=str(e))

if __name__ == "__main__":
    import uvicorn
    logger.info("Starting FastAPI server")
    uvicorn.run(app, host="0.0.0.0", port=8000)

@app.get("/mqtt_status")
async def get_mqtt_status():
    return {
        "status": "Connected" if mqtt_client.is_connected() else "Disconnected",
        "connected": mqtt_client.is_connected()
    }