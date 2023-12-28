/**
 * \file         template.h
 * \author       Aliaksander Kavalchuk (aliaksander.kavalchuk@gmail.com)
 * \brief        This file contains the prototypes functions which use for...
 */

#pragma once

//_____ I N C L U D E S _______________________________________________________
#include <JtagBus.hpp>
#include <JtagCommon.hpp>

//_____ C O N F I G S  ________________________________________________________
//_____ D E F I N I T I O N S _________________________________________________
//_____ C L A S S E S __________________________________________________________
/**
 * \brief
 *
 */
class Jtag
{
public:
  Jtag() = delete;
  explicit Jtag(uint8_t tms, uint8_t tdi, uint8_t tdo, uint8_t tck, uint8_t trst);

  /**
   * \brief
   *
   * \param[] instruction
   * \param[] length
   */
  void ir(uint16_t instruction, uint16_t length);

  /**
   * \brief
   *
   * \param[] data
   * \param[] length
   * \param[] output
   */
  void dr(byte *data, uint32_t length, byte *output);
  // void bitwise();
  uint8_t clock(uint8_t tms, uint8_t tdi);

  /**
   * \brief
   *
   */
  void reset();
  JTAG::ERROR setSpeed(uint32_t khz);

private:
  JtagBus bus;
};
