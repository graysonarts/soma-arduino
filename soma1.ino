#include <Wire.h>
#include <MCP23017.h>
uint8_t oldSelectedChannel = 0;
uint8_t selectedChannel = 0;

#define BTN_CNT 3
#define MCP_ADDR 0x20

int8_t button_pins[BTN_CNT] = {13, 14, 15};
int8_t button_states[BTN_CNT] = {-1};

MCP23017 mcp = MCP23017(MCP_ADDR, Wire1);

void setup()
{

	Serial.begin(115200);
	delay(1000);
	Serial.println("Start up");
  // Setup the MCP to be configured the way we need it
	Wire1.setSDA(2);
	Wire1.setSCL(3);
  Wire1.begin();
	mcp.init();
	delay(1000);

	Serial.println("Setting MCP ports to output");
	mcp.portMode(MCP23017Port::A, 0);
	mcp.portMode(MCP23017Port::B, 0);

	mcp.writeRegister(MCP23017Register::GPIO_A, 0x00);  //Reset port A 
	mcp.writeRegister(MCP23017Register::GPIO_B, 0x00);  //Reset port B

  // Configure button pins for input - We only give a little time
  for (int i = 0; i < BTN_CNT; i++)
  {
    pinMode(button_pins[i], INPUT_PULLUP);
  }

  // Configure led pins for output



  uint8_t conf = mcp.readRegister(MCP23017Register::IODIR_A);
	Serial.print("IODIR_A : ");
	Serial.print(conf, BIN);
	Serial.println();
	
	conf = mcp.readRegister(MCP23017Register::IODIR_B);
	Serial.print("IODIR_B : ");
	Serial.print(conf, BIN);
	Serial.println();

	conf = mcp.readRegister(MCP23017Register::IPOL_A);
	Serial.print("IPOL_A : ");
	Serial.print(conf, BIN);
	Serial.println();

	conf = mcp.readRegister(MCP23017Register::IPOL_B);
	Serial.print("IPOL_B : ");
	Serial.print(conf, BIN);
	Serial.println();

	conf = mcp.readRegister(MCP23017Register::GPINTEN_A);
	Serial.print("GPINTEN_A : ");
	Serial.print(conf, BIN);
	Serial.println();

	conf = mcp.readRegister(MCP23017Register::GPINTEN_B);
	Serial.print("GPINTEN_B : ");
	Serial.print(conf, BIN);
	Serial.println();

	conf = mcp.readRegister(MCP23017Register::DEFVAL_A);
	Serial.print("DEFVAL_A : ");
	Serial.print(conf, BIN);
	Serial.println();

	conf = mcp.readRegister(MCP23017Register::DEFVAL_B);
	Serial.print("DEFVAL_B : ");
	Serial.print(conf, BIN);
	Serial.println();

	conf = mcp.readRegister(MCP23017Register::INTCON_A);
	Serial.print("INTCON_A : ");
	Serial.print(conf, BIN);
	Serial.println();

	conf = mcp.readRegister(MCP23017Register::INTCON_B);
	Serial.print("INTCON_B : ");
	Serial.print(conf, BIN);
	Serial.println();

	conf = mcp.readRegister(MCP23017Register::IOCON);
	Serial.print("IOCON : ");
	Serial.print(conf, BIN);
	Serial.println();

	//conf = mcp.readRegister(IOCONB);
	//Serial.print("IOCONB : ");
	//Serial.print(conf, BIN);
	//Serial.println();

	conf = mcp.readRegister(MCP23017Register::GPPU_A);
	Serial.print("GPPU_A : ");
	Serial.print(conf, BIN);
	Serial.println();

	conf = mcp.readRegister(MCP23017Register::GPPU_B);
	Serial.print("GPPU_B : ");
	Serial.print(conf, BIN);
	Serial.println();

	conf = mcp.readRegister(MCP23017Register::INTF_A);
	Serial.print("INTF_A : ");
	Serial.print(conf, BIN);
	Serial.println();

	conf = mcp.readRegister(MCP23017Register::INTF_B);
	Serial.print("INTF_B : ");
	Serial.print(conf, BIN);
	Serial.println();

	conf = mcp.readRegister(MCP23017Register::INTCAP_A);
	Serial.print("INTCAP_A : ");
	Serial.print(conf, BIN);
	Serial.println();

	conf = mcp.readRegister(MCP23017Register::INTCAP_B);
	Serial.print("INTCAP_B : ");
	Serial.print(conf, BIN);
	Serial.println();

	conf = mcp.readRegister(MCP23017Register::GPIO_A);
	Serial.print("GPIO_A : ");
	Serial.print(conf, BIN);
	Serial.println();

	conf = mcp.readRegister(MCP23017Register::GPIO_B);
	Serial.print("GPIO_B : ");
	Serial.print(conf, BIN);
	Serial.println();

	conf = mcp.readRegister(MCP23017Register::OLAT_A);
	Serial.print("OLAT_A : ");
	Serial.print(conf, BIN);
	Serial.println();

	conf = mcp.readRegister(MCP23017Register::OLAT_B);
	Serial.print("OLAT_B : ");
	Serial.print(conf, BIN);
	Serial.println();

	Serial.println("Startup Done");

}

void loop()
{
  // - Poll the state of all the button pins
  // - Cascade button detection, and only change the state of the selected channel if the button is different than the
  // last time around

  // If we have a new state, update the MCP and turn on the LED for the selected channel, turn off all other leds
  // Wait for 100ms

  for (int i = 0; i < BTN_CNT; i++)
  {
    button_states[i] = digitalRead(button_pins[i]);
  }

  for (int i = 0; i < BTN_CNT; i++)
  {
    if (button_states[i] == LOW)
    {
      selectedChannel = i + 1;
      break;
    }
  }

  if (selectedChannel != oldSelectedChannel)
  {
    Serial.print("switching channel to ");
    Serial.println(selectedChannel);
		mcp.writePort(MCP23017Port::A, selectedChannel); 
    oldSelectedChannel = selectedChannel;
    delay(1000);
  }
  // else
  // {
  //   for (int i = 0; i < BTN_CNT; i++)
  //   {
  //     Serial.print(button_states[i]);
  //     Serial.print(" ");
  //   }
  //   Serial.println();
  // }
  delay(100);
}
