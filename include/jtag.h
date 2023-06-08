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
class JtagPin
{
    public:
        JtagPin(int digital_pin, int dir);

        int write(int value);
        int set();
        int clear();
        int get();
        int pulse_high(unsigned int us);
        int pulse_low(unsigned int us);
        int assign(int jtag_pin, int digital_pin);
        int set_speed(unsigned int khz);
    private:
        int digital_pin;
        int dir;
};




// class Jtag
// {
//     public:
//         Jtag(byte color = 5, byte bright = 30);

//         int jtag_pin_write(int pin, int value);
//         int jtag_pin_set(int pin);
//         int jtag_pin_clear(int pin);
//         int jtag_pin_get(int pin);
//         int jtag_pulse_high(int pin, unsigned int us);
//         int jtag_pulse_low(int pin, unsigned int us);
//         int jtag_assign_pin(int jtag_pin, int digital_pin);


//         int jtag_set_speed(unsigned int khz);
//         int jtag_clock(int tms, int tdi);
//         int jtag_sequence(unsigned int n, const byte *tms, const byte *tdi, byte *tdo);
//         void jtag_reset(void);
//         void jtag_ir(uint32_t length, byte* command, byte* output);
//         void jtag_dr(uint32_t length, byte* data, byte* output);


//     private:
//         void set_bit_in_array(int i_bit, byte *data, int value);
//         int get_bit_from_array(int i_bit, const byte *data);
// };

#endif
