/**
 * \file         template.h
 * \author       Aliaksander Kavalchuk (aliaksander.kavalchuk@gmail.com)
 * \brief        This file contains the prototypes functions which use for...
 */

#pragma once

//_____ I N C L U D E S _______________________________________________________
#include <Arduino.h>
#include <JtagCommon.hpp>
//_____ C O N F I G S  ________________________________________________________
//_____ D E F I N I T I O N S _________________________________________________
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
