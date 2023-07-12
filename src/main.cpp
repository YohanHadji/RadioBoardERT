#include <Arduino.h>
#include <Capsule.h>  
#include <LoopbackStream.h>
#include <LoRa.h>
#include <SPI.h>
#include <Adafruit_NeoPixel.h>
#include "../ERT_RF_Protocol_Interface/PacketDefinition.h"
#include "../ERT_RF_Protocol_Interface/ParameterDefinition.h"
#include "config.h"

uint32_t colors[] = {
    0x32A8A0,
    0x0000FF,
    0xFFEA00,
    0x00FF00,
    0xFF0000,
    0xCF067C,
    0xFF0800
}; 

void handlePacketLoRa(int packetSize);
void handleLoRaCapsule(uint8_t packetId, uint8_t *dataIn, uint32_t len); 
void handleUartCapsule(uint8_t packetId, uint8_t *dataIn, uint32_t len);

Adafruit_NeoPixel led(1, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800); // 1 led
LoopbackStream LoRaRxBuffer(1024);
CapsuleStatic LoRaCapsule(handleLoRaCapsule);
CapsuleStatic UartCapsule(handleUartCapsule);

void setup() {
  SERIAL_TO_PC.begin(SERIAL_TO_PC_BAUD);
  SERIAL_TO_PC.setTxTimeoutMs(0);

  // put your setup code here, to run once:
  UART_PORT.begin(UART_BAUD, 134217756U, 6, 5); // This for radioboard
  //UART_PORT.begin(UART_BAUD, 134217756U, 9, 46); // This for cmdIn

  led.begin();
  uint32_t ledColor = colors[random(0,7)];
  led.fill(ledColor);
  led.show();

  SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_CS); 
  LoRa.setPins(LORA_CS, LORA_RST, LORA_INT0);
  LoRa.setSPI(SPI);
  
  if (!LoRa.begin(LORA_FREQ)) {
    if (DEBUG) {
      SERIAL_TO_PC.println("Starting LoRa failed!");
    }
  }

  LoRa.setSpreadingFactor(LORA_SF);
  LoRa.setSignalBandwidth(LORA_BW);
  LoRa.setCodingRate4(LORA_CR);
  //LoRa.setPreambleLength(LORA_PREAMBLE_LEN);
  //LoRa.setSyncWord(LORA_SYNC_WORD);
  //LoRa.enableCrc();
  LoRa.setTxPower(LORA_POWER);
  //LoRa.setOCP(LORA_CURRENT_LIMIT);
  LoRa.onReceive(handlePacketLoRa);
  LoRa.receive(); 
}

void loop() {
  while (LoRaRxBuffer.available()) {
    LoRaCapsule.decode(LoRaRxBuffer.read());
  }

  while (UART_PORT.available()) {
    UartCapsule.decode(UART_PORT.read());
  }
}

void handlePacketLoRa(int packetSize) {
  for (int i = 0; i < packetSize; i++) {
    LoRaRxBuffer.write(LoRa.read());
  }
}

void handleLoRaCapsule(uint8_t packetId, uint8_t *dataIn, uint32_t len) {
  uint8_t* packetToSend = UartCapsule.encode(packetId,dataIn,len);
  UART_PORT.write(packetToSend,UartCapsule.getCodedLen(len));
  delete[] packetToSend;

  uint32_t ledColor = colors[random(0,7)];
  led.fill(ledColor);
  led.show();
}

void handleUartCapsule(uint8_t packetId, uint8_t *dataIn, uint32_t len) {
  uint8_t* packetToSend = LoRaCapsule.encode(packetId,dataIn,len);
  LoRa.beginPacket();
  LoRa.write(packetToSend,LoRaCapsule.getCodedLen(len));
  LoRa.endPacket();
  LoRa.receive();
  delete[] packetToSend;

  uint32_t ledColor = colors[random(0,7)];
  led.fill(ledColor);
  led.show();
}