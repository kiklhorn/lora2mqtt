#include <LoRa.h>
#include "boards.h"
//dirty hack to keep common files located on MS OneDrive (IDK why relative paths not working here) in one place only - development time
#if __has_include("../LoRa2MQTTgateway/settings.h")  
#include "../LoRa2MQTTgateway/settings.h"
#include "../LoRa2MQTTgateway/structures.h"
#else if __has_include("C:\Users\marti\OneDrive\Dokumenty\Arduino\lora2mqtt\lora2mqtt\LoRa2MQTTgateway\settings.h")
#include "C:\Users\marti\OneDrive\Dokumenty\Arduino\lora2mqtt\lora2mqtt\LoRa2MQTTgateway\settings.h"
#include "C:\Users\marti\OneDrive\Dokumenty\Arduino\lora2mqtt\lora2mqtt\LoRa2MQTTgateway\structures.h"
#endif
// end of dirty hack. 



int counter = 0;

void setup()
{
    initBoard();
    // When the power is turned on, a delay is required.
    delay(1500);

    Serial.println("LoRa Sender");
    initLora();    
}

void loop()
{
    Serial.print("Sending packet: ");
    Serial.println(counter);
      // Serial.print("' SF: ");
      // Serial.print(LoRa.getSpreadingFactor()); //changed from private to public in the LoRa.h library
      // Serial.print("' BW: ");
      // Serial.println(LoRa.getSignalBandwidth());
    // send packet
    LoRa.beginPacket();
    // LoRa.print("hello ");
    LoRa.print(counter);
    LoRa.endPacket();

#ifdef HAS_DISPLAY
    if (u8g2) {
        char buf[256];
        u8g2->clearBuffer();
        u8g2->drawStr(0, 12, "Transmitting: OK!");
        snprintf(buf, sizeof(buf), "Sending: %d", counter);
        u8g2->drawStr(0, 30, buf);
        u8g2->sendBuffer();
    }
#endif
    counter++;
    delay(5000);
}
