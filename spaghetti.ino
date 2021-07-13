
//freertos
#include <Arduino_FreeRTOS.h>

void lcd_speed(void *param);
void led_cntrl(void *param);

TaskHandle_t lcd_speed_handle;
TaskHandle_t led_cntrl_handle;
//////////

//others
#include "Arduino.h"
#include <LiquidCrystal.h>
#include <BigNumbers.h>
#include <TimerOne.h>
#include <FastLED.h>

const int lcdD7Pin = 2; // LCD D7 pin
const int lcdD6Pin = 3; // LCD D6 pin
const int lcdD5Pin = 4; // LCD D5 pin
const int lcdD4Pin = 5; // LCD D4 pin
const int lcdEPin =  17; // LCD E Pin
const int lcdRSPin = 7; // LCD RS pin
const int lcdLed = 11; //lcd led anode pin

#define SS_PIN 10
#define RST_PIN 9

unsigned long currenttime = millis();


const int IRSensorPin = 8;
const int ledPin = 13;

int inputState;
int lastInputState = LOW;
long lastDebounceTime = 0;
long debounceDelay = 5;

long time;
long endTime;
long startTime;
int RPM = 0;
double trip = 0;
double kkbanspd = 0.00223;
float lnTime = 0;


////////leds

//// buttons
#define leftBlinker 15 //Left blinker button
#define rightBlinker 14 //Right blinker button
#define lightButton 16 //turns on/off front white light (defaults to on)
//----//

//// *FastLED magic*
#define LED_PIN 6 //LED Strip Signal Connection 
#define NUM_LEDS 10 //Total no of LEDs in the LED strip
CRGB leds[NUM_LEDS];
//----//

bool endless     =   false;
bool frontLight  =   true;
bool chosen      =   false;

/////////////////

//counter

const long interval     = 333;           // interval at which to blink (milliseconds)
long currentMillis      = 333;
long previousMillis     = 333;
int counter = 0;





LiquidCrystal lcd(lcdRSPin, lcdEPin, lcdD4Pin, lcdD5Pin, lcdD6Pin, lcdD7Pin); // construct LCD object
BigNumbers bigNum(&lcd); // construct BigNumbers object, passing to it the name of our LCD object


int currentSpeed = 0;
int currentRpm = 700;
int delayTime = 200;
int i = 0;

float tripDst = 0;
float vMax = 0;
float vAvr = 0;
float lastAvrs = 0;

const int overlayButton = 9;

int screenLook = 0;
int saveCounter = 0;

bool warned = false;

/////////////////////////////////////////////////////////////////////


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  //create tasks
  xTaskCreate(lcd_speed, "Speed", 100, NULL, 1, &lcd_speed_handle);
  xTaskCreate(led_cntrl, "Light", 100, NULL, 1, &led_cntrl_handle);


  lcd.begin(16, 2); // setup LCD rows and columns
  bigNum.begin(); // set up BigNumbers
  lcd.clear(); // clear display
  pinMode(IRSensorPin, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);
  pinMode(overlayButton, INPUT_PULLUP);

  endTime = 0;

  analogWrite(lcdLed, 675);

  Serial.println("kurde");

  ledSetup();
  welcomeScreen();
  //inf("Spr. poz. paliwa");
  //alert("Niski poz. bat.");
  //welcomeScreen2();
  Timer1.initialize(1000000);  // Set the timer to 60 rpm, 1,000,000 microseconds (1 second)
  Timer1.attachInterrupt(showSport);
}

void loop() {
  // No code here

}



void lcd_speed(void *param) {
  (void) param;
  while (1) {
    //speed code goes here

    //overlay selection
    if (digitalRead(overlayButton) == LOW) {
      while (digitalRead(overlayButton) == LOW) {}
      screenLook += 1;
      delay(100);
    }


    //display selected overlay
    switch (screenLook) {
      case 0:
        Timer1.attachInterrupt(showSport);  // Attach the service routine here
        break;
      case 1:
        Timer1.attachInterrupt(showDst);  // Attach the service routine here
        break;
      case 2:
        Timer1.attachInterrupt(showSpeed);  // Attach the service routine here
        break;
      case 3:
        Timer1.attachInterrupt(showStats);  // Attach the service routine here
        break;
      case 4:
        screenLook = 0;
        break;
    }

    //some weird shit
    time = millis();
    int currentSwitchState = digitalRead(IRSensorPin);

    if (currentSwitchState != lastInputState) {
      lastDebounceTime = millis();
    }

    if ((millis() - lastDebounceTime) > debounceDelay) {
      if (currentSwitchState != inputState) {
        inputState = currentSwitchState;
        if (inputState == LOW) {
          digitalWrite(ledPin, LOW);
          calculateRPM(); // Real RPM from sensor
        }
        else {
          digitalWrite(ledPin, HIGH);
        }
      }
    }

//    //highspeed warning (do wywalenia bo nie dziala)
//    if (currentSpeed > 45 && warned == false) {
//      //Timer1.stop();
//      alert("predkosc > 45");
//      //Timer1.resume();
//      warned = true;
//    }

    //more weird shit
    lastInputState = currentSwitchState;
    currentSpeed = (RPM * kkbanspd) * 60;
    tripDst = trip * kkbanspd;



  }
}



void led_cntrl (void *param) {
  (void) param;
  while (1) {
    //led code goes here

    chosen = false;

    //check if the front light should be on
    if (digitalRead(lightButton) == LOW) {
      if (frontLight == true) {
        frontLight = false;
        Serial.println("set front to false in void loop");
        delay(500);
      }
      else if (frontLight == false) {
        frontLight = true;
        Serial.println("set front to true in void loop");
        delay(500);
      }
    }


    //check if user wants to get right blinker on
    if (digitalRead(rightBlinker) == LOW) {
      chosen = true;
      Serial.println("right");
      blinkRight(frontLight);

    }

    //check if left blinker is needed
    if (digitalRead(leftBlinker) == LOW) {
      chosen = true;
      Serial.println("left");
      blinkLeft(frontLight);

    }

    //reset leds
    backToNormal(frontLight);


  }
}


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
  avrMs = (tripDst * 1000) / t;
  vAvr = avrMs * 3.6;
  //  Serial.print("v: ");
  //  Serial.println(currentSpeed);
  //  Serial.print("tripDst: ");
  //  Serial.println(tripDst);
  //  Serial.print("avrMs: ");
  //  Serial.println(avrMs);
  //  Serial.print("vAvr: ");
  //  Serial.println(vAvr);
  //  Serial.print("lastAvrs: ");
  //  Serial.println(lastAvrs);
  //  Serial.print("mode: ");
  //  Serial.println(screenLook);
  //  Serial.println("--------------------------");
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


//////////////////////////////DATA PRESENTATION//////////////////////////////

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

  //  lcd.setCursor(12, 1);
  //  lcd.print("km/h");

  //delay(500);
  RPM = 0;
  computeStats();
}

void showDst() {
  lcd.clear();
  bigNum.displayLargeInt(currentSpeed, 0, 2, false);
  lcd.setCursor(6, 0);
  lcd.print("km/h");

  lcd.setCursor(9, 1);
  lcd.print(tripDst);

  lcd.setCursor(13, 1);
  lcd.print("km");

  //delay(500);
  RPM = 0;
  computeStats();
}

void showSport() {

  lcd.clear();
  bigNum.displayLargeInt(currentSpeed, 0, 2, false);


  lcd.setCursor(7, 1);

  lcd.print("---------");
  lcd.setCursor(7, 1);

  for (int i = 0; i <= currentSpeed / 6; i++) {
    lcd.print("=");
  }

  lcd.setCursor(6, 0);
  lcd.print("km/h");

  //delay(500);
  RPM = 0;
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
  lcd.print("V/: ");
  lcd.print(int(vAvr));
  lcd.print("/h");
  lcd.setCursor(9, 1);
  lcd.print(tripDst);
  lcd.print("km");

}

//////////////////////////////WELCOME SCREEN//////////////////////////////
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

void welcomeScreen2() {
  lcd.clear();
  bigNum.displayLargeInt(88, 0, 2, false);
  lcd.setCursor(7, 1);
  lcd.print("=========");
  lcd.setCursor(7, 1);
  lcd.setCursor(6, 0);
  lcd.print("km/h   <*>");

  delay(1000);
}


//////////////////////////////GETTING DATA//////////////////////////////
/*
   variables:
   bool rpmTriggerState;
   bool speedTriggerState;
   bool rpmLastState;
   bool speedLastState;

*/

//////////////////////////////DEMO FOR TESTING//////////////////////////////

bool demoNums() {
  delay(250);
  currentSpeed += 1;
  currentRpm += 177;
  tripDst += 0.01;
  if (currentSpeed > 48 || currentRpm > 8500) {
    currentSpeed = 0;
    currentRpm = 700;
    delay(2000);
    return false;
  }
  return true;
}




void demo() {

  lcd.clear();
  bigNum.displayLargeInt(0, 0, 2, false);
  lcd.setCursor(6, 0);
  lcd.print("km/h");

  lcd.setCursor(9, 1);
  lcd.print(700);

  lcd.setCursor(13, 1);
  lcd.print("RPM");
  delay(3000);

  while (1) {
    lcd.clear();
    bigNum.displayLargeInt(currentSpeed, 0, 2, false);
    lcd.setCursor(6, 0);
    lcd.print("km/h");

    lcd.setCursor(9, 1);
    lcd.print(currentRpm);

    lcd.setCursor(13, 1);
    lcd.print("RPM");


    if (currentRpm >= 7000) {
      currentRpm = 700;
      currentSpeed = 0;
      delayTime = 200;
      delay(500);
      return;
    }
    else {
      currentRpm += 157;
      currentSpeed += 1;
      delayTime += 10;
    }
    delay(delayTime);
  }
}

void calculateRPM() {
  startTime = lastDebounceTime;
  lnTime = startTime - endTime;
  RPM = 60000 / (startTime - endTime);
  endTime = startTime;
  trip++;
}

void timerIsr()
{
  // Print RPM every second
  // RPM based on timer
  Serial.println("---------------");
  time = millis() / 1000;
  Serial.print(time);
  Serial.print(" RPM: ");
  Serial.println(RPM);
  Serial.print(trip);


  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Spd:");
  lcd.setCursor(5, 0);
  lcd.print((RPM * kkbanspd) * 60);
  lcd.setCursor(12, 0);
  lcd.print("Km/h");

  lcd.setCursor(0, 1);
  lcd.print("Trp:");
  lcd.setCursor(5, 1);
  lcd.print(trip * kkbanspd);
  lcd.setCursor(12, 1);
  lcd.print("Km");
  delay(500);
  RPM = 0;
}











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
  }
}













//blink to the right

void blinkRight(bool frontLight) {
  if (frontLight == true) {
      for (int i = 0; i < 5; i++) {
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
    }

  if (frontLight == false) {
      for (int i = 0; i < 5; i++) {
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
}

















//blink to the left

void blinkLeft(bool frontLight) {

  if (frontLight == true) {

    int done = 0;

    for (int i = 0; i < 5; i++) {
      if (currenttime - startTime >= 1000) {
        unsigned long startTime = millis();
      }
      unsigned long currentTime = millis();
      if (currenttime - startTime >= 999) {
        if (done == 2) {
          //frame 3
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
          done++;
        }
      }
      else if (currentTime - startTime >= 666) {
        if (done == 1) {
          //frame 2
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
          done++;
        }
      }
      else if (currentTime - startTime >= 333) {
        if (done == 0) {
          //frame 1
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
          done++;
        }
      }
    }
  }

  if (frontLight == false) {

    int done = 0;

    for (int i = 0; i < 5; i++) {

      if (currenttime - startTime >= 1000) {
        unsigned long startTime = millis();
      }

      unsigned long currentTime = millis();

      if (currenttime - startTime >= 999) {
        if (done == 2) {
          //frame 3
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
          done++;
        }
      }
      else if (currentTime - startTime >= 666) {
        if (done == 1) {
          //frame 2
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
          done++;
        }
      }
      else if (currentTime - startTime >= 333) {
        if (done == 0) {
          //frame 1
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
          done++;
        }
      }
    }
  }
  endless = false;
  delay(500);
  endless = false;
}


void ledSetup() {
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
  //  pinMode(endlessButton, INPUT_PULLUP);
  pinMode(leftBlinker, INPUT_PULLUP);
  pinMode(rightBlinker, INPUT_PULLUP);
  pinMode(lightButton, INPUT_PULLUP);

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
