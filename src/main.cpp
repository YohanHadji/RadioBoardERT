#include <Arduino.h>
#include <Capsule.h>  
#include <LoopbackStream.h>
#include <LoRa.h>
#include <SPI.h>
#include <Adafruit_NeoPixel.h>
#include "../ERT_RF_Protocol_Interface/PacketDefinition.h"

#define DEBUG false

#define UART_PORT Serial1
#define UART_BAUD 115200

#define SERIAL_TO_PC    USBSerial
#define SERIAL_TO_PC_BAUD 115200

#define LORA_FREQ   867.0e6
#define LORA_POWER  20
#define LORA_BW     125.0e3
#define LORA_SF     8
#define LORA_CR     7
#define LORA_PREAMBLE_LEN 8
#define LORA_SYNC_WORD    0x12
#define LORA_CRC          true
#define LORA_CURRENT_LIMIT 120

// PIN/GPIO Definition
#define LORA_SCK                42
#define LORA_MOSI               44
#define LORA_MISO               43
#define LORA_CS                 41
#define LORA_INT0               21
#define LORA_INT5               39
#define LORA_RST                -1

#define NEOPIXEL_PIN            18

uint32_t colors[] = {
    0x000000,
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
  uint32_t ledColor = colors[random(0,8)];
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
}

void handleUartCapsule(uint8_t packetId, uint8_t *dataIn, uint32_t len) {
  uint8_t* packetToSend = LoRaCapsule.encode(packetId,dataIn,len);
  LoRa.beginPacket();
  LoRa.write(packetToSend,LoRaCapsule.getCodedLen(len));
  LoRa.endPacket();
  LoRa.receive();
  delete[] packetToSend;
}