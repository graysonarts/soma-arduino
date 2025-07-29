# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is an Arduino-based button controller system with two main components:
- **soma-btn**: Main button controller that reads 12 physical buttons and controls LEDs via MCP23017 I/O expanders
- **soma-ir**: IR transmission controller that receives I2C commands and sends infrared signals to control external devices

## Architecture

The system uses a distributed architecture with I2C communication:

### soma-btn (Button Controller)
- Reads 12 physical buttons using GPIO pins with INPUT_PULLUP
- Controls LEDs through MCP23017 I/O expander at address 0x20
- Sends channel selection commands to IR controller via I2C at address 0x21
- Uses cascade button detection to select active channel
- Supports optional AUTO mode for automatic channel cycling

### soma-ir (IR Controller) 
- Acts as I2C slave at address 0x21
- Receives channel selection commands (format: 0x13 + channel data)
- Controls IR transmission using IRremote library
- Supports bank selection (3 banks) and channel selection (4 channels per bank)
- Uses multiple IR send pins for different banks

## Key Components

- **MCP23017 I/O Expanders**: Used for LED control and I2C protocol simulation
- **Wire Library**: I2C communication between controllers
- **IRremote Library**: IR signal transmission on soma-ir
- **Button Matrix**: 12 buttons mapped to pins {0,1,2,3,6,7,8,9,10,11,12,13}

## Hardware Configuration

- I2C: SDA=4, SCL=5
- LED Controller: MCP23017 at 0x20
- IR Controller: I2C address 0x21
- Button pins use internal pullup resistors
- IR transmission pins: 10 (bank select), 11-13 (channel select per bank)

## Development Commands

Arduino IDE or compatible IDE should be used for compilation and upload. No special build scripts are present.

## Debug Features

- Serial output enabled with ENABLE_SERIAL define
- Debug macros: D(), DLN(), DBIN(), DHEX(), DBR()
- Button state visualization in serial output
- I2C transaction logging on IR controller