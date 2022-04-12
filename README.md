To create authentication.h

in your PlatformIO/Project/SmartSolar-2560/lib/
´mkdir authentication´
´cd authentication´
´touch authentication.h´

And paste this and edit to your needs

/*
    This file holds the authentication information for the MQTT server
*/
const char* mqtt_username = "user"; // MQTT username
const char* mqtt_password = "strongpassword"; // MQTT password
const char* clientID = "My_Client_ID"; // MQTT client ID
const char* mqtt_server = "mqtt.example.com";  // Host of the MQTT broker

// In this example we only have Battery voltage, charging current, and panel voltage
const char* battery_voltage_topic = "topic/subtopic/battery_voltage";
const char* charging_current_topic = "topic/subtopic/charging_current";
const char* vpv_topic = "topic/subtopic/vpv";
