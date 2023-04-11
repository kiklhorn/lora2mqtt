
//ToDo: 
// receive - done
// display - done
// json structure communication https://github.com/plapointe6/HaMqttConfigBuilder
// mqtt autodiscovery - receiver https://www.home-assistant.io/integrations/mqtt#mqtt-discovery  https://github.com/dawidchyrzynski/arduino-home-assistant
// mqtt autodiscovery - multiple clients=senders (similar as z2m devices?) https://github.com/plapointe6/HAMqttDevice
// transmission encryption
// clients authorization
// duplex comm  https://github.com/sandeepmistry/arduino-lora

#include <LoRa.h>
#include "boards.h"
#include <WiFi.h>
#include <WiFiClient.h>
// #include "EspMQTTClient.h"
// #include "HAMqttDevice.h"
// #include <ESP_EEPROM.h>
// #include <Ethernet.h> //replace by wifi
// #include <ESP8266WiFi.h>
#include <ArduinoHA.h>

// // MQTT client setup - utilities.h
// EspMQTTClient client(
//   wifi_ssid,         // Wifi ssid
//   wifi_password,     // Wifi password
//   mqtt_broker_address,  // MQTT broker ip
//   mqtt_user,     // MQTT username
//   mqtt_password, // MQTT password
//   mqtt_client        // MQTT Client name
// );


WiFiClient client;
HADevice device("lora2mqtt");
HAMqtt mqtt(client, device, max_lora_devices); //device types need to be constructed after HAMqtt class.
HASwitch mySwitch("SwitchSchranky");
HASensor schranka("SenzorSchranky");
// HAMqttDevice schranka("Schranka",HAMqttDevice::BINARY_SENSOR,mqtt_discovery_prefix);

void onMessage(const char* topic, const uint8_t* payload, uint16_t length) {
    // this method will be called each time the device receives an MQTT message
    // https://dawidchyrzynski.github.io/arduino-home-assistant/documents/library/mqtt-advanced.html
}

void onConnected() {
    // this method will be called when connection to MQTT broker is established
    // https://dawidchyrzynski.github.io/arduino-home-assistant/documents/library/mqtt-advanced.html

}

String recv = "";

void setup()
{
    initBoard();
    // When the power is turned on, a delay is required.
    delay(1500);

    Serial.println("LoRa Receiver");

    LoRa.setPins(RADIO_CS_PIN, RADIO_RST_PIN, RADIO_DIO0_PIN);
    if (!LoRa.begin(LoRa_frequency)) {
        Serial.println("Starting LoRa failed!");
        while (1);
    }
    LoRa.setSpreadingFactor(lora_spreading_factor);
    u8g2->begin();
    // Serial.begin(115200); //starte den seriellen Monitor mit der angegebenen Baud Rate
    WiFi.begin(wifi_ssid, wifi_password); // Connect to WiFi network
    Serial.println("");

    while (WiFi.status() != WL_CONNECTED) // Wait for connection
    {
      delay(500);
      Serial.print(".");
    }
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(wifi_ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    // your setup logic goes here
    device.setName("LoRa2MQTT Controller");
    device.setSoftwareVersion("0.1.0");
    device.setManufacturer("Kiklhorn");
    device.setModel("LoRa Tiny Gateway");
    device.enableSharedAvailability();
    device.enableLastWill();
    // device.setAvailability(false); // changes default state to offline

    mqtt.onMessage(onMessage); // https://dawidchyrzynski.github.io/arduino-home-assistant/documents/library/mqtt-advanced.html
    mqtt.onConnected(onConnected); //https://dawidchyrzynski.github.io/arduino-home-assistant/documents/library/mqtt-advanced.html
    // MQTT broker connection (use your data here)
    mqtt.begin(mqtt_broker_address, mqtt_broker_port, mqtt_user, mqtt_password);
    // mqtt.setDiscoveryPrefix("lora2mqtt");
    // mqtt.setDataPrefix("lora2mqtt");
}

void loop()
{
  mqtt.loop();
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
      // received a packet
      Serial.print("Received packet '");

      // String recv = ""; //define at global
      recv = "";
      // read packet
      while (LoRa.available()) {
          recv += (char)LoRa.read();
      }
      schranka.setValue(recv.c_str());
      Serial.print(recv);
      // print RSSI of packet
      Serial.print("' with RSSI ");
      Serial.print(LoRa.packetRssi());
      Serial.print("' SNR: ");
      Serial.println(LoRa.packetSnr());
      // Serial.println(schranka.getConfigTopic());
      // Serial.println(schranka.getConfigPayload());
      // Serial.println(schranka.getAttributesTopic());
      // Serial.println(schranka.getAttributesPayload());          
#ifdef HAS_DISPLAY
        if (u8g2) {
            u8g2->clearBuffer();
            char buf[256];
            u8g2->drawStr(0, 12, "Received OK!");
            u8g2->drawStr(0, 26, recv.c_str());
            snprintf(buf, sizeof(buf), "RSSI:%i", LoRa.packetRssi());
            u8g2->drawStr(0, 40, buf);
            snprintf(buf, sizeof(buf), "SNR:%.1f", LoRa.packetSnr());
            u8g2->drawStr(0, 56, buf);
            u8g2->sendBuffer();
        }
#endif
  }
}
