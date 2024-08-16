import requests
import json
from datetime import datetime

url = "http://192.168.1.25:8000/sensor_data"
data = {
    "sensor_value": {
        "event": "data",
        "programType": "Fermentation",
        "rateOrSpeed": 0,
        "duration": 0,
        "tempSetpoint": 25.0,
        "phSetpoint": 7.2,
        "doSetpoint": 5.5,
        "nutrientConc": 10.0,
        "baseConc": 1.0,
        "experimentName": "TestExperiment",
        "comment": "This is a test comment",
        "currentProgram": "Fermentation",
        "programStatus": "Running",
        "airPumpStatus": 1,
        "drainPumpStatus": 0,
        "nutrientPumpStatus": 0,
        "basePumpStatus": 0,
        "stirringMotorStatus": 1,
        "heatingPlateStatus": 1,
        "ledGrowLightStatus": 0,
        "waterTemp": 24.5,
        "airTemp": 22.0,
        "ph": 6.8,
        "turbidity": 1.0,
        "oxygen": 7.5,
        "airFlow": 0.5
    },
    "timestamp": datetime.now().strftime("%Y-%m-%dT%H:%M:%S")
}

response = requests.post(url, json=data)
print(response.json())
