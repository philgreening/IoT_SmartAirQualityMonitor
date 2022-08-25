#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h> 
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Servo.h>


#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

//Adafruit_SSD1306 display(-1);

#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// set the PORT for the webserver`
ESP8266WebServer server(80);

WiFiClient client;
HTTPClient http;

// Wifi detail
const char* ssid = "SKYHEJQH";
const char* password = "kS6cpRHy8qmu";

// Allocate menmory to the JSON document
//DynamicJsonDocument doc(2048);
//StaticJsonDocument<384> doc;

// Initialise varaible to store temp, humidty and ppm values
int temperature_sensor_value = 0;
const char* temperature_sensor_unit = "";

int humidity_sensor_value = 0;
const char* humidity_sensor_unit = "";

int air_quality_sensor_value = 0;
const char* air_quality_sensor_unit = "";

// Initialise the buzzer pin and a variable to check if already signaled
const int buzzer_pin = D5;
bool signaled = false;

// initialise the RGB pins

const int red_led_pin = D8;
const int green_led_pin = D7;
const int blue_led_pin = D6;

// create servo object to control a servo
Servo myservo;
//int servoAngle = 0;


void setup() {

   // LEDs as output
  pinMode(red_led_pin, OUTPUT);
  pinMode(green_led_pin, OUTPUT);
  pinMode(blue_led_pin, OUTPUT);

  pinMode(buzzer_pin, OUTPUT);
  // No sound initially
  digitalWrite(buzzer_pin, LOW);
//  tone(buzzer_pin, 1000);

  // Connect to WiFi
  WiFi.begin(ssid, password);

  //Start the serial to debug values
  Serial.begin(115200);

    while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Waiting to connect..");
  }

  // Print the board IP address
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  server.begin(); // Start the server
  Serial.println("Server listening");

   // Attach the servo on pin D3 to the servo object
  myservo.attach(D3);

  // Send request
  http.useHTTP10(true);

    // initialize with the I2C addr 0x3C
 display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);  
  // Clear the buffer.
  delay(2000);
  display.clearDisplay();
  display.setTextColor(WHITE);        
}

void loop() {
  //Serial.println("Server is running");

  // handling of incoming client requests
  server.handleClient();
  
  get_values();  

  openWindow();

  airqualitySignal();

  displayReadings();
  
}

void get_values() {

  http.begin(client, "http://192.168.0.27/data");
  http.GET();
  
  DynamicJsonDocument doc(2048);

  deserializeJson(doc, http.getStream());

  temperature_sensor_value = doc["temperature_sensor"]["value"];
  temperature_sensor_unit = doc["temperature_sensor"]["unit"];
  Serial.println(String(temperature_sensor_value) + String(temperature_sensor_unit));

  humidity_sensor_value = doc["humidity_sensor"]["value"];
  humidity_sensor_unit = doc["humidity_sensor"]["unit"];
  Serial.println(String(humidity_sensor_value) + String(humidity_sensor_unit));

  air_quality_sensor_value = doc["air_quality_sensor"]["value"];
  air_quality_sensor_unit = doc["air_quality_sensor"]["unit"];
  Serial.println(String(air_quality_sensor_value) + String(air_quality_sensor_unit));

  http.end();
}

// Utility function to control the LED
void rgbLed(int red_led_amount, int green_led_amount, int blue_light_amount){

  analogWrite(red_led_pin, red_led_amount);
  analogWrite(green_led_pin, green_led_amount);
  analogWrite(blue_led_pin, blue_light_amount);
}

void openWindow (){
  if(temperature_sensor_value >= 30){
        myservo.write(90);
  }else{
        myservo.write(0);
  }
}

void airqualitySignal () {
  if(air_quality_sensor_value >= 750 && air_quality_sensor_value < 1200){
    rgbLed(0,0,255);
  }else if(air_quality_sensor_value >= 1200 ){
    rgbLed(255,0,0);
    tone(buzzer_pin, 1000);
    delay(2000);
    noTone(buzzer_pin);
  }else{
    rgbLed(0,255,0);
  }
}

void displayReadings () {
  //clear display
  display.clearDisplay();

  display.setTextSize(1);

  //display temperature
  display.setCursor(0,20);
  display.print("Temperature: " );
  display.print(temperature_sensor_value);
  display.cp437(true);
  display.write(167);
  display.print("C");

  //display humidity
  display.setCursor(0,35);
  display.print("Humidity: " );
  display.print(humidity_sensor_value);
  display.print(humidity_sensor_unit);

  //display air quality
  display.setCursor(0,50);
  display.print("Air quality: " );
  display.print(air_quality_sensor_value);
  display.print(air_quality_sensor_unit);

  display.display();
}
