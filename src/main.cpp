//_____ I N C L U D E S _______________________________________________________
#include <Arduino.h>
#include <SimpleCLI.h>
#include <TimerOne.h>

#include "Jtag.hpp"
#include "version.hpp"

//_____ D E F I N I T I O N S _________________________________________________
#define MAX_DATA_LENGTH 64

#define TCK 2
#define TMS 3
#define TDI 4
#define TDO 5
#define RST 6

//_____ V A R I A B L E S _____________________________________________________
// Create CLI Object
SimpleCLI cli;
Command irCmd;
Command drCmd;
Command reset;
Command sequence;
Command help;
Command version;

Jtag jtag = Jtag(TMS, TDI, TDO, TCK, RST);

byte output[MAX_DATA_LENGTH];
byte dataBuffer[MAX_DATA_LENGTH];
byte dataBuffer2[MAX_DATA_LENGTH];
//_____ F U N C T I O N S _____________________________________________________
static byte hexCharToVal(char c)
{
  if ('0' <= c && c <= '9')
    return c - '0';
  if ('a' <= c && c <= 'f')
    return c - 'a' + 10;
  if ('A' <= c && c <= 'F')
    return c - 'A' + 10;
  return 0;
}

static byte *hexStringToBytes(String hex, byte *bytes)
{
  hex.trim();
  int len = hex.length();
  for (int i = 0; i < len && i < MAX_DATA_LENGTH * 2; i += 2)
  {
    bytes[i / 2] = (hexCharToVal(hex.charAt(i)) << 4) + hexCharToVal(hex.charAt(i + 1));
  }

  return bytes;
}

//_____ C L I  C A L L B A C K S _________________________________________________
static void heartbeat(void)
{
  digitalWrite(LED_BUILTIN, digitalRead(LED_BUILTIN) ^ 1);
}

void helpCmdCallback(cmd *c)
{
  Serial.println("Commands:");
  Serial.println("  reset             Reset the JTAG TAP. No options required.");
  Serial.println("  ir [X] -len [Y]  Write to the IR register of TAP.");
  Serial.println("                    -code X: IR instruction in hex (e.g., 1FE).");
  Serial.println("                    -len Y: Length of the instruction in bits.");
  Serial.println("  dr [X] -len [Y]  Write to the DR register of TAP.");
  Serial.println("                    -data X: Data for DR in hex (e.g., 1A2B).");
  Serial.println("                    -len Y: Length of the data in bits.");
  // Serial.println("  clock -tms [X] -tdi [Y]  Write value for TMS and TDI wires.");
  // Serial.println("                    -tms X: Bit value for TMS.");
  // Serial.println("                    -tdi Y: Bit value for TDI.");
  // Serial.println("  sequence -tms [X] -tdi [Y] -len [Z] Write sequence of bits.");
  // Serial.println("                    -tms X: Bits value for TMS in hex (e.g., 1A2B).");
  // Serial.println("                    -tdi Y: Bits value for TDI in hex (e.g., 1A2B).");
  // Serial.println("                    -len Y: Number of the batch of bits.");
}

void versionCmdCallback(cmd *c)
{
  Serial.print(F("Firmware version: "));
  Serial.println(FIRMWARE_VERSION);
}

// Callback function for ping command
void irCmdCallback(cmd *c)
{
  Command cmd(c);  // Create wrapper object

  // Get arguments
  Argument code_arg = cmd.getArgument("code");
  Argument len_arg = cmd.getArgument("len");

  // Get value
  String codeStr = code_arg.getValue();
  String lenStr = len_arg.getValue();

  uint16_t instruction = strtol(codeStr.c_str(), NULL, 16);
  uint16_t length = lenStr.toInt();

  jtag.ir(instruction, length);

  Serial.print("> ");
  Serial.println("IR written");

  memset(output, 0, sizeof(output));
}

// Callback function for ping command
void drCmdCallback(cmd *c)
{
  Command cmd(c);  // Create wrapper object

  // Get arguments
  Argument data_arg = cmd.getArgument("data");
  Argument len_arg = cmd.getArgument("len");

  String dataStr = data_arg.getValue();
  String lenStr = len_arg.getValue();

  uint32_t length = lenStr.toInt();
  byte *data = hexStringToBytes(dataStr, dataBuffer);

  jtag.dr(data, length, output);

  Serial.print("> ");
  Serial.println("DR written");

  uint16_t bytes_num = (length / 8) + 1;

  for (size_t i = 0; i < bytes_num; i++)
  {
    Serial.print(output[i], HEX);
    Serial.print(" ");
  }

  Serial.println(" ");

  memset(output, 0, sizeof(output));
  memset(dataBuffer, 0, sizeof(dataBuffer));
}

void sequenceCmdCallback(cmd *c)
{
  Command cmd(c);  // Create wrapper object

  // Get arguments
  uint32_t length = cmd.getArgument("len").getValue().toInt();

  String tmsStr = cmd.getArgument("tms").getValue();
  String tdiStr = cmd.getArgument("tdi").getValue();
  uint8_t *tms = hexStringToBytes(tmsStr, dataBuffer);
  uint8_t *tdi = hexStringToBytes(tdiStr, dataBuffer2);

  Serial.print("> ");

  JTAG::ERROR error = jtag.sequence(length, tms, tdi, output);
  if (error != JTAG::ERROR::NO)
  {
    Serial.println("Operation fault!");
  }
  else
  {
    uint16_t bytes_num = (length / 8) + 1;
    for (size_t i = 0; i < bytes_num; i++)
    {
      Serial.print(output[i], HEX);
      Serial.print(" ");
    }

    Serial.println(" ");
  }

  memset(output, 0, sizeof(output));
  memset(dataBuffer, 0, sizeof(dataBuffer));
  memset(dataBuffer2, 0, sizeof(dataBuffer2));
}

void resetCmdCallback(cmd *c)
{
  jtag.reset();
  Serial.println("> Reset");
}

// Callback in case of an error
void errorCallback(cmd_error *e)
{
  CommandError cmdError(e);  // Create wrapper object

  Serial.print("ERROR: ");
  Serial.println(cmdError.toString());

  if (cmdError.hasCommand())
  {
    Serial.print("Did you mean \"");
    Serial.print(cmdError.getCommand().toString());
    Serial.println("\"?");
  }
}

//_____ A R D U I N O ____________________________________________________________
void setup()
{
  Serial.begin(115200);

  cli.setOnError(errorCallback);  // Set error Callback

  // Create the irCmd command with callback function
  irCmd = cli.addCommand("ir", irCmdCallback);
  irCmd.addPositionalArgument("code");
  irCmd.addPositionalArgument("len");

  drCmd = cli.addCommand("dr", drCmdCallback);
  drCmd.addPositionalArgument("data");
  drCmd.addPositionalArgument("len");

  sequence = cli.addCommand("sequence", sequenceCmdCallback);
  sequence.addPositionalArgument("tms");
  sequence.addPositionalArgument("tdi");
  sequence.addPositionalArgument("len");

  reset = cli.addCommand("reset", resetCmdCallback);

  help = cli.addCommand("help", helpCmdCallback);

  version = cli.addCommand("version", versionCmdCallback);

  pinMode(LED_BUILTIN, OUTPUT);
  Timer1.initialize(1000000);
  Timer1.attachInterrupt(heartbeat);
}

void loop()
{
  // Check if user typed something into the serial monitor
  if (Serial.available())
  {
    // Read out string from the serial monitor
    String input = Serial.readStringUntil('\n');

    // Parse the user input into the CLI
    cli.parse(input);
  }

  if (cli.errored())
  {
    CommandError cmdError = cli.getError();

    Serial.print("ERROR: ");
    Serial.println(cmdError.toString());

    if (cmdError.hasCommand())
    {
      Serial.print("Did you mean \"");
      Serial.print(cmdError.getCommand().toString());
      Serial.println("\"?");
    }
  }
}
