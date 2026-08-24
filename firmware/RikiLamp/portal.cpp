#include "portal.h"

#include "config.h"
#include "storage.h"

#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <ESP.h>

PortalManager portal;

namespace {

constexpr uint8_t DNS_PORT = 53;

const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>RiKi Lamp</title>
<style>
*{margin:0;padding:0;box-sizing:border-box;font-family:-apple-system,BlinkMacSystemFont,"Segoe UI",Roboto,Arial,sans-serif}
body{background:#0E1117;color:white;display:flex;justify-content:center;align-items:center;min-height:100vh;padding:18px}
.card{width:100%;max-width:390px;background:#171B24;border-radius:24px;padding:32px;box-shadow:0 20px 80px rgba(0,0,0,.45);animation:fade .5s ease}
@keyframes fade{from{opacity:0;transform:translateY(18px)}to{opacity:1;transform:none}}
.logo{font-size:60px;text-align:center;margin-bottom:8px;animation:breathe 3s infinite}
@keyframes breathe{0%{transform:scale(1)}50%{transform:scale(1.08)}100%{transform:scale(1)}}
h1{text-align:center;font-size:30px;margin-bottom:10px}
.subtitle{text-align:center;color:#9AA5B1;line-height:1.6;font-size:15px;margin-bottom:26px}
.status{display:flex;align-items:center;justify-content:center;gap:10px;margin-bottom:24px;font-size:15px;color:#C7CED8}
.spinner{width:18px;height:18px;border-radius:50%;border:3px solid #3D4654;border-top:3px solid white;animation:spin .8s linear infinite}
@keyframes spin{to{transform:rotate(360deg)}}
label{display:block;margin-bottom:8px;font-size:14px;font-weight:600}
select,input{width:100%;padding:14px;background:#232A36;border:none;outline:none;color:white;font-size:15px;border-radius:12px;margin-bottom:18px}
.password{position:relative}
.eye{position:absolute;right:15px;top:15px;cursor:pointer;user-select:none}
button{width:100%;padding:15px;border:none;border-radius:14px;background:white;color:#111;font-size:16px;font-weight:700;cursor:pointer;transition:.25s}
button:hover{transform:translateY(-2px)}
button:disabled{opacity:.6;cursor:not-allowed;transform:none}
.footer{margin-top:24px;text-align:center;font-size:13px;color:#7B8695}
</style>
</head>
<body>
<div class="card">
<div class="logo">❤️</div>
<h1>RiKi Lamp</h1>
<div class="subtitle">
A little light for every distance.<br><br>
Built with ❤️ for Chakri & Nikki
</div>
<div class="status">
<div class="spinner"></div>
<div id="status">Scanning nearby Wi-Fi...</div>
</div>
<label>Wi-Fi Network</label>
<select id="wifi"><option>Scanning...</option></select>
<label>Password</label>
<div class="password">
<input id="password" type="password" placeholder="Enter Wi-Fi Password">
<div class="eye" onclick="togglePassword()">👁</div>
</div>
<button id="connect">Connect Lamp</button>
<div class="footer">Firmware v1.0.0</div>
</div>
<script>
const wifi=document.getElementById("wifi");
const statusText=document.getElementById("status");
const connectButton=document.getElementById("connect");

window.onload=()=>{scanWifi()};

async function scanWifi(){
    statusText.innerHTML="Scanning nearby Wi-Fi...";
    wifi.innerHTML="<option>Scanning...</option>";
    try{
        const res=await fetch("/scan");
        const list=await res.json();
        wifi.innerHTML="";
        if(list.length===0){
            wifi.innerHTML="<option>No Wi-Fi Found</option>";
            statusText.innerHTML="No nearby Wi-Fi found.";
            return;
        }
        list.forEach(item=>{
            const option=document.createElement("option");
            option.value=item.ssid;
            option.innerHTML=item.icon+" "+item.ssid;
            wifi.appendChild(option);
        });
        statusText.innerHTML=list.length+" network(s) found";
    }catch(e){
        wifi.innerHTML="<option>Scan Failed</option>";
        statusText.innerHTML="Unable to scan.";
    }
}

function togglePassword(){
    const p=document.getElementById("password");
    p.type=p.type==="password"?"text":"password";
}

connectButton.onclick=async()=>{
    if(!wifi.value||wifi.value==="Scanning..."||wifi.value==="No Wi-Fi Found"){
        statusText.innerHTML="Please select a network.";
        return;
    }
    connectButton.disabled=true;
    connectButton.innerHTML="Connecting...";
    statusText.innerHTML="Connecting to Wi-Fi...";
    try{
        const response=await fetch("/connect",{
            method:"POST",
            headers:{"Content-Type":"application/json"},
            body:JSON.stringify({
                ssid:wifi.value,
                password:document.getElementById("password").value
            })
        });
        const result=await response.json();
        if(result.success){
            document.querySelector(".card").innerHTML=`
            <div class="logo">💖</div>
            <h1>Connected!</h1>
            <div class="subtitle">
            RiKi Lamp is restarting...<br><br>
            See you on your home Wi-Fi.
            </div>`;
            return;
        }
        connectButton.disabled=false;
        connectButton.innerHTML="Connect Lamp";
        statusText.innerHTML=result.message;
    }catch(e){
        connectButton.disabled=false;
        connectButton.innerHTML="Connect Lamp";
        statusText.innerHTML="Connection failed. Please try again.";
    }
};
</script>
</body>
</html>
)rawliteral";

const char* signalIcon(int rssi) {
    if (rssi >= -55) return "🟢";
    if (rssi >= -70) return "🟡";
    return "🔴";
}

void sendJsonResponse(WebServer& server, int code, bool success, const String& message) {
    JsonDocument doc;
    doc["success"] = success;
    doc["message"] = message;
    String output;
    serializeJson(doc, output);
    server.send(code, "application/json", output);
}

} // namespace

void PortalManager::redirectToPortal() {
    _server.sendHeader("Location", "http://" AP_IP "/", true);
    _server.send(302, "text/plain", "");
}

void PortalManager::handleRoot() {
    _server.send_P(200, "text/html", INDEX_HTML);
}

void PortalManager::handleScan() {
    Serial.println("Starting Wi-Fi scan...");

    WiFi.mode(WIFI_AP_STA);

    int count = WiFi.scanNetworks(false, true);
    if (count < 0) {
        WiFi.scanDelete();
        _server.send(200, "application/json", "[]");
        return;
    }

    struct ScannedNetwork {
        String ssid;
        int rssi;
    };

    ScannedNetwork best[32];
    int uniqueCount = 0;

    for (int i = 0; i < count; i++) {
        String ssid = WiFi.SSID(i);
        if (ssid.isEmpty()) {
            continue;
        }

        int rssi = WiFi.RSSI(i);
        bool found = false;

        for (int j = 0; j < uniqueCount; j++) {
            if (best[j].ssid == ssid) {
                if (rssi > best[j].rssi) {
                    best[j].rssi = rssi;
                }
                found = true;
                break;
            }
        }

        if (!found && uniqueCount < 32) {
            best[uniqueCount++] = {ssid, rssi};
        }
    }

    for (int i = 0; i < uniqueCount - 1; i++) {
        for (int j = i + 1; j < uniqueCount; j++) {
            if (best[j].rssi > best[i].rssi) {
                ScannedNetwork tmp = best[i];
                best[i] = best[j];
                best[j] = tmp;
            }
        }
    }

    JsonDocument doc;
    JsonArray array = doc.to<JsonArray>();

    for (int i = 0; i < uniqueCount; i++) {
        JsonObject network = array.add<JsonObject>();
        network["ssid"] = best[i].ssid;
        network["rssi"] = best[i].rssi;
        network["icon"] = signalIcon(best[i].rssi);
    }

    String response;
    serializeJson(doc, response);
    WiFi.scanDelete();

    _server.send(200, "application/json", response);
}

void PortalManager::handleConnect() {
    if (!_server.hasArg("plain")) {
        sendJsonResponse(_server, 400, false, "Missing data");
        return;
    }

    JsonDocument doc;
    if (deserializeJson(doc, _server.arg("plain"))) {
        sendJsonResponse(_server, 400, false, "Invalid JSON");
        return;
    }

    const char* ssidRaw = doc["ssid"];
    const char* passRaw = doc["password"];

    if (!ssidRaw || strlen(ssidRaw) == 0) {
        sendJsonResponse(_server, 400, false, "SSID is required");
        return;
    }

    String ssid = ssidRaw;
    String password = passRaw ? String(passRaw) : String("");

    Serial.println();
    Serial.println("====== Connect Request ======");
    Serial.print("SSID: ");
    Serial.println(ssid);

    WiFi.mode(WIFI_AP_STA);
    WiFi.disconnect(false, false);

    WiFi.begin(ssid.c_str(), password.c_str());

    const unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < WIFI_CONNECT_TIMEOUT_MS) {
        delay(250);
    }

    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("Wi-Fi connection failed");
        WiFi.disconnect(false, false);
        sendJsonResponse(_server, 200, false, "Could not connect. Check password and try again.");
        return;
    }

    Serial.print("Connected. IP: ");
    Serial.println(WiFi.localIP());

    storage.saveWiFi(ssid, password);
    sendJsonResponse(_server, 200, true, "Connected to " + ssid);

    delay(500);
    ESP.restart();
}

void PortalManager::setupRoutes() {
    _server.on("/", HTTP_GET, [this]() { handleRoot(); });
    _server.on("/scan", HTTP_GET, [this]() { handleScan(); });
    _server.on("/connect", HTTP_POST, [this]() { handleConnect(); });

    _server.on("/generate_204", HTTP_GET, [this]() { _server.send(204); });
    _server.on("/gen_204", HTTP_GET, [this]() { redirectToPortal(); });

    _server.on("/hotspot-detect.html", HTTP_GET, [this]() { redirectToPortal(); });
    _server.on("/library/test/success.html", HTTP_GET, [this]() { redirectToPortal(); });

    _server.on("/ncsi.txt", HTTP_GET, [this]() { redirectToPortal(); });
    _server.on("/connecttest.txt", HTTP_GET, [this]() { redirectToPortal(); });

    _server.onNotFound([this]() { redirectToPortal(); });
}

void PortalManager::begin() {
    setupRoutes();

    _dns.start(DNS_PORT, "*", WiFi.softAPIP());
    _server.begin();
    _active = true;

    Serial.println("Portal started");
    Serial.print("Setup URL: http://");
    Serial.println(WiFi.softAPIP());
}

void PortalManager::loop() {
    if (!_active) {
        return;
    }

    _dns.processNextRequest();
    _server.handleClient();
}
