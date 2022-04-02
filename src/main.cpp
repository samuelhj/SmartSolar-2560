#include <Arduino.h>
#include "VEdirect.h"
#include <Ethernet.h>
#include <SPI.h>
#include <PubSubClient.h> // MQTT client
#include <avr/sleep.h>
#include <avr/wdt.h>
#include "MemoryFree.h"
#include <math.h>
#include "authentication.h"

#define DEBUG 1
#define INTERVAL1 30000

// software Serial pins
//#define rxPin 2
//#define txPin 3
void mpptCallback(uint16_t id, int32_t value);
VEDirect mppt(Serial2, mpptCallback);



// Ethernet

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = {  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(10, 1, 2, 28);
IPAddress dns1(1, 1, 1, 1); //Adddress of Domain name server 



float charging_voltage = 0.00f;
float charging_current = 0.00f;
float battery_voltage = 0.00f;
float load_current = 0.00f;
float panelVoltage = 0.00f;

EthernetClient ethClient;
PubSubClient client(mqtt_server, 1883, ethClient);

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
  if(id == VEDirect_kBatteryVoltage)
  {
    float battery_voltage = 0.00f;
    battery_voltage = value * 0.01;
    Serial.print(F("battery voltage: "));
    Serial.println(battery_voltage);
  }
  Serial.println(id);
  Serial.println(value);
}

// Watchdog ISR routine
ISR (WDT_vect) 
{
   wdt_disable();  // disable watchdog
   // Return from ISR
}  

void setup() 
{

    Serial.begin(9600); // Debug serial
    Serial.println("Serial debug begins!");
  
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
// send message every 30 seconds
unsigned long counter1 = millis();
static unsigned long counter2 = 0;

if(counter2 - counter1 > INTERVAL1)
{
  counter1 = counter2; // reset counter

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
        Serial.println(battery_voltage);
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
        Serial.println("Panel voltage sent!");
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
  }

  client.disconnect();  // disconnect from MQTT broker
  if(DEBUG == 1)
  {
    Serial.print("Free memory: ");
    Serial.println(freeMemory());
  }
  //delay(1000*10);       // print new values every 1 Minute

} // Main loop ends