/*
 Name:		development.ino
 Created:	1/13/2021 1:02:56 PM
 Author:	Arnd
*/

#include "SmoothLED.h"

//  smoothLED     BOARD, Y1, Y2, RED, GREEN, BLUE;
smoothLED     RED, GREEN, BLUE;
const uint8_t RED_PIN{11};
const uint8_t GREEN_PIN{10};
const uint8_t BLUE_PIN{9};

void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println(F("Starting..."));
  /*
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

    */
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

  RED.hertz(60);
  RED.set(500);
  GREEN.set(500);
  BLUE.set(500);
  //    Y1.set(500);
  //    Y2.set(500);
  //    while (1)
  //     ;
}

// the loop function runs over and over again until power down or reset
void loop() {
  RED.set(0);
  GREEN.set(0);
  BLUE.set(0);
  //    Y1.set(0);
  //    Y2.set(0);
  //    BOARD.set(0);
  //    BOARD.set(1023, 128);
  //    Y1.set(1023, 128);
  //    Y2.set(1023, 128);
  RED.set(1023, 128);
  GREEN.set(1023, 128);
  BLUE.set(1023, 128);
  delay(5000);
  Serial.println(F("Next..."));
}
