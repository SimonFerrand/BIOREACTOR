// WebPageBuilder.cpp
#include "WebPageBuilder.h"
#include <Arduino.h>

String WebPageBuilder::buildIndexPage(const String& deviceInfo) {
    String html = R"(
<!DOCTYPE html>
<html>
<head>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>Water Bath Control</title>
)";
    html += getStyle();
    html += R"(
</head>
<body>
    <h1>Water Bath Control</h1>
    <div class="card">)";
    
    html += deviceInfo;
    
    html += R"(
    </div>
    <div class="card">
        <h2>Latest Data</h2>
        <div id="data"></div>
    </div>
)";
    html += getScript();
    html += R"(
</body>
</html>)";
    
    return html;
}

String WebPageBuilder::buildDataPage(const SensorData& data) {
    String html = R"(
<!DOCTYPE html>
<html>
<head>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>Sensor Data</title>
)";
    html += getStyle();
    html += R"(
</head>
<body>
    <h1>Sensor Data</h1>
    <div class="card">
        <table>
            <tr><th>Parameter</th><th>Value</th></tr>)";
    
    html += "<tr><td>Water Temperature</td><td>" + String(data.waterTemp) + " °C</td></tr>";
    html += "<tr><td>Water Pressure</td><td>" + String(data.pressure) + " bar</td></tr>";
    
    html += R"(
        </table>
    </div>
</body>
</html>)";

    return html;
}

String WebPageBuilder::getStyle() {
    return R"(
    <meta charset="UTF-8">
    <style>
        body {
            font-family: Arial;
            margin: 0;
            padding: 20px;
            background: #121212;
            color: #e0e0e0;
        }
        
        .card {
            background: #1e1e1e;
            border-radius: 8px;
            padding: 20px;
            margin: 15px 0;
            box-shadow: 0 2px 5px rgba(0,0,0,0.5);
        }
        
        h1, h2 {
            color: #ffffff;
            margin-bottom: 20px;
        }
        
        table {
            width: 100%;
            border-collapse: collapse;
            margin: 10px 0;
        }
        
        th, td {
            padding: 12px;
            text-align: left;
            border-bottom: 1px solid #333;
        }
        
        th {
            background-color: #2d2d2d;
            color: #ffffff;
            font-weight: bold;
        }
        
        .value {
            font-weight: bold;
            color: #4CAF50;
        }
        
        tr:hover {
            background-color: #252525;
        }
        
        p {
            margin: 8px 0;
            line-height: 1.5;
        }
    </style>)";
}

String WebPageBuilder::getScript() {
    return R"(
    <script>
        function updateData() {
            fetch('/api/data')
                .then(response => response.json())
                .then(data => {
                    document.getElementById('data').innerHTML = `
                        <p>Water Temp: <span class="value">${data.waterTemp}&deg;C</span></p> 
                    `;
                });
        }
        setInterval(updateData, 5000);
        updateData();
    </script>)";
}

String WebPageBuilder::buildProgramPage() {
    String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <title>Programs Control</title>
    <style>
        body { font-family: Arial; margin: 0; padding: 20px; background: #121212; color: #e0e0e0; }
        .card { background: #1e1e1e; border-radius: 8px; padding: 20px; margin: 15px 0; }
        .button { padding: 10px 20px; margin: 10px; border-radius: 5px; cursor: pointer; background: #2d2d2d; color: white; border: none; }
        .stop { background: #d32f2f; }
        .input { padding: 8px; margin: 5px; border-radius: 4px; background: #333; color: white; border: 1px solid #444; }
    </style>
</head>
<body>
    <h1>Programs Control</h1>
    
    <div class="card">
        <h2>Global Control</h2>
        <button class="button stop" onclick="stopAll()">STOP ALL</button>
    </div>

    <div class="card">
        <h2>CIP Program</h2>
        <input type="number" id="cipTemp" placeholder="Temperature (°C)" class="input">
        <input type="number" id="cipDuration" placeholder="Duration (min)" class="input">
        <button class="button" onclick="startCIP()">Start CIP</button>
    </div>

    <div class="card">
        <h2>Sterilization Program</h2>
        <p>Coming soon...</p>
    </div>

    <script>
        function startCIP() {
            var temp = document.getElementById("cipTemp").value;
            var duration = document.getElementById("cipDuration").value;
            fetch("/cip?temp=" + temp + "&duration=" + duration)
                .then(function(response) { return response.text(); })
                .then(function(text) { alert(text); });
        }

        function stopAll() {
            fetch("/stop")
                .then(function(response) { return response.text(); })
                .then(function(text) { alert(text); });
        }
    </script>
</body>
</html>
)rawliteral";
    return html;
}