#include <Arduino.h>
///////////// DEPENDENCIAS ///////////
#include <time.h>
#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>             // Protocolo UDP (transferencia de datos rapida, pero no segura)
#include <Firebase_ESP_Client.h> //#include <Firebase_ESP_Client.h>
#include <EEPROM.h>              // update de memorias fash
#include <Separador.h>
#include <EEPROM.h>              // update de memorias fash

Separador s; // instancia para separar string

// NTP Servers:
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

// Variables to save date and time
String formattedDate;
String dayStamp;
String timeStamp;

int yearNtp = 0;
int montNtp = 0;
int dayNtp = 0;

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
#define API_KEY "AIzaSyC1quVUqw2XhlUumh6cbXHwlTx0KSSRapI"

/* 3. Define the RTDB URL */
#define DATABASE_URL "display-accidentes-komatsu-default-rtdb.firebaseio.com" //<databaseName>.firebaseio.com or <databaseName>.<region>.firebasedatabase.app

/* 3. Define the project ID */
#define FIREBASE_PROJECT_ID "display-accidentes-komatsu"

/* 4. Define the user Email and password that alreadey registerd or added in your project */
#define USER_EMAIL "display1@komatsu.com"
#define USER_PASSWORD "komatsu"

/////////// ROUTINE PARA ACCTUALIZAR HORA ////////////
unsigned long int prevMillis = 0;
int splitT = 0;
void ntpTimeUpdate()
{
  if (millis() < 500)
    prevMillis = 0; // condicional de protección al desbordamiento

  if ((millis() - prevMillis) > 1000)   // actualizamos he imprimimos cada 1s
  { // actualización time cada 1s
    prevMillis = millis();
    if (!timeClient.update())
    {
      timeClient.forceUpdate();
    }

    // The formattedDate comes with the following format:
    // 2018-05-28T16:00:13Z
    // We need to extract date and time
    formattedDate = timeClient.getFormattedDate();  // example 2018-05-28T16:00:13Z

    // Extract date
    splitT = formattedDate.indexOf("T");
    dayStamp = formattedDate.substring(0, splitT) + '-';  // example 2018-05-28-
    yearNtp = s.separa(dayStamp,'-',0 ).toInt();
    montNtp = s.separa(dayStamp,'-',1 ).toInt();
    dayNtp = s.separa(dayStamp, '-',2).toInt();

    // Serial.println(dayStamp);
    // Serial.println("Y:" + String(yearNtp) + " M:" + String(montNtp) + " D:" + String(dayNtp));
    Serial.println('/'+ String(montNtp) + '/' + readStringFromEEPROM(0));   // enviar data a display

    // Extract time
    // timeStamp = formattedDate.substring(splitT + 1, formattedDate.length() - 1);
    // Serial.print("HOUR: ");
    // Serial.println(timeStamp);
  }
}

void setup()
{
  Serial.begin(9600);
  EEPROM.begin(260);
  ConnectWiFi_STA();
  timeClient.begin();
  timeClient.setTimeOffset(-18000); // -5 * 3600

  // -------------- FIREBASE -------------------------
  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the user sign in credentials */
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h

  // Or use legacy authenticate method
  // config.database_url = DATABASE_URL;
  // config.signer.tokens.legacy_token = "<database secret>";

  Firebase.begin(&config, &auth);

  // Comment or pass false value when WiFi reconnection will control by your code or third party library
  Firebase.reconnectWiFi(true);

  // Optional, use classic HTTP GET and POST requests.
  // This option allows get and delete functions (PUT and DELETE HTTP requests) works for
  // device connected behind the Firewall that allows only GET and POST requests.
  Firebase.RTDB.enableClassicRequest(&fbdo, true);

  // ------ EPROM INIT ----
  // condicional puesto para validar que los datos esten escritos en firebase
  if (Firebase.ready())
  {
    if (String("null") == String(Firebase.RTDB.getString(&fbdo, path_config + "/" + "wifi" + "/" + "ssid" + "/") ? fbdo.to<const char *>() : fbdo.errorReason().c_str()))
    {
      Serial.println(">>> Estableciendo datos por defecto en Firebase");
      jsonConfigDataSetFirstStart(); // si  no estan escritos primero crea los datos en la nube
      Serial.println("<<< Grabando datos en memoria");
      setCofigEprom(); // luego los lee y guarda en eeprom
    }
    else
    {
      Serial.println("<<< Lectura de Data en Firebase y guardado en la EEPROM");
      setCofigEprom();     // si ya existen los datos, primero lee y guarda el la eeprom
      jsonConfigDataSet(); // escribe los datos los nuevos datos en la nuebe
    }
  }
}


void loop()
{
  ntpTimeUpdate();
  actualizarConfigFlash();
}