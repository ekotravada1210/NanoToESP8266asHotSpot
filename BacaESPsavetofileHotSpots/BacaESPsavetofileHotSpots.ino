#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <FS.h>
#include <ESP8266WiFi.h>

const char* ssid = "ESP-Hotspot";    // Nama hotspot
const char* password = "12345678";
ESP8266WebServer server(80);

float currentTemp=0.0;
bool saveData = false;
unsigned long previousSaveMillis = 0;
String serialBuffer;
String request;

String htmlContent = R"=====(
<!DOCTYPE html>
<html>
<head>
    <title>Monitoring Suhu</title>
    <meta charset="UTF-8">
    <style>
        body { font-family: Arial; text-align: center; margin-top: 50px; }
        button { padding: 12px 24px; font-size: 16px; }
        .status { color: %STATUS_COLOR%; font-weight: bold; }
    </style>
    <script>
        function toggleSaving() {
            fetch('/save', { method: 'POST' })
            .then(response => response.json())
            .then(data => {
                document.getElementById('saveButton').textContent = 
                    data.save ? 'Stop Saving' : 'Start Saving';
                document.getElementById('status').style.color = 
                    data.save ? 'green' : 'red';
                document.getElementById('status').textContent = 
                    data.save ? 'AKTIF' : 'NON-AKTIF';
            });
        }

        function updateData() {
            fetch('/data')
            .then(response => response.text())
            .then(text => {
                document.getElementById('currentTemp').textContent = text;
            });
            
            fetch('/status')
            .then(response => response.text())
            .then(text => {
                document.getElementById('lastSave').textContent = text;
            });
        }
        setInterval(updateData, 1000);
    </script>
</head>
<body>
    <h1>Monitoring Suhu ESP8266</h1>
    <p>Suhu Saat Ini: <span id="currentTemp">0.0</span> °C</p>
    <button id="saveButton" onclick="toggleSaving()">Start Saving</button>
    <p>Status Penyimpanan: 
        <span id="status" class="status">NON-AKTIF</span>
    </p>
    <p>Terakhir Disimpan: <span id="lastSave">-</span></p>
   <a href="/download" download="data.txt"><button>Download Data</button></a>
</body>
</html>
)=====";

void handleRoot() {
    String html = htmlContent;
    html.replace("%STATUS_COLOR%", saveData ? "green" : "red");
    server.send(200, "text/html", html);
}

void handleSave() {
    saveData = !saveData;
    String response = "{\"save\": " + String(saveData ? "true" : "false") + "}";
    server.send(200, "application/json", response);
}

void handleData() {
    server.send(200, "text/plain", String(currentTemp));
}

void handleDownload() {
     File file = SPIFFS.open("/data.txt", "r");
    if(file){
        server.sendHeader("Content-Type", "text/plain");
        server.sendHeader("Content-Disposition", "attachment; filename=\"data.txt\"");
        server.streamFile(file, "text/plain");
        file.close();
    } else {
        server.send(404, "text/plain", "File not found");
    }
}
void handleStatus() {
    static unsigned long lastSavedTime = 0;
    if(saveData && (millis() - lastSavedTime >= 10000)) {
        lastSavedTime = millis();
    }
    server.send(200, "text/plain", String(lastSavedTime/1000) + " detik sejak mulai");
}

void setup() {
  Serial.begin(115200);
  SPIFFS.begin();
  
  // Konfigurasi mode Access Point (hotspot)
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);
  
  // Tampilkan alamat IP di Serial Monitor
IPAddress local_IP(192, 168, 10, 1); // Contoh IP kustom
IPAddress gateway(192, 168, 10, 0);
IPAddress subnet(255, 255, 255, 0);

WiFi.softAPConfig(local_IP, gateway, subnet); // Set IP kustom
  IPAddress apIP = WiFi.softAPIP();
  Serial.print("Akses ESP via: http://");
  Serial.println(apIP); // Alamat default: 192.168.4.1

  Serial.println("\nHotspot ESP8266 telah aktif!");
    Serial.print("SSID: ");
    Serial.println(ssid);
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    // Setup routing web server
    server.on("/", handleRoot);
    server.on("/save", handleSave);
    server.on("/data", handleData);
    server.on("/download", handleDownload);
    server.on("/status", handleStatus);
    server.begin();
}

void parseTemperature(String data) {
     if(data.startsWith("Temperature:")) 
     {
        String tempValue = data.substring(13,17);
        tempValue.replace(",", "."); // Handle format desimal
        currentTemp = tempValue.toFloat();
        
        // Debugging ke Serial Monitor
        Serial.print("Raw Data: ");
        Serial.println(data);
        Serial.print("Parsed Temp: ");
        Serial.println(currentTemp);
    }
}


void readSerialData() {
     if(Serial.available()) {
       request = Serial.readStringUntil('\n');
       Serial.println("Suhu terbaca : ");
       Serial.println(request);
       parseTemperature(request);
       delay(2000);
      }
}



void checkSaveData() {
    if(saveData && (millis() - previousSaveMillis >= 10000)) {
        previousSaveMillis = millis();
        File file = SPIFFS.open("/data.txt", "a");
        if(file) {
            file.printf("Waktu: %lu detik, Suhu: %.2f°C\n", millis()/1000, currentTemp);
            file.close();
        }
    }
}

void loop() {
    server.handleClient();
    readSerialData();
    checkSaveData();
    //server.handleClient();
}