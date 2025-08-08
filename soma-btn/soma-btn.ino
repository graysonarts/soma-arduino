#include <Wire.h>
#include <MCP23017.h>
uint8_t oldSelectedChannel = 0;
uint8_t selectedChannel = 0;

#define ENABLE_SERIAL

#define AUTO

#ifdef ENABLE_SERIAL
#define D(x) Serial.print(x);
#define DBIN(x) Serial.print(x, BIN);
#define DHEX(x) Serial.print(x, HEX);
#define DLN(x) Serial.println(x);
#define DBR Serial.println();
#else
#define DBG(x) ;
#define DBGLN(x) ;
#define DBIN(x) ;
#define DHEX(x) ;
#define DBGBR ;
#endif

#define SDA 4
#define SCL 5
#define BTN_CNT 12
#define LED_ADDR 0x20
#define IR_ADDR 0x21

#define MUTE 13 // This sets the bank to 4 which has no input connected to it.
#define IDLE_TIMEOUT 30000 // 30s in ms

//                               0.  1.  2.  3. 4. 5  6. 7. 8. 9. 10 11
int8_t button_pins[BTN_CNT] = { 13, 12, 11, 10, 9, 8, 7, 6, 2, 3, 1, 0 };
int8_t button_states[BTN_CNT] = { -1 };
unsigned long last_auto = 0;

MCP23017 ledMcp = MCP23017(LED_ADDR, Wire);
MCP23017 irMcp = MCP23017(IR_ADDR, Wire);  // Not a real MCP, just using the same protocol for write

void setChannel(uint8_t chan) {
  // Channels 0-7 use PORT B, channels 8-11 use PORT A
  if (chan < 8) {
    ledMcp.writePort(MCP23017Port::A, 0);
    ledMcp.writePort(MCP23017Port::B, 1 << chan);
  } else {
    ledMcp.writePort(MCP23017Port::B, 0);
    ledMcp.writePort(MCP23017Port::A, 1 << (chan - 8));
  }
  irMcp.writePort(MCP23017Port::B, (selectedChannel) | 0x10);
}
 
void setup() {
#ifdef ENABLE_SERIAL
  Serial.begin(115200);
#ifdef DEBUG
  while (!Serial)
    ;
#endif  // DEBUG
#endif  // SERIAL
  DLN("Start up");

  // Setup the MCP to be configured the way we need it
  Wire.setSDA(SDA);
  Wire.setSCL(SCL);
  Wire.begin();
  ledMcp.init();
  irMcp.init();
  delay(1000);

  DLN("Setting MCP ports to output");
  ledMcp.portMode(MCP23017Port::A, 0, 0xFF, 0xFF);
  ledMcp.portMode(MCP23017Port::B, 0, 0xFF, 0xFF);


  ledMcp.writeRegister(MCP23017Register::GPIO_A, 0x00);  //Reset port A
  ledMcp.writeRegister(MCP23017Register::GPIO_B, 0x00);  //Reset port B

  setChannel(MUTE);

  // Configure button pins for input
  // Configuer led pins for output
  for (int i = 0; i < BTN_CNT; i++) {
    pinMode(button_pins[i], INPUT_PULLUP);
  }

  uint8_t conf = ledMcp.readRegister(MCP23017Register::IODIR_A);
  D("IODIR_A : ");
  DBIN(conf);
  DBR;

  conf = ledMcp.readRegister(MCP23017Register::IODIR_B);
  D("IODIR_B : ");
  DBIN(conf);
  DBR;

  DLN("Enabling Watchdog timer");
  rp2040.wdt_begin(8000); // If it doesn't respond for 8 seconds, restart

  DLN("Startup Done");
}

void loop() {
  // - Poll the state of all the button pins
  // - Cascade button detection, and only change the state of the selected channel if the button is different than the
  // last time around

  // If we have a new state, update the MCP and turn on the LED for the selected channel, turn off all other leds
  // Wait for 100ms

  rp2040.wdt_reset(); // Feed the watchdog timer

  for (int i = 0; i < BTN_CNT; i++) {
    button_states[i] = digitalRead(button_pins[i]);
  }

#ifdef AUTO
  if (millis() - last_auto > 1000) {
    Serial.println("Auto Advancing");
    selectedChannel = (selectedChannel + 1) % BTN_CNT;
    last_auto = millis();
  }
#endif

  for (int i = 0; i < BTN_CNT; i++) {
    #ifdef DEBUG
    D(button_states[i] ? "X" : ".");
    #endif // DEBUG
    // Set LED channels
    if (button_states[i] == LOW) {
      selectedChannel = i;
      // break;
    }
  }
#ifdef DEBUG
  DBR;
#endif // DEBUG



  if (selectedChannel != oldSelectedChannel) {
    D("switching channel to ");
    DBIN(selectedChannel | 0x10);
    DBR;
    setChannel(selectedChannel);
    oldSelectedChannel = selectedChannel;
    last_auto = millis();
  }

  if (millis() - last_auto > IDLE_TIMEOUT && oldSelectedChannel != MUTE) {
    DLN("Muting from idle time")
    setChannel(MUTE);
    oldSelectedChannel = MUTE;
    selectedChannel = MUTE;
  }

  delay(20);
}
