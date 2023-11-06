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

// char output[256] = {};
char output[500] = {};

static void jtag_idcode()
{
  Serial.println("Read of `IDCODE` `DP`register");
  arm_jtag.reset();
  memset(output, '\0', sizeof(output));
  arm_jtag.ir("011111111", output);
  memset(output, '\0', sizeof(output));
  arm_jtag.dr("000000000000000000000000000000000", output);
  Serial.print("> ");
  Serial.println(output);
  delay(300);
}

static void jtag_idcode2()
{
  Serial.println("Read of `IDCODE` `DP`register");
  arm_jtag.reset();
  memset(output, '\0', sizeof(output));
  arm_jtag.ir("111110000", output);
  memset(output, '\0', sizeof(output));
  arm_jtag.dr("000000000000000000000000000000000", output);
  Serial.print("> ");
  Serial.println(output);
  delay(300);
}

char d_sample[] =
  "00000000000001101101101100110110110000000000000000000000110110110110110110110110110110110000000000000000"
  "00000011011011011011011011011011000000000000000000000011011011011011011011011011011011011000000000000000"
  "00000001101101101101101101101101101101100000000000000000000001101101101101101101101101101100000000000001"
  "1011011011011011011011011000000000000000000000000000000000000000000011011011000011011011011011";

static void jtag_sample()
{
  Serial.println("SAMPLE");
  arm_jtag.reset();
  // memset(output, '\0', sizeof(output));
  arm_jtag.ir("111101000", output);
  // memset(output, '\0', sizeof(output));
  arm_jtag.dr(d_sample, output);
  // Serial.print("> ");
  // Serial.println(output);
  delay(300);
}

static void jtag_perload()
{
  Serial.println("PERLOAD");
  arm_jtag.reset();
  memset(output, '\0', sizeof(output));
  arm_jtag.ir("111101000", output);
  memset(output, '\0', sizeof(output));
  arm_jtag.dr(
    "000000000000011011011011001101101100000000000000000000001101101101101101101101101101101100000000000000000000001101"
    "101101101101101101101100000000000000000000001001001001001101101101101101101101100000000000000000000001101101101101"
    "101101101101101101100000000000000000000001101101101101101101101101101100000000000001101101101101101101101101100000"
    "0000000000000000000000000000000000000011011011000011011011011011",
    output);
  Serial.print("> ");
  Serial.println(output);
  delay(300);
}

static void jtag_extest()
{
  Serial.println("EXTEST");
  arm_jtag.reset();
  memset(output, '\0', sizeof(output));
  arm_jtag.ir("111100000", output);
  // memset(output, '\0', sizeof(output));
  // arm_jtag.dr(
  //   "000000000000011011011011001101101100000000000000000000001101101101101101101101101101101100000000000000000000001101"
  //   "101101101101101101101100000000000000000000001001001001001101101101101101101101100000000000000000000001101101101101"
  //   "101101101101101101100000000000000000000001101101101101101101101101101100000000000001101101101101101101101101100000"
  //   "0000000000000000000000000000000000000011011011000011011011011011",
  //   output);
  // Serial.print("> ");
  // Serial.println(output);
  delay(300);
}

static void jtag_enable_pwr()
{
  Serial.println("Enable power and clock on DAP");
  arm_jtag.reset();
  memset(output, '\0', sizeof(output));
  arm_jtag.ir("010111111", output);
  memset(output, '\0', sizeof(output));
  arm_jtag.dr("010000000000000000000000000000010100", output);
}

static void jtag_csw_rd()
{
  Serial.print("CSW: ");
  arm_jtag.reset();
  memset(output, '\0', sizeof(output));
  arm_jtag.ir("010111111", output);
  memset(output, '\0', sizeof(output));
  arm_jtag.dr("001000000000000000000000000000000000", output);

  arm_jtag.reset();
  memset(output, '\0', sizeof(output));
  arm_jtag.ir("110111111", output);
  memset(output, '\0', sizeof(output));
  arm_jtag.dr("100000000000000000000000000000000000", output);

  memset(output, '\0', sizeof(output));
  arm_jtag.dr("000000000000000000000000000000000000", output);
  Serial.println(output);
}

static void jtag_base_rd()
{
  Serial.print("BASE: ");
  arm_jtag.reset();
  memset(output, '\0', sizeof(output));
  arm_jtag.ir("010111111", output);
  memset(output, '\0', sizeof(output));
  arm_jtag.dr("001000011110000000000000000000000000", output);

  arm_jtag.reset();
  memset(output, '\0', sizeof(output));
  arm_jtag.ir("110111111", output);
  memset(output, '\0', sizeof(output));
  arm_jtag.dr("101000000000000000000000000000000000", output);

  memset(output, '\0', sizeof(output));
  arm_jtag.dr("000000000000000000000000000000000000", output);
  Serial.println(output);
}

static void jtag_abort_wr()
{
  Serial.print("ABORT: ");
  arm_jtag.reset();
  memset(output, '\0', sizeof(output));
  arm_jtag.ir("010111111", output);
  memset(output, '\0', sizeof(output));
  arm_jtag.dr("000111110000000000000000000000000000", output);
}

static void jtag_csw_wr()
{
  Serial.print("CSW: ");
  arm_jtag.reset();
  memset(output, '\0', sizeof(output));
  arm_jtag.ir("010111111", output);
  memset(output, '\0', sizeof(output));
  arm_jtag.dr("001000000000000000000000000000000000", output);

  arm_jtag.reset();
  memset(output, '\0', sizeof(output));
  arm_jtag.ir("110111111", output);
  memset(output, '\0', sizeof(output));
  arm_jtag.dr("000000000000000000000000000010001000", output);
}

static void jtag_ctrl_read()
{
  Serial.print("CTRL/STAT: ");
  arm_jtag.reset();
  memset(output, '\0', sizeof(output));
  arm_jtag.ir("010111111", output);
  memset(output, '\0', sizeof(output));
  arm_jtag.dr("110000000000000000000000000000000000", output);
  memset(output, '\0', sizeof(output));
  arm_jtag.dr("000000000000000000000000000000000000", output);
  Serial.println(output);
}

static String reverseString(const String &input)
{
  String reversed = "";
  for (int i = input.length() - 1; i >= 0; i--)
  {
    reversed += input.charAt(i);
  }
  return reversed;
}

String toBinaryString(uint32_t num)
{
  String binary = "";
  for (int i = 0; i < 32; i++)
  {
    binary += (num & 0x80000000UL) ? '1' : '0';
    num <<= 1;
  }
  return binary;
}

static void jtag_mem_write(uint32_t addr, uint32_t data)
{
  String address = toBinaryString(addr);
  String rev_addr = reverseString(address);
  String tar = "010" + rev_addr + "0";

  String str_data = toBinaryString(data);
  String rev_data = reverseString(str_data);
  String drw = "011" + rev_data + "0";

  Serial.print("Write of addr: ");
  Serial.print(addr, HEX);
  Serial.print(" -> ");
  Serial.println(tar);

  Serial.println("Step 1: Config AP access");
  arm_jtag.reset();
  memset(output, '\0', sizeof(output));
  arm_jtag.ir("010111111", output);
  memset(output, '\0', sizeof(output));
  arm_jtag.dr("001000000000000000000000000000000000", output);

  Serial.println("Step 2: Configure the CSW register");
  arm_jtag.reset();
  memset(output, '\0', sizeof(output));
  arm_jtag.ir("110111111", output);
  memset(output, '\0', sizeof(output));
  arm_jtag.dr("000010000000000000000000000000001000", output);

  Serial.println("Step 3: Configure the TAR register");
  arm_jtag.reset();
  memset(output, '\0', sizeof(output));
  arm_jtag.ir("110111111", output);
  memset(output, '\0', sizeof(output));
  arm_jtag.dr(tar.c_str(), output);

  // delay(100);

  Serial.println("Step 4: Write memory");
  Serial.print("\t-->: ");
  Serial.println(str_data);
  arm_jtag.reset();
  memset(output, '\0', sizeof(output));
  arm_jtag.ir("110111111", output);
  memset(output, '\0', sizeof(output));
  arm_jtag.dr(drw.c_str(), output);

  Serial.print("\t");
  jtag_ctrl_read();
  Serial.print("\t");
  jtag_csw_rd();
}

static void jtag_mem_read(uint32_t addr)
{
  String address = toBinaryString(addr);
  String rev_addr = reverseString(address);
  String tar = "010" + rev_addr + "0";

  Serial.print("Read of addr: ");
  Serial.print(addr, HEX);
  Serial.print(" -> ");
  Serial.println(tar);

  Serial.println("Step 1: Config AP access");
  arm_jtag.reset();
  memset(output, '\0', sizeof(output));
  arm_jtag.ir("010111111", output);
  memset(output, '\0', sizeof(output));
  arm_jtag.dr("001000000000000000000000000000000000", output);

  Serial.println("Step 2: Configure the CSW register");
  arm_jtag.reset();
  memset(output, '\0', sizeof(output));
  arm_jtag.ir("110111111", output);
  memset(output, '\0', sizeof(output));
  arm_jtag.dr("000010000000000000000000000000001000", output);

  Serial.println("Step 3: Configure the TAR register");
  arm_jtag.reset();
  memset(output, '\0', sizeof(output));
  arm_jtag.ir("110111111", output);
  memset(output, '\0', sizeof(output));
  arm_jtag.dr(tar.c_str(), output);

  delay(100);

  Serial.println("Step 4: Read memory");
  Serial.print("\t<--: ");
  arm_jtag.reset();
  memset(output, '\0', sizeof(output));
  arm_jtag.ir("110111111", output);
  memset(output, '\0', sizeof(output));
  arm_jtag.dr("111000000000000000000000000000000000", output);
  memset(output, '\0', sizeof(output));
  arm_jtag.dr("000000000000000000000000000000000000", output);
  Serial.println(output);

  Serial.print("\t");
  jtag_ctrl_read();
  Serial.print("\t");
  jtag_csw_rd();
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
void irCallback(cmd *c)
{
  Command cmd(c);  // Create wrapper object

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
void drCallback(cmd *c)
{
  Command cmd(c);  // Create wrapper object

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

void resetCallback(cmd *c)
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

  jtag_perload();
  jtag_extest();

  // jtag_enable_pwr();
  // jtag_ctrl_read();
  // jtag_idcode();
  // jtag_idcode2();
  // jtag_sample();
  // jtag_csw_wr();
  // jtag_csw_rd();

  // jtag_mem_write(0x20000000, 0xFF00FFFF);
  // jtag_mem_read(0x20000000);

  // jtag_mem_read(0xE00FF000);

  // jtag_base_rd();
  // jtag_ctrl_read();
  // jtag_csw_rd();

  // jtag_mem_read(0xA0000000);
  // jtag_mem_read(0x60000000);
  // jtag_mem_read(0x50050000);
  // jtag_mem_read(0x40026400);
  // jtag_mem_read(0x40013800);
  // jtag_mem_read(0x40005000);

  // jtag_mem_read(0xE0000000);  // 00010100011010000000000000000000000000000

  // jtag_mem_write(0xE002EDF0, 0xA05F0003);
  // jtag_mem_read(0xE002EDF0);
  // jtag_mem_read(0xE000EDF0);

  // jtag_mem_read(0xE0042000);
  // jtag_mem_read(0xE00FF000);
  // jtag_mem_read(0xE00FFFF0);
  // jtag_mem_read(0xE00FFFF4);
  // jtag_mem_read(0xE00FFFF8);
  // jtag_mem_read(0xE00FFFFC);

  // jtag_abort_wr();
}
