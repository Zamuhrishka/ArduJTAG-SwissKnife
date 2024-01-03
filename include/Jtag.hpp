/**
 * \file         Jtag.hpp
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
  void dr(uint8_t *data, uint32_t length, uint8_t *output);

  JTAG::ERROR sequence(size_t n, const uint8_t tms[], const uint8_t tdi[], uint8_t *tdo);

  /**
   * \brief
   *
   */
  void reset();
  JTAG::ERROR setSpeed(uint32_t khz);

private:
  JtagBus bus;
};
