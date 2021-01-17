[![License: GPL v3](https://zanduino.github.io/Badges/GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0) [![Build](https://github.com/Zanduino/SmoothLED/workflows/Build/badge.svg)](https://github.com/Zanduino/SmoothLED/actions?query=workflow%3ABuild) [![Format](https://github.com/Zanduino/SmoothLED/workflows/Format/badge.svg)](https://github.com/Zanduino/SmoothLED/actions?query=workflow%3AFormat) [![Wiki](https://zanduino.github.io/Badges/Documentation-Badge.svg)](https://github.com/Zanduino/SmoothLED/wiki) [![Doxygen](https://github.com/Zanduino/SmoothLED/workflows/Doxygen/badge.svg)](https://Zanduino.github.io/SmoothLED/html/index.html) [![arduino-library-badge](https://www.ardu-badge.com/badge/SmoothLED.svg?)](https://www.ardu-badge.com/SmoothLED)
# Smooth 10-bit LED control library<br><br>This library is almost finished, please bear with us while we put the finishing touches on it. This shouldn't take more than a couple of days before it is complete.<br><br>


_Arduino_ library that allows any number of pins to be used for 10-bit PWM for LEDs.  


## Library description
The library allows any number of pins, as many as the corresponding Atmel ATMega processor has, to be defined as 10-bit PWM output pins. It supports setting PWM values from 0-1023 (where 0 is "OFF" and 1023 is 100% "ON") on any pin along with calls that automate "fading" - e.g. set the pin to "OFF" and fade to "FULL" linearly over 1 second. The details of how to setup the library along with all of the publicly available methods can be found on the [INA wiki pages](https://github.com/Zanduino/SmoothLED/wiki).

Robert Heinlein made the expression [TANSTAAFL](https://en.wikipedia.org/wiki/There_ain%27t_no_such_thing_as_a_free_lunch) popular and it certainly applies here - "_There ain't no such thing as a free lunch_". While certain pins support hardware PWM, they are bound to specific TIMER{n} registers. All of the other pins are relegated to being mere digital pins with only "on" or "off" settings.  This library uses the ATMega's TIMER1 and creates an interrupt in the background which then takes care of setting the pin to "on" and "off" in the background - quickly enough so that it is effectively a PWM signal. But doing this via interrupts means that CPU cycles are being used and these affect how many CPU cycles are left for the currently active sketch. The more LEDs defined in the library and the higher the defined interrupt rate the less cycles are left over for the sketch.  

Generally Arduino programs do not need the full capacity and most sketches will not notice that there is a lot of activity in the background, but some programs might be adversely affected.

The library is optimized so that these costly interrupts are turned off when there is no active PWM (all pins defined are either OFF or ON) and when there is no active fading happening on any pin. Thus there is only an impact when one or more pins are actively PWM-ing. The chart below shows how PWM affects running programs; note that if the program uses "delay()" or otherwise waits on signals the impact is going to be far less noticeable, if at all.

PWM Hertz shows noticeable flicker at values below 30 or 40 (depending upon LED type and brightness) and at 60Hz there is no visible flicker at all. Higher rates are thus not necessary and are wasteful of CPU cycles and should only be used if there is some reason to do so.

##### Program slowdown chart for a 16MHz Arduino:<br>(all pins at 50% duty cycle)
| Hertz  | 1 LED%    | 2 LEDs%   | 3 LEDs%   | 4 LEDs%   | 5 LEDs%   | 6 LEDs%   |
| ------ | --------- | --------- | --------- | --------- | --------- | --------- |
| _off_  |      0%   |      0%   |      0%   |      0%   |      0%   |      0%   |
|   20   |   15.7%   |   21.0%   |   26.7%   |   33.0%   |   40.0%   |   47.8%   |
|   25   |   20.5%   |   27.7%   |   35.8%   |   45.1%   |   55.7%   |   68.0%   |
| **30** | **25.6%** | **35.2%** | **46.3%** | **59.4%** | **75.2%** | **94.3%** |
|   35   |   31.3%   |   43.7%   |   58.6%   |   77.1%   |  100.5%   |  131.0%   |
|   40   |   37.3%   |   53.1%   |   72.9%   |   98.7%   |  133.5%   |  183.0%   |
|   45   |   44.1%   |   64.1%   |   90.5%   |  127.1%   |  181.1%   |  268.7%   |
|   50   |   51.6%   |   76.8%   |  112.0%   |  164.9%   |  252.7%   |  427.8%   |
|   55   |   59.7%   |   91.3%   |  138.4%   |  216.3%   |  369.8%   |  812.5%   |
|   60   |   69.0%   |  108.9%   |  173.2%   |  295.2%   |  613.2%   |       -   |
|   65   |   79.4%   |  129.7%   |  219.4%   |  424.1%   |       -   |       -   |
|   70   |   90.9%   |  155.0%   |  283.6%   |  674.3%   |       -   |       -   |
|   75   |  104.3%   |  187.0%   |  382.1%   |       -   |       -   |       -   |
|   80   |  119.5%   |  227.9%   |  547.1%   |       -   |       -   |       -   |
|   85   |  136.5%   |  279.6%   |  861.6%   |       -   |       -   |       -   |
|   90   |  159.0%   |  361.7%   |       -   |       -   |       -   |       -   |
|   95   |  183.7%   |  476.0%   |       -   |       -   |       -   |       -   |
|  100   |  213.2%   |  661.7%   |       -   |       -   |       -   |       -   |

Any PWM value where the program slowdown is >1000% (meaning the program is 10x slower or just 1/10 of the time goes to the program and the rest is spent servicing PWM) is deemed unacceptable and isn't shown on the chart. If the program does nothing but waiting in "delay()" or similar loops then this would still work.

## Documentation
The documentation has been done using Doxygen and can be found at [doxygen documentation](https://Zanduino.github.io/SmoothLED/html/index.html)

[![Zanshin Logo](https://zanduino.github.io/Images/zanshinkanjitiny.gif) <img src="https://zanduino.github.io/Images/zanshintext.gif" width="75"/>](https://zanduino.github.io)
