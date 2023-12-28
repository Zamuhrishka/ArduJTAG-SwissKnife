/**
 * \file         template.h
 * \author       Aliaksander Kavalchuk (aliaksander.kavalchuk@gmail.com)
 * \brief        This file contains the prototypes functions which use for...
 */

//_____ I N C L U D E S _______________________________________________________
#include "Jtag.hpp"
#include "JtagBus.hpp"
#include "JtagCommon.hpp"

#include <assert.h>

//_____ C O N F I G S  ________________________________________________________
#define DEBUG_MODE

//_____ D E F I N I T I O N S _________________________________________________
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
}  // namespace JTAG_PRIV

//_____ C L A S S E S __________________________________________________________
Jtag::Jtag(uint8_t tms, uint8_t tdi, uint8_t tdo, uint8_t tck, uint8_t rst):
    bus(JtagPin(tms, OUTPUT), JtagPin(tdi, OUTPUT), JtagPin(tdo, INPUT), JtagPin(tck, OUTPUT), JtagPin(rst, OUTPUT))
{
}

void Jtag::ir(uint16_t instruction, uint16_t length)
{
  assert(length != 0);

  uint8_t tms_pre[1] = {JTAG_PRIV::IR_TMS_PRE};
  uint8_t tms_post[1] = {JTAG_PRIV::IR_TMS_POST};
  uint8_t tdi_pre[1] = {0x00};
  uint8_t tdi_post[1] = {0x00};

  // Sending PreBitwise
  for (uint16_t i_seq = 0; i_seq < JTAG_PRIV::IR_TMS_PRE_LEN; i_seq++)
  {
    this->bus.clock(JTAG::getBitArray(i_seq, &tms_pre[0]), JTAG::getBitArray(i_seq, &tdi_pre[0]));
  }

  for (uint16_t i_seq = 0; i_seq < length - 1; i_seq++)
  {
    this->bus.clock(0, JTAG::getBitArray(i_seq, (byte *)&instruction));
  }

  this->bus.clock(1, JTAG::getBitArray(length - 1, (byte *)&instruction));

  // Sending PostBitwise
  for (uint16_t i_seq = 0; i_seq < JTAG_PRIV::IR_TMS_POST_LEN; i_seq++)
  {
    this->bus.clock(JTAG::getBitArray(i_seq, &tms_post[0]), JTAG::getBitArray(i_seq, &tdi_post[0]));
  }
}

void Jtag::dr(byte *data, uint32_t length, byte *output)
{
  assert(data != nullptr);
  assert(length != 0);

  byte tms_pre[1] = {JTAG_PRIV::DR_TMS_PRE};
  byte tms_post[1] = {JTAG_PRIV::DR_TMS_POST};
  byte tdi_pre[1] = {0x00};
  byte tdi_post[1] = {0x00};
  uint16_t bit_offset = 0;

  for (uint16_t i_seq = 0; i_seq < JTAG_PRIV::DR_TMS_PRE_LEN; i_seq++ /*, bit_offset++*/)
  {
    // #if defined(DEBUG_MODE)
    //     Serial.print("[");
    //     Serial.print(bit_offset);
    //     Serial.println("]");
    // #endif

    JTAG::setBitArray(bit_offset,
                      &output[0],
                      this->bus.clock(JTAG::getBitArray(i_seq, &tms_pre[0]), JTAG::getBitArray(i_seq, &tdi_pre[0])));
    // this->bus.clock(JTAG::getBitArray(i_seq, &tms_pre[0]), JTAG::getBitArray(i_seq, &tdi_pre[0]));
  }

  // #if defined(DEBUG_MODE)
  //   Serial.println("***");
  // #endif

  bit_offset++;

  for (uint16_t i_seq = 0; i_seq < length - 1; i_seq++, bit_offset++)
  {
    // #if defined(DEBUG_MODE)
    //     Serial.print("(");
    //     Serial.print(bit_offset);
    //     Serial.println(")");
    // #endif
    JTAG::setBitArray(bit_offset /* + i_seq*/, &output[0], this->bus.clock(0, JTAG::getBitArray(i_seq, data)));
  }

  // #if defined(DEBUG_MODE)
  //   Serial.print("(");
  //   Serial.print(bit_offset);
  //   Serial.println(")");
  // #endif
  JTAG::setBitArray(bit_offset, &output[0], this->bus.clock(1, JTAG::getBitArray(length - 1, data)));

  // #if defined(DEBUG_MODE)
  //   Serial.println("***");
  // #endif

  bit_offset++;

  for (uint16_t i_seq = 0; i_seq < JTAG_PRIV::DR_TMS_POST_LEN; i_seq++)
  {
    // #if defined(DEBUG_MODE)
    //     Serial.print("{");
    //     Serial.print(bit_offset);
    //     Serial.println("}");
    // #endif

    // JTAG::setBitArray(bit_offset,
    //                   &output[0],
    //                   this->bus.clock(JTAG::getBitArray(i_seq, &tms_post[0]), JTAG::getBitArray(i_seq,
    //                   &tdi_post[0])));
    this->bus.clock(JTAG::getBitArray(i_seq, &tms_post[0]), JTAG::getBitArray(i_seq, &tdi_post[0]));
    // bit_offset++;
  }

  // #if defined(DEBUG_MODE)
  //   Serial.println("***");
  // #endif
}

void Jtag::reset()
{
  byte tms = JTAG_PRIV::RESET_TMS;
  byte tdi = 0x00;
  byte tdo = 0x00;

  this->bus.sequence(JTAG_PRIV::RESET_TMS_LEN, &tms, &tdi, &tdo);
}

JTAG::ERROR Jtag::setSpeed(uint32_t khz)
{
  return this->bus.setSpeed(khz);
}

uint8_t Jtag::clock(uint8_t tms, uint8_t tdi)
{
  return this->bus.clock(tms, tdi);
}
