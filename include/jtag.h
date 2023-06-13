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
}
//_____ C L A S S E S __________________________________________________________
/**
 * \brief
 *
 */
class JtagWire
{
    public:
        JtagWire() = default;
        JtagWire(int digital_pin, int dir);

        void set();
        void clear();
        int get() const;
        void pulse_high(size_t us);
        void pulse_low(size_t us);
        void write(int value);
        void assign(int digital_pin, int dir);
        void set_direction(int dir);
        int get_direction() const;

    private:
        uint32_t digital_pin = 0;
        uint8_t dir = INPUT;
};

/**
 * \brief
 *
 */
class JtagBus
{
    public:
        JtagBus();
        JtagBus(JtagWire bus[], size_t size);

        void assign_pin(JTAG::PIN jtag_pin, JtagWire jtag_wire);
        void assign_pin(JTAG::PIN jtag_pin, int digital_pin);

        JTAG::ERROR set_speed(size_t khz);
        size_t get_speed() const;

        void reset();
        uint8_t clock(uint8_t tms, uint8_t tdi);
        JTAG::ERROR sequence(size_t n, const byte tms[], const byte tdi[], byte tdo[]);

        void set_array_bit(int i_bit, byte *data, int value);
        int get_array_bit(int i_bit, const byte *data);

    private:
        size_t last_tck_micros = 0;
        size_t min_tck_micros = 1;
        JtagWire bus[JTAG::PINS_NUMBER];
};

/**
 * \brief
 *
 */
class Jtag
{
    public:
        Jtag() = default;
        Jtag(JtagBus bus);
        Jtag(uint8_t tms, uint8_t tdi, uint8_t tdo, uint8_t tck, uint8_t trst);

        void ir(uint32_t length, byte* command, byte* output);
        void dr(uint32_t length, byte* data, byte* output);
        void reset();

        void add_bus(JtagBus bus);

    private:
        JtagBus bus;
};
