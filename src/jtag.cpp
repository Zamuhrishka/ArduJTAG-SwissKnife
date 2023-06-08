#include "jtag.h"


JtagPin::JtagPin(int digital_pin, int dir) {
    // if (pin >= N_JTAG_PINS || pin < 0) {
    //     return JTAG_ERROR_BAD_PIN;
    // }

    this->digital_pin = digital_pin;
    this->dir = dir;

    digitalWrite(this->digital_pin, LOW);
    pinMode(this->digital_pin, this->dir);

    // jtag_last_tck_micros = micros();
    // jtag_min_tck_micros = 1;
}


int JtagPin::write(int value) {
    // if (jtag_pin_dir[pin] != OUTPUT) {
    //     return JTAG_ERROR_BAD_PIN;
    // }

    digitalWrite(this->digital_pin, value);
}


int JtagPin::set() {
    this->write(this->digital_pin, HIGH);
}

int JtagPin::clear() {
    this->write(this->digital_pin, LOW);
}

int JtagPin::get() {
    // if (jtag_pin_dir[pin] != INPUT) {
    //     return JTAG_ERROR_BAD_PIN;
    // }

    return (digitalRead(this->digital_pin) == HIGH) ? 1 : 0;
}

int JtagPin::pulse_high(unsigned int us) {
    this->set();
    delayMicroseconds(us);
    this->clear();
}

int JtagPin::pulse_low(unsigned int us) {
    this->set();
    delayMicroseconds(us);
    this->clear();
}

int JtagPin::assign(int jtag_pin, int digital_pin) {

}

int JtagPin::set_speed(unsigned int khz) {

}
