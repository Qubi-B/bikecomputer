#include "Arduino.h"
#include <Arduino_FreeRTOS.h>
#include <LiquidCrystal.h>
#include <BigNumbers.h>
#include <FastLED.h>

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
const int lcdEPin  = 17; // LCD E         pin
const int lcdRSPin = 7;  // LCD RS        pin
const int lcdLed   = 11; // LCD backlight pin
#define SS_PIN 10
#define RST_PIN 9
///////////////////////////////////////////////

///////////////////////////////////////////////
//buttons and sensor pins
const int sensorPin     = 8;  //Wheel revolution sensor............................(nc)
const int leftBlinker   = 15; //Left  blinker button...............................(no)
const int rightBlinker  = 14; //Right blinker button...............................(no)
const int lightButton   = 16; //turns on/off front white light (defaults to on)....(no)
const int overlayButton = 9;  //switches the screen overlays.......................(no)
///////////////////////////////////////////////

///////////////////////////////////////////////
//constants
#define LED_PIN  6       //LED Strip Signal Connection 
#define NUM_LEDS 10      //Amount of LEDs in the strip
CRGB    leds[NUM_LEDS];  //Leds array to  control them
///////////////////////////////////////////////

///////////////////////////////////////////////
//system variables
int   currentSpeed = 0; //current speed
int   screenLook   = 0; //screen overlay id
float tripDst      = 0; //distance      from last power on
float vMax         = 0; //max speed     from last power on
float vAvr         = 0; //average speed from last power on

unsigned long TimerCount;
unsigned long previousTimerCount;
unsigned long LCDUpdateTime;

int pinState = LOW;
int oldpinState = LOW;
int sawPulse = 1;
///////////////////////////////////////////////

///////////////////////////////////////////////
//very important
LiquidCrystal lcd(lcdRSPin, lcdEPin, lcdD4Pin, lcdD5Pin, lcdD6Pin, lcdD7Pin); // construct LCD object
BigNumbers bigNum(&lcd); // construct BigNumbers object, passing to it the name of our LCD object
///////////////////////////////////////////////


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600); //begin serial communication at 115200 baud

  Serial.print(">Setting up the LCD...");
  lcd.begin(16, 2); //start the lcd
  lcd.clear();      //init  the lcd
  bigNum.begin();   //start bigNumbers
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

  Serial.print(">Checking the time...");
  TimerCount = millis();
  previousTimerCount = millis();
  Serial.println("Done!");

  Serial.print(">Initializing tasks...");
  xTaskCreate(lcd_speed, "Speed", 100, NULL, 1, &lcd_speed_handle); //create task for calculating data
  xTaskCreate(led_cntrl, "Light", 100, NULL, 1, &led_cntrl_handle); //create task for checking blink buttons and blinking
  Serial.println("Done!");

  Serial.println("Setup done! \n----------------- \n");
}

void loop() {
  // no code here
}

void lcd_speed(void *param) {
  (void) param;
  while (1) {
   //something to measure the speed and save it to currentSpeed variable
}

void led_cntrl (void *param) {
  (void) param;
  while (1) {
    //i will do that later cause its big code and i dont want to make the thing too long if i dont have speedometer
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
  bigNum.displayLargeInt(currentSpeed, 0, 2, false);
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
  lcd.print(tripDst);
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
  lcd.print("V/: ");
  lcd.print(int(vAvr));
  lcd.print("/h");
  lcd.setCursor(9, 1);
  lcd.print(tripDst);
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
  avrMs = (tripDst * 1000) / t;
  vAvr = avrMs * 3.6;
  Serial.print("v: ");
  Serial.println(currentSpeed);
  Serial.print("tripDst: ");
  Serial.println(tripDst);
  Serial.print("avrMs: ");
  Serial.println(avrMs);
  Serial.print("vAvr: ");
  Serial.println(vAvr);
  //  Serial.print("lastAvrs: ");
  //  Serial.println(lastAvrs);
  Serial.print("mode: ");
  Serial.println(screenLook);
  Serial.println("--------------------------");
}

//////////////////////////////////////
