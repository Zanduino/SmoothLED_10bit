[![License: GPL v3](https://zanduino.github.io/Badges/GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0) [![Build](https://github.com/Zanduino/SmoothLED/workflows/Build/badge.svg)](https://github.com/Zanduino/SmoothLED/actions?query=workflow%3ABuild) [![Format](https://github.com/Zanduino/SmoothLED/workflows/Format/badge.svg)](https://github.com/Zanduino/SmoothLED/actions?query=workflow%3AFormat) [![Wiki](https://zanduino.github.io/Badges/Documentation-Badge.svg)](https://github.com/Zanduino/SmoothLED/wiki) [![Doxygen](https://github.com/Zanduino/SmoothLED/workflows/Doxygen/badge.svg)](https://Zanduino.github.io/SmoothLED/html/index.html) [![arduino-library-badge](https://www.ardu-badge.com/badge/SmoothLED.svg?)](https://www.ardu-badge.com/SmoothLED)
# Smooth 10-bit LED control library<br>

_Arduino_ library that allows one or more pins on any Arduino to be used as 10-bit PWM for LEDs. [at the INA wiki pages]

This library is almost finished, please bear with us while we put the finishing touches on it. This shouldn't take more than a couple of days before it is complete.

## Library description
The library allows any number of pins, up to the number of pins on the corresponding Atmel ATMega processor, to be defined as 10-bit PWM output pins. It support setting PWM values from 0-1023 (where 0 if "OFF" and 1023 is 100% "ON") on any pin along with calls that automate "fading" - e.g. set the pin to "OFF" and fade to "FULL" linearly over 1 second. The details of how to setup the library along with all of the publicly available methods can be found on the [INA wiki pages](https://github.com/Zanduino/SmoothLED/wiki).

As Robert Heinlein made popular, [TANSTAAFL](https://en.wikipedia.org/wiki/There_ain%27t_no_such_thing_as_a_free_lunch) - There ain't no such thing as a free lunch. While certain pins support hardware PWM, they are bound to specific TIMER{n} registers. All of the other pins are relegated to being mere digital pins with only "on" or "off" settings.  This library uses TIMER1 and creates an interrupt in the background which then takes care of setting the pin to "on" and "off" in the background - quickly enough so that it is effectively a PWM signal. But doing this via interrupts means that CPU cycles are being used and these affect how many CPU cycles are left for the currently active sketch. The more LEDs defined in the library and the higher the defined interrupt rate the less cycles are left for the sketch.  

Generally Arduino programs do not need the full capacity and most sketches will not notice that there is a lot of activity in the background, but some programs will be adversely affected.

Along the same lines, while the library supports defining as many LEDs as there are pins, once too many LEDs are defined there might be a noticeable flicker due to processing time.

## Documentation
The documentation has been done using Doxygen and can be found at [doxygen documentation](https://Zanduino.github.io/SmoothLED/html/index.html)

[![Zanshin Logo](https://zanduino.github.io/Images/zanshinkanjitiny.gif) <img src="https://zanduino.github.io/Images/zanshintext.gif" width="75"/>](https://zanduino.github.io)
