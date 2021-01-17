/*
 Name:		development.ino
 Created:	1/13/2021 1:02:56 PM
 Author:	Arnd
*/

#include "SmoothLED.h"

//  smoothLED     BOARD, Y1, Y2, RED, GREEN, BLUE;
smoothLED RED, GREEN, BLUE, BOARD, Y1, Y2;

const uint8_t RED_PIN{11};
const uint8_t GREEN_PIN{10};
const uint8_t BLUE_PIN{9};

void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println(F("Starting..."));

  Serial.print(F("BOARD LED: "));
  if (BOARD.begin(LED_BUILTIN))
    Serial.println(F("OK"));
  else
    Serial.println(F("ERROR!"));
  Serial.print(F("Y1: "));

if (Y1.begin(3))
Serial.println(F("OK"));
else
Serial.println(F("ERROR!"));

Serial.print(F("Y2: "));
if (Y2.begin(6, INVERT_LED))
Serial.println(F("OK"));
else
Serial.println(F("ERROR!"));


  Serial.print(F("RED: "));
  if (RED.begin(RED_PIN, INVERT_LED))
    Serial.println(F("OK"));
  else
    Serial.println(F("ERROR!"));

  Serial.print(F("GREEN: "));
  if (GREEN.begin(GREEN_PIN, INVERT_LED))
    Serial.println(F("OK"));
  else
    Serial.println(F("ERROR!"));

  Serial.print(F("BLUE: "));
  if (BLUE.begin(BLUE_PIN, INVERT_LED))
    Serial.println(F("OK"));
  else
    Serial.println(F("ERROR!"));

  uint32_t startTime = millis();
  for (uint32_t i = 0; i != 0x1FFFFFF; ++i) {
    __asm__ __volatile__("nop\n\t");
  }
  uint32_t normalTime = millis() - startTime;
  Serial.print(F("No PWM time is "));
  Serial.println(normalTime);

  RED.set(512);
  GREEN.set(512);
  BLUE.set(512);
  BOARD.set(512);
  Y1.set(512);
  Y2.set(512);

  for (uint8_t i = 20; i < 105; i += 5) {
    Serial.print(F("Hertz = "));
    Serial.print(i);
    RED.hertz(i);
    startTime = millis();
    for (uint32_t i = 0; i != 0x1FFFFFF; ++i) {
      __asm__ __volatile__("nop\n\t");
    }
    Serial.print(F(" - "));
    uint32_t x = millis() - startTime;
    Serial.print(x);
    Serial.print(F(" "));
    Serial.print(100.0 - ((float)x / normalTime * 100), 2);
    Serial.println(F("%"));
  }

  startTime = millis();
  for (uint32_t i = 0; i != 0x1FFFFFF; ++i) {
    __asm__ __volatile__("nop\n\t");
  }
  Serial.print(F("Time is "));
  Serial.println(millis() - startTime);
}

// the loop function runs over and over again until power down or reset
void loop() {
  /*
    RED.set(1023);
    GREEN.set(1023);
    BLUE.set(1023);
    RED.set(0, 240);
    GREEN.set(0, 240);
    BLUE.set(0, 240);
    delay(10000);
    Serial.println(F("Next..."));
  */
}
