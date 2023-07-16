#include <Arduino.h>
#include <SimpleCLI.h>

#include "jtag.h"


#define TCK 2
#define TMS 3
#define TDI 4
#define TDO 5
#define RST 6


// Create CLI Object
SimpleCLI cli;
Command ir;
Command dr;
Command reset;
Command clock;
Command help;
Command version;

Jtag arm_jtag = Jtag(TMS, TDI, TDO, TCK, RST);

const uint8_t IR_LEN = 9;
const uint8_t IR_FULL_LEN = 16;
const uint8_t DR_LEN = 33;
const uint8_t DR_FULL_LEN = 38;

char output[256] = {};

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

  arm_jtag.ir(code.c_str(), output);

  // Print response
  Serial.print("> ");
  Serial.println(output);

  memset(output, 0, sizeof(output));
}

// Callback function for ping command
void drCallback(cmd* c) {
  Command cmd(c); // Create wrapper object

  // Get arguments
  Argument dr_data = cmd.getArgument("data");

  // Get value
  String data = dr_data.getValue();

  arm_jtag.dr(data.c_str(), output);

  // Print response
  Serial.print("> ");
  Serial.println(output);

  memset(output, 0, sizeof(output));
}

void clockCallback(cmd* c) {
  Command cmd(c); // Create wrapper object

  // Get arguments
  Argument arg_tms = cmd.getArgument("tms");
  Argument arg_tdi = cmd.getArgument("tdi");

  // Get value
  int tms = arg_tms.getValue().toInt();
  int tdi = arg_tdi.getValue().toInt();

  uint8_t tdo = arm_jtag.clock(tms, tdi);

  // Print response
  Serial.print("> ");
  Serial.println(tdo);
}

void resetCallback(cmd* c) {
    digitalWrite(LED_BUILTIN, HIGH);  // turn the LED on (HIGH is the voltage level)
    delay(500);                      // wait for a second
    digitalWrite(LED_BUILTIN, LOW);   // turn the LED off by making the voltage LOW
    delay(500);                      // wait for a second

  arm_jtag.reset();
  Serial.println("> Reset");
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

  clock = cli.addCommand("clock", clockCallback);
  clock.addPositionalArgument("tms");
  clock.addPositionalArgument("tdi");

  reset = cli.addCommand("reset", resetCallback);
  help = cli.addCommand("help", helpCallback);
  version = cli.addCommand("version", versionCallback);

  pinMode(LED_BUILTIN, OUTPUT);
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
