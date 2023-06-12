#include "jtag.h"

#include <assert.h>

JtagWire::JtagWire(int digital_pin, int dir) {
    this->assign(digital_pin, dir);
}

void JtagWire::write(int value) {
    assert(value == LOW || value == HIGH);
    assert(this->dir == OUTPUT);

    digitalWrite(this->digital_pin, value);
}

void JtagWire::set() {
    this->write(HIGH);
}

void JtagWire::clear() {
    this->write(LOW);
}

int JtagWire::get() {
    assert(this->dir == INPUT);
    return digitalRead(this->digital_pin);
}

int JtagWire::pulse_high(unsigned int us) {
    this->set();
    delayMicroseconds(us);
    this->clear();
}

int JtagWire::pulse_low(unsigned int us) {
    this->set();
    delayMicroseconds(us);
    this->clear();
}

int JtagWire::assign(int digital_pin, int dir) {
    this->digital_pin = digital_pin;
    this->dir = dir;

    digitalWrite(this->digital_pin, LOW);
    pinMode(this->digital_pin, this->dir);
}








JtagBus::JtagBus() {
    this->last_tck_micros = micros();
    this->min_tck_micros = 1;
}

void JtagBus::assign_pin(JTAGPin jtag_pin, JtagWire jtag_wire) {
    uint8_t index = static_cast<uint8_t>(jtag_pin);
    this->bus[index] = jtag_wire;
}

int JtagBus::set_speed(unsigned int khz) {
    if (khz == 0 || khz > engine::JTAG_MAX_SPEED_KHZ) {
        return engine::JTAG_ERROR_BAD_SPEED;
    }

    // Mininum time for TCK to be stable is half the clock period.
    // For 100kHz of TCK frequency the period is 10us so jtag_min_tck_micros is
    // 5us.
    this->min_tck_micros = (500U + khz - 1) / khz; // ceil

    return engine::JTAG_NO_ERROR;
}

void JtagBus::reset() {
    this->bus[static_cast<uint8_t>(JTAGPin::TRST)].clear();
    delayMicroseconds(this->min_tck_micros);
    this->bus[static_cast<uint8_t>(JTAGPin::TRST)].set();
}


int JtagBus::clock(int tms, int tdi) {
    unsigned long cur_micros;
    int tdo;

    // Setting TDI and TMS before rising edge of TCK.
    this->bus[static_cast<uint8_t>(JTAGPin::TDI)].write(tdi);
    this->bus[static_cast<uint8_t>(JTAGPin::TMS)].write(tms);

    // Waiting until TCK has been stable for at least jtag_min_tck_micros.
    cur_micros = micros();

    if (cur_micros < this->last_tck_micros + this->min_tck_micros) {
        delayMicroseconds(this->last_tck_micros + this->min_tck_micros - cur_micros);
    }

    //   tdo = jtag_get_pin(TDO); // TDO changes on falling edge of TCK, we are reading
    //                            // value changed during last jtag_clock.

    this->bus[static_cast<uint8_t>(JTAGPin::TCK)].set();
    delayMicroseconds(this->min_tck_micros);
    this->bus[static_cast<uint8_t>(JTAGPin::TCK)].clear();


    this->last_tck_micros = micros(); // Saving timestamp of last TCK change.

    tdo = this->bus[static_cast<uint8_t>(JTAGPin::TDO)].get(); // TDO changes on falling edge of TCK, we are reading
                            // value changed during last jtag_clock.

    return tdo;
}

int JtagBus::sequence(unsigned int n, const byte *tms, const byte *tdi, byte *tdo) {
    unsigned int i_seq;

    if (n > engine::JTAG_MAX_SEQUENCE_LEN) {
        return engine::JTAG_ERROR_BAD_SEQUENCE_LEN;
    }

    for (i_seq = 0; i_seq < n; i_seq++) {
        this->set_array_bit(i_seq, tdo, this->clock(this->get_array_bit(i_seq, tms), this->get_array_bit(i_seq, tdi)));
    }

    return engine::JTAG_NO_ERROR;
}

void JtagBus::set_array_bit(int i_bit, byte *data, int value) {
    int i_byte;
    byte mask;

    i_byte = i_bit >> 3; // floor(i_bit/8)
    mask = 1 << (i_bit & 0x7);

    if (value == 0) {
        data[i_byte] &= ~mask;
    } else {
        data[i_byte] |= mask;
    }
}

int JtagBus::get_array_bit(int i_bit, const byte *data) {
    int i_byte;
    byte mask;

    i_byte = i_bit >> 3; // floor(i_bit/8)
    mask = 1 << (i_bit & 0x7);

    return ((data[i_byte] & mask) == 0) ? 0 : 1;
}



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
