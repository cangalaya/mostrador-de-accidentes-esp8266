#include <Arduino.h>
///////////// DEPENDENCIAS ///////////
#include <time.h>
#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>             // Protocolo UDP (transferencia de datos rapida, pero no segura)
#include <Firebase_ESP_Client.h> //#include <Firebase_ESP_Client.h>
#include <EEPROM.h>              // update de memorias fash
#include <Separador.h>



// NTP Servers:
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

// Variables to save date and time
String formattedDate;
String dayStamp;
String timeStamp;



// Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

FirebaseJson jsonConfigWifi;
FirebaseJson jsonConfigUdp;
FirebaseJson jsonConfigTarjet;
FirebaseJson jsonData;


#include "config.h" // Configurar datos de la red
#include "ESP8266_Utils.hpp"

//////////// variables FIREBASE   ////////////////

// Provide the token generation process info.
#include <addons/TokenHelper.h>

// Provide the RTDB payload printing info and other helper functions.
#include <addons/RTDBHelper.h>

/* 2. Define the API Key */
#define API_KEY "AIzaSyACwNdd8nTeRCLSr6tFyZLQ-jquw8ljKa8"

/* 3. Define the RTDB URL */
#define DATABASE_URL "prueba-2-e4543-default-rtdb.firebaseio.com" //<databaseName>.firebaseio.com or <databaseName>.<region>.firebasedatabase.app

/* 3. Define the project ID */
#define FIREBASE_PROJECT_ID "prueba-2-e4543"

/* 4. Define the user Email and password that alreadey registerd or added in your project */
#define USER_EMAIL "comedor1@tasa-callao.com"
#define USER_PASSWORD "comedor1"



void setup()
{
  Serial.begin(9600);
  ConnectWiFi_STA();
  timeClient.begin();
  timeClient.setTimeOffset(-18000); // -5 * 3600
}
int splitT = 0;
void loop()
{
  if(!timeClient.update()) {
    timeClient.forceUpdate();
  }
  // The formattedDate comes with the following format:
  // 2018-05-28T16:00:13Z
  // We need to extract date and time
  formattedDate = timeClient.getFormattedDate();
  Serial.println(formattedDate);

  // Extract date
  splitT = formattedDate.indexOf("T");
  dayStamp = formattedDate.substring(0, splitT);
  Serial.print("DATE: ");
  Serial.println(dayStamp);
  // Extract time
  timeStamp = formattedDate.substring(splitT+1, formattedDate.length()-1);
  Serial.print("HOUR: ");
  Serial.println(timeStamp);
  delay(1000);
}