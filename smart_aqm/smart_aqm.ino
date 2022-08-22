#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h> 
#include "DHT.h"
#include "MQ135.h"


// set the PORT for the webserver
ESP8266WebServer server(80);

// Wifi detail
const char* ssid = "SKYHEJQH";
const char* password = "kS6cpRHy8qmu";

// inialise temperature and humidity pin
const int temp_hum_pin = D1;

//initialise air quality pin and set variable to store ppm value
const int air_quality_pin = A0;
float ppm = 0;

// Initialise variables to store the temperature and humidity values
int temperature = 0;
int humidity = 0;

// Initialise the DHT11 component
DHT dht(temp_hum_pin, DHT11);

// initialise MQ135 gas sensor and calibrated rzero value
float rzero = 6.25;
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
  
  Serial.println("Server listening");

  //Start the dht component reading
  dht.begin();
}

void loop() {
  // put your main code here, to run repeatedly:

  Serial.println("Server is running");

  // handling of incoming client requests
  server.handleClient();
  
  //read temperature and humidity value
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();

  // needed to initially get the rzeo vaule to calibrate the sensor. 
  //float rzero = gasSensor.getRZero();
  //Serial.println (rzero);

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

//void jsonData() {
//  // Add JSON request data
//  doc["Content-Type"] = "application/json";
//  doc["Status"] = 200;
//
//  JsonObject tempSensor = doc.createNestedObject("temperature_sensor");
//  tempSensor["value"] = temperature;
//  tempSensor["unit"] = "°C";
//
//  JsonObject humiditySensor = doc.createNestedObject("humidity_sensor");
//  humiditySensor["value"] = humidity;
//  humiditySensor["unit"] = "%";
//
//  JsonObject airQualSensor = doc.createNestedObject("air_quality_sensor");
//  airQualSensor["value"] = ppm;
//  airQualSensor["unit"] = "ppm";
//}

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

  server.send(200, "application/json", jsonStr);
  
}

void get_index() {

  String html = "<!DOCTYPE html> <html> ";
  html += "<head><meta http-equiv=\"refresh\" content=\"2\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"></head>";
  html += "<body><h1>The Smart Air Quality Monitor Dashboard</h1>";
  html += "<p> Welcome to the smart air quality monitor dashboard</p>";
  html += "<div><p><strong>The temperature reading is: ";
  html += temperature;
  html += " degrees.</strong></p>";
  html += "<div><p><strong>The humidity reading is: ";
  html += humidity;
  html += " %.</strong></p>";
  html += "<div><p><strong>The air quality reading is: ";
  html += ppm;
  html += "</strong></p></div>";

  html += "</body></html>";

  // Print a welcome message on the index page
  server.send(200, "text/html", html);
}
