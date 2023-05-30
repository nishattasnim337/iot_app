#include "DHT.h"
#define DHTPIN 5
#define DHTTYPE DHT11

#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#define ONE_WIRE_BUS 4
OneWire oneWire(ONE_WIRE_BUS);// setup of instance to communicate with one wire sensor
DallasTemperature sensors(&oneWire);// pass one wire ref to dallas temp library

int pulseSensorPin = A0;

const int normalThreshold = 500;  // Adjust these threshold values based on your sensor and requirements
const int highThreshold = 850;
const int lowThreshold = 300;


#if defined(ESP32)
  #include <WiFi.h>
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
#endif
#include <Firebase_ESP_Client.h>

DHT dht(DHTPIN, DHTTYPE);

//Provide the token generation process info.
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

// Insert your network credentials
#define WIFI_SSID "Library"
#define WIFI_PASSWORD "LIB&*775$"

// Insert Firebase project API Key
#define API_KEY "AIzaSyB5vt4Qj6BXDomIn4jmFs_oRRj8e_3adEU"

// Insert RTDB URLefine the RTDB URL */
#define DATABASE_URL "https://health-monitoring-system-b5496-default-rtdb.firebaseio.com/" 

//Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

//unsigned long sendDataPrevMillis = 0;
//int count = 0;
bool signupOK = false;


// array for check duplicate value

float h,t,bt,pr;  // Variables to store the sensor readings
float prevValues[4] = {-1,-1,-1,-1};  // Array to store the previous three readings

void setup(){
  pinMode(DHTPIN, INPUT);
  dht.begin();
  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Sign up */
  if (Firebase.signUp(&config, &auth, "", "")){
    Serial.println("ok");
    signupOK = true;
  }
  else{
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h
  
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

void loop(){
 delay(1000);
  float h = dht.readHumidity();

  float t = dht.readTemperature();

// body temprature
   sensors.requestTemperatures();
  float bt = sensors.getTempFByIndex(0);
  float ct = sensors.getTempCByIndex(0);
  //pulse rate.........

   float pr=analogRead(pulseSensorPin);
 
 // duplicate value check using array//////////////////////////////////////////

if(h == prevValues[0] && t == prevValues[1] && bt == prevValues[2] && pr == prevValues[3]){
  Serial.print("Duplicate value found ");
}
else {
    Serial.println("New values sent.");
    // Update the previous values array
    prevValues[0] = h;
    prevValues[1] = t;
    prevValues[2] = bt;
    prevValues[3]=pr;
}
 
  if (Firebase.ready() && signupOK ) {
    
    
    if (Firebase.RTDB.setFloat(&fbdo, "DHT/humidity",h)){
//      Serial.println("PASSED");
       Serial.print("Humidity: ");
       Serial.println(h);
      
    }
    else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
    
    
    // Write an Float number on the database path test/float
    if (Firebase.RTDB.setFloat(&fbdo, "DHT/Roomtemperature", t)){
//      Serial.println("PASSED");
       Serial.print("Temperature: ");
       Serial.println(t);
    }
    else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }

    
    //send body temprature...............................

    if (Firebase.RTDB.setFloat(&fbdo, "DHT/bodyTemp",bt)){
       Serial.print("BodyTemperature: ");
       Serial.println(bt);

    }
    else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
 
    }

   //pulse rate

   if ((pr >= highThreshold)&& (Firebase.RTDB.setString(&fbdo, "DHT/pulseRate","HIGH"))) {
    // High pulse rate
    Serial.println(pr);
    Serial.println("High pulse rate");
    // Perform additional actions or calculations as needed
  } else if ((pr >= lowThreshold)&& (Firebase.RTDB.setString(&fbdo, "DHT/pulseRate","LOW")))  {
    Serial.println(pr);
    // Low pulse rate
    Serial.println("Low pulse rate");
    // Perform additional actions or calculations as needed
  } else {
    // Normal pulse rate
    //Firebase.RTDB.setString(&fbdo, "DHT/pulseRate","Normal");
    Serial.println("Normal pulse rate");
    // Perform additional actions or calculations as needed
  }

  }
  Serial.println("______________________________");
 Serial.println(millis());
 delay(1000);
}
