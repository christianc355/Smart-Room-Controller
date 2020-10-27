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

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64 //changed for higher resolution
#define OLED_RESET 4

int envRead = 0;
int sceneControl = 1;
int homeControl = 2;
int autoWind = 3;

int encButton = 23;
int blackButton = 22;
int textSize = 1;

OneButton button1(blackButton, false, false); //may need to change false true since button code is borrowed from encoder lesson
Adafruit_BME280 bme;
Adafruit_NeoPixel pixel(12, 17, NEO_GRB + NEO_KHZ800);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);





int homeState; //will cycle between functions of controller

float temp;
float pressure;
float humidity;
float tempRange;
int neoRange;

void setup() {

button1.attachClick(click1); //black button
button1.setClickTicks(250);
button1.setPressTicks(2000);

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

  if(homeState == envRead) {
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

}

void click1() { //black button press will cycle between functions
  if(homeState > 3){
    homeState = 0;
  }
  else {
    homeState++;
  }
}
