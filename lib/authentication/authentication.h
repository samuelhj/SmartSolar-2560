/*
    This file holds the authentication information for the MQTT server
*/
const char* mqtt_username = "elmar"; // MQTT username
const char* mqtt_password = "elmar.2022"; // MQTT password
const char* clientID = "Varmaskoli_WX1"; // MQTT client ID
// MQTT
const char* mqtt_server = "mqtt.ulfraf.space";  // IP of the MQTT broker
//const char* mqtt_server = "10.1.2.27";
const char* load_current_topic = "Elmar/WX1/load_current";
const char* battery_voltage_topic = "Elmar/WX1/battery_voltage";
const char* charging_voltage_topic = "Elmar/WX1/charging_voltage";
const char* charging_current_topic = "Elmar/WX1/charging_current";
const char* power_topic = "Elmar/WX1/power";
const char* vpv_topic = "Elmar/WX1/vpv";