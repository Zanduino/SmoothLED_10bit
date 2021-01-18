/*
 Name:		development.ino
 Created:	1/13/2021 1:02:56 PM
 Author:	Arnd
*/

#include "SmoothLED.h"

smoothLED BOARD, Y1, Y2, RED, GREEN, BLUE;
// smoothLED GREEN;

const uint8_t RED_PIN{11};
const uint8_t GREEN_PIN{10};
const uint8_t BLUE_PIN{9};

void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println(F("Starting..."));

  GREEN.begin(GREEN_PIN, INVERT_LED);
  BOARD.begin(LED_BUILTIN);
  Y2.begin(6, INVERT_LED);
  Y1.begin(3);
  RED.begin(RED_PIN, INVERT_LED);
  BLUE.begin(BLUE_PIN, INVERT_LED);

  Serial.print("SREG ");
  Serial.println(SREG, BIN);
  Serial.print("TIMSK0 ");
  Serial.println(TIMSK0, BIN);
  Serial.print("TCCR1A ");
  Serial.println(TCCR1A, BIN);
  Serial.print("TCCR1B ");
  Serial.println(TCCR1B, BIN);
  GREEN.hertz(45);
}

// the loop function runs over and over again until power down or reset
void loop() {
  GREEN.set(0);
  GREEN.set(1023, 200);
  delay(30000);

  Serial.println(F("Next..."));
  /*
    RED.set(1023);
    GREEN.set(1023);
    BLUE.set(1023);
    RED.set(0, 240);
    GREEN.set(0, 240);
    BLUE.set(0, 240);
  */
}
