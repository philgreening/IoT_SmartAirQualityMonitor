#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h> 
#include <ArduinoJson.h> 
#include "DHT.h"
#include "MQ135.h"
#include <Ticker.h>


// set the PORT for the webserver
ESP8266WebServer server(80);

// add a websocket to the server
WebSocketsServer webSocket = WebSocketsServer(81);

// Wifi detail
const char* ssid = "SKYHEJQH";
const char* password = "kS6cpRHy8qmu";

Ticker timer;

// inialise temperature and humidity pin
const int temp_hum_pin = D1;

//initialise air quality pin and set variable to store ppm value
const int air_quality_pin = A0;
int ppm = 0;

// Initialise variables to store the temperature and humidity values
int temperature = 0;
int humidity = 0;

// Initialise the DHT11 component
DHT dht(temp_hum_pin, DHT11);

// initialise MQ135 gas sensor and calibrated rzero value
float rzero = 3.3;
MQ135 gasSensor = MQ135(air_quality_pin, rzero);

// Allocate menmory to the JSON document
//DynamicJsonDocument doc(2048);
//StaticJsonDocument<384> doc;


void setup() {
  // Connect to WiFi
  WiFi.begin(ssid, password);

  //set up pins as input
  pinMode(temp_hum_pin, INPUT);
  pinMode(air_quality_pin, INPUT);

  //Start the serial to debug values
  Serial.begin(9600);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Waiting to connect..");
  }

  // Print the board IP address
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  setup_routes();

  webSocket.begin();
  webSocket.onEvent(webSocketsEvent);
  
  Serial.println("Server listening");

  //Start the dht component reading
  dht.begin();
  timer.attach(5, get_json);
  

    // needed to initially get the rzeo vaule to calibrate the sensor. 
  rzero = gasSensor.getRZero();
//  Serial.println (rzero);
}

void loop() {
  // put your main code here, to run repeatedly:

  Serial.println("Server is running");

  webSocket.loop();
  // handling of incoming client requests
  server.handleClient();
  
  //read temperature and humidity value
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();

  //read air quality value
  ppm = gasSensor.getPPM();

}

void setup_routes() {
  server.on("/", get_index); // Get the index page on root route

  // Get the JSON sensor data on each sensor route 
//  server.on("/temperature", get_json); 
//  server.on("/humidity", get_json);
//  server.on("/air_quality", get_json);
  server.on("/data", get_json);

  server.begin(); // Start the server
}

void webSocketsEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  
}

void get_json() {
  //jsonData();
  DynamicJsonDocument doc(2048);

  doc["Content-Type"] = "application/json";
  doc["Status"] = 200;

  JsonObject tempSensor = doc.createNestedObject("temperature_sensor");
  tempSensor["value"] = temperature;
  tempSensor["unit"] = "°C";

  JsonObject humiditySensor = doc.createNestedObject("humidity_sensor");
  humiditySensor["value"] = humidity;
  humiditySensor["unit"] = "%";

  JsonObject airQualSensor = doc.createNestedObject("air_quality_sensor");
  airQualSensor["value"] = ppm;
  airQualSensor["unit"] = "ppm";
  

  String jsonStr;
  serializeJsonPretty(doc, jsonStr);
  
  webSocket.broadcastTXT(jsonStr.c_str(), jsonStr.length());
  server.send(200, "application/json", jsonStr);  
}

void get_index() {

  String html = "<!DOCTYPE html> <html> ";
  html += "<head>";
  html += "<meta><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">";
  html += "<script src='https://cdn.jsdelivr.net/npm/chart.js'></script>";
  html += "</head>";
  html += "<body onload='javascript:init()'><h1>The Smart Air Quality Monitor Dashboard</h1>";
  html += "<p> Welcome to the smart air quality monitor dashboard</p>";
  html += "<div><p id='temperature'></p>";
  html += "<div><canvas id='tempChart' width='400' height='400'></canvas></div>";
  html += "<div><p id='humidity'></p>";
  html += "<div><canvas id='humChart' width='400' height='400'></canvas></div>";
  html += "<div><p id='air_quality'></p>";
  html += "<div><canvas id='airQualChart' width='400' height='400'></canvas></div>";

  html += "<script type='text/javascript'>";
  
  html += "var websocket, tempDataPlot, humDataPlot, airQualDataPlot;";
  html += "const maxDataPoints =20;";
  html += "function addData(label, data){";
  html += "if(tempDataPlot.data.labels.length > maxDataPoints) removeData();"; 
  html += "console.log('addData',data, 'label: ', label);";
  html += "console.log('tempdataplot: ', tempDataPlot.data);";
  html += "tempDataPlot.data.labels.push(label);";
  html += "tempDataPlot.data.datasets[0].data.push(data.temperature_sensor.value);";
  html += "tempDataPlot.update();";
  html += "humDataPlot.data.labels.push(label);";
  html += "humDataPlot.data.datasets[0].data.push(data.humidity_sensor.value);";
  html += "humDataPlot.update();";
  html += "airQualDataPlot.data.labels.push(label);";
  html += "airQualDataPlot.data.datasets[0].data.push(data.air_quality_sensor.value);";
  html += "airQualDataPlot.update();}";
  
  html += "function removeData(){";
  html += "tempDataPlot.data.labels.shift();";
  html += "tempDataPlot.data.datasets[0].data.shift();";
  html += "humDataPlot.data.labels.shift();";
  html += "humDataPlot.data.datasets[0].data.shift();";
  html += "airQualDataPlot.data.labels.shift();";
  html += "airQualDataPlot.data.datasets[0].data.shift();}";

  html += "function init(){";
  html += "webSocket = new WebSocket('ws://' + window.location.hostname + ':81/');";
  html += "tempDataPlot = new Chart(document.getElementById('tempChart'),{";
  html += "type: 'line',";
  html += "data: {";
  html += "labels: [], datasets: [{ data: [], label: 'Temperature (c)', borderColor: '#3e95cd', fill: false}]";
  html += "}});";
  html += "humDataPlot = new Chart(document.getElementById('humChart'),{";
  html += "type: 'line',";
  html += "data: {";
  html += "labels: [], datasets: [{ data: [], label: 'Humidity (%)', borderColor: '#3e95cd', fill: false}]";
  html += "}});";
  html += "airQualDataPlot = new Chart(document.getElementById('airQualChart'),{";
  html += "type: 'line',";
  html += "data: {";
  html += "labels: [], datasets: [{ data: [], label: 'Air quality (ppm)', borderColor: '#3e95cd', fill: false}]";
  html += "}});";
  
  html += "webSocket.onmessage = function(event){";
  html += "var data = JSON.parse(event.data);console.log('data called:',data.temperature_sensor.value);"; 
  html += "var today = new Date();";
  html += "var hourMinSec = today.getHours() + ':' + today.getMinutes() + ':' + today.getSeconds();";
  html += "addData(hourMinSec, data);";
  html += "let currentTemp = '<strong> The current temperature is : ' + data.temperature_sensor.value + data.temperature_sensor.unit + '</strong>';";
  html += "document.getElementById('temperature').innerHTML = currentTemp;";
  html += "let currentHum = '<strong> The current humidity is : ' + data.humidity_sensor.value + data.humidity_sensor.unit + '</strong>';";
  html += "document.getElementById('humidity').innerHTML = currentHum;";
  html += "let currentAirQual = '<strong> The current air quality is : ' + data.air_quality_sensor.value + data.air_quality_sensor.unit + '</strong>';";
  html += "document.getElementById('air_quality').innerHTML = currentAirQual;";
  html += "}}";
  
  html += "</Script>";

  html += "</body></html>";

  // Print a welcome message on the index page
  server.send(200, "text/html", html);
}
