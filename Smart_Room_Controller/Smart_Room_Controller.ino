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
#include <Wire.h> //not sure what this does
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SPI.h>
#include <mac.h>
#include "wemo.h" //in lesson code this was in parenthesis
#include <Encoder.h>
#include <hue.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET 4

int envRead = 0;
int lightControl = 1;
int homeControl = 2;
int autoWind = 3;
int knockState = 4;

int encButton = 23;
int blackButton = 22;
int textSize = 2;
int homeTextSize = 3;
int micro; //microphone for door knock automation

bool HueOn;
int HueColor;
int HueBright = 255; //declared immediatly to set initial brightness to 255
int brightPos; //mapped to encPos to control brightness of hue
int encPos; //posistion of encoder
int wemoPos; //posistion of Wemo switch mapped to encoder
int neoPos; //posistion on neopixel mapped to encoder
int hueOne = 1; //variables to choose hue bulb
int hueTwo = 2;
int hueThree = 3; 
int hueFour = 4;
int hueFive = 5;
int rainColor; //chooses color of rainbow from hue color array
bool encRead;

//maybe modify hue.h to add white to rainbow array*******

int homeState; //will cycle between functions of controller
int lastState; //will save homeState position

float temp;
float pressure;
float humidity;
float tempRange;
int neoRange;

const int alien = 0; //numbers relate to wemo outlets
const int whiteFan = 1;
const int tea = 2;
const int blueFan = 3;

bool buttonState; 

bool alienState; //will switch states of different wemo outlets between true and false
bool whiteFanState;
bool teaState;
bool blueFanState;

OneButton button1(blackButton, false, false); //may need to change false true since button code is borrowed from encoder lesson****
OneButton button2(encButton, false, false);
Adafruit_BME280 bme;
Adafruit_NeoPixel pixel(14, 17, NEO_GRB + NEO_KHZ800);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Encoder myEnc(2,3);

//wemo wemoClass;

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

pinMode (17, OUTPUT); //is this needed?
pinMode (A7, INPUT); //microphone

pixel.begin();
pixel.show();
pixel.setBrightness(68);

display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
display.display();
delay(2000); //why delay?
display.clearDisplay();
display.display();

}

void loop() { 

  button1.tick();
  button2.tick();

  digitalWrite(6,LOW);



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

  micro = analogRead(A7);
  if(micro > 1000){
    homeState = 4;
  }
  if(homeState == knockState){
    doorKnock();
//    delay(9000);
    homeState = lastState;
  }

  encRead = digitalRead(23);

  Serial.printf("Temperature: %.2f°F, Pressure: %.2f inHG, Humidity: %.2f Percent\n", temp, pressure, humidity);
  Serial.printf("Home State: %i Encoder: %i ButtonState: %i rainColor: %i micro: %i\n", homeState, encPos, buttonState, rainColor, micro);
  Serial.printf("Wemo Position: %i, Alien: %i, White Fan; %i, Tea: %i, Blue Fan: %i\n", wemoPos, alienState, whiteFanState, teaState, blueFanState);
}


void readEnvironment(){      //could add functionality to turn lights on and off as well as read environment 

    temp = ((bme.readTemperature()*9/5)+32); //reads temp and converts to Fehrenheit
    pressure = ((bme.readPressure()/100.0)/33.86); //reads pressure and converts from pascals to inHG
    humidity = bme.readHumidity(); //reads humidity percentage

    tempRange = temp;
    neoRange = map(tempRange, 65, 80, 0, 11);

    pixel.clear();
    pixel.fill(red, 0, neoRange);
    pixel.fill(blue, neoRange, 11);
    pixel.show();

    display.clearDisplay();
    display.setTextSize(textSize);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0,0);
//    display.printf("Temperature: %.2f %cF\n Pressure: %.2f inHG\n Humidity: %.2f%c\n", temp, (char)247, pressure, humidity, (char)37);
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
    
    HueOn = buttonState;
    HueColor = HueRainbow[rainColor];
    HueBright = brightPos;
    setHue(hueOne, HueOn, HueColor, HueBright);//may need to adjust lightNum arr
    setHue(hueTwo, HueOn, HueColor, HueBright);
    setHue(hueThree, HueOn, HueColor, HueBright);
    setHue(hueFour, HueOn, HueColor, HueBright);
    setHue(hueFive, HueOn, HueColor, HueBright);
    
    if(buttonState == true){//neoPixel will show same color as hue bulb without delay
      if(brightPos < 20){
        brightPos = 20; //will prevent neoPixels from shutting off completely when hue bulb at lowest brightness
      }
      pixel.clear();
      pixel.setBrightness(brightPos);//pixel will possibly show same brightness as hue bulb***
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

    display.clearDisplay();
    display.setTextSize(homeTextSize); //maybe increase text size****
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0,0);
    display.printf("Light\nControl");
    display.display();
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

  pixel.clear();
  pixel.setPixelColor(neoPos, blue);
  pixel.show();

  
  display.clearDisplay();
  display.setTextSize(homeTextSize); //maybe increase text size****
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.printf("Home\nControl");
  display.display();

  if(wemoPos == 0){ //all these if statements added to be able to individually control each wemo individually without interupting the fuction of others
    if(alienState == true){
//      wemoClass.switchON(alien);
    }
    else{
//      wemoClass.switchOFF(alien); 
    }
  }
  if(wemoPos == 1){
    if(whiteFanState == true){
 //     wemoClass.switchON(whiteFan);   
    }
    else{
//      wemoClass.switchOFF(whiteFan); 
    }
  }
  if(wemoPos == 2){
    if(teaState == true){
//      wemoClass.switchON(tea);       
    }
    else{
//      wemoClass.switchOFF(tea);  
    }
  }
  if(wemoPos == 3){
    if(blueFanState == true){
 //     wemoClass.switchON(blueFan);  
    }
    else{
 //     wemoClass.switchOFF(blueFan);
    }
  }
}

void sleepTimer(){
  //will use timer to slowly turn on and brigthen lights followed by red flashes to act as a wake up alarm. When encoder button is pressed lights and teapot will turn on. 
}

void lightsOn(){
      pixel.clear();
      pixel.setBrightness(brightPos);
      pixel.fill(rainbow[rainColor], 0, 2);
      pixel.fill(rainbow[rainColor], 4, 2);
      pixel.fill(rainbow[rainColor], 8, 2);
      pixel.show();
}

void doorKnock(){

    display.clearDisplay();
    display.setTextSize(textSize);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0,0);
    display.printf("Door Knock");    
    display.display();
  
    pixel.clear();
    pixel.fill(red, 0, 12);
    pixel.fill(white, 0, 12);
    pixel.show();
        
    HueOn = true; //paramaters set here for door knock function
    HueColor = HueRed;
    HueBright = 255;
  setHue(hueOne, HueOn, HueColor, HueBright);//maybe only use hue 
//  setHue(hueTwo, HueOn, HueColor, HueBright);
//  setHue(hueThree, HueOn, HueColor, HueBright);
  setHue(hueFour, HueOn, HueColor, HueBright);
//  setHue(hueFive, HueOn, HueColor, HueBright);
  delay(10000);
  
}

void click1() { //black button press will cycle between functions
  if(homeState >= 3){
    homeState = 0;
  }
  else {
    homeState++;
  }
  if(lastState >= 3){
    lastState = 0;
  }
  else {
    lastState++;
  }
  
}

void longPressStart1() { 


}

 
void click2() { //encoder button will cycle different states depeding on homeState
  if(homeState == 1) { //controls what encoder button will do if within lightControl funcionality
    if(buttonState == true) { 
      buttonState = false;
    }
    else {
      buttonState = true;
    } //maybe this button state can turn on or off extra LEDs
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
