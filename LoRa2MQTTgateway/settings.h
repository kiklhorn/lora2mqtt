#pragma once
const char* wifi_ssid = "ssid";
const char* wifi_password = "pass";
const char* mqtt_broker_address = "homeassistant.local";
const int mqtt_broker_port = 1883;
const char* mqtt_user = "";
const char* mqtt_password = "";
const char* mqtt_client = "loragw";
const char* mqtt_discovery_prefix = "homeassistant"; //https://www.home-assistant.io/integrations/mqtt/#mqtt-discovery
const int lora_max_devices = 6;
const int lora_spreading_factor = 10; // From 7(high speed, "low" range) to 12(low speed, high range) 
const int lora_network_id = 0x12;
