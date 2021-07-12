void blinkLeft(bool frontLight) {
  if (frontLight == true) {
    for (int i = 0; i < 5; i++) {
      //////frame 1//////
      leds[0] = CRGB(128, 128, 128);
      leds[1] = CRGB(128, 128, 128);
      leds[2] = CRGB(128, 128, 128);

      leds[3] = CRGB(0, 0, 0);
      leds[4] = CRGB(0, 0, 0);

      leds[5] = CRGB(0, 0, 0);
      leds[6] = CRGB(0, 0, 0);

      leds[7] = CRGB(128, 0, 0);
      leds[8] = CRGB(128, 0, 0);
      leds[9] = CRGB(128, 0, 0);
      FastLED.show();
      delay(333);

      //////frame 2//////
      leds[0] = CRGB(128, 128, 128);
      leds[1] = CRGB(128, 128, 128);
      leds[2] = CRGB(128, 128, 128);

      leds[3] = CRGB(128, 165, 0);
      leds[4] = CRGB(0, 0, 0);

      leds[5] = CRGB(0, 0, 0);
      leds[6] = CRGB(128, 165, 0);

      leds[7] = CRGB(128, 0, 0);
      leds[8] = CRGB(128, 0, 0);
      leds[9] = CRGB(128, 0, 0);
      FastLED.show();
      delay(333);

      //////frame 3//////
      leds[0] = CRGB(128, 128, 128);
      leds[1] = CRGB(128, 128, 128);
      leds[2] = CRGB(128, 128, 128);

      leds[3] = CRGB(128, 165, 0);
      leds[4] = CRGB(128, 165, 0);

      leds[5] = CRGB(128, 165, 0);
      leds[6] = CRGB(128, 165, 0);

      leds[7] = CRGB(128, 0, 0);
      leds[8] = CRGB(128, 0, 0);
      leds[9] = CRGB(128, 0, 0);
      FastLED.show();
      delay(333);
    }
  }

  if (frontLight == false) {

    for (int i = 0; i < 5; i++) {
      //////frame 1//////
      leds[0] = CRGB(0, 0, 0);
      leds[1] = CRGB(0, 0, 0);
      leds[2] = CRGB(0, 0, 0);

      leds[3] = CRGB(0, 0, 0);
      leds[4] = CRGB(0, 0, 0);

      leds[5] = CRGB(0, 0, 0);
      leds[6] = CRGB(0, 0, 0);

      leds[7] = CRGB(128, 0, 0);
      leds[8] = CRGB(128, 0, 0);
      leds[9] = CRGB(128, 0, 0);
      FastLED.show();
      delay(333);

      //////frame 2//////
      leds[0] = CRGB(0, 0, 0);
      leds[1] = CRGB(0, 0, 0);
      leds[2] = CRGB(0, 0, 0);

      leds[3] = CRGB(128, 165, 0);
      leds[4] = CRGB(0, 0, 0);

      leds[5] = CRGB(0, 0, 0);
      leds[6] = CRGB(128, 165, 0);

      leds[7] = CRGB(128, 0, 0);
      leds[8] = CRGB(128, 0, 0);
      leds[9] = CRGB(128, 0, 0);
      FastLED.show();
      delay(333);

      //////frame 3//////
      leds[0] = CRGB(0, 0, 0);
      leds[1] = CRGB(0, 0, 0);
      leds[2] = CRGB(0, 0, 0);

      leds[3] = CRGB(128, 165, 0);
      leds[4] = CRGB(128, 165, 0);

      leds[5] = CRGB(128, 165, 0);
      leds[6] = CRGB(128, 165, 0);

      leds[7] = CRGB(128, 0, 0);
      leds[8] = CRGB(128, 0, 0);
      leds[9] = CRGB(128, 0, 0);
      FastLED.show();
      delay(333);
    }
  }
  endless = false;
  delay(500);
  endless = false;
}
