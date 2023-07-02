#include "jtag.h"

#include <assert.h>

namespace JTAG
{
    void setBitArray(int i_bit, byte *data, int value) {
        int i_byte;
        uint32_t mask;

        i_byte = i_bit >> 3; // floor(i_bit/8)
        mask = 1 << (i_bit & 0x7);

        if (value == 0) {
            data[i_byte] &= ~mask;
        } else {
            data[i_byte] |= mask;
        }
    }

    int getBitArray(int i_bit, const byte *data) {
        int i_byte;
        byte mask;

        i_byte = i_bit >> 3; // floor(i_bit/8)
        mask = 1 << (i_bit & 0x7);

        return ((data[i_byte] & mask) == 0) ? 0 : 1;
    }
}

namespace JTAG_PRIV
{
    const uint8_t RESET_TMS = 0x1F;
    const uint8_t RESET_TMS_LEN = 5;

    const uint8_t IR_TMS_PRE = 6;
    const uint8_t IR_TMS_POST = 1;
    const uint8_t IR_TMS_PRE_LEN = 5;
    const uint8_t IR_TMS_POST_LEN = 2;

    const uint8_t DR_TMS_PRE = 1;
    const uint8_t DR_TMS_POST = 1;
    const uint8_t DR_TMS_PRE_LEN = 3;
    const uint8_t DR_TMS_POST_LEN = 2;
}

JtagPin::JtagPin(int pin, int dir) {
    assert(dir == INPUT || dir == OUTPUT);
    this->assign(pin, dir);
}

void JtagPin::setHigh() {
    this->setValue(HIGH);
}

void JtagPin::setLow() {
    this->setValue(LOW);
}

int JtagPin::get() const {
    assert(this->dir == INPUT);
    return digitalRead(this->pin);
}

void JtagPin::pulseHigh(size_t us) {
    this->setHigh();
    delayMicroseconds(us);
    this->setLow();
}

void JtagPin::pulseLow(size_t us) {
    this->setHigh();
    delayMicroseconds(us);
    this->setLow();
}

void JtagPin::setValue(int value) {
    assert(value == LOW || value == HIGH);
    assert(this->dir == OUTPUT);

    digitalWrite(this->pin, value);
}

void JtagPin::assign(int pin, int dir) {
    assert(dir == INPUT || dir == OUTPUT);

    this->pin = pin;
    this->dir = dir;

    digitalWrite(this->pin, LOW);
    pinMode(this->pin, this->dir);
}

void JtagPin::setDir(int dir) {
    assert(dir == INPUT || dir == OUTPUT);
    this->dir = dir;
}

int JtagPin::getDir() const {
    return this->dir;
}






JtagBus::JtagBus(JtagPin tms, JtagPin tdi, JtagPin tdo, JtagPin tck, JtagPin rst) :
    _tms(tms), _tdi(tdi), _tdo(tdo), _tck(tck), _rst(rst)
{
    this->last_tck_micros = micros();
    this->min_tck_micros = 1;
}

JTAG::ERROR JtagBus::setSpeed(uint32_t khz) {
    if (khz == 0 || khz > static_cast<uint32_t>(JTAG::CONSTANTS::MAX_SPEED_KHZ)) {
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

uint32_t JtagBus::getSpeed() const {
    return this->min_tck_micros;
}

void JtagBus::reset() {
    this->_rst.setLow();
    delayMicroseconds(this->min_tck_micros);
    this->_rst.setHigh();
}

uint8_t JtagBus::clock(uint8_t tms, uint8_t tdi) {
    assert(tms == HIGH || tms == LOW);
    assert(tdi == HIGH || tdi == LOW);

    // Setting TDI and TMS before rising edge of TCK.
    this->_tdi.setValue(tdi);
    this->_tms.setValue(tms);

//   Serial.print(tms);
//   Serial.print(tdi);

    // Waiting until TCK has been stable for at least jtag_min_tck_micros.
    size_t cur_micros = micros();

    if (cur_micros < this->last_tck_micros + this->min_tck_micros) {
        delayMicroseconds(this->last_tck_micros + this->min_tck_micros - cur_micros);
    }

    this->_tck.setHigh();
    delayMicroseconds(this->min_tck_micros);
    this->_tck.setLow();

    // Saving timestamp of last TCK change
    this->last_tck_micros = micros();

    // TDO changes on falling edge of TCK, we are reading
    // value changed during last jtag_clock.
    uint8_t tdo = this->_tdo.get();

// Serial.println(tdo);

    return tdo;
}

JTAG::ERROR JtagBus::sequence(size_t n, const byte tms[], const byte tdi[], byte *tdo) {
    assert(tms != nullptr);
    assert(tdi != nullptr);
    assert(tdo != nullptr);
    assert(n != 0);

    if (n > static_cast<uint32_t>(JTAG::CONSTANTS::MAX_SEQUENCE_LEN)) {
        return JTAG::ERROR::INVALID_SEQUENCE_LEN;
    }

    for (size_t i = 0; i < n; i++) {
        JTAG::setBitArray(i, tdo, this->clock(JTAG::getBitArray(i, tms), JTAG::getBitArray(i, tdi)));
    }

    return JTAG::ERROR::NO;
}




Jtag::Jtag(uint8_t tms, uint8_t tdi, uint8_t tdo, uint8_t tck, uint8_t rst) :
    bus(JtagPin(tms, OUTPUT),
        JtagPin(tdi, OUTPUT),
        JtagPin(tdo, INPUT),
        JtagPin(tck, OUTPUT),
        JtagPin(rst, OUTPUT))
{
}

void Jtag::ir(uint32_t length, byte* command, byte* output) {
    assert(command != nullptr);
    assert(output != nullptr);
    assert(length != 0);

    uint8_t tms_pre[1] = {JTAG_PRIV::IR_TMS_PRE};
    uint8_t tms_post[1] = {JTAG_PRIV::IR_TMS_POST};
    uint8_t tdi_pre[1] = {0x00};
    uint8_t tdi_post[1] = {0x00};
    size_t i_seq = 0;
    uint32_t bit_offset = 0;

    for (i_seq = 0; i_seq < JTAG_PRIV::IR_TMS_PRE_LEN; i_seq++) {
        JTAG::setBitArray(i_seq, &output[0], this->bus.clock(JTAG::getBitArray(i_seq, &tms_pre[0]), JTAG::getBitArray(i_seq, &tdi_pre[0])));
    }

    bit_offset += JTAG_PRIV::IR_TMS_PRE_LEN;

    for (i_seq = 0; i_seq < (length-1); i_seq++) {
        JTAG::setBitArray(bit_offset + i_seq, &output[0], this->bus.clock(0, JTAG::getBitArray(i_seq, command)));
    }

    bit_offset += i_seq;

    JTAG::setBitArray(bit_offset, &output[0], this->bus.clock(1, JTAG::getBitArray(length-1, command)));

    bit_offset++;

    for (i_seq = 0; i_seq < JTAG_PRIV::IR_TMS_POST_LEN; i_seq++) {
        JTAG::setBitArray(bit_offset + i_seq, &output[0], this->bus.clock(JTAG::getBitArray(i_seq, &tms_post[0]), JTAG::getBitArray(i_seq, &tdi_post[0])));
    }
}

void Jtag::ir(const char *command, byte* output) {
    assert(command != nullptr);
    assert(output != nullptr);

    size_t len = strlen(command);
    uint8_t tms_pre[1] = {JTAG_PRIV::IR_TMS_PRE};
    uint8_t tms_post[1] = {JTAG_PRIV::IR_TMS_POST};
    uint8_t tdi_pre[1] = {0x00};
    uint8_t tdi_post[1] = {0x00};
    size_t i_seq = 0;
    uint32_t bit_offset = 0;

    for (i_seq = 0; i_seq < JTAG_PRIV::IR_TMS_PRE_LEN; i_seq++) {
        JTAG::setBitArray(i_seq, &output[0], this->bus.clock(JTAG::getBitArray(i_seq, &tms_pre[0]), JTAG::getBitArray(i_seq, &tdi_pre[0])));
    }

    bit_offset += JTAG_PRIV::IR_TMS_PRE_LEN;

    for (i_seq = 0; i_seq < (len-1); i_seq++) {
        JTAG::setBitArray(bit_offset + i_seq, &output[0], this->bus.clock(0, command[i_seq] == '1' ? 1 : 0));
    }

    bit_offset += i_seq;

    JTAG::setBitArray(bit_offset, &output[0], this->bus.clock(1, command[len-1] == '1' ? 1 : 0));

    bit_offset++;

    for (i_seq = 0; i_seq < JTAG_PRIV::IR_TMS_POST_LEN; i_seq++) {
        JTAG::setBitArray(bit_offset + i_seq, &output[0], this->bus.clock(JTAG::getBitArray(i_seq, &tms_post[0]), JTAG::getBitArray(i_seq, &tdi_post[0])));
    }
}


void Jtag::dr(uint32_t length, byte* data, byte* output) {
    assert(data != nullptr);
    assert(output != nullptr);
    assert(length != 0);

    byte tms_pre[1] = {JTAG_PRIV::DR_TMS_PRE};
    byte tms_post[1] = {JTAG_PRIV::DR_TMS_POST};
    byte tdi_pre[1] = {0x00};
    byte tdi_post[1] = {0x00};
    size_t i_seq = 0;
    uint32_t bit_offset = 0;

    for (i_seq = 0; i_seq < JTAG_PRIV::DR_TMS_PRE_LEN; i_seq++) {
        JTAG::setBitArray(i_seq, &output[0], this->bus.clock(JTAG::getBitArray(i_seq, &tms_pre[0]), JTAG::getBitArray(i_seq, &tdi_pre[0])));
    }

    bit_offset += JTAG_PRIV::DR_TMS_PRE_LEN;

    for (i_seq = 0; i_seq < (length-1); i_seq++) {
        JTAG::setBitArray(bit_offset + i_seq, &output[0], this->bus.clock(0, JTAG::getBitArray(i_seq, data)));
    }

    bit_offset += i_seq;

    JTAG::setBitArray(bit_offset, &output[0], this->bus.clock(1, JTAG::getBitArray(length-1, data)));


    bit_offset++;

    for (i_seq = 0; i_seq < JTAG_PRIV::DR_TMS_POST_LEN; i_seq++) {
        JTAG::setBitArray(bit_offset + i_seq, &output[0], this->bus.clock(JTAG::getBitArray(i_seq, &tms_post[0]), JTAG::getBitArray(i_seq, &tdi_post[0])));
    }
}

void Jtag::dr(const char *data, byte* output) {
    assert(data != nullptr);
    assert(output != nullptr);

    size_t len = strlen(data);
    byte tms_pre[1] = {JTAG_PRIV::DR_TMS_PRE};
    byte tms_post[1] = {JTAG_PRIV::DR_TMS_POST};
    byte tdi_pre[1] = {0x00};
    byte tdi_post[1] = {0x00};
    size_t i_seq = 0;
    uint32_t bit_offset = 0;

    for (i_seq = 0; i_seq < JTAG_PRIV::DR_TMS_PRE_LEN; i_seq++) {
        JTAG::setBitArray(i_seq, &output[0], this->bus.clock(JTAG::getBitArray(i_seq, &tms_pre[0]), JTAG::getBitArray(i_seq, &tdi_pre[0])));
    }

    bit_offset += JTAG_PRIV::DR_TMS_PRE_LEN;

    for (i_seq = 0; i_seq < (len-1); i_seq++) {
        JTAG::setBitArray(bit_offset + i_seq, &output[0], this->bus.clock(0, data[i_seq] == '1' ? 1 : 0));
    }

    bit_offset += i_seq;

    JTAG::setBitArray(bit_offset, &output[0], this->bus.clock(1, data[len-1] == '1' ? 1 : 0));


    bit_offset++;

    for (i_seq = 0; i_seq < JTAG_PRIV::DR_TMS_POST_LEN; i_seq++) {
        JTAG::setBitArray(bit_offset + i_seq, &output[0], this->bus.clock(JTAG::getBitArray(i_seq, &tms_post[0]), JTAG::getBitArray(i_seq, &tdi_post[0])));
    }
}

void Jtag::reset() {
    byte tms = JTAG_PRIV::RESET_TMS;
    byte tdi = 0x00;
    byte tdo = 0x00;

    this->bus.sequence(JTAG_PRIV::RESET_TMS_LEN, &tms, &tdi, &tdo);
}

JTAG::ERROR Jtag::setSpeed(uint32_t khz) {
    return this->bus.setSpeed(khz);
}
