#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Wire.h>
#include <U8x8lib.h>
#include <DHT.h>
#include <time.h>

const char* ssid = "xxxxx";
const char* password = "xxxxxx";

#define DHTPIN D5
#define DHTTYPE DHT11

#define MOVE_INPUT D6
#define ALARM_INPUT D7
#define ARDUINO_COMMAND_PIN D0

DHT dht(DHTPIN, DHTTYPE);
ESP8266WebServer server(80);
U8X8_SH1106_128X64_NONAME_HW_I2C oled(U8X8_PIN_NONE);

float temperatureValue = 0.0;
float humidityValue = 0.0;

bool alarmOn = true;
bool rawMovementSignal = false;
bool movementVisible = false;

String movementHistory = "";

const int chartSize = 20;
float temperatureHistory[chartSize];
float humidityHistory[chartSize];
String timeHistory[chartSize];

int chartIndex = 0;
bool chartFilled = false;

unsigned long lastSensorRead = 0;
const unsigned long sensorInterval = 2000;

unsigned long lastChartSample = 0;
const unsigned long chartInterval = 5000;

unsigned long movementVisibleUntil = 0;
const unsigned long movementVisibleTime = 5000;

unsigned long lastMovementHistoryTime = 0;
unsigned long movementHistoryCooldown = 60000;

unsigned long lastOledUpdate = 0;
const unsigned long oledInterval = 1000;

const char MAIN_page[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>Smart Security IoT Dashboard</title>
<style>
:root{
  --bg:#0f172a;
  --card:#1e293b;
  --text:#ffffff;
  --muted:#94a3b8;
  --header:#111827;
  --blue:#38bdf8;
  --green:#22c55e;
  --red:#ef4444;
  --orange:#f97316;
}
body.light{
  --bg:#f1f5f9;
  --card:#ffffff;
  --text:#0f172a;
  --muted:#475569;
  --header:#e2e8f0;
  --blue:#0284c7;
  --green:#16a34a;
  --red:#dc2626;
  --orange:#ea580c;
}
body{
  margin:0;
  font-family:Arial,Helvetica,sans-serif;
  background:var(--bg);
  color:var(--text);
  text-align:center;
}
.header{
  background:var(--header);
  padding:22px;
  font-size:28px;
  color:var(--blue);
  font-weight:bold;
}
.container{
  padding:16px;
  max-width:1100px;
  margin:auto;
}
.grid{
  display:grid;
  grid-template-columns:repeat(3, 1fr);
  gap:14px;
}
@media(max-width:900px){
  .grid{
    grid-template-columns:repeat(2, 1fr);
  }
}
@media(max-width:600px){
  .grid{
    grid-template-columns:1fr;
  }
}
.card{
  background:var(--card);
  border-radius:18px;
  padding:20px;
  box-shadow:0 4px 12px #0006;
  margin-bottom:16px;
}
.value{
  font-size:30px;
  color:var(--blue);
  font-weight:bold;
}
.ok{
  font-size:27px;
  color:var(--green);
  font-weight:bold;
}
.danger{
  font-size:27px;
  color:var(--red);
  font-weight:bold;
}
.armed{
  font-size:27px;
  color:var(--orange);
  font-weight:bold;
}
.small{
  color:var(--muted);
  font-size:13px;
}
.btn{
  border:none;
  border-radius:12px;
  padding:12px 18px;
  font-weight:bold;
  background:var(--blue);
  color:white;
  margin:8px;
  cursor:pointer;
}
select{
  border:none;
  border-radius:10px;
  padding:10px;
  margin:8px;
  font-weight:bold;
}
.history{
  text-align:left;
  font-size:17px;
  line-height:1.7;
  max-height:220px;
  overflow:auto;
  color:var(--text);
}
.chart{
  height:230px;
  display:flex;
  align-items:end;
  justify-content:center;
  gap:7px;
  border-left:2px solid var(--muted);
  border-bottom:2px solid var(--muted);
  padding:10px;
  overflow:hidden;
}
.barBox{
  display:flex;
  flex-direction:column;
  align-items:center;
  justify-content:end;
  height:100%;
  min-width:30px;
}
.bar{
  width:22px;
  background:var(--blue);
  border-radius:8px 8px 0 0;
  min-height:4px;
}
.barHumidity{
  background:var(--green);
}
.readingLabel{
  font-size:11px;
  color:var(--text);
  margin-bottom:4px;
}
.barLabel{
  font-size:10px;
  color:var(--muted);
  margin-top:4px;
  writing-mode:vertical-rl;
}
</style>
</head>

<body>
<div class="header">Smart Security IoT Dashboard</div>

<div class="container">
<button class="btn" onclick="toggleMode()">Toggle Dark / Light Mode</button>

<div class="grid">
  <div class="card"><h2>Date</h2><div class="value" id="date">--/--/----</div></div>
  <div class="card"><h2>Time</h2><div class="value" id="time">--:--:--</div></div>
  <div class="card"><h2>Temperature</h2><div class="value"><span id="temperature">--</span> &deg;C</div></div>
  <div class="card"><h2>Humidity</h2><div class="value"><span id="humidity">--</span> %</div></div>
  <div class="card"><h2>Alarm Status</h2><div id="alarm" class="armed">ARMED</div></div>
  <div class="card"><h2>Movement Detection</h2><div id="movement" class="ok">NO MOVEMENT</div></div>
</div>

<div class="card">
  <h2>Movement History</h2>
  <button class="btn" onclick="clearHistory()">Clear History</button>
  <div class="history" id="history">No movement recorded</div>
</div>

<div class="card">
  <h2>Movement Settings</h2>
  <p class="small">This changes movement alert time on Arduino and movement history interval on web.</p>
  <select id="movementIntervalSelect" onchange="changeMovementInterval()">
    <option value="10000">10 seconds</option>
    <option value="30000">30 seconds</option>
    <option value="60000" selected>1 minute</option>
    <option value="300000">5 minutes</option>
  </select>
  <p class="small">Current movement interval: <span id="currentMovementInterval">1 minute</span></p>
</div>

<div class="card">
  <h2>Temperature Chart</h2>
  <div class="chart" id="temperatureChart"></div>
  <p class="small">Chart updates automatically every 5 seconds.</p>
</div>

<div class="card">
  <h2>Humidity Chart</h2>
  <div class="chart" id="humidityChart"></div>
  <p class="small">Chart updates automatically every 5 seconds.</p>
</div>

</div>

<script>
function toggleMode(){
  document.body.classList.toggle('light');
}

function intervalText(v){
  if(v==10000)return '10 seconds';
  if(v==30000)return '30 seconds';
  if(v==60000)return '1 minute';
  if(v==300000)return '5 minutes';
  return v + ' ms';
}

function changeMovementInterval(){
  let v = document.getElementById('movementIntervalSelect').value;
  document.getElementById('currentMovementInterval').innerHTML = intervalText(Number(v));
  fetch('/setMovementInterval?value=' + v).then(() => updateData());
}

function clearHistory(){
  fetch('/clearHistory').then(() => updateData());
}

function buildChart(elementId, values, labels, type){
  let box = document.getElementById(elementId);
  box.innerHTML = '';

  if(!values || values.length === 0){
    box.innerHTML = '<p class="small">Waiting for samples...</p>';
    return;
  }

  for(let i = 0; i < values.length; i++){
    let value = Number(values[i]);
    let height = type === 'temperature' ? value * 5 : value * 2;
    height = Math.max(5, Math.min(190, height));

    let group = document.createElement('div');
    group.className = 'barBox';

    let reading = document.createElement('div');
    reading.className = 'readingLabel';
    reading.innerHTML = value.toFixed(1);

    let bar = document.createElement('div');
    bar.className = type === 'temperature' ? 'bar' : 'bar barHumidity';
    bar.style.height = height + 'px';

    let label = document.createElement('div');
    label.className = 'barLabel';
    label.innerHTML = labels[i] || '';

    group.appendChild(reading);
    group.appendChild(bar);
    group.appendChild(label);
    box.appendChild(group);
  }
}

function updateData(){
  fetch('/data')
    .then(r => r.json())
    .then(d => {
      document.getElementById('date').innerHTML = d.date;
      document.getElementById('time').innerHTML = d.time;
      document.getElementById('temperature').innerHTML = Number(d.temperature).toFixed(1);
      document.getElementById('humidity').innerHTML = Number(d.humidity).toFixed(1);

      let alarmBox = document.getElementById('alarm');
      alarmBox.innerHTML = d.alarm;
      alarmBox.className = d.alarm === 'ARMED' ? 'armed' : 'ok';

      let movementBox = document.getElementById('movement');
      movementBox.innerHTML = d.movement;
      movementBox.className = d.movement === 'MOVEMENT DETECTED' ? 'danger' : 'ok';

      document.getElementById('history').innerHTML = d.history || 'No movement recorded';

      document.getElementById('movementIntervalSelect').value = d.movementHistoryCooldown;
      document.getElementById('currentMovementInterval').innerHTML = intervalText(Number(d.movementHistoryCooldown));

      buildChart('temperatureChart', d.temperatureHistory, d.timeHistory, 'temperature');
      buildChart('humidityChart', d.humidityHistory, d.timeHistory, 'humidity');
    })
    .catch(e => console.log(e));
}

updateData();
setInterval(updateData, 1000);
</script>

</body>
</html>
)rawliteral";

String getDateText() {
  time_t now = time(nullptr) + 3600;
  struct tm* timeInfo = localtime(&now);

  if (now < 100000) {
    return "Waiting";
  }

  char dateText[11];
  strftime(dateText, sizeof(dateText), "%d/%m/%Y", timeInfo);
  return String(dateText);
}

String getTimeText() {
  time_t now = time(nullptr) + 3600;
  struct tm* timeInfo = localtime(&now);

  if (now < 100000) {
    return "Waiting";
  }

  char timeText[9];
  strftime(timeText, sizeof(timeText), "%H:%M:%S", timeInfo);
  return String(timeText);
}

String escapeJson(String text) {
  text.replace("\\", "\\\\");
  text.replace("\"", "\\\"");
  text.replace("\n", "");
  text.replace("\r", "");
  return text;
}

void sendPulsesToArduino(int pulses) {
  for (int i = 0; i < pulses; i++) {
    digitalWrite(ARDUINO_COMMAND_PIN, HIGH);
    delay(180);
    digitalWrite(ARDUINO_COMMAND_PIN, LOW);
    delay(250);
  }
}

void addMovementHistory() {
  String entry = getDateText();
  entry += " ";
  entry += getTimeText();
  entry += " - Movement detected<br>";

  movementHistory = entry + movementHistory;

  if (movementHistory.length() > 3000) {
    movementHistory.remove(3000);
  }
}

void addChartSample() {
  temperatureHistory[chartIndex] = temperatureValue;
  humidityHistory[chartIndex] = humidityValue;
  timeHistory[chartIndex] = getTimeText();

  chartIndex++;

  if (chartIndex >= chartSize) {
    chartIndex = 0;
    chartFilled = true;
  }
}

String buildFloatArray(float values[]) {
  String json = "[";
  int count = chartFilled ? chartSize : chartIndex;

  for (int i = 0; i < count; i++) {
    int realIndex = chartFilled ? ((chartIndex + i) % chartSize) : i;

    if (i > 0) {
      json += ",";
    }

    json += String(values[realIndex], 1);
  }

  json += "]";
  return json;
}

String buildStringArray(String values[]) {
  String json = "[";
  int count = chartFilled ? chartSize : chartIndex;

  for (int i = 0; i < count; i++) {
    int realIndex = chartFilled ? ((chartIndex + i) % chartSize) : i;

    if (i > 0) {
      json += ",";
    }

    json += "\"" + escapeJson(values[realIndex]) + "\"";
  }

  json += "]";
  return json;
}

void handleRoot() {
  server.send_P(200, "text/html", MAIN_page);
}

void handleData() {
  String movementText = movementVisible ? "MOVEMENT DETECTED" : "NO MOVEMENT";

  String json = "{";
  json += "\"date\":\"" + getDateText() + "\",";
  json += "\"time\":\"" + getTimeText() + "\",";
  json += "\"temperature\":" + String(temperatureValue, 1) + ",";
  json += "\"humidity\":" + String(humidityValue, 1) + ",";
  json += "\"alarm\":\"" + String(alarmOn ? "ARMED" : "DISARMED") + "\",";
  json += "\"movement\":\"" + movementText + "\",";
  json += "\"history\":\"" + escapeJson(movementHistory) + "\",";
  json += "\"movementHistoryCooldown\":" + String(movementHistoryCooldown) + ",";
  json += "\"temperatureHistory\":" + buildFloatArray(temperatureHistory) + ",";
  json += "\"humidityHistory\":" + buildFloatArray(humidityHistory) + ",";
  json += "\"timeHistory\":" + buildStringArray(timeHistory);
  json += "}";

  server.send(200, "application/json", json);
}

void handleSetMovementInterval() {
  if (server.hasArg("value")) {
    unsigned long newInterval = server.arg("value").toInt();

    if (newInterval >= 10000 && newInterval <= 300000) {
      movementHistoryCooldown = newInterval;
      lastMovementHistoryTime = 0;

      if (newInterval == 10000) {
        sendPulsesToArduino(1);
      } 
      else if (newInterval == 30000) {
        sendPulsesToArduino(2);
      } 
      else if (newInterval == 60000) {
        sendPulsesToArduino(3);
      } 
      else if (newInterval == 300000) {
        sendPulsesToArduino(4);
      }

      Serial.print("Movement interval changed: ");
      Serial.println(movementHistoryCooldown);
    }
  }

  server.send(200, "text/plain", "OK");
}

void handleClearHistory() {
  movementHistory = "";
  server.send(200, "text/plain", "OK");
}

void updateOled() {
  oled.clear();

  oled.setCursor(0, 0);
  oled.print("Date:");
  oled.print(getDateText());

  oled.setCursor(0, 2);
  oled.print("Alarm:");
  oled.print(alarmOn ? "ARMED" : "OFF");

  oled.setCursor(0, 4);
  oled.print("T:");
  oled.print(temperatureValue, 1);
  oled.print("C H:");
  oled.print(humidityValue, 0);
  oled.print("%");

  oled.setCursor(0, 6);
  oled.print("Time:");
  oled.print(getTimeText());
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(MOVE_INPUT, INPUT);
  pinMode(ALARM_INPUT, INPUT);

  pinMode(ARDUINO_COMMAND_PIN, OUTPUT);
  digitalWrite(ARDUINO_COMMAND_PIN, LOW);

  Wire.begin(D2, D1);
  dht.begin();

  oled.begin();
  oled.setFont(u8x8_font_chroma48medium8_r);
  oled.clear();
  oled.drawString(0, 0, "Connecting WiFi");

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  unsigned long startTime = millis();

  while (WiFi.status() != WL_CONNECTED && millis() - startTime < 20000) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("CONNECTED");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("WIFI FAILED");
    WiFi.mode(WIFI_AP);
    WiFi.softAP("SmartSecurityESP", "12345678");
    Serial.println("Backup WiFi started");
    Serial.println("Open: 192.168.4.1");
  }

  configTime(0, 0, "pool.ntp.org", "time.nist.gov");

  server.on("/", handleRoot);
  server.on("/data", handleData);
  server.on("/setMovementInterval", handleSetMovementInterval);
  server.on("/clearHistory", handleClearHistory);

  server.begin();
  Serial.println("WEB SERVER STARTED");

  for (int i = 0; i < chartSize; i++) {
    temperatureHistory[i] = 0.0;
    humidityHistory[i] = 0.0;
    timeHistory[i] = "";
  }

  addChartSample();
  lastChartSample = millis();

  oled.clear();
  oled.drawString(0, 0, "WiFi connected");
  oled.setCursor(0, 2);

  if (WiFi.status() == WL_CONNECTED) {
    oled.print(WiFi.localIP());
  } else {
    oled.print("192.168.4.1");
  }
}

void loop() {
  server.handleClient();

  alarmOn = digitalRead(ALARM_INPUT) == HIGH;
  rawMovementSignal = digitalRead(MOVE_INPUT) == HIGH;

  if (alarmOn && rawMovementSignal) {
    movementVisible = true;
    movementVisibleUntil = millis() + movementVisibleTime;

    if (millis() - lastMovementHistoryTime >= movementHistoryCooldown || lastMovementHistoryTime == 0) {
      addMovementHistory();
      lastMovementHistoryTime = millis();

      Serial.println("Movement saved to web history");
    }
  }

  if (movementVisible && millis() > movementVisibleUntil) {
    movementVisible = false;
  }

  if (millis() - lastSensorRead >= sensorInterval) {
    lastSensorRead = millis();

    float h = dht.readHumidity();
    float t = dht.readTemperature();

    if (!isnan(h) && !isnan(t)) {
      humidityValue = h;
      temperatureValue = t;
    }
  }

  if (millis() - lastChartSample >= chartInterval) {
    lastChartSample = millis();
    addChartSample();
  }

  if (millis() - lastOledUpdate >= oledInterval) {
    lastOledUpdate = millis();
    updateOled();
  }
}
