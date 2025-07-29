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

// Interrupt-based button handling
volatile bool button_interrupt_flags[BTN_CNT] = { false };
volatile unsigned long interrupt_timestamps[BTN_CNT] = { 0 };

#define DEBOUNCE_DELAY 50

MCP23017 ledMcp = MCP23017(LED_ADDR, Wire);
MCP23017 irMcp = MCP23017(IR_ADDR, Wire);  // Not a real MCP, just using the same protocol for write

// Generic button interrupt handler
void buttonInterrupt(int button_index) {
  unsigned long current_time = millis();
  
  // Simple time-based debouncing in ISR
  if (current_time - interrupt_timestamps[button_index] > DEBOUNCE_DELAY) {
    button_interrupt_flags[button_index] = true;
    interrupt_timestamps[button_index] = current_time;
  }
}

// Individual interrupt handlers for each button
void button0_isr() { buttonInterrupt(0); }
void button1_isr() { buttonInterrupt(1); }
void button2_isr() { buttonInterrupt(2); }
void button3_isr() { buttonInterrupt(3); }
void button4_isr() { buttonInterrupt(4); }
void button5_isr() { buttonInterrupt(5); }
void button6_isr() { buttonInterrupt(6); }
void button7_isr() { buttonInterrupt(7); }
void button8_isr() { buttonInterrupt(8); }
void button9_isr() { buttonInterrupt(9); }
void button10_isr() { buttonInterrupt(10); }
void button11_isr() { buttonInterrupt(11); }

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

  // Configure button pins for input with interrupts
  void (*button_handlers[BTN_CNT])() = {
    button0_isr, button1_isr, button2_isr, button3_isr,
    button4_isr, button5_isr, button6_isr, button7_isr,
    button8_isr, button9_isr, button10_isr, button11_isr
  };
  
  for (int i = 0; i < BTN_CNT; i++) {
    pinMode(button_pins[i], INPUT_PULLUP);
    button_states[i] = digitalRead(button_pins[i]);
    prev_button_states[i] = button_states[i];
    
    // Attach interrupt for FALLING edge (button press with pullup)
    attachInterrupt(digitalPinToInterrupt(button_pins[i]), button_handlers[i], FALLING);
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

  // Process interrupt flags from button presses
  for (int i = 0; i < BTN_CNT; i++) {
    if (button_interrupt_flags[i]) {
      // Disable interrupts temporarily while processing
      noInterrupts();
      button_interrupt_flags[i] = false;
      interrupts();
      
      // Verify button is still pressed (additional debouncing)
      if (digitalRead(button_pins[i]) == LOW) {
        selectedChannel = i;
        channel_changed = true;
        D("Button ");
        D(i);
        DLN(" pressed (interrupt)");
        
        // Update button state
        button_states[i] = LOW;
      }
    } else {
      // Update released button states
      int8_t current_state = digitalRead(button_pins[i]);
      if (button_states[i] == LOW && current_state == HIGH) {
        button_states[i] = HIGH;
      }
    }
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

  delay(2);
}
