/**
 * \file         JtagCommon.hpp
 * \author       Aliaksander Kavalchuk (aliaksander.kavalchuk@gmail.com)
 * \brief        This file contains the prototypes functions which use for...
 */

#pragma once

//_____ I N C L U D E S _______________________________________________________
#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

//_____ C O N F I G S  ________________________________________________________
//_____ D E F I N I T I O N S _________________________________________________
namespace JTAG
{
  const uint8_t PINS_NUMBER = 5;

  enum class CONSTANTS : uint32_t
  {
    MAX_SPEED_KHZ = 500,
    MAX_SEQUENCE_LEN = 256,
    MAX_SEQUENCE_LEN_BYTES = MAX_SEQUENCE_LEN / 8,  // 32
  };

  enum class PIN : uint8_t
  {
    TCK = 0,
    TMS = 1,
    TDI = 2,
    TDO = 3,
    TRST = 4,
  };

  enum class ERROR : int32_t
  {
    NO = 0,
    INVALID_PIN = -1,
    INVALID_SPEED = -2,
    INVALID_SEQUENCE_LEN = -3,
  };

  void setBitArray(int i_bit, uint8_t *data, int value);
  int getBitArray(int i_bit, const uint8_t *data);
}
//_____ C L A S S E S __________________________________________________________
