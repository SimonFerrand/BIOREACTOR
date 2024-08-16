# restart FastAPI
# cd /Raspberry/Bioreactor
# sudo -E env "PATH=$PATH" uvicorn backend:app --host 0.0.0.0 --port 8000 --ssl-keyfile /etc/letsencrypt/live/bioreactor.ddns.net/privkey.pem --ssl-certfile /etc/letsencrypt/live/bioreactor.ddns.net/fullchain.pem
 
from fastapi import FastAPI, Form
import csv
from datetime import datetime
import os

app = FastAPI()

# Define the directory and file path
data_dir = "/Raspberry/Bioreactor/data"  # Update this to your actual path
filename = os.path.join(data_dir, "data.csv")

# Ensure the data directory exists
os.makedirs(data_dir, exist_ok=True)

def ensure_csv_header():
    """ Ensure that the CSV file has the correct header """
    file_exists = os.path.isfile(filename)
    if not file_exists:
        with open(filename, 'w', newline='') as file:
            writer = csv.writer(file)
            writer.writerow(["Backend_Time", "ESP_Time", "Temperature", "pH", "Turbidity_Voltage", "O2"])

@app.post("/sensor_data")
async def receive_data(sensor_value: str = Form(...), timestamp: str = Form(...)):
    ensure_csv_header()  # Ensure the header is present

    backend_time = datetime.now().strftime("%Y-%m-%d %H:%M:%S")

    # Assume sensor_value is a comma-separated string
    data = sensor_value.split(',')
    temperature = data[0].split(':')[1].strip()
    ph = data[1].split(':')[1].strip()
    turbidity_voltage = data[2].split(':')[1].strip()
    o2 = data[3].split(':')[1].strip()

    with open(filename, 'a', newline='') as file:
        writer = csv.writer(file)
        writer.writerow([backend_time, timestamp, temperature, ph, turbidity_voltage, o2])

    # Log the received data
    log_file = '/Raspberry/Bioreactor/logs/logfile.txt'  # Update this to your actual log file path
    with open(log_file, 'a') as log:
        log.write(f"{backend_time} - Received data: {sensor_value} with timestamp: {timestamp}\n")

    return {"status": "success", "message": "Data received"}
