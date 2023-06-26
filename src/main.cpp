#include <Arduino.h>
#include <jtag.h>

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

void send_ir(byte command[])
{
  byte output[33] = {0};

  Serial.println("IR: ");
  arm_jtag.ir(IR_LEN, command, &output[0]);
  // arm_jtag.ir("011111111", &output[0]);

  Serial.print("TMS: 01100");
  for (size_t i = 0; i < IR_LEN-1; i++)
  {
    Serial.print("0");
  }
  Serial.println("110");

  Serial.print("TDI: 00000");
  for (size_t i = 0; i < IR_LEN-1; i++)
  {
    Serial.print(jtag_bus.get_array_bit(i, command));
  }
  Serial.print(jtag_bus.get_array_bit(IR_LEN-1, &command[IR_LEN-1]));
  Serial.println("00");

  Serial.print("TDO: ");
  for (size_t i = 0; i < IR_LEN + 7; i++) {
    Serial.print(jtag_bus.get_array_bit(i, output));
  }
  Serial.println("");
  Serial.println("");
}

void send_dr(byte data[], uint32_t size)
{
  byte id[10] = {0,0,0,0,0,0,0,0,0,0};

  Serial.println("DR: ");
  arm_jtag.dr(size, data, &id[0]);
  // arm_jtag.dr("000000000000000000000000000000000", &id[0]);

  Serial.print("TMS: 100");
  for (size_t i = 0; i < size-1; i++)
  {
  Serial.print("0");
  }
  Serial.print("1");
  Serial.println("10");

  Serial.print("TDI: 000");
  for (size_t i = 0; i < size-1; i++)
  {
    Serial.print(jtag_bus.get_array_bit(i, data));
  }
  Serial.print(jtag_bus.get_array_bit(size-1, data));
  Serial.println("00");

  Serial.print("TDO: ");
  for (size_t i = 0; i < DR_LEN+8; i++) {
    Serial.print(jtag_bus.get_array_bit(i, id));
  }
  Serial.println("");
  Serial.println("");
}


void setup() {
   Serial.begin(9600);

  // put your setup code here, to run once:
  jtag_bus.assign_pin(JTAG::PIN::TCK, tck);
  jtag_bus.assign_pin(JTAG::PIN::TMS, tms);
  jtag_bus.assign_pin(JTAG::PIN::TDI, tdi);
  jtag_bus.assign_pin(JTAG::PIN::TDO, tdo);
  jtag_bus.assign_pin(JTAG::PIN::TRST, trst);
  jtag_bus.set_speed(800000);

  arm_jtag.add_bus(jtag_bus);
}

void loop() {
  // put your main code here, to run repeatedly:
  byte data[33] = {0};
  byte command[] = {0xFE, 0x01};
  byte output[33] = {0};
  byte id[10] = {0,0,0,0,0,0,0,0,0,0};

  byte DPACC[] = {0xFA, 0x01};
  byte APACC[] = {0xFB, 0x01};

  byte data_0[33] = {0x02, 0x01, 0x00, 0x80, 0x02, 0x00, 0x00};
  byte data_1[33] = {0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  byte data_2[33] = {0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  byte data_3[33] = {0x02, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00};
  byte data_4[33] = {0x56, 0x55, 0x55, 0x55, 0x05, 0x00, 0x00};
  byte data_4_1[33] = {0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  byte data_5[33] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};



  // Serial.println("===================================== ID =====================================");
  // arm_jtag.reset();

  // arm_jtag.ir("011111111", &output[0]);
  // arm_jtag.dr("000000000000000000000000000000000", &id[0]);

  // Serial.print("TDO: ");
  // for (size_t i = 0; i < DR_LEN+8; i++) {
  //   Serial.print(jtag_bus.get_array_bit(i, id));
  // }
  // Serial.println("");
  // Serial.println("");


  Serial.println("===================================== CTRL/STAT =====================================");
  arm_jtag.reset();

  arm_jtag.ir("010111111", &output[0]);
  arm_jtag.dr("010000001000000000000000000000010100", &output[0]);

  Serial.println("===================================== WRITE =====================================");
  arm_jtag.reset();
  arm_jtag.ir("010111111", &output[0]);
  arm_jtag.dr("001000000000000000000000000000000000", &output[0]);

  arm_jtag.reset();
  arm_jtag.ir("110111111", &output[0]);
  arm_jtag.dr("000010000000000000000000000000000000", &output[0]);

  arm_jtag.reset();
  arm_jtag.ir("110111111", &output[0]);
  arm_jtag.dr("010000000000000000000000000000001000", &output[0]);

  arm_jtag.reset();
  arm_jtag.ir("110111111", &output[0]);
  arm_jtag.dr("011101010100101010110101010010101010", &output[0]);

  Serial.println("===================================== READ =====================================");

  arm_jtag.reset();
  arm_jtag.ir("010111111", &output[0]);
  arm_jtag.dr("001000000000000000000000000000000000", &output[0]);

  arm_jtag.reset();
  arm_jtag.ir("110111111", &output[0]);
  arm_jtag.dr("000010000000000000000000000000000000", &output[0]);

  arm_jtag.reset();
  arm_jtag.ir("110111111", &output[0]);
  arm_jtag.dr("010000000000000000000000000000001000", &output[0]);

  arm_jtag.reset();
  arm_jtag.ir("110111111", &output[0]);
  arm_jtag.dr("111000000000000000000000000000000000", &output[0]);

  delay(1);

  arm_jtag.dr("000000000000000000000000000000000000", &output[0]);

  Serial.print("TDO: ");
  for (size_t i = 0; i < DR_LEN+8; i++) {
    Serial.print(jtag_bus.get_array_bit(i, output));
  }
  Serial.println("");
  Serial.println("");



























  // Serial.println("===================================== ID =====================================");
  // arm_jtag.reset();

  // send_ir(command);
  // send_dr(data, DR_LEN);

  // Serial.println("===================================== CTRL/STAT =====================================");
  // arm_jtag.reset();

  // send_ir(DPACC);
  // send_dr(data_0, 36);



  // Serial.println("===================================== WRITE =====================================");
  // arm_jtag.reset();

  // send_ir(DPACC);
  // send_dr(data_1, 36);

  // arm_jtag.reset();

  // send_ir(APACC);
  // send_dr(data_2, 36);

  // arm_jtag.reset();

  // send_ir(APACC);
  // send_dr(data_3, 36);
  // arm_jtag.reset();

  // send_ir(APACC);
  // send_dr(data_4, 36);

  // Serial.println("===================================== READ =====================================");
  // arm_jtag.reset();

  // send_ir(DPACC);
  // send_dr(data_1, 36);
  // arm_jtag.reset();

  // send_ir(APACC);
  // send_dr(data_2, 36);
  // arm_jtag.reset();

  // send_ir(APACC);
  // send_dr(data_3, 36);
  // arm_jtag.reset();

  // send_ir(APACC);
  // send_dr(data_4_1, 36);

  // delay(1);

  // send_dr(data_5, 36);

  delay(5000);
}
