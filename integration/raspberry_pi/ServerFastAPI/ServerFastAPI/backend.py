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
                "Backend_Time", "ESP_Time", "event", "programType", "rateOrSpeed", "duration",
                "tempSetpoint", "phSetpoint", "doSetpoint", "nutrientConc", "baseConc", 
                "experimentName", "comment", "currentProgram", "programStatus",
                "airPumpStatus", "drainPumpStatus", "nutrientPumpStatus", "basePumpStatus",
                "stirringMotorStatus", "heatingPlateStatus", "ledGrowLightStatus",
                "waterTemp", "airTemp", "ph", "turbidity", "oxygen", "airFlow"
            ])

class SensorData(BaseModel):
    sensor_value: dict = Field(..., example={
        "prog": "None", "stat": "Idle", "ap": 0, "dp": 0, "np": 0, "bp": 0,
        "sm": 0, "hp": 0, "lg": 0, "wT": 0.0, "aT": 0.0, "pH": 0.0, "tb": 0.0,
        "ox": 0.0, "af": 0.0,
        "event": "data", "programType": "", "rateOrSpeed": 0, "duration": 0,
        "tempSetpoint": 0.0, "phSetpoint": 0.0, "doSetpoint": 0.0, "nutrientConc": 0.0, 
        "baseConc": 0.0, "experimentName": "", "comment": "", 
        "currentProgram": "None", "programStatus": "", "airPumpStatus": 0,
        "drainPumpStatus": 0, "nutrientPumpStatus": 0, "basePumpStatus": 0,
        "stirringMotorStatus": 0, "heatingPlateStatus": 0, "ledGrowLightStatus": 0,
        "waterTemp": 0.0, "airTemp": 0.0, "ph": 0.0, "turbidity": 0.0, "oxygen": 0.0, "airFlow": 0.0
    })
    timestamp: str = Field(..., example="2023-05-06T12:00:00Z")

@app.post("/sensor_data")
async def receive_data(data: SensorData):
    logger.info("Received sensor data")
    ensure_csv_header()  # Ensure the header is present

    backend_time = datetime.now().strftime("%Y-%m-%d %H:%M:%S")

    try:
        sensor_data = data.sensor_value
        # Ensure all required fields are present
        required_fields = [
            "event", "programType", "rateOrSpeed", "duration",
            "tempSetpoint", "phSetpoint", "doSetpoint", "nutrientConc", "baseConc", 
            "experimentName", "comment", "currentProgram", "programStatus",
            "airPumpStatus", "drainPumpStatus", "nutrientPumpStatus", "basePumpStatus",
            "stirringMotorStatus", "heatingPlateStatus", "ledGrowLightStatus",
            "waterTemp", "airTemp", "ph", "turbidity", "oxygen", "airFlow"
        ]
        
        for field in required_fields:
            if field not in sensor_data:
                logger.error(f"Missing field in sensor data: {field}")
                raise HTTPException(status_code=400, detail=f"Missing field: {field}")

        with open(filename, 'a', newline='') as file:
            writer = csv.writer(file)
            writer.writerow([
                backend_time, data.timestamp,
                sensor_data.get("event", ""),
                sensor_data.get("programType", ""), sensor_data.get("rateOrSpeed", 0),
                sensor_data.get("duration", 0), sensor_data.get("tempSetpoint", 0.0), 
                sensor_data.get("phSetpoint", 0.0), sensor_data.get("doSetpoint", 0.0),
                sensor_data.get("nutrientConc", 0.0), sensor_data.get("baseConc", 0.0),
                sensor_data.get("experimentName", ""), sensor_data.get("comment", ""),
                sensor_data.get("currentProgram", ""), sensor_data.get("programStatus", ""),
                sensor_data.get("airPumpStatus", 0), sensor_data.get("drainPumpStatus", 0), 
                sensor_data.get("nutrientPumpStatus", 0), sensor_data.get("basePumpStatus", 0),
                sensor_data.get("stirringMotorStatus", 0), sensor_data.get("heatingPlateStatus", 0), 
                sensor_data.get("ledGrowLightStatus", 0), sensor_data.get("waterTemp", 0.0), 
                sensor_data.get("airTemp", 0.0), sensor_data.get("ph", 0.0), 
                sensor_data.get("turbidity", 0.0), sensor_data.get("oxygen", 0.0), 
                sensor_data.get("airFlow", 0.0)
            ])

        logger.info("Sensor data successfully saved")
        return {"status": "success", "message": "Data received"}

    except ValidationError as e:
        logger.error(f"Validation error: {str(e)}")
        raise HTTPException(status_code=422, detail=str(e))
    except Exception as e:
        logger.error(f"Error processing data: {str(e)}")
        raise HTTPException(status_code=400, detail=f"Error processing data: {e}")

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