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
#include <wemo.h> //in lesson code this was in parenthesis
#include <Encoder.h>
#include <hue.h>
#include <Ethernet.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64 //changed for higher resolution
#define OLED_RESET 4

int envRead = 0;
int lightControl = 1;
int homeControl = 2;
int autoWind = 3;

int encButton = 23;
int blackButton = 22;
int textSize = 1;

bool HueOn;
int HueColor;
int HueBright = 255; //declared immediatly to set initial brightness to 255
int brightPos; //mapped to encPos to control brightness of hue
int encPos; //posistion of encoder
int lightNum [4]; //array for choosing hue light
int rainColor; //chooses color of rainbow from hue color array
//maybe modify hue.h to add white to rainbow array*******

int homeState; //will cycle between functions of controller

float temp;
float pressure;
float humidity;
float tempRange;
int neoRange;

bool buttonState;

OneButton button1(blackButton, false, false); //may need to change false true since button code is borrowed from encoder lesson
OneButton button2(encButton, false, false);
Adafruit_BME280 bme;
Adafruit_NeoPixel pixel(12, 17, NEO_GRB + NEO_KHZ800);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Encoder myEnc(3,2);

EthernetClient client;

void setup() {

Ethernet.begin(mac);

button1.attachClick(click1); //black button
button1.setClickTicks(250);
button1.setPressTicks(2000);

button2.attachClick(click2); //encoder button
button2.setClickTicks(250);
button2.setPressTicks(2000);

pinMode (17, OUTPUT); //is this needed?

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

  if(homeState == envRead) { //could add functionality to turn lights on and off as well as read environment 
    temp = ((bme.readTemperature()*9/5)+32); //reads temp and converts to Fehrenheit
    pressure = ((bme.readPressure()/100.0)/3386); //reads pressure and converts from pascals to inHG
    humidity = bme.readHumidity(); //reads humidity percentage

    tempRange = temp;
    neoRange = map(tempRange, 65, 80, 0, 11);

    pixel.clear();
    pixel.fill(red, 0, neoRange);
    pixel.fill(blue, neoRange, 12);
    pixel.show();

    display.clearDisplay();
    display.setTextSize(textSize);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0,0);
    display.printf("Temperature: %.2f %cF\n Pressure: %2f inHG\n Humidity: %.2f%c\n", temp, (char)247, pressure, humidity, (char)37);
    display.display();
  }

  if(homeState == lightControl) {

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
    setHue(lightNum[4], HueOn, HueColor, HueBright); //may need to adjust lightNum array or add several setHue

    if(buttonState == true){ //neoPixel will show same color as hue bulb without delay
      pixel.clear();
      pixel.setBrightness(brightPos);//pixel will possibly show same brightness as hue bulb***
      pixel.fill(rainbow[rainColor], 0, 12);
      pixel.show();
      }
      else {
      pixel.clear();
      pixel.fill(rainbow[rainColor], 0, 1);
      pixel.show();
      }
        
  }

}

void click1() { //black button press will cycle between functions
  if(homeState > 3){
    homeState = 0;
  }
  else {
    homeState++;
  }
}

void click2() { //encoder button will cycle button state
  if(buttonState == true) { 
    buttonState = false;
  }
  else {
    buttonState = true;
  } //maybe this button state can turn on or off extra LEDs
}

void longPressStart2() { //encoder long press will cycle bulb through colors. 
   if(rainColor > 6){
     rainColor = 0;
   }
   else{
      rainColor++;
  }
}


//could easily implement long press 1 to cycle language on display 
