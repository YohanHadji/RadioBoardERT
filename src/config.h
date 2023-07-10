#define DEBUG false

#define UART_PORT Serial1
#define UART_BAUD 115200

#define SERIAL_TO_PC    USBSerial
#define SERIAL_TO_PC_BAUD 115200

#ifdef AV_UPLINK
  #define LORA_FREQ         AV_UPLINK_FREQUENCY
  #define LORA_POWER        AV_UPLINK_POWER
  #define LORA_BW           AV_UPLINK_BW
  #define LORA_SF           AV_UPLINK_SF
  #define LORA_CR           AV_UPLINK_CR
  #define LORA_PREAMBLE_LEN AV_UPLINK_PREAMBLE_LEN
  #define LORA_SYNC_WORD    AV_UPLINK_SYNC_WORD
  #define LORA_CRC          AV_UPLINK_CRC
#elif AV_DOWNLINK
  #define LORA_FREQ         AV_DOWNLINK_FREQUENCY
  #define LORA_POWER        AV_DOWNLINK_POWER
  #define LORA_BW           AV_DOWNLINK_BW
  #define LORA_SF           AV_DOWNLINK_SF
  #define LORA_CR           AV_DOWNLINK_CR
  #define LORA_PREAMBLE_LEN AV_DOWNLINK_PREAMBLE_LEN
  #define LORA_SYNC_WORD    AV_DOWNLINK_SYNC_WORD
  #define LORA_CRC          AV_DOWNLINK_CRC
#elif GSE
  #define LORA_FREQ         GSE_FREQUENCY
  #define LORA_POWER        GSE_POWER
  #define LORA_BW           GSE_BW
  #define LORA_SF           GSE_SF
  #define LORA_CR           GSE_CR
  #define LORA_PREAMBLE_LEN GSE_PREAMBLE_LEN
  #define LORA_SYNC_WORD    GSE_SYNC_WORD
  #define LORA_CRC          GSE_CRC
#endif

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