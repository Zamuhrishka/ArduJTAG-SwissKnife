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
  // put your setup code here, to run once:
  // pinMode(LED_BUILTIN, OUTPUT);
  jtag_bus.assign_pin(JTAG::PIN::TCK, tck);
  jtag_bus.assign_pin(JTAG::PIN::TMS, tms);
  jtag_bus.assign_pin(JTAG::PIN::TDI, tdi);
  jtag_bus.assign_pin(JTAG::PIN::TDO, tdo);
  jtag_bus.assign_pin(JTAG::PIN::TRST, trst);

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

  jtag_bus.clock(1, 1);
  delay(1000);
  jtag_bus.clock(0, 0);
  delay(1000);
}
