#include <SoftwareSerial.h>
// IMPORT ALL REQUIRED LIBRARIES

#ifndef ARDUINOJSON_H
#include <ArduinoJson.h>
#endif

#ifndef STDLIB_H
#include <stdlib.h>
#endif

#ifndef STDIO_H
#include <stdio.h>
#endif

#ifndef ARDUINO_H
#include <Arduino.h>
#endif

#include <math.h>


//**********ENTER IP ADDRESS OF SERVER******************//
#define HOST_IP     "192.168.86.21"       // REPLACE WITH IP ADDRESS OF SERVER ( IP ADDRESS OF COMPUTER THE BACKEND IS RUNNING ON) 
#define HOST_PORT   "8080"            // REPLACE WITH SERVER PORT (BACKEND FLASK API PORT)
#define route       "api/update"      // LEAVE UNCHANGED 
#define idNumber    "620156694"       // REPLACE WITH YOUR ID NUMBER 

// WIFI CREDENTIALS
#define SSID        "WPS"             // "REPLACE WITH YOUR WIFI's SSID"
#define password    "W0LM3R$WP$"      // "REPLACE WITH YOUR WiFi's PASSWORD"

// #define SSID        "MonaConnect"     // "REPLACE WITH YOUR WIFI's SSID"
// #define password    ""                // "REPLACE WITH YOUR WiFi's PASSWORD"

#define stay        100
#define ARDUINOJSON_USE_DOUBLE 1

//**********PIN DEFINITIONS******************//

 
#define espRX         10
#define espTX         11
#define espTimeout_ms 300


/* VARIABLE DECLARATIONS: */
int trig                           = 2;    // TriggerPin
int echo                           = 3;    // EchoPin
long duration, radar_val           = 0;
double water_height                = 0;
double water_reserve               = 0;
double reserve_percentage          = 0;
double MaxWaterLevel               = 77.763; // inches


/* Declare your functions below */
double calc_water_height(double );
double calc_volume(double );
double toPercentage(double, const );
 

SoftwareSerial esp(espRX, espTX); 
 

void setup(){
  Serial.begin(115200);
  // Configure GPIO pins here
  pinMode(trig, OUTPUT);
  pinMode(echo, INPUT);

  espInit();
}

void loop(){ 
  // emit ultrasonic waves:
  digitalWrite(trig, LOW);
  delayMicroseconds(5);
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);

  // read ultrasonic signals:
  pinMode(echo, INPUT);
  duration = pulseIn(echo, HIGH);

  // calc. "radar_val" reading in inches:
  radar_val = (duration/2) / 74;

  // calc. water_height (inches):
  water_height = calc_water_height(radar_val);

  // calc. water_reserve in gallons:
  water_reserve = calc_volume(water_height);

  // conv. water_reserves to percent:
  reserve_percentage = toPercentage(water_reserve);

  // Create JSon object
  StaticJsonDocument<1000> doc;
  char message[1100] = { 0 };

  // send updates with schema ‘{"id": "student_id", "type": "ultrasonic", "radar": 0, "waterheight": 0, "reserve": 0, "percentage": 0}’
  // Add key:value pairs to JSon object based on above schema
  doc["id"]           = "620156694";
  doc["type"]         = "ultasonic";
  doc["radar"]        = radar_val;
  doc["waterheight"]  = water_height;
  doc["reserve"]      = water_reserve;
  doc["percentage"]   = reserve_percentage;

  serializeJson(doc, message);
  espUpdate(message);
  delay(450);
}

 
void espSend(char command[] ){   
    esp.print(command); // send the read character to the esp    
    while(esp.available()){ Serial.println(esp.readString());}    
}


void espUpdate(char mssg[]){
  char espCommandString[50] = {0};
  char post[290]            = {0};

  snprintf(espCommandString, sizeof(espCommandString),"AT+CIPSTART=\"TCP\",\"%s\",%s\r\n",HOST_IP,HOST_PORT); 
  espSend(espCommandString);    //starts the connection to the server
  delay(stay);

  // GET REQUEST 
  // snprintf(post,sizeof(post),"GET /%s HTTP/1.1\r\nHost: %s\r\nContent-Type: application/x-www-form-urlencoded\r\nContent-Length: %d\r\n\r\n%s\r\n\r\n",route,HOST_IP,strlen(mssg),mssg);

  // POST REQUEST
  snprintf(post,sizeof(post),"POST /%s HTTP/1.1\r\nHost: %s\r\nContent-Type: application/json\r\nContent-Length: %d\r\n\r\n%s\r\n\r\n",route,HOST_IP,strlen(mssg),mssg);
  
  snprintf(espCommandString, sizeof(espCommandString),"AT+CIPSEND=%d\r\n", strlen(post));
  espSend(espCommandString);    //sends post length
  delay(stay);
  Serial.println(post);
  espSend(post);                //sends POST request with the parameters 
  delay(stay);
  espSend("AT+CIPCLOSE\r\n");   //closes server connection
}

void espInit(){
  char connection[100] = {0};
  esp.begin(115200);
  Serial.println("Initiallizing");
  esp.println("AT");
  delay(1000);
  esp.println("AT+CWMODE=1");
  delay(1000);
  while(esp.available()){ Serial.println(esp.readString());}

  snprintf(connection, sizeof(connection),"AT+CWJAP=\"%s\",\"%s\"\r\n",SSID,password);
  esp.print(connection);

  delay(3000);  //gives ESP some time to get IP

  if(esp.available()){   Serial.print(esp.readString());}
   
  Serial.println("\nFinish Initializing"); 
}

//***** Design and implement all util functions below ******
// water height function:
double calc_water_height(double radarVal){
  return 94.5 - radarVal;
}

// water reserve function:
double calc_volume(double waterHeight){
  return (3.14159265 * pow(30.75,2) * waterHeight)/231.0;
}

// toPercentage:
double toPercentage(double waterReserve){
  return (waterReserve/MaxWaterLevel)*100;
}