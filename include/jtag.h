/**
 * @file jtag.h
 * @author Aleksander Kovalchuk (aliaksander.kavalchuk@gmail.com)
 * @brief
 * @date 2023-05-31
 */

#pragma once

//_____ I N C L U D E S _______________________________________________________
#include <Arduino.h>
//_____ C O N F I G S  ________________________________________________________
//_____ D E F I N I T I O N S _________________________________________________
namespace JTAG
{
    const uint8_t PINS_NUMBER = 5;

    enum class CONSTANTS : uint32_t {
        MAX_SPEED_KHZ = 500,
        MAX_SEQUENCE_LEN = 256,
        MAX_SEQUENCE_LEN_BYTES = MAX_SEQUENCE_LEN / 8, // 32
    };

    enum class PIN : uint8_t
    {
        TCK = 0,
        TMS = 1,
        TDI = 2,
        TDO = 3,
        TRST = 4,
    };

    enum class ERROR : int32_t {
        NO = 0,
        INVALID_PIN = -1,
        INVALID_SPEED = -2,
        INVALID_SEQUENCE_LEN = -3,
    };

    void setBitArray(int i_bit, byte *data, int value);
    void setBitArray(uint32_t i_bit, char *data, char value);
    int getBitArray(int i_bit, const byte *data);
}
//_____ C L A S S E S __________________________________________________________
/**
 * \brief
 *
 */
class JtagPin
{
    public:
        JtagPin() = delete;
        explicit JtagPin(int pin, int dir);

        void setHigh();
        void setLow();
        int get() const;
        void pulseHigh(size_t us);
        void pulseLow(size_t us);
        void setValue(int value);
        void assign(int pin, int dir);
        void setDir(int dir);
        int getDir() const;
    private:
        uint32_t pin = 0;
        uint8_t dir = INPUT;
};

/**
 * \brief
 *
 */
class JtagBus
{
    public:
        JtagBus() = delete;
        explicit JtagBus(JtagPin tms, JtagPin tdi, JtagPin tdo, JtagPin tck, JtagPin rst);

        JTAG::ERROR setSpeed(uint32_t khz);
        uint32_t getSpeed() const;
        void reset();
        uint8_t clock(uint8_t tms, uint8_t tdi);
        JTAG::ERROR sequence(size_t n, const byte tms[], const byte tdi[], byte *tdo);
    private:
        uint32_t last_tck_micros = 0;
        uint32_t min_tck_micros = 1;
        JtagPin _tms;
        JtagPin _tdi;
        JtagPin _tdo;
        JtagPin _tck;
        JtagPin _rst;
};

/**
 * \brief
 *
 */
class Jtag
{
    public:
        Jtag() = delete;
        explicit Jtag(uint8_t tms, uint8_t tdi, uint8_t tdo, uint8_t tck, uint8_t trst);

        void ir(uint32_t length, byte* command, byte* output);
        void ir(const char *command, byte* output);
        void dr(uint32_t length, byte* data, byte* output);
        void dr(const char *data, byte* output);
        void reset();
        JTAG::ERROR setSpeed(uint32_t khz);
    private:
        JtagBus bus;
};
