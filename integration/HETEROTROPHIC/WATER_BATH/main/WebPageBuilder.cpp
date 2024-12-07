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
    
    html += R"(
        </table>
    </div>
</body>
</html>)";

    return html;
}

String WebPageBuilder::getStyle() {
    return R"(
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