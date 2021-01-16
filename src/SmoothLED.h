/*! @file SmoothLED.h

@mainpage Arduino Library Header to use 10-bit PWM on any pin

@section Smooth_LED_intro_section Description

Hardware PWM on the Atmel ATMega microprocessors can only be done on certain pins, and these are
attached to specific timers and these, in turn, are either 8-bit or 16-bit. The 8-bit timers limit
hardware PWM to a resolution of 8-bits, which causes visible flickering when changing PWM values for
certain brightness levels (mainly in the lower end).

This library is written to allow 10-bit resolution on all pins, even those not associated with a
timer for PWM. It accomplishes this by performing the PWM in software using a high-speed interrupt,
using the first 16-bit timer on the Atmel ATMega processor it is compiled on.

Since the PWM is done in software, it "steals" CPU cycles from the main sketch and the more LEDs
defined in the library the more CPU cycles it consumes.


@section Smooth_LED_doxygen Doxyygen configuration
This library is built with the standard "Doxyfile", which is located at
https://github.com/Zanduino/Common/blob/main/Doxygen. As described on that page, there are only 5
environment variables used, and these are set in the project's actions file, located at
https://github.com/Zanduino/Smooth_LED/blob/master/.github/workflows/ci-doxygen.yml
Edit this file and set the 5 variables: PRETTYNAME, PROJECT_NAME, PROJECT_NUMBER, PROJECT_BRIEF and
PROJECT_LOGO so that these values are used in the doxygen documentation. The local copy of the
doxyfile should be in the project's root directory in order to do local doxygen testing, but the
file is ignored on upload to GitHub.

@section Smooth_LEDclang clang-format
Part of the GitHub actions for CI is running every source file through "clang-format" to ensure
that coding formatting is done the same for all files. The configuration file ".clang-format" is
located at https://github.com/Zanduino/Common/tree/main/clang-format and this is used for CI tests
when pushing to GitHub. The local file, if present in the root directory, is ignored when
committing and uploading.

@section Smooth_LEDlicense GNU General Public License v3.0

This program is free software: you can redistribute it and/or modify it under the terms of the GNU
General Public License as published by the Free Software Foundation, either version 3 of the
License, or (at your option) any later version. This program is distributed in the hope that it will
be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details. You should have
received a copy of the GNU General Public License along with this program.  If not, see
<http://www.gnu.org/licenses/>.

@section Smooth_LED_author Author

Written by Arnd <Arnd@Zanduino.Com> at https://www.github.com/SV-Zanshin

@section Smooth_LED_versions Changelog

| Version| Date       | Developer  | Comments                                                      |
| ------ | ---------- | ---------- | ------------------------------------------------------------- |
| 1.0.0  | 2021-01-15 | SV-Zanshin | Optimized instantiation and completed coding and testing      |
| 1.0.0  | 2021-01-10 | SV-Zanshin | Created new library for the class                             |
*/

#ifndef _smoothLED_h
#define _smoothLED_h

#if defined(ARDUINO) && ARDUINO >= 100
#include <Arduino.h>
#else
#include <WProgram.h>
#endif

/***************************************************************************************************
** Define all constants that are to be globally visible                                           **
***************************************************************************************************/
const bool INVERT_LED{true};      //!< A Value of 0 denotes 100% duty cycle when set
const bool NO_INVERT_LED{false};  //!< Default. When value is 0 it means off

class smoothLED {
  /*!
    @class   smoothLED
    @brief   Class to allow PWM pins to be used with 10-bit PWM regardless of which timer they are
             attached to
  */
 public:                                          // Declare all publicly visible members
  smoothLED();                                    // Class constructor
  ~smoothLED();                                   // Class destructor
  smoothLED(const smoothLED&) = delete;           // disable copy constructor
  smoothLED(smoothLED&& led)  = delete;           // disable move constructor
  smoothLED&  operator++();                       // prefix increment overload
  smoothLED   operator++(int) = delete;           // disallow postfix increment
  smoothLED&  operator--();                       // prefix decrement overload
  smoothLED   operator--(int) = delete;           // disallow postfix decrement
  smoothLED&  operator+=(const int16_t& value);   // addition overload
  smoothLED&  operator-=(const int16_t& value);   // subtraction overload
  smoothLED&  operator=(const smoothLED& value);  // equals overload
  smoothLED&  operator+(const int16_t& value);    // addition overload
  smoothLED&  operator-(const int16_t& value);    // subtraction overload
  bool        begin(const uint8_t pin,            // Initialize a pin for PWM
                    const bool    invert = false);   // optional invert values
  void        hertz(const uint8_t hertz) const;   // Set hertz rate for PWM
  static void pwmISR();                           // Actual PWM function
  static void faderISR();                         // Actual fader function
  void        set(const uint16_t& val,            // Set a pin's value
                  const uint8_t   speed = 0);       // optional change speed
 private:                                         // declare the private class members
  static smoothLED* _firstLink;                   //!< Static pointer to first instance in list
  static uint16_t   counterPWM;                   //!< loop counter 0-1023 for software PWM
  volatile uint8_t* _portRegister{nullptr};       //!< Pointer to the actual PORT{n} Register
  smoothLED*        _nextLink{nullptr};           //!< Pointer to the next instance in  list
  uint8_t           _registerBitMask{0};          //!< bit mask for the bit used in PORT{n}
  volatile uint16_t _currentLevel{0};             //!< Current PWM level 0-1023
  volatile uint16_t _currentCIE{0};               //!< Current PWM level from cie table
  uint16_t          _targetLevel{0};              //!< Target PWM level 0-1023
  uint8_t           _changeSpeed{0};              //!< Speed at which fading happens 0-255
  uint8_t           _changeTicker{0};             //!< Countdown in ticks used for fading
  uint8_t           _flags{0};                    //!< Status bits, see cpp for details
  inline void       pinOn() const __attribute__((always_inline));   // Turn LED on
  inline void       pinOff() const __attribute__((always_inline));  // Turn LED off
  static void       setInterrupts(const bool status);               // Turn interrupts on or off
};  // of class smoothLED                                           //
#endif
