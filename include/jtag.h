/**
 * @file jtag.h
 * @author Aleksander Kovalchuk (aliaksander.kavalchuk@gmail.com)
 * @brief
 * @date 2023-05-31
 */

#ifndef jtag.h
#define jtag.h

//_____ I N C L U D E S _______________________________________________________
#include <Arduino.h>
//_____ C O N F I G S  ________________________________________________________

//_____ D E F I N I T I O N S _________________________________________________

class JtagWire
{
    public:
        JtagWire() = default;
        JtagWire(int digital_pin, int dir);

        void set();
        void clear();
        int get();
        int pulse_high(unsigned int us);
        int pulse_low(unsigned int us);
        int assign(int digital_pin, int dir);
        void write(int value);
    private:
        int digital_pin;
        int dir;
};


enum class JTAGPin {
  TCK = 0,
  TMS = 1,
  TDI = 2,
  TDO = 3,
  TRST = 4,
};

namespace engine {
    #define N_JTAG_PINS 5

    enum jtag_constants {
        JTAG_MAX_SPEED_KHZ = 500,
        JTAG_MAX_SEQUENCE_LEN = 256,
        JTAG_MAX_SEQUENCE_LEN_BYTES = JTAG_MAX_SEQUENCE_LEN / 8, // 32
    };

    enum jtag_errors {
        JTAG_NO_ERROR = 0,
        JTAG_ERROR_BAD_PIN = -1,
        JTAG_ERROR_BAD_SPEED = -2,
        JTAG_ERROR_BAD_SEQUENCE_LEN = -3,
    };
}


class JtagBus
{
    public:
        JtagBus();

        void assign_pin(JTAGPin jtag_pin, JtagWire jtag_wire);
        int set_speed(unsigned int khz);
        void reset();
        int clock(int tms, int tdi);
        int sequence(unsigned int n, const byte *tms, const byte *tdi, byte *tdo);

        void set_array_bit(int i_bit, byte *data, int value);
        int get_array_bit(int i_bit, const byte *data);

    private:

        unsigned long last_tck_micros;
        unsigned long min_tck_micros;
        JtagWire bus[N_JTAG_PINS];
};


class Jtag
{
    public:
        Jtag() = default;
        Jtag(JtagBus bus);
        Jtag(int tms, );

        void ir(uint32_t length, byte* command, byte* output);
        void dr(uint32_t length, byte* data, byte* output);
        void reset();

        void add_bus(JtagBus bus);

    private:
        JtagBus bus;
};


//     private:
//         void set_bit_in_array(int i_bit, byte *data, int value);
//         int get_bit_from_array(int i_bit, const byte *data);
// };

#endif
