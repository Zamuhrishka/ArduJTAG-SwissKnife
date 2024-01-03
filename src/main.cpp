//_____ I N C L U D E S _______________________________________________________
#include <Arduino.h>
#include <SimpleCLI.h>
#include <TimerOne.h>

#include "Jtag.hpp"
#include "version.hpp"

//_____ D E F I N I T I O N S _________________________________________________
#define MAX_DATA_LENGTH 128

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
// Command clock;
Command sequence;
Command help;
Command version;

Jtag arm_jtag = Jtag(TMS, TDI, TDO, TCK, RST);

byte output[500] = {};
byte dataBuffer[MAX_DATA_LENGTH];

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

static uint8_t swap_nibbles(uint8_t x)
{
  return (x << 4) | (x >> 4);
}

static void jtag_test_1(void)
{
  uint16_t instruction = 0x1FE;
  uint16_t length = 9;
  uint32_t data = 0;
  uint32_t id = 0;

  arm_jtag.reset();
  arm_jtag.ir(instruction = 0x1FE, length = 9);
  arm_jtag.dr((byte *)&data, length = 32, (uint8_t *)&id);

  Serial.print("> ");
  Serial.println(id, HEX);
}

static void jtag_test_2(void)
{
  uint8_t tms[] = {0x06, 0x60, 0x01, 0x00, 0x00, 0x00, 0x0C};
  uint8_t tdi[] = {0xC0, 0x3F, 0x00, 0x00, 0x00, 0x00, 0x00};
  size_t length = 54;

  arm_jtag.reset();
  arm_jtag.sequence(length = 54, tms, tdi, output);

  Serial.print("> ");

  for (size_t i = 0; i < length / 8; i++)
  {
    // uint8_t bb = swap_nibbles(output[i]);
    Serial.print(output[i], HEX);
    Serial.print(" ");
  }
  Serial.println(" ");
}

static void jtag_test_3(void)
{
  uint16_t instruction = 0x1FA;
  uint16_t length = 9;
  uint8_t data[] = {0x04, 0x00, 0x00, 0x00, 0x00};
  uint8_t data_1[] = {0x10, 0x00, 0x00, 0x00, 0x00};
  uint8_t data_2[] = {0x02, 0x00, 0x00, 0x00, 0x01};
  uint8_t data_3[] = {0x2E, 0x55, 0x55, 0x55, 0x55};
  uint8_t data_4[] = {0x07, 0x00, 0x00, 0x00, 0x00};
  uint8_t zeros[] = {0x00, 0x00, 0x00, 0x00, 0x00};
  uint8_t enable[] = {0x02, 0x00, 0x00, 0x80, 0x02};
  uint8_t enable_rd[] = {0x03, 0x00, 0x00, 0x80, 0x02};
  uint32_t id = 0;

  // ENABLE
  Serial.println("E============================E");
  arm_jtag.reset();
  arm_jtag.ir(instruction = 0x1FA, length = 9);
  arm_jtag.dr(enable, length = 36, output);

  Serial.println("----------------------------");

  arm_jtag.reset();
  arm_jtag.ir(instruction = 0x1FA, length = 9);
  arm_jtag.dr(enable_rd, length = 36, output);

  Serial.println("----------------------------");

  // arm_jtag.reset();
  // arm_jtag.dr(zeros, length = 36, output);

  // WRITE
  Serial.println("W============================W");
  arm_jtag.reset();
  arm_jtag.ir(instruction = 0x1FA, length = 9);
  arm_jtag.dr(data, length = 36, output);

  Serial.println("----------------------------");

  arm_jtag.reset();
  arm_jtag.ir(instruction = 0x1FB, length = 9);
  arm_jtag.dr(data_1, length = 36, output);

  Serial.println("----------------------------");

  arm_jtag.reset();
  arm_jtag.ir(instruction = 0x1FB, length = 9);
  arm_jtag.dr(data_2, length = 36, output);

  Serial.println("----------------------------");

  arm_jtag.reset();
  arm_jtag.ir(instruction = 0x1FB, length = 9);
  arm_jtag.dr(data_3, length = 36, output);

  Serial.println("----------------------------");

  // READ
  Serial.println("R============================R");
  arm_jtag.reset();
  arm_jtag.ir(instruction = 0x1FA, length = 9);
  arm_jtag.dr(data, length = 36, output);

  Serial.println("----------------------------");

  arm_jtag.reset();
  arm_jtag.ir(instruction = 0x1FB, length = 9);
  arm_jtag.dr(data_1, length = 36, output);

  Serial.println("----------------------------");

  arm_jtag.reset();
  arm_jtag.ir(instruction = 0x1FB, length = 9);
  arm_jtag.dr(data_2, length = 36, output);

  Serial.println("----------------------------");

  arm_jtag.reset();
  arm_jtag.ir(instruction = 0x1FB, length = 9);
  arm_jtag.dr(data_4, length = 36, output);

  arm_jtag.dr(zeros, length = 36, output);

  Serial.println("----------------------------");

  Serial.print("> ");

  for (size_t i = 0; i < 36 / 8; i++)
  {
    Serial.print(output[i], HEX);
    Serial.print(" ");
  }

  Serial.println(" ");
}

//_____ C L I  C A L L B A C K S _________________________________________________
static void heartbeat(void)
{
  // digitalWrite(LED_BUILTIN, HIGH);
  // delay(500);
  // digitalWrite(LED_BUILTIN, LOW);
  // delay(500);
  digitalWrite(LED_BUILTIN, digitalRead(LED_BUILTIN) ^ 1);
}

void helpCmdCallback(cmd *c)
{
  Serial.println("Commands:");
  Serial.println("  reset             Reset the JTAG TAP. No options required.");
  Serial.println("  ir [X] -len [Y]  Write to the IR register of TAP.");
  Serial.println("                    -ir X: IR instruction in hex (e.g., 1FE).");
  Serial.println("                    -len Y: Length of the instruction in bits.");
  Serial.println("  dr [X] -len [Y]  Write to the DR register of TAP.");
  Serial.println("                    -dr X: Data for DR in hex (e.g., 1A2B).");
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

  arm_jtag.ir(instruction, length);

  Serial.print("> ");
  Serial.println("IR written");

  memset(output, 0, sizeof(output));
}

// Callback function for ping command
void drCmdCallback(cmd *c)
{
  Command cmd(c);  // Create wrapper object

  String dataStr = cmd.getArgument("data").getValue();

  uint32_t length = cmd.getArgument("len").getValue().toInt();
  byte *data = hexStringToBytes(dataStr, dataBuffer);

  arm_jtag.dr(data, length, output);

  Serial.print("> ");
  Serial.println("DR written");

  for (size_t i = 0; i < length / 8; i++)
  {
    Serial.print(output[i], HEX);
    Serial.print(" ");
  }

  Serial.println(" ");

  memset(output, 0, sizeof(output));
  memset(dataBuffer, 0, sizeof(dataBuffer));
}

// void clockCmdCallback(cmd *c)
// {
//   Command cmd(c);  // Create wrapper object

//   // Get arguments
//   Argument arg_tms = cmd.getArgument("tms");
//   Argument arg_tdi = cmd.getArgument("tdi");

//   // Get value
//   int tms = arg_tms.getValue().toInt();
//   int tdi = arg_tdi.getValue().toInt();

//   uint8_t tdo = arm_jtag.clock(tms, tdi);

//   // Print response
//   Serial.print("> ");
//   Serial.println(tdo);
// }

void sequenceCmdCallback(cmd *c)
{
  Command cmd(c);  // Create wrapper object

  // Get arguments
  uint32_t length = cmd.getArgument("len").getValue().toInt();

  String tmsStr = cmd.getArgument("tms").getValue();
  String tdiStr = cmd.getArgument("tdi").getValue();
  uint8_t *tms = hexStringToBytes(tmsStr, dataBuffer);
  uint8_t *tdi = hexStringToBytes(tdiStr, dataBuffer);

  for (size_t i = 0; i < length / 8; i++)
  {
    Serial.print(tms[i], HEX);
    Serial.print(" ");
  }

  Serial.println(" ");

  // arm_jtag.sequence(length, tms, tdi, output);

  // // Print response
  // Serial.print("> ");

  // for (size_t i = 0; i < length / 8; i++)
  // {
  //   Serial.print(output[i], HEX);
  //   Serial.print(" ");
  // }

  // Serial.println(" ");

  // memset(output, 0, sizeof(output));
  // memset(dataBuffer, 0, sizeof(dataBuffer));
}

void resetCmdCallback(cmd *c)
{
  // blink();

  arm_jtag.reset();
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

  // clock = cli.addCommand("clock", clockCmdCallback);
  // clock.addPositionalArgument("tms");
  // clock.addPositionalArgument("tdi");

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

  // jtag_test_2();
  jtag_test_3();
  delay(1000);
}
