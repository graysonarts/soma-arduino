#include <Wire.h>
#include <MCP23017.h>
uint8_t oldSelectedChannel = 0;
uint8_t selectedChannel = 0;

#define ENABLE_SERIAL

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

//                              C0. C1. C2. C3...
int8_t button_pins[BTN_CNT] = { 0, 1, 2, 3, 6, 7, 8, 9, 10, 11, 12, 13 };
int8_t button_states[BTN_CNT] = { HIGH };
int8_t prev_button_states[BTN_CNT] = { HIGH };
unsigned long button_debounce_time[BTN_CNT] = { 0 };
unsigned long last_auto = 0;

#define DEBOUNCE_DELAY 50

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

  setChannel(0);

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

  DLN("Startup Done");
}

void loop() {
  unsigned long current_time = millis();
  bool channel_changed = false;

  // Read and debounce all buttons
  for (int i = 0; i < BTN_CNT; i++) {
    int8_t reading = digitalRead(button_pins[i]);
    
    // Check if button state has changed
    if (reading != prev_button_states[i]) {
      // Reset debounce timer
      button_debounce_time[i] = current_time;
    }
    
    // If enough time has passed since last change, update the state
    if ((current_time - button_debounce_time[i]) > DEBOUNCE_DELAY) {
      if (reading != button_states[i]) {
        button_states[i] = reading;
        
        // Detect button press (HIGH to LOW transition with pullup)
        if (button_states[i] == LOW) {
          selectedChannel = i;
          channel_changed = true;
          D("Button ");
          D(i);
          DLN(" pressed");
        }
      }
    }
    
    prev_button_states[i] = reading;
  }

#ifdef AUTO
  if (current_time - last_auto > 1000) {
    DLN("Auto Advancing");
    selectedChannel = (selectedChannel + 1) % BTN_CNT;
    last_auto = current_time;
    channel_changed = true;
  }
#endif

  // Display button states for debugging
  for (int i = 0; i < BTN_CNT; i++) {
    D(button_states[i] ? "X" : ".");
  }
  DBR;

  // Only update channel if it actually changed
  if (selectedChannel != oldSelectedChannel || channel_changed) {
    D("switching channel to ");
    DBIN(selectedChannel | 0x10);
    DBR;
    setChannel(selectedChannel);
    oldSelectedChannel = selectedChannel;
  }

  // Minimal delay for stability - much more responsive than 100ms
  delay(5);
}
