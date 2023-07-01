#include <Arduino.h>
#include <SimpleCLI.h>

// Create CLI Object
SimpleCLI cli;
Command ir;
Command dr;
Command help;
Command version;

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

  // arm_jtag.reset();
  // arm_jtag.ir(code.c_str(), &output[0]);

  // Print response
  Serial.print("> ");
  Serial.print("ir = ");
  Serial.println(code);
  // Serial.println(output);
}

// Callback function for ping command
void drCallback(cmd* c) {
  Command cmd(c); // Create wrapper object

  // Get arguments
  Argument dr_data = cmd.getArgument("data");

  // Get value
  String data = dr_data.getValue();

  // arm_jtag.reset();
  // arm_jtag.ir(code.c_str(), &output[0]);

  // Print response
  Serial.print("> ");
  Serial.print("dr = ");
  Serial.println(data);
  // Serial.println(output);
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
