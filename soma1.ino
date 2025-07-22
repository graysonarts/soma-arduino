#include <Wire.h>
#include <MCP23017.h>
uint8_t oldSelectedChannel = 0;
uint8_t selectedChannel = 0;

#define ENABLE_SERIAL

#ifdef ENABLE_SERIAL
#define D(x) Serial.print(x);
#define DLN(x) Serial.println(x);
#define DBR Serial.println();
#else
#define DBG(x) ;
#define DBGLN(x) ;
#define DBGBR ;
#endif

#define SDA 4
#define SCL 5
#define BTN_CNT 12
#define MCP_ADDR 0x20

int8_t button_pins[BTN_CNT] = { 16, 17, 18, 19, 20, 21, 22, 26, 27, 0, 1 };
int8_t button_states[BTN_CNT] = { -1 };
int8_t led_pins[BTN_CNT] = { 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 2, 3 };
unsigned long last_auto = 0;

MCP23017 mcp = MCP23017(MCP_ADDR, Wire);

void setup() {
  #ifdef ENABLE_SERIAL
  Serial.begin(115200);
  while (!Serial);
  #endif
  DLN("Start up");

  // Setup the MCP to be configured the way we need it
  Wire.setSDA(SDA);
  Wire.setSCL(SCL);
  Wire.begin();
  mcp.init();
  delay(1000);

  DLN("Setting MCP ports to output");
  mcp.portMode(MCP23017Port::A, 0);
  mcp.portMode(MCP23017Port::B, 0);

  mcp.writeRegister(MCP23017Register::GPIO_A, 0x00);  //Reset port A
  mcp.writeRegister(MCP23017Register::GPIO_B, 0x00);  //Reset port B

  // Configure button pins for input
  // Configuer led pins for output
  for (int i = 0; i < BTN_CNT; i++) {
    pinMode(button_pins[i], INPUT_PULLUP);
    // pinMode(led_pins[i], OUTPUT);
  }




  uint8_t conf = mcp.readRegister(MCP23017Register::IODIR_A);
  D("IODIR_A : ");
  D(conf, BIN);
  DBR;

  conf = mcp.readRegister(MCP23017Register::IODIR_B);
  D("IODIR_B : ");
  D(conf, BIN);
  DBR;

  DLN("Startup Done");
}

void loop() {
  // - Poll the state of all the button pins
  // - Cascade button detection, and only change the state of the selected channel if the button is different than the
  // last time around

  // If we have a new state, update the MCP and turn on the LED for the selected channel, turn off all other leds
  // Wait for 100ms

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
    D(button_states[i] ? "X" : ".");
    // Set LED channels
    if (button_states[i] == LOW) {
      selectedChannel = i;
      // break;
    }
  }
  DBR;


  if (selectedChannel != oldSelectedChannel) {
    D("switching channel to ");
    DLN(selectedChannel | 0x10, BIN);
    mcp.writePort(MCP23017Port::B, (selectedChannel) | 0x10);
    oldSelectedChannel = selectedChannel;
    delay(2000);
  }

  delay(100);
}
