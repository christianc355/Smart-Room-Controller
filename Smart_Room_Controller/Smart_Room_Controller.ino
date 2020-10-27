/*
 * Project:     Smart Room Controller
 * Description: A Home Controller Designed for the Hard of Hearing
 * Author:      Christian Chavez
 * Date:        10-27-2020
 */

#include <OneButton.h>

int envRead = 0;
int sceneControl = 1;
int homeControl = 2;
int autoWind = 3;

int encButton = 23;
int blackButton = 22;

OneButton button1(blackButton, false, false); //may need to change false true since button code is borrowed from encoder lesson

int homeState; //will cycle between functions of controller

void setup() {

button1.attachClick(click1); //black button
button1.setClickTicks(250);
button1.setPressTicks(2000);

}

void loop() {

  if(homeState == envRead) {
    
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
