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
#include "arduino.h"
#else
#include "WProgram.h"
#endif

/***************************************************************************************************
** Define all constants that are to be globally visible                                           **
***************************************************************************************************/
const bool     INVERT_LED{true};      //!< A Value of 0 denotes 100% duty cycle when set
const bool     NO_INVERT_LED{false};  //!< Default. When value is 0 it means off
const uint16_t MAX10BIT{0x3FF};       //!< 1023 decimal - biggest value for 10 bits
/*! @brief   Linear PWM brightness progression table using CIE brightness levels
    @details A linear progression of PWM values of 0 through to 1023 (from "off" to 100%
             "on") does not correspond to a linear increase in percieved brightness to the
             human eye. In order to make this look linear, the CIE 1931 color space and
             lightness formula is used. The kcie table was generated using a program written
             by Jared Sanson and explained on https://jared.geek.nz/2013/feb/linear-led-pwm;
             this table is 1023 X 16-bits and that is a big chunk of Atmel memory */
const PROGMEM uint16_t kcie[] = {
    0,    0,    0,    0,   0,   1,   1,   1,   1,   1,   1,   1,    1,    1,    2,    2,    2,
    2,    2,    2,    2,   2,   2,   3,   3,   3,   3,   3,   3,    3,    3,    3,    4,    4,
    4,    4,    4,    4,   4,   4,   4,   5,   5,   5,   5,   5,    5,    5,    5,    5,    6,
    6,    6,    6,    6,   6,   6,   6,   6,   7,   7,   7,   7,    7,    7,    7,    7,    7,
    8,    8,    8,    8,   8,   8,   8,   8,   8,   9,   9,   9,    9,    9,    9,    9,    9,
    9,    10,   10,   10,  10,  10,  10,  10,  10,  10,  11,  11,   11,   11,   11,   11,   11,
    11,   12,   12,   12,  12,  12,  12,  12,  13,  13,  13,  13,   13,   13,   13,   14,   14,
    14,   14,   14,   14,  14,  15,  15,  15,  15,  15,  15,  16,   16,   16,   16,   16,   16,
    16,   17,   17,   17,  17,  17,  17,  18,  18,  18,  18,  18,   19,   19,   19,   19,   19,
    19,   20,   20,   20,  20,  20,  21,  21,  21,  21,  21,  22,   22,   22,   22,   22,   23,
    23,   23,   23,   23,  24,  24,  24,  24,  24,  25,  25,  25,   25,   26,   26,   26,   26,
    26,   27,   27,   27,  27,  28,  28,  28,  28,  28,  29,  29,   29,   29,   30,   30,   30,
    30,   31,   31,   31,  31,  32,  32,  32,  32,  33,  33,  33,   34,   34,   34,   34,   35,
    35,   35,   35,   36,  36,  36,  37,  37,  37,  37,  38,  38,   38,   39,   39,   39,   39,
    40,   40,   40,   41,  41,  41,  41,  42,  42,  42,  43,  43,   43,   44,   44,   44,   45,
    45,   45,   46,   46,  46,  47,  47,  47,  48,  48,  48,  49,   49,   49,   50,   50,   50,
    51,   51,   51,   52,  52,  52,  53,  53,  53,  54,  54,  55,   55,   55,   56,   56,   56,
    57,   57,   58,   58,  58,  59,  59,  59,  60,  60,  61,  61,   61,   62,   62,   63,   63,
    63,   64,   64,   65,  65,  65,  66,  66,  67,  67,  68,  68,   68,   69,   69,   70,   70,
    71,   71,   71,   72,  72,  73,  73,  74,  74,  75,  75,  75,   76,   76,   77,   77,   78,
    78,   79,   79,   80,  80,  81,  81,  82,  82,  82,  83,  83,   84,   84,   85,   85,   86,
    86,   87,   87,   88,  88,  89,  89,  90,  90,  91,  91,  92,   93,   93,   94,   94,   95,
    95,   96,   96,   97,  97,  98,  98,  99,  99,  100, 101, 101,  102,  102,  103,  103,  104,
    104,  105,  106,  106, 107, 107, 108, 108, 109, 110, 110, 111,  111,  112,  113,  113,  114,
    114,  115,  116,  116, 117, 117, 118, 119, 119, 120, 120, 121,  122,  122,  123,  124,  124,
    125,  126,  126,  127, 127, 128, 129, 129, 130, 131, 131, 132,  133,  133,  134,  135,  135,
    136,  137,  137,  138, 139, 139, 140, 141, 141, 142, 143, 144,  144,  145,  146,  146,  147,
    148,  149,  149,  150, 151, 151, 152, 153, 154, 154, 155, 156,  157,  157,  158,  159,  159,
    160,  161,  162,  163, 163, 164, 165, 166, 166, 167, 168, 169,  169,  170,  171,  172,  173,
    173,  174,  175,  176, 177, 177, 178, 179, 180, 181, 181, 182,  183,  184,  185,  186,  186,
    187,  188,  189,  190, 191, 191, 192, 193, 194, 195, 196, 196,  197,  198,  199,  200,  201,
    202,  203,  203,  204, 205, 206, 207, 208, 209, 210, 211, 211,  212,  213,  214,  215,  216,
    217,  218,  219,  220, 221, 222, 223, 223, 224, 225, 226, 227,  228,  229,  230,  231,  232,
    233,  234,  235,  236, 237, 238, 239, 240, 241, 242, 243, 244,  245,  246,  247,  248,  249,
    250,  251,  252,  253, 254, 255, 256, 257, 258, 259, 260, 261,  262,  263,  264,  265,  266,
    267,  268,  269,  271, 272, 273, 274, 275, 276, 277, 278, 279,  280,  281,  282,  284,  285,
    286,  287,  288,  289, 290, 291, 292, 294, 295, 296, 297, 298,  299,  300,  301,  303,  304,
    305,  306,  307,  308, 310, 311, 312, 313, 314, 315, 317, 318,  319,  320,  321,  323,  324,
    325,  326,  327,  329, 330, 331, 332, 333, 335, 336, 337, 338,  340,  341,  342,  343,  345,
    346,  347,  348,  350, 351, 352, 353, 355, 356, 357, 359, 360,  361,  362,  364,  365,  366,
    368,  369,  370,  372, 373, 374, 376, 377, 378, 380, 381, 382,  384,  385,  386,  388,  389,
    390,  392,  393,  394, 396, 397, 399, 400, 401, 403, 404, 405,  407,  408,  410,  411,  412,
    414,  415,  417,  418, 420, 421, 422, 424, 425, 427, 428, 430,  431,  433,  434,  435,  437,
    438,  440,  441,  443, 444, 446, 447, 449, 450, 452, 453, 455,  456,  458,  459,  461,  462,
    464,  465,  467,  468, 470, 472, 473, 475, 476, 478, 479, 481,  482,  484,  486,  487,  489,
    490,  492,  493,  495, 497, 498, 500, 501, 503, 505, 506, 508,  510,  511,  513,  514,  516,
    518,  519,  521,  523, 524, 526, 528, 529, 531, 533, 534, 536,  538,  539,  541,  543,  544,
    546,  548,  550,  551, 553, 555, 556, 558, 560, 562, 563, 565,  567,  569,  570,  572,  574,
    576,  577,  579,  581, 583, 584, 586, 588, 590, 592, 593, 595,  597,  599,  601,  602,  604,
    606,  608,  610,  612, 613, 615, 617, 619, 621, 623, 625, 626,  628,  630,  632,  634,  636,
    638,  640,  641,  643, 645, 647, 649, 651, 653, 655, 657, 659,  661,  662,  664,  666,  668,
    670,  672,  674,  676, 678, 680, 682, 684, 686, 688, 690, 692,  694,  696,  698,  700,  702,
    704,  706,  708,  710, 712, 714, 716, 718, 720, 722, 724, 726,  728,  731,  733,  735,  737,
    739,  741,  743,  745, 747, 749, 751, 753, 756, 758, 760, 762,  764,  766,  768,  770,  773,
    775,  777,  779,  781, 783, 786, 788, 790, 792, 794, 796, 799,  801,  803,  805,  807,  810,
    812,  814,  816,  819, 821, 823, 825, 827, 830, 832, 834, 837,  839,  841,  843,  846,  848,
    850,  852,  855,  857, 859, 862, 864, 866, 869, 871, 873, 876,  878,  880,  883,  885,  887,
    890,  892,  894,  897, 899, 901, 904, 906, 909, 911, 913, 916,  918,  921,  923,  925,  928,
    930,  933,  935,  938, 940, 942, 945, 947, 950, 952, 955, 957,  960,  962,  965,  967,  970,
    972,  975,  977,  980, 982, 985, 987, 990, 992, 995, 997, 1000, 1002, 1005, 1008, 1010, 1013,
    1015, 1018, 1020, 1023};

class smoothLED {
  /*!
    @class   smoothLED
    @brief   Class to allow PWM pins to be used with 10-bit PWM regardless of which timer they are
             attached to
  */
 public:                                                      // publicly available members
  smoothLED();                                                // Class constructor
  ~smoothLED();                                               // Class destructor
  smoothLED(const smoothLED&) = delete;                       // disable copy constructor
  smoothLED(smoothLED&& led)  = delete;                       // disable move constructor
  smoothLED&  operator++();                                   // prefix increment overload
  smoothLED   operator++(int) = delete;                       // disallow postfix increment
  smoothLED&  operator--();                                   // prefix decrement overload
  smoothLED   operator--(int) = delete;                       // disallow postfix decrement
  smoothLED&  operator+=(const int16_t& value);               // addition overload
  smoothLED&  operator-=(const int16_t& value);               // subtraction overload
  smoothLED&  operator=(const smoothLED& value);              // equals overload
  smoothLED&  operator+(const int16_t& value);                // addition overload
  smoothLED&  operator-(const int16_t& value);                // subtraction overload
  bool        begin(const uint8_t pin,                        // Initialize a pin for PWM
                    const bool    invert = false);               // optional invert values
  void        hertz(const uint8_t hertz) const;               // Set hertz rate for PWM
  static void pwmISR();                                       // Actual PWM function
  static void faderISR();                                     // Actual fader function
  void        set(const uint16_t& val   = 0,                  // Set a pin's value
                  const uint8_t&  speed = 0);                  // optional change speed
 private:                                                     // declare the private class members
  inline void       pinOn() __attribute__((always_inline));   // Turn LED on
  inline void       pinOff() __attribute__((always_inline));  // Turn LED off
  static void       setInterrupts(const bool status);
  static smoothLED* _firstLink;              //!< Static ptr to first instance in  list
  smoothLED*        _nextLink{nullptr};      //!< Ptr to next instance in  list
  volatile uint16_t _currentLevel{0};        //!< Contains the current PWM level
  uint16_t          _targetLevel{0};         //!< Contains the target PWM level
  uint8_t           _changeSpeed{0};         //!< Contains the transition speed
  uint8_t           _changeTicker{0};        //!< Used in counting ticks for change speed
  volatile uint8_t* _portRegister{nullptr};  //!< Ptr to the actual PORT{n} Register
  uint8_t           _registerBitMask{0};     //!< bit mask for the bit used in PORT{n}
  uint8_t           _flags{0};               //!< Status bits, see cpp for details
};                                           // of class smoothLED
#endif
