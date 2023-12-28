/**
 * \file         template.h
 * \author       Aliaksander Kavalchuk (aliaksander.kavalchuk@gmail.com)
 * \brief        This file contains the prototypes functions which use for...
 */

//_____ I N C L U D E S _______________________________________________________
#include "JtagCommon.hpp"

#include <assert.h>

//_____ C O N F I G S  ________________________________________________________
//_____ D E F I N I T I O N S _________________________________________________
//_____ C L A S S E S _________________________________________________________

namespace JTAG
{
  void setBitArray(int i_bit, uint8_t *data, int value)
  {
    int i_byte;
    uint32_t mask;

    i_byte = i_bit >> 3;  // floor(i_bit/8)
    mask = 1 << (i_bit & 0x7);

    if (value == 0)
    {
      data[i_byte] &= ~mask;
    }
    else
    {
      data[i_byte] |= mask;
    }
  }

  void setBitArray(uint32_t i_bit, char *data, char value)
  {
    assert(value == '1' || value == '0');
    data[i_bit] = value;
  }

  int getBitArray(int i_bit, const uint8_t *data)
  {
    int i_byte;
    uint8_t mask;

    i_byte = i_bit >> 3;  // floor(i_bit/8)
    mask = 1 << (i_bit & 0x7);

    return ((data[i_byte] & mask) == 0) ? 0 : 1;
  }

  char getBitArray(uint32_t i_bit, const char *data)
  {
    return data[i_bit];
  }
}  // namespace JTAG