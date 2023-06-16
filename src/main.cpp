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

  // tdi.set();
  // delay(1000);
  // tdi.clear();
  // tdi.pulse_high(1000000UL);

  // jtag_bus.clock(1, 1);
  // delay(1000);
  // jtag_bus.clock(0, 0);
  // delay(1000);

  byte data[33] = {0};
  byte output[33] = {0};
  byte command[] = {0xFE, 0x01};


  arm_jtag.reset();
  arm_jtag.ir(9, command, output);
  Serial.println("\n-------------------------------------------------");
  arm_jtag.dr(33, data, output);

  Serial.println("================================================");



  // uint8_t id[4] = {0,0,0,0};
  // uint8_t data[4] = {0,0,0,0};

  // arm_jtag.dr(32, data, id);

  // for (uint8_t i_seq = 0; i_seq < 4; i_seq++) {
  //   Serial.print(data[i_seq], HEX);
  // }

  Serial.println("");
  delay(1000);
}
