#include <Arduino.h>
#include <SimpleCLI.h>

#include "jtag.h"

// Create CLI Object
SimpleCLI cli;
Command ir;
Command dr;
Command help;
Command version;

JtagWire tck = JtagWire(2, OUTPUT);
JtagWire tms = JtagWire(3, OUTPUT);
JtagWire tdi = JtagWire(4, OUTPUT);
JtagWire tdo = JtagWire(5, INPUT);
JtagWire trst = JtagWire(6, OUTPUT);

JtagBus jtag_bus = JtagBus();
Jtag arm_jtag = Jtag();

const uint8_t IR_LEN = 9;
const uint8_t IR_FULL_LEN = 16;
const uint8_t DR_LEN = 33;
const uint8_t DR_FULL_LEN = 38;

byte output[256] = {};

void helpCallback(cmd* c) {
  Serial.println("This is Help command");
}

void versionCallback(cmd* c) {
  Serial.println("ver.0.0.1b");
}

// Callback function for ping command
void irCallback(cmd* c) {
  Command cmd(c); // Create wrapper object

  // Get arguments
  Argument ir_code = cmd.getArgument("code");

  // Get value
  String code = ir_code.getValue();

  arm_jtag.reset();
  arm_jtag.ir(code.c_str(), &output[0]);

  // Print response
  Serial.print("> TDO: ");
  for (size_t i = 0; i < IR_FULL_LEN + 7; i++) {
    Serial.print(jtag_bus.get_array_bit(i, output));
  }
  Serial.println("");
  Serial.println("");

  memset(output, 0, sizeof(output));
}

// Callback function for ping command
void drCallback(cmd* c) {
  Command cmd(c); // Create wrapper object

  // Get arguments
  Argument dr_data = cmd.getArgument("data");

  // Get value
  String data = dr_data.getValue();

  // arm_jtag.reset();
  arm_jtag.dr(data.c_str(), &output[0]);

  // Print response
  Serial.print("> TDO: ");
  for (size_t i = 0; i < DR_FULL_LEN + 7; i++) {
    Serial.print(jtag_bus.get_array_bit(i, output));
  }
  Serial.println("");
  Serial.println("");

  memset(output, 0, sizeof(output));
}

// Callback in case of an error
void errorCallback(cmd_error* e) {
  CommandError cmdError(e); // Create wrapper object

  Serial.print("ERROR: ");
  Serial.println(cmdError.toString());

  if (cmdError.hasCommand()) {
      Serial.print("Did you mean \"");
      Serial.print(cmdError.getCommand().toString());
      Serial.println("\"?");
  }
}

void setup() {
  Serial.begin(9600);

  cli.setOnError(errorCallback); // Set error Callback

  // Create the ir command with callback function
  ir = cli.addCommand("ir", irCallback);
  ir.addPositionalArgument("code");

  dr = cli.addCommand("dr", drCallback);
  dr.addPositionalArgument("data");

  help = cli.addCommand("help", helpCallback);
  version = cli.addCommand("version", versionCallback);

  jtag_bus.assign_pin(JTAG::PIN::TCK, tck);
  jtag_bus.assign_pin(JTAG::PIN::TMS, tms);
  jtag_bus.assign_pin(JTAG::PIN::TDI, tdi);
  jtag_bus.assign_pin(JTAG::PIN::TDO, tdo);
  jtag_bus.assign_pin(JTAG::PIN::TRST, trst);
  jtag_bus.set_speed(800000);

  arm_jtag.add_bus(jtag_bus);

  Serial.println("Welcome to the Arduino JTAG Box!");
}

void loop() {
  // Check if user typed something into the serial monitor
  if (Serial.available()) {
    // Read out string from the serial monitor
    String input = Serial.readStringUntil('\n');

    // Parse the user input into the CLI
    cli.parse(input);
  }

  if (cli.errored()) {
    CommandError cmdError = cli.getError();

    Serial.print("ERROR: ");
    Serial.println(cmdError.toString());

    if (cmdError.hasCommand()) {
      Serial.print("Did you mean \"");
      Serial.print(cmdError.getCommand().toString());
      Serial.println("\"?");
    }
  }
}
