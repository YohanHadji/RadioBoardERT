#include <Adafruit_NeoPixel.h>
#include <Arduino.h>
#include <Capsule.h>
#include <LoRa.h>
#include <LoopbackStream.h>
#include <SPI.h>

#include "../ERT_RF_Protocol_Interface/PacketDefinition.h"
#include "../ERT_RF_Protocol_Interface/ParameterDefinition.h"
#include "config.h"


#if SEND_TO_DB
  #include <WiFiMulti.h>
  #include <InfluxDbClient.h>

  WiFiMulti wifiMulti;

  // WiFi AP SSID
  #define WIFI_SSID "MASSCHALLENGE"
  // WiFi password
  #define WIFI_PASSWORD "innovation2016++"
  // InfluxDB  server url. Don't use localhost, always server name or ip address.
  // E.g. http://192.168.1.48:8086 (In InfluxDB 2 UI -> Load Data -> Client Libraries),
  // #define INFLUXDB_URL "http://172.31.112.228:8086"
  #define INFLUXDB_URL "http://10.10.3.168:8086"
  // InfluxDB 2 server or cloud API authentication token (Use: InfluxDB UI -> Load Data -> Tokens -> <select token>)
  #define INFLUXDB_TOKEN "PJj8u6PZN1QVggN1lkhb1bkoX9rtegXEsdh8Mk9VeWw_mvqTobYfZJpXRM2T5Z_EDWziw1zN-MdUIEo6aGB5pQ=="  // NUC token
  // #define INFLUXDB_TOKEN "iuRjFlnrry3usfeOw62O_T03RmIqyppnPkUqsuAkGflfXoBdSpkgme-kg5IH1NBQNp7_cEMUY53q8KhtQDp6MA=="
  // InfluxDB 2 organization id (Use: InfluxDB UI -> Settings -> Profile -> <name under tile> )
  // #define INFLUXDB_ORG "Xstrato"
  #define INFLUXDB_ORG "29306b5a85a43289"
  #define INFLUXDB_BUCKET "Nordend"

    // InfluxDB client instance
  InfluxDBClient client(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN);

  // Data point
  //Point AVTelemetry("AVTelemetry");
  Point GSETelemetry("GSETelemetry");

  void setupInfluxDb();
#endif 

#define LED_COLOR_TIME 100 // Color of the led will be changed for x ms each time a packet is received
static unsigned long lastPacketReceived = 0;


uint32_t colors[] = {
    0xFF0000,  // Red
    0x00FF00,  // Green
    0x0000FF,  // Blue
    0x32A8A0,  // Cyan
    0xFFEA00,  // Yellow
    0xCF067C,  // Purple
    0xFF0800   // Orange
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
#if !MODE_STANDALONE
    UART_PORT.begin(UART_BAUD, 134217756U, 6, 5); // This for radioboard
    //UART_PORT.begin(UART_BAUD, 134217756U, 9, 46); // This for cmdIn
#endif

    led.begin();
    led.fill(colors[INITIAL_LED_COLOR]);
    led.show();

    SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_CS); 
    LoRa.setPins(LORA_CS, LORA_RST, LORA_INT0);
    LoRa.setSPI(SPI);
    
    if (!LoRa.begin(LORA_FREQ)) {
        if (DEBUG) {
            SERIAL_TO_PC.println("Starting LoRa failed!");
        }
    }

    // Set LoRa parameters
    LoRa.setTxPower(LORA_POWER);
    LoRa.setSpreadingFactor(LORA_SF);
    LoRa.setSignalBandwidth(LORA_BW);
    LoRa.setCodingRate4(LORA_CR);
    LoRa.setPreambleLength(LORA_PREAMBLE_LEN);
#if (LORA_CRC)
    LoRa.enableCrc();  // not necessary to work with miaou, even if miaou enbale it...:-|
#else
    LoRa.disableCrc();
#endif
#if (LORA_INVERSE_IQ)
    LoRa.enableInvertIQ();
#else
    LoRa.disableInvertIQ();
#endif
    LoRa.onReceive(handlePacketLoRa);
    LoRa.receive(); 
}

void loop() {
    while (LoRaRxBuffer.available()) {
        // char x = LoRaRxBuffer.read();
        // LoRaCapsule.decode(x);
        // USBSerial.println(x);
        // USBSerial.print("m");
        LoRaCapsule.decode(LoRaRxBuffer.read());
    }

    while (UART_PORT.available()) {
        UartCapsule.decode(UART_PORT.read());
    }

    if ((millis()-lastPacketReceived)>LED_COLOR_TIME) {
        led.fill(colors[INITIAL_LED_COLOR]);
        led.show();
    }

    // static uint32_t t = 0;
    // if (millis() - t > 4000) {
    //     t = millis();
    //     uint8_t x[] = {1,2,3,4,5,6,7};
    //     uint8_t *packetToSend = LoRaCapsule.encode(0x12, x, sizeof(x));
    //     LoRa.beginPacket();
    //     LoRa.write(packetToSend, LoRaCapsule.getCodedLen(sizeof(x)));
    //     LoRa.endPacket();
    //     LoRa.receive();
    //     delete[] packetToSend;
    // }
}

void handlePacketLoRa(int packetSize) {
    for (int i = 0; i < packetSize; i++) {
        LoRaRxBuffer.write(LoRa.read());
    }
}

void handleLoRaCapsule(uint8_t packetId, uint8_t *dataIn, uint32_t len) {
    lastPacketReceived = millis();
    uint32_t ledColor = colors[INITIAL_LED_COLOR+1];
    led.fill(ledColor);
    led.show();

    uint8_t* packetToSend = UartCapsule.encode(packetId,dataIn,len);
    UART_PORT.write(packetToSend,UartCapsule.getCodedLen(len));
    delete[] packetToSend;
}

void handleUartCapsule(uint8_t packetId, uint8_t *dataIn, uint32_t len) {
    lastPacketReceived = millis();
    uint32_t ledColor = colors[INITIAL_LED_COLOR+1];
    led.fill(ledColor);
    led.show();

    uint8_t* packetToSend = LoRaCapsule.encode(packetId,dataIn,len);
    LoRa.beginPacket();
    LoRa.write(packetToSend,LoRaCapsule.getCodedLen(len));
    LoRa.endPacket();
    LoRa.receive();
    delete[] packetToSend;
}

#if SEND_TO_DB
    void setupInfluxDb() {
    // Connect WiFi
    SERIAL_TO_PC.println("Connecting to WiFi");
    WiFi.mode(WIFI_STA);
    wifiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);
    while (wifiMulti.run() != WL_CONNECTED) {
        SERIAL_TO_PC.print(".");
        delay(500);
    }
    SERIAL_TO_PC.println();
    SERIAL_TO_PC.println("Wifi connected :-)");

    // Set InfluxDB 1 authentication params
    // client.setConnectionParamsV1(INFLUXDB_URL, INFLUXDB_DB_NAME, INFLUXDB_USER, INFLUXDB_PASSWORD);

    // Add constant tags - only once
    GSETelemetry.addTag("device", RADIOMODULE_NAME);

    // Check server connection
    if (client.validateConnection()) {
        SERIAL_TO_PC.print("Connected to InfluxDB: ");
        SERIAL_TO_PC.println(client.getServerUrl());
    } else {
        SERIAL_TO_PC.print("InfluxDB connection failed: ");
        SERIAL_TO_PC.println(client.getLastErrorMessage());
    }
  }
#endif