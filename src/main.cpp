#include <Arduino.h>
#include <jtag.h>

JtagWire tck = JtagWire(2, OUTPUT);
JtagWire tms = JtagWire(3, OUTPUT);
JtagWire tdi = JtagWire(4, OUTPUT);
JtagWire tdo = JtagWire(5, INPUT);
JtagWire trst = JtagWire(6, OUTPUT);

JtagBus jtag_bus = JtagBus();
Jtag arm_jtag = Jtag();

void setup() {
   Serial.begin(9600);

  // put your setup code here, to run once:
  // pinMode(LED_BUILTIN, OUTPUT);
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
  // digitalWrite(LED_BUILTIN, HIGH);  // turn the LED on (HIGH is the voltage level)
  // delay(1000);                      // wait for a second
  // digitalWrite(LED_BUILTIN, LOW);   // turn the LED off by making the voltage LOW
  // delay(1000);

  byte data[33] = {0};
  byte output[33] = {0};
  byte id[10] = {0,0,0,0,0,0,0,0,0,0};
  byte* ptr = &output[0];
  byte command[] = {0xFE, 0x01};




  Serial.println("\n-------------------------------------------------");
  arm_jtag.reset();

  Serial.println("IR: ");
  arm_jtag.ir(9, command, &output[0]);

  Serial.print("TMS: 01100");
  for (size_t i = 0; i < 8; i++)
  {
  Serial.print("0");
  }
  Serial.println("110");

  Serial.print("TDI: 00000");
  for (size_t i = 0; i < 8; i++)
  {
    Serial.print(jtag_bus.get_array_bit(i, command));
  }
  Serial.print(jtag_bus.get_array_bit(8, &command[8]));
  Serial.println("00");

  Serial.print("TDO: ");
  for (size_t i = 0; i < 16; i++) {
    Serial.print(jtag_bus.get_array_bit(i, output));
  }
  Serial.println("");



  Serial.println("DR: ");
  arm_jtag.dr(33, data, &id[0]);

  Serial.print("TMS: 100");
  for (size_t i = 0; i < 32; i++)
  {
  Serial.print("0");
  }
  Serial.print("1");
  Serial.println("10");

  Serial.print("TDI: 000");
  for (size_t i = 0; i < 32; i++)
  {
    Serial.print(jtag_bus.get_array_bit(i, data));
  }
  Serial.print(jtag_bus.get_array_bit(32, data));
  Serial.println("00");

  Serial.print("TDO: ");
  for (size_t i = 0; i < 38; i++) {
    Serial.print(jtag_bus.get_array_bit(i, id));
  }
  Serial.println("");

  Serial.println("\n-------------------------------------------------");

  delay(1000);
}
