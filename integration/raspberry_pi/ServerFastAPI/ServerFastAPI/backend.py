from fastapi import FastAPI, HTTPException
from pydantic import BaseModel, Field
import csv
from datetime import datetime
import os

app = FastAPI()

# Define the directory and file path
data_dir = "/Raspberry/Bioreactor/ServerFastAPI/data"  # Update this to your actual path
filename = os.path.join(data_dir, "data.csv")

# Ensure the data directory exists
os.makedirs(data_dir, exist_ok=True)

def ensure_csv_header():
    """Ensure that the CSV file has the correct header"""
    file_exists = os.path.isfile(filename)
    if not file_exists:
        with open(filename, 'w', newline='') as file:
            writer = csv.writer(file)
            writer.writerow([
                "Backend_Time", "ESP_Time", "experimentName", "comment", "programStatus",
                "airPumpStatus", "drainPumpStatus", "nutrientPumpStatus", "basePumpStatus",
                "stirringMotorStatus", "heatingPlateStatus", "ledGrowLightStatus",
                "waterTemp", "airTemp", "ph", "turbidity", "oxygen", "airFlow"
            ])

class SensorData(BaseModel):
    sensor_value: dict = Field(..., example={"experimentName": "None", "comment": "", "programStatus": "", "airPumpStatus": 0})
    timestamp: str = Field(..., example="2023-05-06T12:00:00Z")

@app.post("/sensor_data")
async def receive_data(data: SensorData):
    ensure_csv_header()  # Ensure the header is present

    backend_time = datetime.now().strftime("%Y-%m-%d %H:%M:%S")

    try:
        with open(filename, 'a', newline='') as file:
            writer = csv.writer(file)
            writer.writerow([
                backend_time, data.timestamp, data.sensor_value.get("experimentName", ""), 
                data.sensor_value.get("comment", ""), data.sensor_value.get("programStatus", ""),
                data.sensor_value.get("airPumpStatus", 0), data.sensor_value.get("drainPumpStatus", 0), 
                data.sensor_value.get("nutrientPumpStatus", 0), data.sensor_value.get("basePumpStatus", 0),
                data.sensor_value.get("stirringMotorStatus", 0), data.sensor_value.get("heatingPlateStatus", 0), 
                data.sensor_value.get("ledGrowLightStatus", 0), data.sensor_value.get("waterTemp", 0.0), 
                data.sensor_value.get("airTemp", 0.0), data.sensor_value.get("ph", 0.0), 
                data.sensor_value.get("turbidity", 0.0), data.sensor_value.get("oxygen", 0.0), 
                data.sensor_value.get("airFlow", 0.0)
            ])

        return {"status": "success", "message": "Data received"}

    except Exception as e:
        raise HTTPException(status_code=400, detail=f"Error processing data: {e}")
