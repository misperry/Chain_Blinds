/****************************************************
* Project Name:  Rotery Blinds
* Project File:  Rotery_Blinds.ino
*
* Description:  This is the code that will be used to 
* control the rotery blinds using the ESP8266-01 Board.
* We will be using Pins 0 and 2 for motor power control
* and direction control respectivly.  We will also
* set the serial mode to be transmit only so we can
* use the RX (GIPO 3) pin for our INPUT to read the 
* Rotery encoder on the motor for possition control.
*
* Author:  M. Sperry
* Date:  11/9/2018
*/
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define DIREC_1 2
#define PWR_RLY 0
#define ENC 3

#define ENC_LIM 5500
#define ENC_OFF 800
#define WDT_LIM 100

// Update these with values suitable for your network.
const char* ssid = "";
const char* password = "";
const char* mqtt_server = "";  //IP address of your Home assistant server if hosting on HASS or the MQTT server you have
#define mqtt_user "homeassistant" //enter your MQTT username
#define mqtt_password "" //enter your password

WiFiClient espClient;
PubSubClient client(espClient);

String switch1;
String strTopic;
String strPayload;
bool Position = true;

void setup_wifi() {
  delay(2000);

  // We start by connecting to a WiFi network
 
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
 
  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  
}

void callback(char* topic, byte* payload, unsigned int length) {
  payload[length] = '\0';
  strTopic = String((char*)topic);
  long ENC_Count = 0 , WDT_Count = 0;
  if(strTopic == "ha/rblind")
    {
    switch1 = String((char*)payload);
    if(switch1 == "OFF")
      {
        Serial.println("FRWD Pressed");

        ENC_Count = 0;
        WDT_Count = 0;
        
        //close in relay to actuate in the forward direction
        while (ENC_Count != ENC_LIM)
        {
          digitalWrite(DIREC_1,HIGH);
          digitalWrite(PWR_RLY,HIGH);
          if (!digitalRead(ENC))
          {
            ENC_Count++;
            Serial.println(ENC_Count);
            while(!digitalRead(ENC))
            {
              delay(1);
            }
          }
          else if(ENC_Count == 0)  //watch dog incase motor stalls
          {
            WDT_Count++;
            if (WDT_Count > WDT_LIM)
            {
              digitalWrite(PWR_RLY,LOW);
              digitalWrite(DIREC_1,LOW);
              Serial.println("WDT Timeout");
              Position = true;
              break;
            }
            delay(1);
          }
        }
        digitalWrite(PWR_RLY,LOW);
        digitalWrite(DIREC_1,LOW);
        Position = false;
        delay(100);
      }
    else if(switch1 =="ON")
      {
        Serial.println("REVR Pressed");

        ENC_Count = 0;
        WDT_Count = 0;
        
        //close in relay to actuate in the reverse direction
        while (ENC_Count != ENC_LIM + ENC_OFF)
        {
          digitalWrite(DIREC_1, LOW);
          digitalWrite(PWR_RLY,HIGH);
          if (!digitalRead(ENC))
          {
            ENC_Count++;
            Serial.println(ENC_Count);
            while (!digitalRead(ENC))
            {
              delay(1);
            }
          }
          else if (ENC_Count == 0)//watch dog incase motor stalls
          {
            WDT_Count++;
            if (WDT_Count > WDT_LIM)
            {
              digitalWrite(PWR_RLY,LOW);
              Serial.println("WDT Timeout");
              Position = false;
              break;
            }
            delay(1);
          }
        }
        digitalWrite(PWR_RLY,LOW);
        Position = true;
        delay(100);
      }
    }
}
 
 
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("arduinoClient", mqtt_user, mqtt_password)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.subscribe("ha/#");
    } 
    else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
 
void setup()
{

  //Sets the serial port to function in XMIT only so 
  //RX can be used as GPIO
  Serial.begin(115200,SERIAL_8N1,SERIAL_TX_ONLY);
  
  setup_wifi(); 
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  pinMode(ENC, INPUT);
  pinMode(PWR_RLY,OUTPUT);
  pinMode(DIREC_1,OUTPUT);

  digitalWrite(PWR_RLY,LOW);
  digitalWrite(DIREC_1,LOW);

}
 
void loop()
{
  if (!client.connected()) {
    reconnect();
  }
  
  client.loop();
  delay(100);
}
