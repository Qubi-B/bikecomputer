#include "Arduino.h"
#include <Arduino_FreeRTOS.h>
#include <LiquidCrystal.h>
#include <BigNumbers.h>
#include <FastLED.h>
#include <TimerOne.h>

void lcd_speed(void *param);
void led_cntrl(void *param);

TaskHandle_t lcd_speed_handle;
TaskHandle_t led_cntrl_handle;


///////////////////////////////////////////////
//lcd pins
const int lcdD7Pin = 2;  // LCD D7        pin
const int lcdD6Pin = 3;  // LCD D6        pin
const int lcdD5Pin = 4;  // LCD D5        pin
const int lcdD4Pin = 5;  // LCD D4        pin
const int lcdEPin  = 19; // LCD E         pin
const int lcdRSPin = 7;  // LCD RS        pin
const int lcdLed   = 11; // LCD backlight pin
#define SS_PIN 10
#define RST_PIN 9
///////////////////////////////////////////////

///////////////////////////////////////////////
//buttons and sensor pins
const int sensorPin     = 8;  //Wheel revolution sensor............................(nc)
const int leftBlinker   = 16; //Left  blinker button...............................(no)
const int rightBlinker  = 15; //Right blinker button...............................(no)
const int lightButton   = 18; //turns on/off front white light (defaults to on)....(no)
const int overlayButton = 17;  //switches the screen overlays.......................(no)
///////////////////////////////////////////////

///////////////////////////////////////////////
//constants
#define LED_PIN  6       //LED Strip Signal Connection 
#define NUM_LEDS 10      //Amount of LEDs in the strip
CRGB    leds[NUM_LEDS];  //Leds array to  control them
///////////////////////////////////////////////

///////////////////////////////////////////////
//system variables
float currentSpeed = 0; //current speed
int   screenLook   = 0; //screen overlay id
float vMax         = 0; //max speed     from last power on
float distance     = 0; //distance travelled from power on
unsigned long TimerCount;
unsigned long previousTimerCount;
//unsigned long LCDUpdateTime;
int pinState = LOW;
int oldpinState = LOW;
//int sawPulse = 1;
//bool chosen = false;
bool frontLight = false;
///////////////////////////////////////////////

///////////////////////////////////////////////
//very important
LiquidCrystal lcd(lcdRSPin, lcdEPin, lcdD4Pin, lcdD5Pin, lcdD6Pin, lcdD7Pin); // construct LCD object
BigNumbers bigNum(&lcd); // construct BigNumbers object, passing to it the name of our LCD object
///////////////////////////////////////////////


void setup() {
  // put your setup code here, to run once:
  Serial.begin(2000000); //begin serial communication at 115200 baud

  Serial.print(">Setting up the LCD...");
  lcd.begin(16, 2); //start the lcd
  lcd.clear();      //init  the lcd
  bigNum.begin();   //start bigNumbers
  Serial.println("Done!");

  Serial.print(">Configuring leds...");
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
  Serial.println("Done!");


  Serial.print(">Declaring pins...");
  pinMode(sensorPin    , INPUT_PULLUP); //sensor pin           as input (no so pullup)
  pinMode(overlayButton, INPUT_PULLUP); //overlay       button as input
  pinMode(leftBlinker  , INPUT_PULLUP); //left  blinker button as input
  pinMode(rightBlinker , INPUT_PULLUP); //right blinker button as input
  pinMode(lightButton  , INPUT_PULLUP); //front light   button as input
  Serial.println("Done!");

  analogWrite(lcdLed, 675); //turn on lcd backlight at ~3.3v (pwm)
  Serial.println(">Turned on lcd backlight");

  Serial.print(">Welcoming the user...");
  ledSetup();
  welcomeScreen();
  Serial.println("Done!");

  Serial.print(">Initializing tasks...");
  xTaskCreate(lcd_speed, "Speed", 100, NULL, 1, &lcd_speed_handle); //create task for calculating data
  xTaskCreate(led_cntrl, "Light", 100, NULL, 1, &led_cntrl_handle); //create task for checking blink buttons and blinking
  Serial.println("Done!");

  Serial.print(">Starting the timer...");
  Timer1.initialize(1000000);         //start the timer for 1 sec
  Timer1.attachInterrupt(showSport);  //attach showing data on interrupt
  Serial.println("Done!");

  Serial.println("Setup done! \n----------------- \n");
}

void loop() {
  // no code here
}

void lcd_speed(void *param) {
  (void) param;
  unsigned long currentTime;
  unsigned long lastTime;
  unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
  unsigned long debounceDelay = 50;    // the debounce time "DarwinWasWrong #4059 — Dziś o 04:51 @h-bi_h I calculate that 100 ms is 144k/h if the sensor hits at that rate"
  float currentspeed;
  while (1) {
    int reading = digitalRead(sensorPin);
    if (reading != oldpinState) {
      lastDebounceTime = millis();
    }
    if ((millis() - lastDebounceTime) > debounceDelay) {
      if (reading != pinState) {
        pinState = reading;
        if (pinState == HIGH) {
          currentTime = millis();
          distance += 4.31;
          currentSpeed = (4310 / (currentTime - lastTime)) * 3.6;
          lastTime = currentTime;

          if (currentSpeed > vMax)vMax = currentSpeed;

        }
      }
    }

    oldpinState = reading;
  }
}


void led_cntrl(void *param) {
  (void) param;
  while (1) {


//    //overlay selection
//    if (digitalRead(overlayButton) == LOW) {
//      while (digitalRead(overlayButton) == LOW) {}
//      screenLook += 1;
//    }
//    //display selected overlay
//    switch (screenLook) {
//      case 0:
//        Timer1.detachInterrupt();
//        Timer1.attachInterrupt(showSport);  // Attach the service routine here
//        break;
//      case 1:
//        Timer1.detachInterrupt();
//        Timer1.attachInterrupt(showDst);  // Attach the service routine here
//        break;
//      case 2:
//        Timer1.detachInterrupt();
//        Timer1.attachInterrupt(showSpeed);  // Attach the service routine here
//        break;
//      case 3:
//        Timer1.detachInterrupt();
//        Timer1.attachInterrupt(showStats);  // Attach the service routine here
//        break;
//      case 4:
//        screenLook = 0;
//        break;
//    }
    
    bool back  = false;

    //check if the front light should be on
    if (digitalRead(lightButton) == LOW) {                          //if(digitalRead(lightButton)){
      if (frontLight == true) {                                     //  fronLight = true;
        frontLight = false;                                         //}
        Serial.println("set front to false in void loop");          //else{
        delay(500);                                                 //  frontLight = false;
        back = true;
      }                                                             //}
      else if (frontLight == false) {
        frontLight = true;
        Serial.println("set front to true in void loop");
        delay(500);
        back = true;
      }
    }
    //check if user wants to get right blinker on
    if (digitalRead(rightBlinker) == LOW) {
      blinkRight(frontLight);
      //delay(1000);
      back = true;
    }
    //check if left blinker is needed
    if (digitalRead(leftBlinker) == LOW) {
      blinkLeft(frontLight);
      //delay(1000);
      back = true;
    }
    if (back == true){
      backToNormal(frontLight);
      delay(500);
      back = false;
    }
    delay(random(1, 3));
  }
}


//////////////////////////////////////
// LCD Utility
//////////////////////////////////////
void welcomeScreen() {
  static int del = 150;
  lcd.clear();
  lcd.print("     Witaj!");
  lcd.setCursor(0, 1);
  lcd.print("                ");
  delay(del);
  lcd.setCursor(0, 1);
  lcd.print("       --       ");
  delay(del);
  lcd.setCursor(0, 1);
  lcd.print("      ----      ");
  delay(del);
  lcd.setCursor(0, 1);
  lcd.print("     ------     ");
  delay(del);
  lcd.setCursor(0, 1);
  lcd.print("    --------    ");
  delay(del);
  lcd.setCursor(0, 1);
  lcd.print("   ----------   ");
  delay(del);
  lcd.setCursor(0, 1);
  lcd.print("  ------------  ");
  delay(del);
  lcd.setCursor(0, 1);
  lcd.print(" -------------- ");
  delay(del);
  lcd.setCursor(0, 1);
  lcd.print("----------------");
  delay(1000);
}

void inf(String text) {
  lcd.clear();
  lcd.home();
  lcd.print("(i) info");
  lcd.setCursor(0, 1);
  lcd.print(text);

  delay(2000);
}

void alert(String text) {
  lcd.clear();
  lcd.home();
  lcd.print("(!)");
  lcd.setCursor(0, 1);
  lcd.print(text);

  delay(2000);
}
//////////////////////////////////////

//////////////////////////////////////
//Displaying information
//////////////////////////////////////
void showSport() {
  lcd.clear();
  bigNum.displayLargeInt(int(currentSpeed), 0, 2, false);
  lcd.setCursor(7, 1);
  lcd.print("---------");
  lcd.setCursor(7, 1);
  for (int i = 0; i <= currentSpeed / 6; i++) {
    lcd.print("=");
  }
  lcd.setCursor(6, 0);
  lcd.print("km/h");
  computeStats();
}

void showDst() {
  lcd.clear();
  bigNum.displayLargeInt(currentSpeed, 0, 2, false);
  lcd.setCursor(6, 0);
  lcd.print("km/h");
  lcd.setCursor(9, 1);
  lcd.print(distance / 1000);
  lcd.setCursor(13, 1);
  lcd.print("km");
  computeStats();
}

void showSpeed() {
  lcd.clear();
  if (currentSpeed < 10) {
    bigNum.displayLargeInt(currentSpeed, 3, 2, false);
  }
  else if (currentSpeed >= 10 && currentSpeed < 20) {
    bigNum.displayLargeInt(currentSpeed, 4 , 2, false);
  }
  else {
    bigNum.displayLargeInt(currentSpeed, 5 , 2, false);
  }
  computeStats();
}

void showStats() {
  bool returned = false;
  lcd.clear();
  lcd.print("V^: ");
  lcd.print(int(vMax));
  lcd.print("/h");
  lcd.setCursor(12, 0);
  lcd.print("Dst:");


  lcd.setCursor(0, 1);
  //lcd.print("V/: ");
  //lcd.print(int(vAvr));
  lcd.print("/h");
  lcd.setCursor(9, 1);
  lcd.print(distance / 1000);
  lcd.print("km");
}
//////////////////////////////////////

//////////////////////////////////////
//Computing & calculations
//////////////////////////////////////
void computeStats() {
  static float avrMs;
  double t = (millis() / 1000);
  //Serial.print(millis());
  //Serial.print(",");
  //Serial.println(t);

  if (currentSpeed > vMax) {
    vMax = currentSpeed;
  }
  //  if (currentSpeed > 0) {
  //    vAvr = (vAvr + currentSpeed) / 2;
  //  }
  //  avrMs = (tripDst * 1000) / t;
  //  vAvr = avrMs * 3.6;
  //  Serial.print("v: ");
  //  Serial.println(currentSpeed);
  //  Serial.print("tripDst: ");
  //  Serial.println(tripDst);
  //  Serial.print("avrMs: ");
  //  Serial.println(avrMs);
  //  Serial.print("vAvr: ");
  //  Serial.println(vAvr);
  //  //  Serial.print("lastAvrs: ");
  //  //  Serial.println(lastAvrs);
  //  Serial.print("mode: ");
  //  Serial.println(screenLook);
  //  Serial.println("--------------------------");
}

//////////////////////////////////////


//////////////////////////////LED FUNCTIONS

void backToNormal(bool lightOn) {
  if (lightOn) {
    leds[2] = CRGB(128, 128, 128);
    leds[7] = CRGB(128, 0, 0);

    leds[1] = CRGB(128, 128, 128);
    leds[3] = CRGB(128, 128, 128);
    leds[6] = CRGB(128, 0, 0);
    leds[8] = CRGB(128, 0, 0);


    leds[0] = CRGB(128, 128, 128);
    leds[4] = CRGB(128, 128, 128);
    leds[5] = CRGB(128, 0, 0);
    leds[9] = CRGB(128, 0, 0);
    FastLED.show();
    delay(random(1,3));
  }
  else {
    leds[2] = CRGB(0, 0, 0);
    leds[7] = CRGB(128, 0, 0);

    leds[1] = CRGB(0, 0, 0);
    leds[3] = CRGB(0, 0, 0);
    leds[6] = CRGB(128, 0, 0);
    leds[8] = CRGB(128, 0, 0);


    leds[0] = CRGB(0, 0, 0);
    leds[4] = CRGB(0, 0, 0);
    leds[5] = CRGB(128, 0, 0);
    leds[9] = CRGB(128, 0, 0);
    FastLED.show();
    delay(random(1,3));
  }
}













//blink to the right

void blinkRight(bool frontLight) {
  if (frontLight == true) {
    //////frame 1//////
    leds[0] = CRGB(0, 0, 0);
    leds[1] = CRGB(0, 0, 0);

    leds[2] = CRGB(128, 128, 128);
    leds[3] = CRGB(128, 128, 128);
    leds[4] = CRGB(128, 128, 128);

    leds[5] = CRGB(128, 0, 0);
    leds[6] = CRGB(128, 0, 0);
    leds[7] = CRGB(128, 0, 0);

    leds[8] = CRGB(0, 0, 0);
    leds[9] = CRGB(0, 0, 0);
    FastLED.show();
    delay(333);

    //////frame 2//////
    leds[0] = CRGB(0, 0, 0);
    leds[1] = CRGB(128, 165, 0);

    leds[2] = CRGB(128, 128, 128);
    leds[3] = CRGB(128, 128, 128);
    leds[4] = CRGB(128, 128, 128);

    leds[5] = CRGB(128, 0, 0);
    leds[6] = CRGB(128, 0, 0);
    leds[7] = CRGB(128, 0, 0);

    leds[8] = CRGB(128, 165, 0);
    leds[9] = CRGB(0, 0, 0);
    FastLED.show();
    delay(333);

    //////frame 3//////
    leds[0] = CRGB(128, 165, 0);
    leds[1] = CRGB(128, 165, 0);

    leds[2] = CRGB(128, 128, 128);
    leds[3] = CRGB(128, 128, 128);
    leds[4] = CRGB(128, 128, 128);

    leds[5] = CRGB(128, 0, 0);
    leds[6] = CRGB(128, 0, 0);
    leds[7] = CRGB(128, 0, 0);

    leds[8] = CRGB(128, 165, 0);
    leds[9] = CRGB(128, 165, 0);
    FastLED.show();
    delay(333);
  }

  if (frontLight == false) {
    //////frame 1//////
    leds[0] = CRGB(0, 0, 0);
    leds[1] = CRGB(0, 0, 0);

    leds[2] = CRGB(0, 0, 0);
    leds[3] = CRGB(0, 0, 0);
    leds[4] = CRGB(0, 0, 0);

    leds[5] = CRGB(128, 0, 0);
    leds[6] = CRGB(128, 0, 0);
    leds[7] = CRGB(128, 0, 0);

    leds[8] = CRGB(0, 0, 0);
    leds[9] = CRGB(0, 0, 0);
    FastLED.show();
    delay(333);

    //////frame 2//////
    leds[0] = CRGB(0, 0, 0);
    leds[1] = CRGB(128, 165, 0);

    leds[2] = CRGB(0, 0, 0);
    leds[3] = CRGB(0, 0, 0);
    leds[4] = CRGB(0, 0, 0);

    leds[5] = CRGB(128, 0, 0);
    leds[6] = CRGB(128, 0, 0);
    leds[7] = CRGB(128, 0, 0);

    leds[8] = CRGB(128, 165, 0);
    leds[9] = CRGB(0, 0, 0);
    FastLED.show();
    delay(333);

    //////frame 3//////
    leds[0] = CRGB(128, 165, 0);
    leds[1] = CRGB(128, 165, 0);

    leds[2] = CRGB(0, 0, 0);
    leds[3] = CRGB(0, 0, 0);
    leds[4] = CRGB(0, 0, 0);

    leds[5] = CRGB(128, 0, 0);
    leds[6] = CRGB(128, 0, 0);
    leds[7] = CRGB(128, 0, 0);

    leds[8] = CRGB(128, 165, 0);
    leds[9] = CRGB(128, 165, 0);
    FastLED.show();
    delay(333);
  }
}

















//blink to the left

void blinkLeft(bool frontLight) {
  if (frontLight == true) {
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

  if (frontLight == false) {
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

void ledSetup() {

  for (int j = 0; j < 9; j++) {
    leds[j] = CRGB(0, 0, 0);
    FastLED.show();
  }

  delay(100);

  leds[2] = CRGB(128, 128, 128);
  leds[7] = CRGB(128, 0, 0);
  FastLED.show();
  delay(300);

  leds[1] = CRGB(128, 128, 128);
  leds[3] = CRGB(128, 128, 128);
  leds[6] = CRGB(128, 0, 0);
  leds[8] = CRGB(128, 0, 0);
  FastLED.show();
  delay(300);

  leds[0] = CRGB(128, 128, 128);
  leds[4] = CRGB(128, 128, 128);
  leds[5] = CRGB(128, 0, 0);
  leds[9] = CRGB(128, 0, 0);
  FastLED.show();
}
