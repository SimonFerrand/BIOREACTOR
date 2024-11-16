from fastapi import FastAPI, HTTPException, WebSocket, WebSocketDisconnect
from fastapi.middleware.cors import CORSMiddleware
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

app = FastAPI()

# Define the directory and file path
data_dir = "/Raspberry/Bioreactor/ServerFastAPI/data"
filename = os.path.join(data_dir, "data.csv")

# Ensure the data directory exists
os.makedirs(data_dir, exist_ok=True)

# Add the CORS middleware to allow requests from the frontend
app.add_middleware(
    CORSMiddleware,
    allow_origins=["http://192.168.1.25:8080"],  # Adjust this based on your frontend origin
    allow_credentials=True,
    allow_methods=["*"],
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
            "actuatorData_stirringMotor", "actuatorData_heatingPlate", "actuatorData_ledGrowLight"
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

esp32_connection = None
frontend_connection = None

@app.websocket("/ws")
async def websocket_endpoint(websocket: WebSocket):
    await websocket.accept()
    global esp32_connection, frontend_connection
    
    client_type = websocket.headers.get("X-Client-Type")
    
    if client_type == "ESP32":
        esp32_connection = websocket
        logger.info("ESP32 WebSocket connection established")
    else:
        frontend_connection = websocket
        logger.info("Frontend WebSocket connection established")

    try:
        while True:
            data = await websocket.receive_text()
            logger.info(f"Received WebSocket message: {data}")
            
            if websocket == esp32_connection:
                logger.info(f"Received message from ESP32: {data}")
                if frontend_connection:
                    await frontend_connection.send_text(data)
            else:
                if esp32_connection:
                    await esp32_connection.send_text(data)
                    logger.info(f"Sent message to ESP32: {data}")
                else:
                    logger.warning("Cannot send message to ESP32: No connection")
            
            await websocket.send_text(f"Message received: {data}")
    except WebSocketDisconnect:
        if websocket == esp32_connection:
            logger.info("ESP32 WebSocket disconnected")
            esp32_connection = None
        else:
            logger.info("Frontend WebSocket disconnected")
            frontend_connection = None

@app.post("/execute/mix")
async def execute_mix(command: MixCommand):
    logger.info(f"Received mix command: speed={command.speed}")
    logger.info(f"ESP32 connection status: {'Connected' if esp32_connection else 'Not connected'}")
    if esp32_connection:
        await esp32_connection.send_text(json.dumps({"program": "mix", "speed": command.speed}))
        logger.info(f"Sent mix command to ESP32: speed {command.speed}")
        return {"message": f"Mix program started with speed {command.speed}"}
    logger.warning("ESP32 not connected")
    return {"message": "ESP32 not connected"}

@app.post("/execute/drain")
async def execute_drain(command: DrainCommand):
    logger.info(f"Received drain command: rate={command.rate}, duration={command.duration}")
    logger.info(f"ESP32 connection status: {'Connected' if esp32_connection else 'Not connected'}")
    if esp32_connection:
        await esp32_connection.send_text(json.dumps({"program": "drain", "rate": command.rate, "duration": command.duration}))
        logger.info(f"Sent drain command to ESP32: rate {command.rate}, duration {command.duration}")
        return {"message": f"Drain program started with rate {command.rate} and duration {command.duration}"}
    logger.warning("ESP32 not connected")
    return {"message": "ESP32 not connected"}

@app.post("/execute/fermentation")
async def execute_fermentation(command: FermentationCommand):
    logger.info(f"Received fermentation command: {command}")
    logger.info(f"ESP32 connection status: {'Connected' if esp32_connection else 'Not connected'}")
    if esp32_connection:
        await esp32_connection.send_text(json.dumps({
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
        }))
        logger.info("Sent fermentation command to ESP32")
        return {"message": "Fermentation program started"}
    logger.warning("ESP32 not connected")
    return {"message": "ESP32 not connected"}

@app.post("/execute/stop")
async def execute_stop():
    logger.info("Received stop command")
    logger.info(f"ESP32 connection status: {'Connected' if esp32_connection else 'Not connected'}")
    if esp32_connection:
        await esp32_connection.send_text(json.dumps({"program": "stop"}))
        logger.info("Sent stop command to ESP32")
        return {"message": "All programs stopped"}
    logger.warning("ESP32 not connected")
    return {"message": "ESP32 not connected"}

if __name__ == "__main__":
    import uvicorn
    logger.info("Starting FastAPI server")
    uvicorn.run(app, host="0.0.0.0", port=8000)