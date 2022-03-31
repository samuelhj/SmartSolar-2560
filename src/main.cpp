#include <Arduino.h>
#include "VEdirect.h"
#include <Ethernet.h>
#include <SPI.h>
#include <PubSubClient.h> // MQTT client
#include <avr/sleep.h>
#include <avr/wdt.h>
#include "MemoryFree.h"
#include <math.h>
#define DEBUG 1

// software Serial pins
//#define rxPin 2
//#define txPin 3
void mpptCallback(uint16_t id, int32_t value);
VEDirect mppt(Serial2, mpptCallback);

float panelVoltage = 0.00f;
float chargeCurrent = 0.00f;

// Ethernet

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = {  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(10, 1, 2, 28);
IPAddress dns1(1, 1, 1, 1); //Adddress of Domain name server 

// MQTT
const char* mqtt_server = "mqtt.ulfraf.space";  // IP of the MQTT broker
//const char* mqtt_server = "10.1.2.27";
const char* load_current_topic = "Elmar/WX1/load_current";
const char* battery_voltage_topic = "Elmar/WX1/battery_voltage";
const char* charging_voltage_topic = "Elmar/WX1/charging_voltage";
const char* charging_current_topic = "Elmar/WX1/charging_current";
const char* power_topic = "Elmar/WX1/power";
const char* vpv_topic = "Elmar/WX1/vpv";
const char* mqtt_username = "elmar"; // MQTT username
const char* mqtt_password = "elmar.2022"; // MQTT password
const char* clientID = "Varmaskoli_WX1"; // MQTT client ID

float charging_voltage = 0.00f;
float charging_current = 0.00f;
float battery_voltage = 0.00f;
float load_current = 0.00f;

EthernetClient ethClient;
PubSubClient client(mqtt_server, 1883, ethClient);

// 32 bit ints to collect the data from the device
int16_t VE_soc, VE_power, VE_voltage, VE_current;
// Boolean to collect an ON/OFF value
uint8_t VE_alarm;

// VEDirect instantiated with relevant serial object
//VEDirect myve(rxPin, txPin);

// Functions
void mpptCallback(uint16_t id, int32_t value) 
{
  if (id == VEDirect_kPanelVoltage)
  {
    panelVoltage = value * 0.01;
    Serial.print(F("Vpv : "));
    Serial.println(value * 0.01);
  }
  if (id == VEDirect_kChargeCurrent)
  {
    charging_current = value * 0.1;
    Serial.print(F("Ich : "));
    Serial.println(value * 0.1);
  }
  if(id == VEDirect_kLoadVoltage)
  {
    battery_voltage = value * 0.01;
    Serial.print(F("Vb: "));
    Serial.println(value * 0.01);
  }
  if(id == VEDirect_kLoadCurrent)
  {
    load_current = value * 0.1;
    Serial.print(F("Load current: "));
    Serial.println(load_current * 0.1);
  }
}

// Watchdog ISR routine
ISR (WDT_vect) 
{
   wdt_disable();  // disable watchdog
   // Return from ISR
}  

void setup() 
{
  if(DEBUG == 1)
  {
    Serial.begin(9600); // Debug serial
    Serial.println("Serial debug begins!");
  }
  //Ethernet
  Ethernet.init(10);
  Ethernet.begin(mac);
  client.setServer(mqtt_server, 1883);
  Ethernet.setDnsServerIP(dns1);

  mppt.begin();

} // Setup ends

void loop() 
{
  Ethernet.maintain(); // Maintain DHCP lease

  static unsigned long secondsTimer = 0;
  mppt.update();
  unsigned long m = millis();
  if(m - secondsTimer > 1000L)
  {
    secondsTimer = m;
    mppt.ping();  // send ping every second
  }

// MQTT
  if (client.connect(clientID, mqtt_username, mqtt_password)) 
  {
    if(DEBUG == 1)
    {
      Serial.println("Connected to MQTT Broker!");
    }
    if (client.publish(battery_voltage_topic, String(battery_voltage).c_str())) 
    {
      if(DEBUG == 1)
      {
        Serial.println("Battery voltage sent!");
        Serial.print("Battery voltage: ");
        Serial.println(VE_voltage);
      }
    }
    // Again, client.publish will return a boolean value depending on whether it succeded or not.
    // If the message failed to send, we will try again, as the connection may have broken.
    else 
    {
      if(DEBUG == 1)
      {
        Serial.println("Battery voltage failed to send. Reconnecting to MQTT Broker and trying again");
      }
      client.connect(clientID, mqtt_username, mqtt_password);
      delay(10); // This delay ensures that client.publish doesn't clash with the client.connect call
      client.publish(battery_voltage_topic, String(battery_voltage).c_str());
    } // Battery voltage ends

    // Load current
    if (client.publish(load_current_topic, String(load_current).c_str())) 
    {
      if(DEBUG == 1)
      {
        Serial.println("Load current sent!");
        Serial.print("Load current: ");
        Serial.println(load_current);
      }
    }
    // Again, client.publish will return a boolean value depending on whether it succeded or not.
    // If the message failed to send, we will try again, as the connection may have broken.
    else 
    {
      if(DEBUG == 1)
      {
        Serial.println("Load current failed to send. Reconnecting to MQTT Broker and trying again");
      }
      client.connect(clientID, mqtt_username, mqtt_password);
      delay(10); // This delay ensures that client.publish doesn't clash with the client.connect call
      client.publish(load_current_topic, String(load_current).c_str());
    } //load current ends

    // Panel voltage
    if (client.publish(vpv_topic, String(panelVoltage).c_str())) 
    {
      if(DEBUG == 1)
      {
        Serial.println("Panvel voltage sent!");
        Serial.print("Panel voltage: ");
        Serial.println(panelVoltage);
      }
    }
    // Again, client.publish will return a boolean value depending on whether it succeded or not.
    // If the message failed to send, we will try again, as the connection may have broken.
    else 
    {
      if(DEBUG == 1)
      {
        Serial.println("Panel Voltage failed to send. Reconnecting to MQTT Broker and trying again");
      }
      client.connect(clientID, mqtt_username, mqtt_password);
      delay(10); // This delay ensures that client.publish doesn't clash with the client.connect call
      client.publish(vpv_topic, String(panelVoltage).c_str());
    } //SOC ends

  } //MQTT transmission ends
  else
  {
    if(DEBUG == 1)
    {
      Serial.println("Connection to MQTT Broker failed...");
    }
  }

  client.disconnect();  // disconnect from MQTT broker
  if(DEBUG == 1)
  {
    Serial.print("Free memory: ");
    Serial.println(freeMemory());
  }
  delay(1000*10);       // print new values every 1 Minute

} // Main loop ends