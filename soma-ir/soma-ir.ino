#include <Arduino.h>
#include <IRremote.hpp>
#include <Wire.h>

// #define AUTO_ADVANCE

#define SDA 4
#define SCL 5
#define I2CADDR 0x21

#define BANK_SELECT_PIN 10
#define BANK_1_CHANNEL_PIN 11
#define BANK_2_CHANNEL_PIN 12
#define BANK_3_CHANNEL_PIN 13

#define CHANNEL_CODE_1 0x7
#define CHANNEL_CODE_2 0x19
#define CHANNEL_CODE_3 0x9
#define CHANNEL_CODE_4 0x40

#define ADDRESS 0x00
#define REPEATS 0

#define DELAY_AFTER_SEND 50

bool hasSentInitialData = false;

uint8_t bankChannelPins[] = {BANK_1_CHANNEL_PIN, BANK_2_CHANNEL_PIN, BANK_3_CHANNEL_PIN};
uint16_t commands[] = {CHANNEL_CODE_1, CHANNEL_CODE_2, CHANNEL_CODE_3, CHANNEL_CODE_4};
uint8_t commandIndex = 0;
uint8_t COMMANDS_SIZE = 4;

uint8_t selectedBank = 0;
uint8_t selectedChannel = 0;
uint8_t lastBank = 0;
uint8_t lastChannel = 0;

unsigned long last_auto = 0;
uint8_t fakeIn = 0;

void handleRx(int numBytes)
{
  byte rxByte[2];

  Serial.print("Receiving I2C with byte count ");
  Serial.println(numBytes);

  for (uint8_t i = 0; i < max(numBytes, 2); i++)
  {
    rxByte[i] = Wire.read();
  }
  if (numBytes > 2)
  {
    // Eat everything after the first two bytes
    while (Wire.available())
    {
      Wire.read();
    }
  }

  Serial.print("Bytes received: ");
  Serial.print(rxByte[0]);
  Serial.print(" ");
  Serial.println(rxByte[1]);

  // Decode
  if (rxByte[0] != 0x13)
  {
    // UNKNOWN COMMAND, IGNORING
    return;
  }

  selectedChannel = (rxByte[1] & 0x0F) % 4;
  selectedBank = (rxByte[1] & 0x0F) / 4;
}

void selectBank(uint8_t bank)
{
  Serial.print("Selecting bank ");
  Serial.println(bank);

  // Bank select
  IrSender.setSendPin(BANK_SELECT_PIN);
  IrSender.sendNEC(ADDRESS, commands[bank], REPEATS);
}

void selectChannelOnBank(uint8_t channel, uint8_t bank)
{
  Serial.print("selecting channel ");
  Serial.print(channel);
  Serial.print(" on bank ");
  Serial.println(bank);

  // Channel select
  IrSender.setSendPin(bankChannelPins[bank]);
  IrSender.sendNEC(ADDRESS, commands[channel], REPEATS);
}

void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200); // Status message will be sent to PC at 9600 baud

  // This is useful for debugging, but it needs to be wrapped in some USB Serial ifdef, and I'm not sure which at the moment
  // while (!Serial);

  Serial.println(F("START " __FILE__ " from " __DATE__ "\r\nUsing library version " VERSION_IRREMOTE));

  // IR Setup
  uint8_t tSendPin = 0;
  IrSender.begin(tSendPin, ENABLE_LED_FEEDBACK, USE_DEFAULT_FEEDBACK_LED_PIN); // Specify send pin and enable feedback LED at default feedback LED pin
  Serial.print(F("Send IR signals at pin "));
  Serial.println(tSendPin);
  selectChannelOnBank(selectedChannel, selectedBank);
  selectBank(selectedBank);

  // I2C Setup
  Wire.setSDA(SDA);
  Wire.setSCL(SCL);
  Wire.begin(I2CADDR);
  Wire.onReceive(handleRx);
}

void loop()
{
  // Some debug output
  /*
  Serial.println();
  Serial.print(F("address=0x"));
  Serial.print(sAddress, HEX);
  Serial.print(F(" command=0x"));
  Serial.print(commands[commandIndex], HEX);
  Serial.print(F(" repeats="));
  Serial.println(sRepeats);
  Serial.println();
  Serial.println();
  Serial.flush();

  Serial.println(F("Send NEC"));
  Serial.flush();
  */
#ifdef AUTO_ADVANCE
  if (millis() - last_auto > 1000) {
    Serial.println("Auto Advancing");
    fakeIn = (fakeIn + 1) % 12;
    selectedChannel = fakeIn % 4;
    selectedBank = fakeIn / 4;
    last_auto = millis();
  }
#endif

  bool transmitted = false;

  if (selectedBank != lastBank) {
    selectChannelOnBank(selectedChannel, selectedBank);
    selectBank(selectedBank);
    transmitted = true;    
  } else if (selectedChannel != lastChannel) {
    selectChannelOnBank(selectedChannel, selectedBank);
    transmitted = true;
  }

  if (transmitted) {
    lastChannel = selectedChannel;
    lastBank = selectedBank;
    delay(DELAY_AFTER_SEND);
  }
}