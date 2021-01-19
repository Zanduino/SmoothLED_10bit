/*! @file SmoothLED.cpp
 @section SmoothLEDcpp_intro_section Description

Arduino Library for Smooth LED library\n\n
See main library header file for details
*/

#include "SmoothLED.h"

#include "util/atomic.h"
const uint16_t MAX10BIT{0x3FF};   //!< 1023 decimal - biggest value for 10 bits
const uint8_t  FLAG_INVERTED{1};  //!< Bit mask for inverted LED flag
const uint8_t  FLAG_PWM{2};       //!< Bit mask for LED is not 0 or 1023

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
  smoothLED::pwmISR();
}  // Call the ISR every millisecond
#endif
ISR(TIMER0_COMPA_vect) {
  /*!
    @brief   Interrupt vector for TIMER0_COMPA
    @details Indirect call to the TimerISR()
  */
  smoothLED::faderISR();
}  // Call the ISR every millisecond
ISR(TIMER0_COMPB_vect) {
  /*!
    @brief   Interrupt vector for TIMER0_COMPB
    @details Indirect call to the TimerISR()
  */

  smoothLED::faderISR();
}  // Call the ISR every millisecond
smoothLED::smoothLED() {
  /*!
  @brief   Class constructor
  @details There can be many instances of this class, as many as one per pin. Instead of pre-
           allocating storage we'll create a linked list (with just forward-pointers) to the list of
           instances. The interrupt routine needs to use this list to iterate through all instances
           and perform the appropriate PWM actions.
           The first instantiation will enable the interrupts and ISR for OCR0A and OCR0B which is
           used to fade the LEDs
*/
  if (_firstLink == nullptr) {  // If first instantiation
    _firstLink = this;          // This is the first link (static variable)
    _nextLink  = nullptr;       // no next link in list
    /***********************************************************************************************
     ** TIMER0 is used by the Arduino system for timing. Set OCR0A and OCR0B so that they also    **
     ** trigger an interrupt. Each triggers once a millisecond, so with both defined we get an    **
     ** interrupt rate of 2000Hz for brightening and fading effects.                              **
     **********************************************************************************************/
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {  // disable interrupts while changing registers
      OCR0A = 0x40;                      // Comparison register A to 64
      OCR0B = 0xC0;                      // Comparison register B to 192
#if defined(TIMSK0)
      TIMSK0 |= _BV(OCIE0A);
      TIMSK0 |= _BV(OCIE0B);
#elif defined(TIMSK)
      TIMSK |= _BV(OCIE0A);
      TIMSK |= _BV(OCIE0B);
#else
#error Neither TIMSK (ATtiny) nor TIMSK0 defined on this platform
#endif
    }
  } else {                                // otherwise
    smoothLED *last = _firstLink;         // Working pointer to determine last link
    while (last->_nextLink != nullptr) {  // loop to find last link
      last = last->_nextLink;             // go to next link
    }                                     // while not last link loop
    last->_nextLink = this;               // previous link now points to this instance
    _nextLink       = nullptr;            // this instance has no next link
  }                                       // if-then-else first instance of class
}  // of smoothLED class constructor
smoothLED::~smoothLED() {
  /*!
  @brief   Class destructor
  @details Class instances are constructed like a stack, so the first instance destroyed is the last
           one that was instantiated. So when we destroy an instance we just need to remove the last
           link in the list of instances.  When destroying the last surviving instance we disable
           any interrupt that has been set
  */
  if (this == _firstLink) {              // remove interrupts if this is the only instance
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {  // disable interrupts while changing registers
      TIMSK1 &= ~_BV(OCIE1A);            // Unset interrupt on Match A
#if defined(TIMSK0)
      TIMSK0 &= ~_BV(OCIE0A);  // TIMER0_COMPA trigger on 0x01
      TIMSK0 &= ~_BV(OCIE0B);  // TIMER0_COMPB trigger on 0x80
#elif defined(TIMSK)
      TIMSK &= ~_BV(OCIE0A);  // TIMER0_COMPA trigger on 0x01 (ATtiny25-45-85)
      TIMSK &= ~_BV(OCIE0B);  // TIMER0_COMPB trigger on 0x80
#endif
    }                               // re-enable interrupts and leave atomic block
  } else {                          // otherwise
    smoothLED *p = _firstLink;      // set pointer to first link in order to traverse list
    while (p->_nextLink != this) {  // loop until we get to next-to-last link in list
      p = p->_nextLink;             // increment to next element
    }                               // of while loop to traverse linked list
    p->_nextLink = nullptr;         // remove the last element from linked list
  }                                 // if-then-else only link
}  // of of smoothLED class destructor
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
  @details The "=" operator sets the LED values
*/
  this->_currentLevel = value._currentLevel;
  this->_targetLevel  = value._targetLevel;
  this->_changeDelays = value._changeDelays;
  this->_changeTicker = value._changeTicker;
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
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {                              // disable interrupts in block
    if (pin > NUM_DIGITAL_PINS) return false;                      // return on bad pin number
    _registerBitMask = digitalPinToBitMask(pin);                   // get the bitmask for pin
    _portRegister    = portOutputRegister(digitalPinToPort(pin));  // get PORTn for pin
    smoothLED *p     = _firstLink;                                 // set pointer to first link
    bool       firstBegin{true};                                   // Remains true if no others init
    while (p != nullptr) {                                         // loop through all instances
      if (p->_portRegister == _portRegister &&                     // Check to see if re-using
          p->_registerBitMask == _registerBitMask &&               // a pin already defined
          p != this) {                                             // and skip our own link
        _portRegister = nullptr;                                   // set back to null
        return false;                                              // return error
      } else {                                                     // otherwise
        if (p->_portRegister != nullptr &&                         // If _portRegister is not
            p->_portRegister != _portRegister) {                   // null and not current,
          firstBegin = false;                                      // then set flag to false
        }                                                          // if-then first begin() call
      }                                                            // if-then reusing pin
      p = p->_nextLink;                                            // increment to next element
    }                                                              // of while loop
    if (firstBegin) {                                              // If this is the first begin()
#if defined(OCR1AL)
      TCNT1  = 0;          // Initialize counter to 0
      TCCR1B = 0;          // Clear Timer 1 Control Register B
      OCR1A  = 532;        // 30Hz interrupt rate
      sbi(TCCR1B, CS10);   // Set 3 "Clock Select" bits to no pre-scaling
      cbi(TCCR1B, CS11);   // That is Bit 0 is "ON", bit 1 is "OFF",
      cbi(TCCR1B, CS12);   // and bit 2 is "OFF"
      cbi(TCCR1A, WGM10);  // Set "Wave Generation Mode" bits to mode 4: CTC
      cbi(TCCR1A, WGM11);  // Only WGM12 is set, the others are off. The
      sbi(TCCR1B, WGM12);  // interrupt is triggered and the counter is reset
      cbi(TCCR1B, WGM13);  // when the value in OCR1A is matched
#else
#error TIMER not yet defined for this microprocessor
#endif
    }                                                                 // if-then first begin call
    if (invert) {                                                     // If the LED is inverted,
      _flags |= FLAG_INVERTED;                                        // Set the flag bit
    } else {                                                          // otherwise
      _flags &= ~FLAG_INVERTED;                                       // Unset the flag bit
    }                                                                 // if-then-else inverted LED
    volatile uint8_t *ddr = portModeRegister(digitalPinToPort(pin));  // get DDRn port for pin
    *ddr |= _registerBitMask;                                         // make the pin an output
    set(0);                                                           // Turn off pin
  }                                                                   // of atomic block
  return true;                                                        // Return success
}  // of function "begin()"
void smoothLED::pinOn() const {
  /*!
  @brief   Turn the LED to 100% on
  @details Since keeping PWM on and setting the register to the highest value doesn't actually
           result in a 100% duty cyle, this function turns on PWM and does a digital write to set
           the pin to 1
  @return  void returns nothing
*/
  if (_flags & FLAG_INVERTED) {
    *_portRegister &= ~_registerBitMask;
  } else {
    *_portRegister |= _registerBitMask;
  }  // if-then-else _inverted
}
void smoothLED::pinOff() const {
  /*!
    @brief   Turn the LED off
    @details Since keeping PWM on and setting the register to the lowest value doesn't actually
             result in the LED being completely off, this function turns off PWM and does a digital
             write to set  the pin to 0
    @return  void returns nothing
  */
  if (_flags & FLAG_INVERTED) {
    *_portRegister |= _registerBitMask;
  } else {
    *_portRegister &= ~_registerBitMask;
  }  // if-then-else _inverted
}
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
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
#if defined(OCR1AL)
    OCR1A = static_cast<uint16_t>(
        (F_CPU / static_cast<unsigned long>(1023) / static_cast<unsigned long>(hertz)) - 1);
#else
#error Undefined 16-bit register
#endif
  }  // atomic block for interrupts
}  // of function "hertz()"
void smoothLED::set(const uint16_t &val, const uint16_t &speed) {
  /*!
  @brief     sets the LED
  @details   This function does not actually set the pin, it just writes the corresponding parameter
             values to the instance variables. The setting of the pin state is done in the
             "pwmISR()" function which is called by the interrupt triggered by the timer.
  @param[in] val    The value 0-1023 to set the LED. Defaults to 0 (OFF)
  @param[in] speed  The rate of change from 0 (immediate) 1 - slow to 255 - fast. Defaults to 0
*/
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {  // disable interrupts in block
    _flags |= FLAG_PWM;                // Enable PWM for the pin by default
    if (speed == 0) {                  // If we just set a value
      _currentLevel = val & MAX10BIT;  // set current to value and clamp to range
#ifdef CIE_MODE
      _currentCIE = pgm_read_word(kcie + _currentLevel);
#else
      _currentCIE = _currentLevel;
#endif
      _targetLevel  = _currentLevel;    // and set target to value as well
      _changeDelays = 0;                // change speed is not used
      if (_currentCIE == 0) {           // if PWM on and value is OFF
        _flags &= ~FLAG_PWM;            // turn off PWM flag
        pinOff();                       // turn off pin
      } else {                          // otherwise
        if (_currentCIE == MAX10BIT) {  // if PWM on and value is ON
          _flags &= ~FLAG_PWM;          // turn off PWM flag
          pinOn();                      // turn off pin
        }                               // if-then ON
      }                                 // if-then-else OFF
    } else {                            // otherwise we have a change
      _targetLevel = val & MAX10BIT;    // just set a new target and clamp
      /*********************************************************************************************
      ** The interrupt fires 2000 times per second. If the fading/brightening happens at the      **
      ** maximum range (from 0 to 1023) at the fastest speed of 1 level per call, then we have a  **
      ** top speed of 500ms. Compute the number of delay cycles into "_changeDelays" multiplied by**
      ** 100, i.e. for the example above: speed 500 * 2 * 100 / delta = 97 (rounded to integer).  **
      ** The "_changeTicker" is set to the this value as well. In the fade handler the            **
      ** "changeTicker" is decremented by 100 each iteration until it is equal to or less than 0, **
      ** whereupon the brightness value is changed and the "_changeTicker" is reset.              **
      *********************************************************************************************/
      uint32_t temp = (_currentLevel > _targetLevel) ? _currentLevel - _targetLevel
                                                     : _targetLevel - _currentLevel;
      temp = ((uint32_t)speed * 2 * 100) / temp;    // compute the delay factor, see comments above
      if (temp > UINT16_MAX) {                      // if the value is bigger than fits
        temp = UINT16_MAX;                          // clamp it to range,
      } else if (temp < 101) {                      // and if it is less than minimum
        temp = 100;                                 // then set it to minimum
      }                                             // if-then-else out of range
      _changeDelays = static_cast<uint16_t>(temp);  // Set the value, knowing it is in range
      _changeTicker = _changeDelays;  // and then set the ticker variable to that value
    }                                 // if-then-else immediate
    if (_flags & FLAG_PWM) {          // If PWM is needed, then
#if defined(OCR1AL)
      TIMSK1 |= _BV(OCIE1A);  // Set interrupt on Match A for TIMER1
#else
#error TIMER not yet defined for this microprocessor
#endif
    }  // if-then PWM needed
  }    // of atomic block
}  // of function "set()"
void smoothLED::pwmISR() {
  /*!
  @brief     Function to actually perform the PWM on all pins
  @details   This function is the interrupt handler for TIMER1_COMPA and performs the PWM turning ON
             and OFF of all the pins defined in the instances of the class.  It is called very often
             and therefore needs to be as compact as possible. If the LED is set to 30Hz then this
             ISR is call 30*1023 = 30690 times a second, or every 32.5 microseconds. At 16MHz the
             microprocessor only executes 16 instructions per microsecond so it is really important
             to minimize time spent here.
             This function iterates through all the instances of the smoothLED class and sets each
             pin ON or OFF for the appropriate number of cycles.
  */
  static uint16_t _counterPWM{0};           //!< loop counter 0-1023 for software PWM
  smoothLED *     p = _firstLink;           // Set ptr to start of linked list of class instances
  while (p != nullptr) {                    // Loop through linked list of all class instances
    if (p->_portRegister != nullptr) {      // Skip processing if the pin is not initialized
      if (p->_currentCIE == _counterPWM) {  // If we've reached the PWM threshold
        p->pinOff();                        // turn pin off
      } else {                              // otherwise
        if (_counterPWM == 0) {             // if we've rolled over and are at start,
          p->pinOn();                       // turn the pin on
        }                                   // if-then turn ON LED
      }                                     // if-then-else turn off LED
    }                                       // if then a valid pin
    p = p->_nextLink;                       // go to next class instance
  }                                         // of while loop to traverse  list
  ++_counterPWM &= MAX10BIT;                // Pre-increment and clamp to range 0 - 1023
}  // of function "pwmISR()"
void smoothLED::faderISR() {
  /*!
    @brief   Performs fading PWM functions
    @details While the "pwmISR()" needs to be called very frequently in order to perform PWM on the
             pins, the actual fading effect doesn't need to be called that often. Hence this
             function is attached to the TIMER0_COMPA_vect and TIMER0_COMPB_vect and triggered by
             those. The TIMER0 is used by the Arduino for timing (millis() and micros() functions),
             and it is set to overflow roughly every millisecond. By adding these COMPA and COMPB
             triggers, we get a rate of about 500ms for this function, which is enough for a full
             fade from 0 to 1023 to take half a second at top speed.
  */
  smoothLED *p = _firstLink;                      // set ptr to first link for loop
  bool       noPWM{true};                         // Turned off if any pin uses PWM
  while (p != nullptr) {                          // loop through all class instances
    if (p->_portRegister != nullptr) {            // Skip processing if the pin is not initialized
      if (p->_currentLevel == p->_targetLevel) {  // if we have a static PWM value
        /*******************************************************************************************
        ** If the PWM is static and either OFF or ON, then set the value and the FLAG_PWM bit so  **
        ** the ISR doesn't need to process it.                                                    **
        *******************************************************************************************/
        if (p->_currentCIE == 0 && (p->_flags & FLAG_PWM)) {  // if PWM on and value is OFF
          p->_flags &= ~FLAG_PWM;                             // turn off PWM flag
          p->pinOff();                                        // turn off pin
        } else {
          if (p->_currentCIE == MAX10BIT && (p->_flags & FLAG_PWM)) {  // if PWM on and value is ON
            p->_flags &= ~FLAG_PWM;                                    // turn off PWM flag
            p->pinOn();                                                // turn off pin
          }
        }  // if-then-else PWM and OFF
      } else {
        /*******************************************************************************************
        ** Perform the dynamic PWM change at the appropriate speed                                **
        *******************************************************************************************/
        p->_changeTicker -= 100;
        if (p->_changeTicker <= 0) {
          p->_changeTicker += p->_changeDelays;      // add delay factor to ticker
          if (p->_currentLevel > p->_targetLevel) {  // choose direction
            --p->_currentLevel;                      // current > target
          } else {                                   // otherwise
            ++p->_currentLevel;                      // current < target
          }                                          // if-then-else get dimmer
#ifdef CIE_MODE
          p->_currentCIE = pgm_read_word(kcie + p->_currentLevel);
#else
          p->_currentCIE = p->_currentLevel;
#endif
        }  // if-then  change current value
      }    // if-then-else no change in PWM
    }
    if (p->_flags & FLAG_PWM) noPWM = false;  // At least one pin uses PWM
    p = p->_nextLink;                         // go to next class instance
  }                                           // of while loop to traverse  list
  /*************************************************************************************************
  ** If no pins in our class instances are using PWM,  then we can save lots of CPU cycles by     **
  ** disabling the TIMER1 interrupt. Interrupts are re-enabled in the "set()" function            **
  *************************************************************************************************/
  if (noPWM) {               // If no pins are using PWM
    TIMSK1 &= ~_BV(OCIE1A);  // Unset interrupt on Match A
  }                          // if-then no pins are using PWM
}  // of function "faderISR()"
