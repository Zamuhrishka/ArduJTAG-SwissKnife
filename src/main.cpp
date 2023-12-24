#include <Arduino.h>
#include <SimpleCLI.h>

#include "jtag.h"

#define MAX_DATA_LENGTH 128  // Максимальная длина входных данных в байтах

#define TCK 2
#define TMS 3
#define TDI 4
#define TDO 5
#define RST 6

// Create CLI Object
SimpleCLI cli;
Command irCmd;
Command drCmd;
Command reset;
Command clock;
Command help;
Command version;

Jtag arm_jtag = Jtag(TMS, TDI, TDO, TCK, RST);

const uint8_t IR_LEN = 9;
const uint8_t IR_FULL_LEN = 16;
const uint8_t DR_LEN = 33;
const uint8_t DR_FULL_LEN = 38;

// char output[256] = {};
byte output[500] = {};

byte dataBuffer[MAX_DATA_LENGTH];  // Статический массив для данных

static void jtag_idcode3()
{
  Serial.println("Read of `IDCODE` `DP`register");
  arm_jtag.reset();
  memset(output, '\0', sizeof(output));

  // byte instruction[] = {0xFF, 0x00};
  uint16_t instruction = 0x1FE;
  byte data11[] = {0x00, 0x00, 0x00, 0x00};
  byte output1[8] = {0, 0, 0, 0, 0, 0, 0, 0};
  uint32_t id = 0;

  uint32_t d = 0x0000FF;

  arm_jtag.ir(instruction, 9);
  memset(output1, '\0', sizeof(output1));

  Serial.println("------------------------------------------------------------------------------------------");
  arm_jtag.dr(data11, 33, /*output1*/ (byte *)&id);
  // arm_jtag.dr((byte *)&d, 24, output1);
  Serial.println("------------------------------------------------------------------------------------------");
  Serial.print("> ");
  Serial.println(id, HEX);  // Новая строка в конце

  delay(300);
}

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
  hex.trim();  // Убедитесь, что нет лишних пробелов
  int len = hex.length();
  for (int i = 0; i < len && i < MAX_DATA_LENGTH * 2; i += 2)
  {
    bytes[i / 2] = (hexCharToVal(hex.charAt(i)) << 4) + hexCharToVal(hex.charAt(i + 1));
  }

  // hex.trim();  // Убедитесь, что нет лишних пробелов
  // int len = hex.length();
  // byte *bytes = new byte[len / 2];  // 2 символа на байт
  // for (int i = 0; i < len; i += 2)
  // {
  //   bytes[i / 2] = (hexCharToVal(hex.charAt(i)) << 4) + hexCharToVal(hex.charAt(i + 1));
  // }
  // return bytes;
}

static void blink(void)
{
  digitalWrite(LED_BUILTIN, HIGH);
  delay(500);
  digitalWrite(LED_BUILTIN, LOW);
  delay(500);
}

void helpCallback(cmd *c)
{
  Serial.println("This is Help command");
}

void versionCallback(cmd *c)
{
  Serial.println("ver.0.0.1b");
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
  String lenStr = code_arg.getValue();

  uint16_t instruction = strtol(codeStr.c_str(), NULL, 16);
  uint16_t length = lenStr.toInt();

  arm_jtag.ir(instruction, length);

  Serial.print("> ");
  Serial.println(instruction);
  Serial.println(length);
  Serial.println("IR written");

  // String codeStr = c->getValueArg("code").getValue();
  // arm_jtag.ir(ir_code.toInt(), 9);

  // Print response
  // Serial.print("> ");
  // Serial.println(output);

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

  // Освобождение памяти для data

  Serial.print("> ");
  Serial.println("DR written");

  memset(output, 0, sizeof(output));
  memset(dataBuffer, 0, sizeof(dataBuffer));

  // uint32_t id = 0;

  // Command cmd(c);  // Create wrapper object

  // // Get arguments
  // Argument dr_data = cmd.getArgument("data");

  // // Get value
  // String data = dr_data.getValue();

  // int d = data.toInt();
  // byte *pD = (byte *)&d;

  // arm_jtag.dr(pD, 33, (byte *)&id);
  // // arm_jtag.dr(data.c_str(), output);

  // // Print response
  // Serial.print("> ");
  // Serial.println(id);

  // memset(output, 0, sizeof(output));
}

void clockCallback(cmd *c)
{
  Command cmd(c);  // Create wrapper object

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

void resetCmdCallback(cmd *c)
{
  blink();

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

void setup()
{
  Serial.begin(9600);

  cli.setOnError(errorCallback);  // Set error Callback

  // Create the irCmd command with callback function
  irCmd = cli.addCommand("ir", irCmdCallback);
  irCmd.addPositionalArgument("code");
  irCmd.addPositionalArgument("len");

  drCmd = cli.addCommand("dr", drCmdCallback);
  drCmd.addPositionalArgument("data");
  drCmd.addPositionalArgument("len");

  clock = cli.addCommand("clock", clockCallback);
  clock.addPositionalArgument("tms");
  clock.addPositionalArgument("tdi");

  reset = cli.addCommand("reset", resetCmdCallback);
  help = cli.addCommand("help", helpCallback);
  version = cli.addCommand("version", versionCallback);

  pinMode(LED_BUILTIN, OUTPUT);
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

  delay(500);

  // jtag_idcode3();
}
