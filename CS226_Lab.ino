#include "UbidotsESPMQTT.h"
// Please note that, above header doesn't mean it is official UbidotsESPMQTT Library.
// If you want to run this locally, Download my fork as zip file and Add the zip library
// Download Link https://github.com/CITIZENDOT/ubidots-mqtt-esp/archive/refs/heads/master.zip
#include <DHT.h>
#include <stdio.h>

/****************************************
   Define Constants
 ****************************************/
#define BUZZER D7
#define LDR A0
#define TEMP D5
#define RELAY D4
#define DHTTYPE DHT11                                 // DHT 11
#define DHTPIN D5

#define WIFISSID "My A"                               // Put your WifiSSID here
#define PASSWORD "appajichintimi"                     // Put your wifi password here
#define TOKEN "BBFF-YSfLdMkT52zPM7GbMCFGank2tJXVWj"   // Ubidots Token 
#define VARIABLE_TO_SUBSCRIBE "home_bulb"
#define DEVICE_LABEL "esp8266"


Ubidots client(TOKEN);
DHT dht = DHT(DHTPIN, DHTTYPE);
bool isMailSent = false;

/****************************************
   Auxiliary Functions
 ****************************************/
void callback(char* topic, byte* payload, unsigned int length) {
  // Controlling Bulb with Website
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  if ((char)payload[0] == '0')
  {
    Serial.println("Since, Output is 0, I'm going to turn off LED");
    digitalWrite(RELAY, LOW);
  }
  else if ((char)payload[0] == '1')
  {
    Serial.println("Since, Output is 1, I'm going to turn on LED");
    digitalWrite(RELAY, HIGH);
  }
  else {
    Serial.print(" Ubidots Value is ");
    Serial.println((char)payload[0]);
  }
}

/****************************************
   Main Functions
 ****************************************/

void setup() {
  Serial.begin(115200);

  pinMode(BUZZER, OUTPUT);
  pinMode(RELAY, OUTPUT);
  pinMode(TEMP, INPUT);

  client.ubidotsSetBroker("industrial.api.ubidots.com");
  client.setDebug(true);
  client.wifiConnection(WIFISSID, PASSWORD);
  client.begin(callback);
  client.ubidotsSubscribe(DEVICE_LABEL, VARIABLE_TO_SUBSCRIBE);

  if (client.connected())
  {
    digitalWrite(BUZZER, HIGH);
    delay(200);
    digitalWrite(BUZZER, LOW);
  }

  dht.begin();
}

void loop() {
  int LDRValue = analogRead(LDR);
  Serial.println(LDRValue);
  if (LDRValue < 400) {
    // Light is falling on LDR
    digitalWrite(BUZZER, LOW);
    if (isMailSent) {
      isMailSent = false;
    }
  }
  else {
    digitalWrite(BUZZER, HIGH);
    // Interuppted
    if (!isMailSent) {
      String mailSubject = "Hi,\nI am your personal asssistant at Home, monitoring LDR Sensor.\n";
      mailSubject += "It looks like someone interuppted the Signal.\n";
      mailSubject += "Could you check what happened?\n";

      // Reading Temperature and Humidity
      mailSubject += "For your Information, The current Room temperature is " + String(dht.readTemperature()) + " Degree Celsius" + "\n";
      mailSubject += "The current Room Relative Humidity is " + String(dht.readHumidity()) + " %\n";
      Serial.print("Starting to Send Mail...");
      isMailSent = client.sendMail("mail.smtp2go.com", 2525, "Q0lUSVpFTkRPVA==", "S3VYTFB4SkQ0YTZP",
                                   "appaji12368@gmail.com", "appaji12368@gmail.com",
                                   "Mail from NodeMCU", mailSubject);
      if (isMailSent) {
        Serial.println("Mail Sent Successfully!");
      }
    }
  }

  if (!client.connected()) {
    client.reconnect();
    client.ubidotsSubscribe(DEVICE_LABEL, VARIABLE_TO_SUBSCRIBE);
  }
  client.loop();
  delay(100);
}
