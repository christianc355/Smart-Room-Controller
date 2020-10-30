/*
 * Project:     Smart Room Controller
 * Description: A Home Controller Designed for the Hard of Hearing
 * Author:      Christian Chavez
 * Date:        10-27-2020
 */

#include <OneButton.h>
#include <Adafruit_BME280.h>
#include <Adafruit_NeoPixel.h>
#include <colors.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SPI.h>
#include <mac.h>
#include "wemo.h"
#include <Encoder.h>
#include <hue2.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET 4

int envRead = 0;
int lightControl = 1;
int homeControl = 2;
int autoWind = 3;
int lightsOn = 4;
int knockState = 5;


int encButton = 23;
int blackButton = 22;
int textSize = 2;
int homeTextSize = 3;
int micro; //MAX4466 microphone

unsigned int lastTime = 0;
unsigned int wakeTimer = 5000;
unsigned int currentTime;
unsigned int lastSecond;
unsigned int lastMinute;
unsigned int knockSecond;

bool HueOn;
bool HueSat;
int HueColor;
int HueBright = 255;
int brightPos; //mapped to encPos to control brightness of hue
int encPos; //posistion of encoder
int wemoPos; //posistion of Wemo switch mapped to encoder
int neoPos; //posistion on neopixel mapped to encoder
int hueOne = 1; //variables to choose hue bulb
int hueTwo = 2;
int hueThree = 3; 
int hueFour = 4;
int hueFive = 5;
int rainColor; //chooses color of rainbow from color array
bool encRead;

int homeState; //will cycle between functions of controller
int lastState; //will save homeState position

float temp;
float pressure;
float humidity;
float tempRange;
int neoRange;
int displayBright;

const int alien = 0; //following numbers relate to wemo outlets
const int whiteFan = 1;
const int tea = 2;
const int blueFan = 3;

bool buttonState; 
bool lightState;
bool timerState; //state to determine whether timer should be on or off
bool timerOn;

bool alienState; //following will switch states of different wemo outlets between true and false
bool whiteFanState;
bool teaState;
bool blueFanState;

OneButton button1(blackButton, false, false);
OneButton button2(encButton, false, false);
Adafruit_BME280 bme;
Adafruit_NeoPixel pixel(14, 17, NEO_GRB + NEO_KHZ800);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Encoder myEnc(2,3);

wemo wemoClass;

EthernetClient client;

void setup() {
  
Serial.begin(9600);
  
Ethernet.begin(mac);

bme.begin(0x76);

button1.attachClick(click1); //black button
button1.attachLongPressStart(longPressStart1);
button1.setClickTicks(250);
button1.setPressTicks(1000);

button2.attachClick(click2); //encoder button
button2.attachLongPressStart(longPressStart2);
button2.setClickTicks(250);
button2.setPressTicks(500);

pinMode (17, OUTPUT);
pinMode (A7, INPUT); //microphone

pixel.begin();
pixel.show();
pixel.setBrightness(68);

display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
display.display();
delay(2000);
display.clearDisplay();
display.display();

}

void loop() { 

  button1.tick();
  button2.tick();

  if(homeState == envRead) {
     readEnvironment();
    }

  if(homeState == lightControl) {
    controlLights();
   }
  
  if(homeState == homeControl){
    controlHome();
  }

  if(homeState == autoWind){
    sleepTimer();
  }

  if(homeState == lightsOn){
    whiteLights();
  }

  micro = analogRead(A7);
  currentTime = millis();
  if(micro > 1000){
    homeState = 5;
  }
  if(homeState == knockState){
    doorKnock();
    if((currentTime-lastSecond)>15000) {
    homeState = lastState;
    lastSecond = millis();
    }
  }
  encRead = digitalRead(23);
}


void readEnvironment(){      //could add functionality to turn lights on and off as well as read environment 

    temp = ((bme.readTemperature()*9/5)+32); //reads temp and converts to Fehrenheit
    pressure = ((bme.readPressure()/100.0)/33.86); //reads pressure and converts from pascals to inHG
    humidity = bme.readHumidity(); //reads humidity percentage

    tempRange = temp;
    neoRange = map(tempRange, 60, 80, 0, 11);

    pixel.clear();
    pixel.fill(red, 0, neoRange);
    pixel.fill(blue, neoRange, 11);
    pixel.show();

    display.clearDisplay();
    display.setTextSize(textSize);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0,0);
    display.printf("%.2f %cF\n%.2f inHG\n%.2f%c\n", temp, (char)247, pressure, humidity, (char)37);    
    display.display();
}

void controlLights(){
  
  encPos = myEnc.read();
    if(encPos > 95){
      encPos = 95;
    }
    if(encPos < 0){
      encPos = 0;
    }
    
    myEnc.write(encPos);
    brightPos = map(encPos, 0, 95, 0, 255);
    
    if(buttonState == true){//neoPixel will show same color as hue bulb without delay
      if(brightPos < 20){
        brightPos = 20; //will prevent neoPixels from shutting off completely when hue bulb at lowest brightness
      }
      pixel.clear();
      pixel.setBrightness(brightPos);
      pixel.fill(rainbow[rainColor], 0, 12);
      pixel.show();
      }
      else {
        if(brightPos < 20){
          brightPos = 20;
        }
      pixel.clear();
      pixel.setBrightness(brightPos);
      pixel.fill(rainbow[rainColor], 0, 2);
      pixel.fill(rainbow[rainColor], 4, 2);
      pixel.fill(rainbow[rainColor], 8, 2);
      pixel.show();
      } 
    
    HueOn = buttonState;
    HueColor = HueRainbow[rainColor];
    HueBright = brightPos;
    setHue(hueOne, HueOn, HueColor, HueBright, 255);//may need to adjust lightNum arr
    setHue(hueTwo, HueOn, HueColor, HueBright, 255);
    setHue(hueThree, HueOn, HueColor, HueBright, 255);
    setHue(hueFour, HueOn, HueColor, HueBright, 255);
    setHue(hueFive, HueOn, HueColor, HueBright, 255);

    displayBright = map(HueBright, 0, 255, 0, 100); //will display brightness as a percentage on display

    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0,0);
    if(buttonState == true){
    display.clearDisplay();
    display.printf(" Hue Light\nLumen:%i%c\nHue: ON", displayBright,(char)37);
    display.display();      
    }
    else{
    display.clearDisplay();
    display.printf("Hue Light\nLumen:%i%c\nHue: OFF", displayBright,(char)37);
    display.display();
    }
}

void controlHome(){
  
  encPos = myEnc.read();
  if(encPos > 95){
    encPos = 95;
  }
  if(encPos < 0){
    encPos = 0;
  }

  myEnc.write(encPos);
  neoPos = map(encPos, 0, 95, 0, 11);//map neopixel to encoder
  wemoPos = map(encPos, 0, 95, 0, 3);//map wemo number to encoder

  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);

  pixel.setBrightness(20);

  if(wemoPos == 0){ //all these if statements added to be able to individually control each wemo individually without interupting the fuction of others
    display.clearDisplay();
    display.printf("Alien<\nWhite Fan\nTea\nBlue Fan\n");
    display.display();
    if(alienState == true){
         wemoClass.switchON(alien);
        pixel.fill(green, 0, 3);
        pixel.show();
    }
    else{
        wemoClass.switchOFF(alien);
        pixel.fill(black, 0, 3);
        pixel.show();
    }
  }
  if(wemoPos == 1){
    display.clearDisplay();
    display.printf("Alien\nWhite Fan<\nTea\nBlue Fan\n");
    display.display();
    if(whiteFanState == true){
        wemoClass.switchON(whiteFan);
        pixel.fill(gray, 3, 3);
        pixel.show();   
    }
    else{
        wemoClass.switchOFF(whiteFan); 
        pixel.fill(black, 3, 3);
        pixel.show();
    }
  }
  if(wemoPos == 2){
    display.clearDisplay();
    display.printf("Alien\nWhite Fan\nTea<\nBlue Fan\n");
    display.display();
    if(teaState == true){
        wemoClass.switchON(tea);
        pixel.fill(teal, 6, 3);
        pixel.show();         
    }
    else{
        wemoClass.switchOFF(tea);
        pixel.fill(black, 6, 3);
        pixel.show();    
    }
  }
  if(wemoPos == 3){
    display.clearDisplay();
    display.printf("Alien\nWhite Fan\nTea\nBlue Fan<\n");
    display.display();
    if(blueFanState == true){
        wemoClass.switchON(blueFan);
        pixel.fill(blue, 9, 3);
        pixel.show();    
    }
    else{
        wemoClass.switchOFF(blueFan);
        pixel.fill(black, 9, 3);
        pixel.show();   
    }
  }
}

void sleepTimer(){ 

    currentTime = millis();
    
    if(timerOn == true){
    display.clearDisplay();
    display.setTextSize(4);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0,0);
    display.printf("Alarm\n ON+");
    display.display();
      if(millis()-lastTime >= wakeTimer){
       if((currentTime-lastSecond)>500) {
        HueColor = HueOrange;
        HueBright = 255;
        HueOn = true;
        setHue(hueOne, HueOn, HueColor, HueBright, 255);
        setHue(hueTwo, HueOn, HueColor, HueBright, 255);
        setHue(hueThree, HueOn, HueColor, HueBright, 255);
        setHue(hueFour, HueOn, HueColor, HueBright, 255);
        setHue(hueFive, HueOn, HueColor, HueBright, 255);
        pixel.clear();
        pixel.fill(orange, 0, 12);
        pixel.show();
        lastSecond = millis();
       }
         if((currentTime-lastMinute)>1000) {
          HueColor = HueBlue;
          HueBright = 255;
          HueOn = true;
          setHue(hueOne, HueOn, HueColor, HueBright, 255);
          setHue(hueTwo, HueOn, HueColor, HueBright, 255);
          setHue(hueThree, HueOn, HueColor, HueBright, 255);
          setHue(hueFour, HueOn, HueColor, HueBright, 255);
          setHue(hueFive, HueOn, HueColor, HueBright, 255);
          pixel.clear();
          pixel.fill(blue, 0, 12);
          pixel.show();
          lastMinute = millis();
          }
       }
      }
      else{
      display.clearDisplay();
      display.setTextSize(4);
      display.setTextColor(SSD1306_WHITE);
      display.setCursor(0,0);
      display.printf("Alarm\n OFF-");
      display.display();
      pixel.clear();
      pixel.setBrightness(20);
      pixel.fill(turquoise, 0, 12);
      pixel.show();
      }
}

void whiteLights(){
  encPos = myEnc.read();
    if(encPos > 95){
      encPos = 95;
    }
    if(encPos < 0){
      encPos = 0;
    }
    
    myEnc.write(encPos);
    brightPos = map(encPos, 0, 95, 0, 255);
    
    if(buttonState == true){//neoPixel will show same color as hue bulb without delay
      if(brightPos < 20){
        brightPos = 20; //will prevent neoPixels from shutting off completely when hue bulb at lowest brightness
      }
      pixel.clear();
      pixel.setBrightness(brightPos);
      pixel.fill(white, 0, 12);
      pixel.show();
      }
      else {
        if(brightPos < 20){
          brightPos = 20;
        }
      pixel.clear();
      pixel.setBrightness(brightPos);
      pixel.clear();
      pixel.setBrightness(brightPos);
      pixel.fill(white, 0, 2);
      pixel.fill(white, 4, 2);
      pixel.fill(white, 8, 2);
      pixel.show();
      pixel.show();
      } 
    
    HueOn = buttonState;
    HueColor = HueRainbow[rainColor];
    HueBright = brightPos;
    setHue(hueOne, HueOn, HueColor, HueBright, 0);
    setHue(hueTwo, HueOn, HueColor, HueBright, 0);
    setHue(hueThree, HueOn, HueColor, HueBright, 0);
    setHue(hueFour, HueOn, HueColor, HueBright, 0);
    setHue(hueFive, HueOn, HueColor, HueBright, 0);

    displayBright = map(HueBright, 0, 255, 0, 100); //will display brightness as a percentage on display

    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0,0);
    if(buttonState == true){
    display.clearDisplay();
    display.printf(" -Lights-\nLumen:%i%c\nHue: ON", displayBright,(char)37);
    display.display();      
    }
    else{
    display.clearDisplay();
    display.printf(" -Lights-\nLumen:%i%c\nHue: OFF", displayBright,(char)37);
    display.display();
    }
}


void doorKnock(){

    display.clearDisplay();
    display.setTextSize(4);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(15,0);
    display.printf("Door\nKnock");    
    display.display();
  
    pixel.clear();
    pixel.fill(red, 0, 12);
    pixel.fill(white, 0, 12);
    pixel.show();

    HueOn = true; //paramaters set here for door knock function

    if((currentTime-knockSecond)>1000) {
      HueColor = HueRed;
      HueBright = 255;
      setHue(hueOne, HueOn, HueColor, HueBright, 255);
      setHue(hueTwo, HueOn, HueColor, HueBright, 255);
      setHue(hueThree, HueOn, HueColor, HueBright, 255);
      setHue(hueFour, HueOn, HueColor, HueBright, 255);
      setHue(hueFive, HueOn, HueColor, HueBright, 255);     
      knockSecond = millis();
    }
    if((currentTime-knockSecond)>3000) {
      HueColor = HueBlue;
      HueBright = 255;
      setHue(hueOne, HueOn, HueColor, HueBright, 255);
      setHue(hueOne, HueOn, HueColor, HueBright, 255);
      setHue(hueTwo, HueOn, HueColor, HueBright, 255);
      setHue(hueThree, HueOn, HueColor, HueBright, 255);
      setHue(hueFour, HueOn, HueColor, HueBright, 255);
      setHue(hueFive, HueOn, HueColor, HueBright, 255); 
      knockSecond = millis();
    }       
}

void click1() { //black button press will cycle between functions
  if(homeState >= 4){
    homeState = 0;
  }
  else {
    homeState++;
  }
  if(lastState >= 4){
    lastState = 0;
  }
  else {
    lastState++;
  }
}

void longPressStart1() { /////not used***********
    if(lightState == true) { 
      lightState = false;
    }
    else {
      lightState = true;
    }

}

 
void click2() { //encoder button will cycle different states depeding on homeState
  if(homeState == 1) { //controls what encoder button will do if within lightControl funcionality
    if(buttonState == true) { 
      buttonState = false;
    }
    else {
      buttonState = true;
    }
  }

  if(homeState == lightsOn) { //controls what encoder button will do if within lightControl funcionality
    if(buttonState == true) { 
      buttonState = false;
    }
    else {
      buttonState = true;
    }
  }

  if(homeState == homeControl){ //controls what button will do within homeControl
    if(wemoPos == 0){
      if(alienState == true) { 
         alienState = false;
      }
      else {
        alienState = true;
      }
    }
     
    if(wemoPos == 1){    
     if(whiteFanState == true) { 
        whiteFanState = false;
     }
     else {
        whiteFanState = true;
     }
    }
    
    if(wemoPos == 2){     
     if(teaState == true) { 
       teaState = false;
    }
      else {
        teaState = true;
     }
    }
    
    if(wemoPos == 3){
     if(blueFanState == true) { 
       blueFanState = false;
    }
     else {
       blueFanState = true;
     }
    } 
  }

  if(homeState == autoWind){
    if(timerOn == true) { 
      timerOn = false;
    }
    else {
      timerOn = true;
      lastTime = millis();
    }
  }
}
  
void longPressStart2() { 

   if(homeState == lightControl){ //encoder long press will cycle through colors. 
     if(rainColor >= 6){
       rainColor = 0;
     }
     else{
        rainColor++;
    }
   }
}
