class Device {
public:
  int deviceID;
  String deviceType;
  String deviceStatus;
  float sensorData;
  int actuatorData;
};

class Network {
public:
  Device devices[10]; // pole zařízení v síti
  int numDevices; // počet zařízení v síti
  Device mainStation; // hlavní stanice v síti
  
  void addDevice(Device device) {
    devices[numDevices] = device;
    numDevices++;
  }
  
  void sendData(Device sender, Device receiver, float data) {
    // zde by se odeslaly data ze senzoru na hlavní stanici
  }
  
  void sendCommand(Device sender, Device receiver, int command) {
    // zde by se odeslal příkaz pro aktuátor
  }
};


void initLora () {
      LoRa.setPins(RADIO_CS_PIN, RADIO_RST_PIN, RADIO_DIO0_PIN);
    if (!LoRa.begin(LoRa_frequency)) {
        Serial.println("Starting LoRa failed!");
        while (1);
    }
    LoRa.setSpreadingFactor(lora_spreading_factor);
    LoRa.channelActivityDetection();
    // nastavení konfiguračního registru pro CRC
    LoRa.enableCrc();
    // LoRa.setPacketCrcOn(true);

    // nastavení registru pro délku očekávané zprávy
    LoRa.setPreambleLength(lora_spreading_factor+4);
    LoRa.setSyncWord(lora_network_id);
    LoRa.dumpRegisters(Serial); 
}