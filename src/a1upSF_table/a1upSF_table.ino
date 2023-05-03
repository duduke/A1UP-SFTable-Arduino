/*
    Street Fighter Table a1up
*/

#include <SoftwareSerial.h>
#include <Joystick.h>
#include <HID-Project.h>

Joystick_ Joystick[2] = 
{ Joystick_(0x20, JOYSTICK_TYPE_GAMEPAD, 7, 1, 0,0,0,0,0,0,0,0,0,0,0),
  Joystick_(0x30, JOYSTICK_TYPE_GAMEPAD, 7, 1, 0,0,0,0,0,0,0,0,0,0,0)};

SoftwareSerial portOne(10,11);

const byte volumeUp = B00000010;

const byte sendSignal[3] = {0xA6, 0x01, 0x00};

//                             Down   Left   Up     Right  VUP    Start  Frwrd  Strong Fierce Short  Jab    RndHose Vdown
byte fixers[2][13];

int timers[2][13];

unsigned long exit_time = 0;
bool check_exit = false;

void setup() {
    pinMode(10,INPUT);
    pinMode(11,OUTPUT);
    
    portOne.begin(115200);
    
    Joystick[0].begin();
    Joystick[1].begin();
    
    Keyboard.begin();
}

void readyExit()
{
  check_exit = true;
  exit_time = millis();
}

boolean controllers[2][13];

void loop() {
  // clear controller inputs from last loop.
  memset(controllers[0],0,sizeof(controllers[0]));
  memset(controllers[1],0,sizeof(controllers[1]));
  
  readInput();
  
  writeJoysticks();
  
  writeVolumeSwitch();
  // check if p1 start is being pressed for 5secs or more, and send a keyboard ESC signal
  if (fixers[0][5] > 1) {
    if (!check_exit) readyExit();
    
    if (millis() < exit_time) exit_time = 0;
    
    if ((millis()-exit_time >= 5000)) Keyboard.write(KEY_ESC);
    return;
  } 
  
  check_exit = false;
}
