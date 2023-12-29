/**
 * \file         JtagBus.hpp
 * \author       Aliaksander Kavalchuk (aliaksander.kavalchuk@gmail.com)
 * \brief        This file contains the prototypes functions which use for...
 */

#pragma once

//_____ I N C L U D E S _______________________________________________________
#include <JtagCommon.hpp>
#include <JtagPin.hpp>
//_____ C O N F I G S  ________________________________________________________
//_____ D E F I N I T I O N S _________________________________________________
//_____ C L A S S E S __________________________________________________________
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
  JTAG::ERROR sequence(size_t n, const uint8_t tms[], const uint8_t tdi[], uint8_t *tdo);

private:
  uint32_t last_tck_micros = 0;
  uint32_t min_tck_micros = 1;
  JtagPin _tms;
  JtagPin _tdi;
  JtagPin _tdo;
  JtagPin _tck;
  JtagPin _rst;
};
