/*
 *  This code is intended to be used in the distributed alarm system modules 
 *  
 *  Copyright 2020 Kjell Kernen
 *  
 *  The modules are built using ESP8266 controllers on Lolin D1 mini boards.
 *  
 *  There are four different module types, a button, a temperature sensor, a 
 *  PIR sensor and a ultrasonic distance sensor. All module types use the same code.
 *  The module type is identified by monitoring the hard coded setting of 
 *  two type pins.
 *  Each module have two LEDs that are used to display the current state.
 *  The modules publish their sensor values to a MQTT server and subscribe to  
 *  system state information from the same server.
 *  
 *  Permission is hereby granted, free of charge, to any person obtaining a copy 
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights 
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell 
 *  copies of the Software, and to permit persons to whom the Software is 
 *  furnished to do so, subject to the following conditions:
 *  The above copyright notice and this permission notice shall be included in all 
 *  copies or substantial portions of the Software.
 *  
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE 
 *  SOFTWARE.
 */

#include "lcd_serial.h"
#include <ESP8266WiFi.h>  // Verified with ESP8266WiFi 1.0 by ?
#include <PubSubClient.h> // Verified with PubSubClient 2.7 by Nick O'Leary
#include <Ultrasonic.h>   // Verified with Ultrasonic 3.0.0 by Eric Simões

/********************************************************************************************
 * Definitions
 ********************************************************************************************/
// Network settings
const char *ssid = "AfryAlarmNet";
const char *password = "afrypassword";

const char *ssid2 = "dlink";
const char *password2 = "";

const char *mqttServer = "broker.hivemq.com"; // HiveMQ cloud server
const char *mqttServer2 = "192.168.0.198";    // Local Raspberry Pi Server
const int mqttPort = 1883;

// PIN names
#define ALARM_LED_PIN D0
#define LCD_SCL_PIN D1
#define LCD_SDA_PIN D2
#define SR04_TRIG_PIN D3
#define STATUS_LED_PIN D4
#define TYPE1_PIN D5
#define PIR_PIN D6
#define SR04_ECHO_PIN D6
#define BUTTON_PIN D6
#define TYPE0_PIN D7
#define SIREN_PIN D8

// Module types
#define BUTTON 1
#define SR04 2
#define PIR 3
#define SIREN 4

// All other definitions
#define SIREN_MAX 1023

/********************************************************************************************
 * Global Variables
 ********************************************************************************************/

char message_buffer[255];
char *module_name;
unsigned int module_type;
bool wifi_connected = false;
bool mqtt_connected = false;
struct
{
  bool fire_alarm;
  bool entry_alarm;
  bool armed;
} state;
WiFiClient espClient;
PubSubClient client(espClient);
Ultrasonic ultrasonic(SR04_TRIG_PIN, SR04_ECHO_PIN, 5634UL); // 5634UL == 100 cm max distance
lcd_serial lcdSerial;

/********************************************************************************************
 * WiFi connection function
 ********************************************************************************************/
bool connect_to_wifi(const char *ssid, const char *password)
{
  unsigned int connection_attempts = 0;
  lcdSerial.print("Connecting to ");
  lcdSerial.println(ssid);
  WiFi.begin(ssid, password);
  while ((WiFi.status() != WL_CONNECTED) && (connection_attempts < 15))
  {
    connection_attempts++;
    delay(500);
    lcdSerial.print(".");
  }
  lcdSerial.println("");
  return (WiFi.status() == WL_CONNECTED);
}

/********************************************************************************************
 * MQTT connection function
 ********************************************************************************************/
bool connect_to_mqtt(const char *mqttServer, const int mqttPort, const char *module_name)
{
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);
  lcdSerial.println("MQTT server:");
  lcdSerial.println(mqttServer);
  return (client.connect(module_name));
}

/********************************************************************************************
 * MQTT callback function
 ********************************************************************************************/
void callback(char *topic, byte *payload, unsigned int length)
{
  lcdSerial.print(topic);
  lcdSerial.print(" ");
  for (int i = 0; i < length; i++)
  {
    lcdSerial.print((char)payload[i]);
  }
  lcdSerial.println();

  if (!strncmp(topic, "alarm/entry_alarm", strlen(topic)))
    state.entry_alarm = (payload[0] == '1');
  if (!strncmp(topic, "alarm/fire_alarm", strlen(topic)))
    state.fire_alarm = (payload[0] == '1');
  if (!strncmp(topic, "alarm/armed", strlen(topic)))
    state.armed = (payload[0] == '1');
}

/********************************************************************************************
 * setup function
 ********************************************************************************************/
void setup()
{
  // Init Pin settings
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(ALARM_LED_PIN, OUTPUT);
  pinMode(STATUS_LED_PIN, OUTPUT);
  pinMode(SR04_TRIG_PIN, OUTPUT);
  pinMode(SIREN_PIN, OUTPUT);

  pinMode(PIR_PIN, INPUT_PULLUP); // Also covers SR04_ECHO_PIN and BUTTON_PIN
  pinMode(TYPE0_PIN, INPUT_PULLUP);
  pinMode(TYPE1_PIN, INPUT_PULLUP);

  // Turn off all digital outputs
  digitalWrite(LED_BUILTIN, LOW);    // Default LED off
  digitalWrite(ALARM_LED_PIN, LOW);  // Alarm LED OFF
  digitalWrite(STATUS_LED_PIN, LOW); // Status LED OFF
  digitalWrite(SIREN_PIN, HIGH);     // SIREN OFF

  // Initialize serial communication
  lcdSerial.println();
  lcdSerial.println();

  // Identify module type
  bool t0 = digitalRead(TYPE0_PIN);
  bool t1 = digitalRead(TYPE1_PIN);
  if (t0 && t1)
    module_type = SIREN;
  else if (t0 && !t1)
    module_type = PIR;
  else if (!t0 && t1)
    module_type = SR04;
  else if (!t0 && !t1)
    module_type = BUTTON;

  // Module init and test
  lcdSerial.print("Module is ");
  switch (module_type)
  {
  case SIREN:
    module_name = "Siren";
    analogWrite(SIREN_PIN, SIREN_MAX / 4); // SIREN ON
    delay(100);
    analogWrite(SIREN_PIN, 0); // SIREN OFF
    break;
  case PIR:
    module_name = "Motion Detector";
    // Put motion read here
    break;
  case SR04:
    module_name = "Ultrasonic Sensor";
    // Read distance here
    break;
  case BUTTON:
    module_name = "Button";
    break;
  }
  lcdSerial.println(module_name);

  // Turn on Status and Alarm LEDs
  digitalWrite(ALARM_LED_PIN, HIGH);  //LED ON
  digitalWrite(STATUS_LED_PIN, HIGH); // LED ON

  // Connect to any existing known WiFi network
  int n = WiFi.scanNetworks();
  for (int i = 0; n != 0 && i < n; i++)
  {
    if (!strcmp(WiFi.SSID(i).c_str(), ssid))
    {
      connect_to_wifi(ssid, password);
      break;
    }
    else if (!strcmp(WiFi.SSID(i).c_str(), ssid2))
    {
      connect_to_wifi(ssid2, password2);
      break;
    }
  }
  wifi_connected = ((WiFi.status() == WL_CONNECTED));
  if (!wifi_connected)
  {
    lcdSerial.println("WiFi connect failed");
  }
  else
  {
    lcdSerial.println("WiFi connected");
    lcdSerial.print("IP: ");
    lcdSerial.println(WiFi.localIP().toString().c_str());

    // Connect to MQTT server
    if (!(mqtt_connected = connect_to_mqtt(mqttServer, mqttPort, module_name)))
    {
      mqtt_connected = connect_to_mqtt(mqttServer2, mqttPort, module_name);
    }
    if (mqtt_connected)
    {
      client.subscribe("alarm/#");
      client.publish("alarm/connected", module_name);
      lcdSerial.println("MQTT connected");
    }
    else
    {
      lcdSerial.println("MQTT connect failed");
      lcdSerial.print("Error code: ");
      lcdSerial.println(client.state());
    }
  }
  digitalWrite(ALARM_LED_PIN, LOW);  //LED OFF
  digitalWrite(STATUS_LED_PIN, LOW); //LED OFF
}

/********************************************************************************************
 * loop function
 ********************************************************************************************/
void loop()
{
  if (!wifi_connected || !mqtt_connected)
  {
    // No connection - Just blink LEDs
    delay(200);
    digitalWrite(ALARM_LED_PIN, !digitalRead(ALARM_LED_PIN));
    digitalWrite(STATUS_LED_PIN, !digitalRead(STATUS_LED_PIN));
  }
  else
  {
    // Connected - publish data and run module code
    bool button_state;
    static bool old_button_state = true;
    bool pir_state;
    static bool old_pir_state = true;

    delay(200);
    client.loop();

    // Handle alarm and status LEDs
    if (!state.armed)
    { // Alarm not active
      digitalWrite(STATUS_LED_PIN, HIGH);
      digitalWrite(ALARM_LED_PIN, LOW);
    } // Alarm active but not triggered
    else if (!(state.fire_alarm || state.entry_alarm))
    {
      digitalWrite(STATUS_LED_PIN, LOW);
      digitalWrite(ALARM_LED_PIN, HIGH);
    }
    else
    { // Alarm active and triggered
      digitalWrite(STATUS_LED_PIN, LOW);
      digitalWrite(ALARM_LED_PIN, !digitalRead(ALARM_LED_PIN));
    }

    // Button module
    if (module_type == BUTTON)
    {
      button_state = digitalRead(BUTTON_PIN);
      if (old_button_state != button_state)
      {
        old_button_state = button_state;
        if (button_state == LOW)
        {
          client.publish("alarm/button", "1");
        }
      }
    }

    // PIR module
    if (module_type == PIR)
    {
      pir_state = digitalRead(PIR_PIN);
      if (old_pir_state != pir_state)
      {
        old_pir_state = pir_state;
        if (pir_state == HIGH)
        {
          lcdSerial.println("Detected Motion");
          client.publish("alarm/pir", "1");
        }
      }
    }

    // Siren module
    if ((module_type == SIREN) && state.armed && (state.entry_alarm || state.fire_alarm))
    {
      analogWrite(SIREN_PIN, SIREN_MAX / 2);
    }
    else
    {
      analogWrite(SIREN_PIN, 0); // Siren OFF
    }

    // Ultrasonic Sensor HC-SR04 module
    if (module_type == SR04)
    {
      unsigned int distance = ultrasonic.read();
      if ((distance < 100) && (ultrasonic.read() < 100))
      { // Double check for noice
        lcdSerial.println("Detected Distance");
        lcdSerial.print("Dist: ");
        lcdSerial.print(distance);
        lcdSerial.println("cm");
        client.publish("alarm/pir", "1");
      }
    }
  }
}
