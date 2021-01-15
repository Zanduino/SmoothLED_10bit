/*! @file SmoothLED.cpp
 @section SmoothLEDcpp_intro_section Description

Arduino Library for Smooth LED library\n\n
See main library header file for details
*/

#include "SmoothLED.h"

smoothLED *smoothLED::_firstLink{nullptr};  // static member declaration outside of class for init

/***************************************************************************************************
** Not all of these macros are defined on all platforms, so redefine them here just in case       **
***************************************************************************************************/
#ifndef _BV
#define _BV(bit) (1 << (bit))  //!< bit shift macro
#endif
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))  //!< clear bit macro
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))  //!<  set bit macro
#endif

#if defined(OCR1AL)
ISR(TIMER1_COMPA_vect) {
  /*!
    @brief   Interrupt vector for TIMER1_COMPA
    @details Indirect call to the TimerISR()
  */
  smoothLED::performPWM();
}  // Call the ISR every millisecond
#endif

smoothLED::smoothLED() {
  /*!
  @brief   Class constructor
  @details There can be many instances of this class, as many as one per pin. Instead of pre-
           allocating storage we'll create a linked list (with just forward-pointers) to the list of
           instances. The interrupt routine needs to use this list to iterate through all instances
           and perform the appropriate PWM actions.
           The first instantiation will enable the interrupts and ISR for OCR0A and OCR0B
*/
  if (_firstLink == nullptr) {            // If first instantiation
    _firstLink = this;                    // This is the first link (static variable)
    _nextLink  = nullptr;                 // no next link (local variable)
  } else {                                // otherwise
    smoothLED *last = _firstLink;         // Working pointer to determine last link
    while (last->_nextLink != nullptr) {  // loop to find last link
      last = last->_nextLink;             // go to next link
    }                                     // while not last link loop
    last->_nextLink = this;               // previous link now points to this instance
    _nextLink       = nullptr;            // this instance has no next link
  }                                       // if-then-else first instance of class
}  // of constructor
smoothLED::~smoothLED() {
  /*!
  @brief   Class destructor
  @details Class instances are constructed like a stack, so the first instance destroyed is the last
           one that was instantiated. So when we destroy an instance we just need to remove the last
           link in the list of instances.  When destroying the last surviving instance we disable
           any interrupt that has been set
  */
  if (this == _firstLink) {  // remove interrupts if this is the only instance
    cli();                   // Disable interrupts while changing values
#if defined(OCR1AL)
    TIMSK1 &= ~_BV(OCIE1A);  // turn off interrupt on Match A
#endif
    sei();                          // Re-enable interrupts after changing values
  } else {                          // otherwise
    smoothLED *p = _firstLink;      // set pointer to first link in order to traverse list
    while (p->_nextLink != this) {  // loop until we get to next-to-last link in list
      p = p->_nextLink;             // increment to next element
    }                               // of while loop to traverse linked list
    p->_nextLink = nullptr;         // remove the last element from linked list
  }                                 // if-then-else only link
}  // of destructor
smoothLED &smoothLED::operator++() {
  /*!
    @brief   ++ Overload
    @details The "++" pre-increment operator increments the target LED level
  */
  ++_targetLevel &= MAX10BIT;  // increment target and clamp to range
  return *this;
}
smoothLED &smoothLED::operator--() {
  /*!
    @brief   -- Overload
    @details The "--" pre-decrement operator increments the target LED level
  */
  --_targetLevel &= MAX10BIT;  // decrement target and clamp to range
  return *this;
}
smoothLED &smoothLED::operator+(const int16_t &value) {
  /*!
    @brief   + Overload
    @details The "+" operator increments the target LED level by the specified value
  */
  this->_targetLevel = (this->_targetLevel + value) & MAX10BIT;  // increment target and clamp
  return *this;
}
smoothLED &smoothLED::operator-(const int16_t &value) {
  /*!
    @brief   - Overload
    @details The "-" operator decrements the target LED level by the specified value
  */
  this->_targetLevel = (this->_targetLevel - value) & MAX10BIT;  // decrement target and clamp
  return *this;
}
smoothLED &smoothLED::operator+=(const int16_t &value) {
  /*!
  @brief   += Overload
  @details The "+=" operator increments the target LED level by the specified value
*/
  this->_targetLevel = (this->_targetLevel + value) & MAX10BIT;  // increment target and clamp
  return *this;
}
smoothLED &smoothLED::operator-=(const int16_t &value) {
  /*!
  @brief   - Overload
  @details The "-" operator decrements the target LED level by the specified value
*/
  this->_targetLevel = (this->_targetLevel - value) & MAX10BIT;  // decrement target and clamp
  return *this;
}
smoothLED &smoothLED::operator=(const smoothLED &value) {
  /*!
  @brief   = Overload
  @details The "=" operator sets the LED value of _currentLevel, _targetLevel and _changeSpeed
*/
  this->_currentLevel = value._currentLevel;
  this->_targetLevel  = value._targetLevel;
  this->_changeSpeed  = value._changeSpeed;
  return *this;
}
bool smoothLED::begin(const uint8_t pin, const bool invert) {
  /*!
  @brief     Initializes the LED
  @details   The function returns an error (false) if a pin doesn't exist, or the pin has already
             been defined. The pin is made an output pin and the register address for the PORT
             number and bitmask are stored with the class instance along with flag on whether the
             LED is inverted (where 0 denotes full ON and 1023 means OFF); as LEDs can be attached
             to the pin in either direction.
             The TIMER is set to no prescaling and the mode is set to CTC. This is done here, rather
             than in the class constructor, since the Arduino IDE overwrites the timing registers
             and the class is typically constructed before the setup() call. Although each instance
             sets these values, they apply globally to all instances of the class.
  @param[in] pin    The Arduino pin number of the LED
  @param[in] invert Boolean - when false then a value of "0" represents "OFF" and the max
                    value represent "ON". Some LEDs are hooked up differently with and the values
                    are reversed
  @return    bool   TRUE on success, FALSE when the pin is not a PWM-Capable one
*/

  if (pin > NUM_DIGITAL_PINS) return false;                         // return on bad pin number
  _bitMaskRegister = digitalPinToBitMask(pin);                      // get the bitmask for pin
  _portRegister    = portOutputRegister(digitalPinToPort(pin));     // get PORTn for pin
  smoothLED *p     = _firstLink;                                    // set pointer to first link
  while (p != nullptr) {                                            // loop through all instances
    if (p->_portRegister == _portRegister &&                        // Check to see if re-using
        p->_bitMaskRegister == _bitMaskRegister &&                  // a pin already defined
        p != this) {                                                // and skip our own link
      _portRegister = nullptr;                                      // set back to null
      return false;                                                 // return error
    }                                                               // if-then reusing
    p = p->_nextLink;                                               // increment to next element
  }                                                                 // of while loop
  _inverted             = invert;                                   // Normally 0 is off
  volatile uint8_t *ddr = portModeRegister(digitalPinToPort(pin));  // get DDRn port for pin
  *ddr |= _bitMaskRegister;                                         // make the pin an output
  /*************************************************************************************************
   ** The Arduino IDE resets TIMER{n} values, and if the instances of this class are defined      **
   ** globally then any TIMER{n} setup information gets lost, so just overwrite the values during **
   ** the begin() call. Since the calls are fast, don't bother checking and just overwrite them   **
   ************************************************************************************************/
  cli();  // Disable interrupts while changing values
#if defined(OCR1AL)
  TCCR1B = 0;             // Clear Timer 1 Control Register B
  sbi(TCCR1B, CS10);      // Set 3 "Clock Select" bits to no pre-scaling
  cbi(TCCR1B, CS11);      // That is Bit 0 is "ON", bit 1 is "OFF",
  cbi(TCCR1B, CS12);      // and bit 2 is "OFF"
  cbi(TCCR1A, WGM10);     // Set "Wave Generation Mode" bits to mode 4: CTC
  cbi(TCCR1A, WGM11);     // Only WGM12 is set, the others are off. The
  sbi(TCCR1B, WGM12);     // interrupt is triggered and the counter is reset
  cbi(TCCR1B, WGM13);     // when the value in OCR1A is matched
  TIMSK1 |= _BV(OCIE1A);  // Set interrupt on Match A
#else
#error TIMER not yet defined for this micrprocessor
#endif

  sei();      // Re-enable interrupts after changing values
  hertz(40);  // Start off with 40Hz
}  // of function "begin()"
void smoothLED::hertz(const uint8_t hertz) const {
  /*!
@brief     Set the PWM frequency
@details   The function sets the PWM frequency. The actual interrupt rate is 1023 times the Hertz
           value specified, that is accounted for in the formula.  While rate down to 1Hz can be
           given, anything below 30 (depending on the LED and brightness) causes visible flickering
           and should be avoided. Any number beyond XXXX is ignored, as the interrupt rate would be
           so high that there are no CPU cycles left to process the main program.
@param[in] hertz    Unsigned integer Hertz setting for LED PWM
*/
#if defined(OCR1AL)
  OCR1A = static_cast<uint16_t>(
      (F_CPU / static_cast<unsigned long>(1023) / static_cast<unsigned long>(hertz)) - 1);
#endif
}  // of function "hertz()"
void smoothLED::set(const uint16_t &val, const uint8_t &speed) {
  /*!
  @brief     sets the LED
  @details   This function does not actually set the LED, it just writes the corresponding parameter
             values to the instance variables. The setting of the pin state is done in the
             "performPWM()" function which is called by the interrupt triggered byt the timer.
  @param[in] val    The value 0-1023 to set the LED. Defaults to 0 (OFF)
  @param[in] speed  The rate of change from 0 (immediate) 1 - slow to 255 - fast. Defaults to 0
*/
  if (speed == 0) {
    _currentLevel = val & MAX10BIT;  // set current to value and clamp
    _targetLevel  = _currentLevel;   // and set target to value as well
    _changeSpeed  = 0;               // change speed is not used
  } else {                           // otherwise we have a change
    _targetLevel  = val & MAX10BIT;  // just set a new target and clamp
    _changeSpeed  = speed;           // and set a change rate
    _changeTicker = speed;           // and set the ticker variable
  }                                  // if-then-else immediate
}  // of function "set()"
void smoothLED::performPWM() {
  /*!
  @brief     Function to actually perform the PWM on all pins
  @details   This function is not called by any user functions, it is triggered directly from the
             ISR which gets called when the TIMER1_COMPA condition is met. This condition is a timed
             condition which can be modified by changing the interrupt rate using the "hertz()"
             function. Any values below 30Hz result in visible flickering at certain PWM values, but
             the higher the Hertz the more time is spent in the interrupt and the less time is
             available to the user program.
             This function iterates through all the instances of the smoothLED class and sets each
             pin to the appropriate PWM rate.
*/
  static uint16_t counter{0};      // loop 0-1023, remember value from last
  smoothLED *     p = _firstLink;  // set ptr to first link for loop
  while (p != nullptr) {           // loop through all class instances
    /***********************************************************************************************
    ** First we perform the actual PWM by turning the pin ON or OFF for the amount of cycles that **
    ** correspond to the width set. This is done by using "counter" (a static variable) which     **
    ** cycles from 0 to 1023. There are 4 conditions that are checked:                            **
    ** 1. PWM level is    0  Turn pin OFF                                                         **
    ** 2. PWM level is 1023  Turn pin ON                                                          **
    ** 3. counter is 0       Turn pin ON                                                          **
    ** 4. counter = PWM      Turn pin OFF                                                         **
    **                                                                                            **
    ** Turning ON/OFF is done by writing directly to register, inverting the value when required. **
    ***********************************************************************************************/
    if (p->_portRegister != nullptr) {                      // skip if not initialized
      if (p->_currentLevel == 0) {                          // turn off LED if level is 0
        if (p->_inverted) {                                 // If inverted
          *p->_portRegister |= p->_bitMaskRegister;         // inverted, so set pin
        } else {                                            // otherwise
          *p->_portRegister &= ~p->_bitMaskRegister;        // not inverted, unset pin
        }                                                   // if-then-else _inverted
      } else {                                              // otherwise level not 0
        if (p->_currentLevel == MAX10BIT) {                 // turn on LED if level is max
          if (p->_inverted) {                               // if inverted
            *p->_portRegister &= ~p->_bitMaskRegister;      // inverted, so unset bit
          } else {                                          // otherwise
            *p->_portRegister |= p->_bitMaskRegister;       // not inverted, so set bit
          }                                                 // if-then-else _inverted
        } else {                                            // otherise neither 0 nor max
          if (counter == 0) {                               // if at beginning
            if (p->_inverted) {                             // if inverted
              *p->_portRegister &= ~p->_bitMaskRegister;    // inverted, so unset bit
            } else {                                        // otherwise
              *p->_portRegister |= p->_bitMaskRegister;     // not inverted, set bit
            }                                               // if-then-else _inverted
          } else {                                          // otherwise if not at beginning
            if (p->_currentLevel == counter) {              // if level matches counter
              if (p->_inverted) {                           // we need to turn off
                *p->_portRegister |= p->_bitMaskRegister;   // if inverted then set bit
              } else {                                      // otherwise
                *p->_portRegister &= ~p->_bitMaskRegister;  // if not inverted the unset bit
              }                                             // if-then-else _inverted
            }                                               // if-then match counter
          }                                                 // if-then-else start new cycle
        }                                                   // if-then-else MAX level
      }                                                     // if-then-else MIN level
    }                                                       // if then a valid link
    /***********************************************************************************************
    ** Now we perform the dynamic PWM change, if current and target values differ.                **
    ***********************************************************************************************/
    if (p->_currentLevel != p->_targetLevel && ++p->_changeTicker == 0) {  // if changing and ticker
      p->_changeTicker = p->_changeSpeed;                                  // then reset counter
      if (p->_currentLevel > p->_targetLevel) {                            // choose direction
        --p->_currentLevel;                                                // current > target
      } else {                                                             // otherwise
        ++p->_currentLevel;                                                // current < target
      }                   // if-then-else get dimmer
    }                     // if-then we need to change current value
    p = p->_nextLink;     // go to next class instance
  }                       // of while loop to traverse  list
  ++counter &= MAX10BIT;  // increment and clamp 0-1023
}  // of function "performPWM()"
