
//ToDo: 
// receive - done
// display - done
// json structure communication https://github.com/plapointe6/HaMqttConfigBuilder
// mqtt autodiscovery - receiver https://www.home-assistant.io/integrations/mqtt#mqtt-discovery  https://github.com/dawidchyrzynski/arduino-home-assistant
// mqtt autodiscovery - multiple clients=senders (similar as z2m devices?) https://github.com/plapointe6/HAMqttDevice
// transmission encryption
// clients authorization
// duplex comm  https://github.com/sandeepmistry/arduino-lora  https://hutscape.com/tutorials/lora-duplex-a https://www.youtube.com/watch?v=tO1hYr6hNa4 
// Automatic data rate - https://www.thethingsnetwork.org/docs/lorawan/spreading-factors/

#include <LoRa.h>
#include "boards.h"
#include "settings.h"
#include <WiFi.h>
#include <WiFiClient.h>
// #include "EspMQTTClient.h"
// #include "HAMqttDevice.h"
// #include <ESP_EEPROM.h>
// #include <Ethernet.h> //replace by wifi
// #include <ESP8266WiFi.h>
#include <ArduinoHA.h>

// // MQTT client setup - utilities.h

WiFiClient client;
HADevice device("lora2mqtt");
HAMqtt mqtt(client, device, lora_max_devices); //device types need to be constructed after HAMqtt class.
HASwitch mySwitch("SwitchSchranky");
HASensor schranka("SenzorSchranky");
// HAMqttDevice schranka("Schranka",HAMqttDevice::BINARY_SENSOR,mqtt_discovery_prefix);

String recv = "";
byte localAddress = 0xAA;
byte destinationAddress = 0xBB;
long lastSendTime = 0;
int interval = 2000;
int count = 0;
bool justreceiving = false, justsending = false;

void sendData(){

    justsending = true;
    sendMessage("Sending String Message");
    Serial.print("Sending String Message");
    justsending = false;
    mySwitch.setState(!mySwitch.getCurrentState()); //Just toggle switch for testing. Othervise need ACK from end device.  
}

void onMessage(const char* topic, const uint8_t* payload, uint16_t length) {
    // this method will be called each time the device receives an MQTT message
    // https://dawidchyrzynski.github.io/arduino-home-assistant/documents/library/mqtt-advanced.html
    sendData();
}

void onConnected() {
    // this method will be called when connection to MQTT broker is established
    // https://dawidchyrzynski.github.io/arduino-home-assistant/documents/library/mqtt-advanced.html

}

void setup()
{
    initBoard();
    delay(1500); // When the power is turned on, a delay is required.
    Serial.println("LoRa Receiver");

    LoRa.setPins(RADIO_CS_PIN, RADIO_RST_PIN, RADIO_DIO0_PIN);
    if (!LoRa.begin(LoRa_frequency)) {
        Serial.println("Starting LoRa failed!");
        while (1);
    }
    LoRa.setSpreadingFactor(lora_spreading_factor);
    // LoRa.setCADTimeout(1000); // timeout pro CAD funkci (v milisekundách)
    LoRa.onCadDone(onCadDone);// register the channel activity dectection callback
    LoRa.onReceive(onReceive); // register the receive callback
    LoRa.enableCrc(); // nastavení konfiguračního registru pro CRC
    LoRa.setPreambleLength(68); // nastavení registru pro délku očekávané zprávy
    LoRa.setSyncWord(lora_network_id);
    LoRa.channelActivityDetection(); // put the radio into CAD mode
    LoRa.dumpRegisters(Serial);

#ifdef HAS_DISPLAY
    u8g2->begin();
#endif
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
    delay(500);
} //************* END setup()

void loop()
{
  mqtt.loop();
  // // int packetSize = LoRa.parsePacket();
  // if (!justsending)
  //   {
  //   justreceiving = true;
  //   receiveMessage(LoRa.parsePacket());
  //   }
}
void onCadDone(boolean signalDetected) {
  // detect preamble
  if (signalDetected) {
    Serial.println("Signal detected");
    int packetSize = LoRa.parsePacket();
  // read packet
  for (int i = 0; i < packetSize; i++) {
    Serial.print((char)LoRa.read());
  }

    // put the radio into continuous receive mode
    LoRa.receive();
  } else {
    // try next activity dectection
    LoRa.channelActivityDetection();
  }
}

void onReceive(int packetSize) {
  // received a packet
  Serial.print("Received packet '");

  // read packet
  for (int i = 0; i < packetSize; i++) {
    Serial.print((char)LoRa.read());
  }

  // print RSSI of packet
  Serial.print("' with RSSI ");
  Serial.println(LoRa.packetRssi());

  // put the radio into CAD mode
  // LoRa.channelActivityDetection();
}

void sendMessage(String outgoing) {
  LoRa.beginPacket();
  LoRa.write(destinationAddress);
  LoRa.write(localAddress);
  LoRa.write(outgoing.length());
  LoRa.print(outgoing);
  LoRa.endPacket();
}

void receiveLoRaData() {
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    byte buffer[packetSize];
    for (int i = 0; i < packetSize; i++) {
      buffer[i] = LoRa.read();
    }
    int rssi = LoRa.packetRssi();
    float snr = LoRa.packetSnr();
    // if (!LoRa.packetRxCrcError()) {
      if (true) {
      // paket byl přijat správně
      Serial.println("CRC - Paket přijat správně.");
      
      // dekódování timestampu a int čísla
      uint32_t timestamp = *(uint32_t*)(&buffer[0]);
      int intNumber = *(int*)(&buffer[4]);
      
      // výpis timestampu a int čísla
      Serial.print("Timestamp: ");
      Serial.println(timestamp);
      Serial.print("Int číslo: ");
      Serial.println(intNumber);
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
    } else {
      // paket nebyl přijat správně
      Serial.println("CRC - Paket nebyl přijat správně.");
    }
  }
}

void receiveMessage(int packetSize) {
  if (packetSize) {
      // received a packet
      Serial.print("Received packet '");

      // String recv = ""; //define at global
      recv = "";
      // read packet
      while (LoRa.available()) {
          recv += (char)LoRa.read();
      }
      justreceiving = false;
      schranka.setValue(recv.c_str());
      Serial.print(recv);
      // print RSSI of packet
      Serial.print("' with RSSI ");
      Serial.print(LoRa.packetRssi());
      Serial.print("' SNR: ");
      Serial.println(LoRa.packetSnr());
      // Serial.print("' SF: ");
      // Serial.print(LoRa.getSpreadingFactor()); //changed from private to public in the LoRa.h library
      // Serial.print("' BW: ");
      // Serial.println(LoRa.getSignalBandwidth());
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

  // int recipient = LoRa.read();
  // byte sender = LoRa.read();
  // byte incomingLength = LoRa.read();

  // String recv = "";

  // while (LoRa.available()) {
  //   recv += (char)LoRa.read();
  // }

  // if (incomingLength != recv.length()) {
  //   // Serial.println("Error: Message length does not match length");
  //   return;
  // }

  // if (recipient != localAddress) {
  //   // Serial.println("Error: Recipient address does not match local address");
  //   return;
  // }

  // Serial.print("Received data " + recv);
  // Serial.print(" from 0x" + String(sender, HEX));
  // Serial.println(" to 0x" + String(recipient, HEX));
}  

