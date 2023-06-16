#include "jtag.h"

#include <assert.h>

namespace JTAG_PRIV
{
    const uint8_t RESET_TMS = 0x1F;
    const uint8_t RESET_TMS_LEN = 5;

    const uint8_t IR_TMS_PRE = 6;
    const uint8_t IR_TMS_POST = 1;
    const uint8_t IR_TMS_PRE_LEN = 5;
    const uint8_t IR_TMS_POST_LEN = 2;

    const uint8_t DR_TMS_PRE = 2;
    const uint8_t DR_TMS_POST = 1;
    const uint8_t DR_TMS_PRE_LEN = 4;
    const uint8_t DR_TMS_POST_LEN = 2;
}

JtagWire::JtagWire(int digital_pin, int dir) : JtagWire() {
    assert(dir == INPUT || dir == OUTPUT);
    this->assign(digital_pin, dir);
}

void JtagWire::set() {
    this->write(HIGH);
}

void JtagWire::clear() {
    this->write(LOW);
}

int JtagWire::get() const {
    assert(this->dir == INPUT);
    return digitalRead(this->digital_pin);
}

void JtagWire::pulse_high(size_t us) {
    this->set();
    delayMicroseconds(us);
    this->clear();
}

void JtagWire::pulse_low(size_t us) {
    this->set();
    delayMicroseconds(us);
    this->clear();
}

void JtagWire::write(int value) {
    assert(value == LOW || value == HIGH);
    assert(this->dir == OUTPUT);

    digitalWrite(this->digital_pin, value);
}

void JtagWire::assign(int digital_pin, int dir) {
    assert(dir == INPUT || dir == OUTPUT);

    this->digital_pin = digital_pin;
    this->dir = dir;

    digitalWrite(this->digital_pin, LOW);
    pinMode(this->digital_pin, this->dir);
}

void JtagWire::set_direction(int dir) {
    assert(dir == INPUT || dir == OUTPUT);
    this->dir = dir;
}

int JtagWire::get_direction() const {
    return this->dir;
}






JtagBus::JtagBus() {
    this->bus[static_cast<uint8_t>(JTAG::PIN::TCK)].set_direction(OUTPUT);
    this->bus[static_cast<uint8_t>(JTAG::PIN::TDI)].set_direction(OUTPUT);
    this->bus[static_cast<uint8_t>(JTAG::PIN::TMS)].set_direction(OUTPUT);
    this->bus[static_cast<uint8_t>(JTAG::PIN::TRST)].set_direction(OUTPUT);
    this->bus[static_cast<uint8_t>(JTAG::PIN::TDO)].set_direction(INPUT);
    this->last_tck_micros = micros();
    this->min_tck_micros = 1;
}

JtagBus::JtagBus(JtagWire bus[], size_t size) : JtagBus() {
    assert(bus != nullptr);
    assert(size == JTAG::PINS_NUMBER);

    for (size_t i = 0; i < size; i++)
    {
        this->bus[i] = bus[i];
    }
}

void JtagBus::assign_pin(JTAG::PIN jtag_pin, JtagWire jtag_wire) {
    uint8_t index = static_cast<uint8_t>(jtag_pin);
    this->bus[index] = jtag_wire;
}

void JtagBus::assign_pin(JTAG::PIN jtag_pin, int digital_pin) {
    uint8_t index = static_cast<uint8_t>(jtag_pin);
    this->bus[index].assign(digital_pin, this->bus[index].get_direction());
}

JTAG::ERROR JtagBus::set_speed(size_t khz) {
    if (khz == 0 || khz > static_cast<uint8_t>(JTAG::CONSTANTS::MAX_SPEED_KHZ)) {
        return JTAG::ERROR::INVALID_SPEED;
    }

    /*
     * Mininum time for TCK to be stable is half the clock period.
     * For 100kHz of TCK frequency the period is 10us so jtag_min_tck_micros is
     * 5us.
     */
    this->min_tck_micros = (500U + khz - 1) / khz; // ceil

    return JTAG::ERROR::NO;
}

size_t JtagBus::get_speed() const {
    return this->min_tck_micros;
}

void JtagBus::reset() {
    this->bus[static_cast<uint8_t>(JTAG::PIN::TRST)].clear();
    delayMicroseconds(this->min_tck_micros);
    this->bus[static_cast<uint8_t>(JTAG::PIN::TRST)].set();
}

uint8_t JtagBus::clock(uint8_t tms, uint8_t tdi) {
    assert(tms == HIGH || tms == LOW);
    assert(tdi == HIGH || tdi == LOW);

    // Setting TDI and TMS before rising edge of TCK.
    this->bus[static_cast<uint8_t>(JTAG::PIN::TDI)].write(tdi);
    this->bus[static_cast<uint8_t>(JTAG::PIN::TMS)].write(tms);

    // Waiting until TCK has been stable for at least jtag_min_tck_micros.
    size_t cur_micros = micros();

    if (cur_micros < this->last_tck_micros + this->min_tck_micros) {
        delayMicroseconds(this->last_tck_micros + this->min_tck_micros - cur_micros);
    }

    this->bus[static_cast<uint8_t>(JTAG::PIN::TCK)].set();
    delayMicroseconds(this->min_tck_micros);
    this->bus[static_cast<uint8_t>(JTAG::PIN::TCK)].clear();

    // Saving timestamp of last TCK change
    this->last_tck_micros = micros();

    // TDO changes on falling edge of TCK, we are reading
    // value changed during last jtag_clock.
    uint8_t tdo = this->bus[static_cast<uint8_t>(JTAG::PIN::TDO)].get();

    return tdo;
}

JTAG::ERROR JtagBus::sequence(size_t n, const byte tms[], const byte tdi[], byte tdo[]) {
    assert(tms != nullptr);
    assert(tdi != nullptr);
    assert(tdo != nullptr);
    assert(n != 0);

    if (n > static_cast<uint8_t>(JTAG::CONSTANTS::MAX_SEQUENCE_LEN)) {
        return JTAG::ERROR::INVALID_SEQUENCE_LEN;
    }

    for (size_t i = 0; i < n; i++) {
        this->set_array_bit(i, tdo, this->clock(this->get_array_bit(i, tms), this->get_array_bit(i, tdi)));
    }

    return JTAG::ERROR::NO;
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








Jtag::Jtag(JtagBus bus) : Jtag() {
    this->add_bus(bus);
}

Jtag::Jtag(uint8_t tms, uint8_t tdi, uint8_t tdo, uint8_t tck, uint8_t trst) : Jtag() {
    this->bus.assign_pin(JTAG::PIN::TMS, tms);
    this->bus.assign_pin(JTAG::PIN::TDI, tdi);
    this->bus.assign_pin(JTAG::PIN::TDO, tdo);
    this->bus.assign_pin(JTAG::PIN::TCK, tck);
    this->bus.assign_pin(JTAG::PIN::TRST, trst);
}

void Jtag::reset() {
    byte tms = JTAG_PRIV::RESET_TMS;
    byte tdi = 0x00;
    byte tdo = 0x00;

    this->bus.sequence(JTAG_PRIV::RESET_TMS_LEN, &tms, &tdi, &tdo);
}

void Jtag::ir(uint32_t length, byte* command, byte* output) {
    assert(command != nullptr);
    assert(output != nullptr);
    assert(length != 0);

    byte tms_pre = JTAG_PRIV::IR_TMS_PRE;
    byte tms_post = JTAG_PRIV::IR_TMS_POST;;
    byte tdi_pre = 0x00;
    byte tdi_post = 0x00;
    size_t i_seq = 0;

    // this->reset();

    this->bus.sequence(JTAG_PRIV::IR_TMS_PRE_LEN, &tms_pre, &tdi_pre, output);

    // for (size_t i = 0; i < JTAG_PRIV::IR_TMS_PRE_LEN; i++)
    // {
    //     Serial.println(this->bus.get_array_bit(i, &tms_pre));
    //     Serial.println(this->bus.get_array_bit(i, &tdi_pre));
    //     Serial.println(this->bus.get_array_bit(i, output));
    // }

    for (i_seq = 0; i_seq < length-1; i_seq++) {
        this->bus.set_array_bit(i_seq, output, this->bus.clock(0, this->bus.get_array_bit(i_seq, command)));
    }

    // for (size_t i = 0; i < length-1; i++)
    // {
    //     Serial.println(0);
    //     Serial.println(this->bus.get_array_bit(i, command));
    //     Serial.println(this->bus.get_array_bit(i, output));
    // }

    this->bus.set_array_bit(i_seq, output, this->bus.clock(1, this->bus.get_array_bit(i_seq, command)));
    // Serial.println(1);
    // Serial.println(this->bus.get_array_bit(i_seq, command));
    // Serial.println(this->bus.get_array_bit(i_seq, output));
    // Serial.print(this->bus.get_array_bit(i_seq, output));

    this->bus.sequence(JTAG_PRIV::IR_TMS_POST_LEN, &tms_post, &tdi_post, output);
    // for (size_t i = 0; i < JTAG_PRIV::IR_TMS_POST_LEN; i++)
    // {
    //     Serial.println(this->bus.get_array_bit(i, &tms_post));
    //     Serial.println(this->bus.get_array_bit(i, &tdi_post));
    //     Serial.println(this->bus.get_array_bit(i, output));
    // }








    for (size_t i = 0; i < JTAG_PRIV::IR_TMS_PRE_LEN; i++)
    {
        Serial.print(this->bus.get_array_bit(i, &tms_pre));
    }

    for (size_t i = 0; i < length-1; i++)
    {
        Serial.print(0);
    }

    Serial.print(1);

    for (size_t i = 0; i < JTAG_PRIV::IR_TMS_POST_LEN; i++)
    {
        Serial.print(this->bus.get_array_bit(i, &tms_post));
    }
    Serial.println("");


    for (size_t i = 0; i < JTAG_PRIV::IR_TMS_PRE_LEN; i++)
    {
        Serial.print(this->bus.get_array_bit(i, &tdi_pre));
    }

    for (size_t i = 0; i < length-1; i++)
    {
        Serial.print(this->bus.get_array_bit(i, command));
    }

    Serial.print(this->bus.get_array_bit(length, command));

    for (size_t i = 0; i < JTAG_PRIV::IR_TMS_POST_LEN; i++)
    {
        Serial.print(this->bus.get_array_bit(i, &tdi_post));
    }

    Serial.println("");

    for (size_t i = 0; i < length + JTAG_PRIV::IR_TMS_POST_LEN + JTAG_PRIV::IR_TMS_PRE_LEN; i++)
    {
        Serial.print(this->bus.get_array_bit(i, output));
    }


    Serial.println("");
}

void Jtag::dr(uint32_t length, byte* data, byte* output) {
    assert(data != nullptr);
    assert(output != nullptr);
    assert(length != 0);

    byte tms_pre = JTAG_PRIV::DR_TMS_PRE;
    byte tms_post = JTAG_PRIV::DR_TMS_POST;
    byte tdi_pre = 0x00;
    byte tdi_post = 0x00;
    size_t i_seq = 0;

    // this->reset();

    this->bus.sequence(JTAG_PRIV::DR_TMS_PRE_LEN, &tms_pre, &tdi_pre, output);

    for (i_seq = 0; i_seq < length-1; i_seq++) {
        this->bus.set_array_bit(i_seq, output, this->bus.clock(0, this->bus.get_array_bit(i_seq, data)));
    }

    this->bus.set_array_bit(i_seq, output, this->bus.clock(1, this->bus.get_array_bit(i_seq, data)));
    this->bus.sequence(JTAG_PRIV::DR_TMS_POST_LEN, &tms_post, &tdi_post, output);









    for (size_t i = 0; i < JTAG_PRIV::DR_TMS_PRE_LEN; i++)
    {
        Serial.print(this->bus.get_array_bit(i, &tms_pre));
    }

    for (size_t i = 0; i < length-1; i++)
    {
        Serial.print(0);
    }

    Serial.print(1);

    for (size_t i = 0; i < JTAG_PRIV::DR_TMS_POST_LEN; i++)
    {
        Serial.print(this->bus.get_array_bit(i, &tms_post));
    }
    Serial.println("");









    for (size_t i = 0; i < JTAG_PRIV::DR_TMS_PRE_LEN; i++)
    {
        Serial.print(this->bus.get_array_bit(i, &tdi_pre));
    }

    for (size_t i = 0; i < length-1; i++)
    {
        Serial.print(this->bus.get_array_bit(i, data));
    }

    Serial.print(this->bus.get_array_bit(length, data));

    for (size_t i = 0; i < JTAG_PRIV::DR_TMS_POST_LEN; i++)
    {
        Serial.print(this->bus.get_array_bit(i, &tdi_post));
    }

    Serial.println("");






    for (size_t i = 0; i < length + JTAG_PRIV::DR_TMS_POST_LEN + JTAG_PRIV::DR_TMS_PRE_LEN; i++)
    {
        Serial.print(this->bus.get_array_bit(i, output));
    }


    Serial.println("");
}

void Jtag::add_bus(JtagBus bus) {
    this->bus = bus;
}
