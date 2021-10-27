/*
  Used to interface with turtles control panel
*/
#define HID_CUSTOM_LAYOUT
#define LAYOUT_US_ENGLISH


#include <SoftwareSerial.h>
#include <HID-Project.h>

// We have two ports used by the turtles board, One for one/two, Two for three/four
SoftwareSerial portTwo(8, 9);
SoftwareSerial portOne(10, 11);

const byte volumeUp = B00000010;

const byte sendSignal[3] = {0xA6, 0x01, 0x00};

const KeyboardKeycode keyCodes[4][8] = {{KEY_D, KEY_S, KEY_A, KEY_W, KEY_3, KEY_Z, KEY_2, KEY_1},
                                {KEY_H, KEY_G, KEY_F, KEY_T, KEY_6, KEY_V, KEY_5, KEY_4},
                                {KEY_L, KEY_K, KEY_J, KEY_I, KEY_9, KEY_M, KEY_8, KEY_7},
                                {KEY_RIGHT,KEY_DOWN,KEY_LEFT,KEY_UP, KEY_EQUAL, KEY_COMMA, KEY_MINUS, KEY_0}};

//                             right  down   left   up     turbo  start  jump   attack vDown  vUp
boolean controllers[4][10];

// I don't know if it's my logic level voltage shifter introducing lag or just a problem with the cab, but we get phantom output every
// once in a while. My solution is to poll twice as fast and only take a press if it's pressed for 2 polls. Same thing for a release.
byte fixers[4][10];

// timers used for debouncing and volume limiters
long timers[4][10];

const ConsumerKeycode cons[] = {MEDIA_VOLUME_DOWN, MEDIA_VOLUME_UP};

// used when holding 1p start to exit game
unsigned long exit_time = 0;
bool check_exit = false;

void setup() 
{
  pinMode(8, INPUT);
  pinMode(9, OUTPUT);
  pinMode(10, INPUT);
  pinMode(11, OUTPUT);

  //Both ports are 115200
  portOne.begin(115200);
  portTwo.begin(115200);
  Keyboard.begin();
  Consumer.begin();
}

// Checks button j input on controller i using bounce protection. 
bool checkButtonInput(int i,int j) 
{
  switch(fixers[i][j]) 
  {
  case 0: // Unpressed
    if (controllers[i][j]) fixers[i][j]++;
    break;
  case 1:  // pressed for one polls
    if (controllers[i][j]) 
    {
      fixers[i][j]++;
      timers[i][j] = millis();
      return true;
    }
    fixers[i][j]--;
    break;
  case 2: // pressed for two or more polls
    if (millis() < timers[i][j]) timers[i][j] = 0;
    if (!controllers[i][j]) fixers[i][j]++;
    return true;
  case 3: // unpressed for one polls
    if (controllers[i][j] || millis() - timers[i][j] < 5)
    {
      fixers[i][j]--;
      return true;
    } 
    fixers[i][j] = 0;
    break;
  default:
    break;
  }
  return false;
}

void clearControls() 
{
  for(int i = 0; i < 4; i++) {
    for (int j = 0; j < 10; j++) {
      controllers[i][j] = false;
    }
  }
}

// polls the controller and reads data
void readControls(int set) 
{
  SoftwareSerial *port;
  switch(set) 
  {
    case 0:
      port = &portOne;
      break;
    case 1:
      port = &portTwo;
      break;
    default:
      return;
  }
  byte loopCount = 0;
  byte buffer[18];
  port->listen();
  port->write(sendSignal, 3);
  delayMicroseconds(1563); // time it takes to send 18 bytes 8n1 on 115200
  while(port->available() > 0) 
  {
    buffer[loopCount] = port->read();
    loopCount++;
  }
  if (loopCount == 18 && buffer[0] == 0xA7 && buffer[1] == 0x10 && buffer[3] == 0x01 && buffer[7] == 0x02) 
  {
    // we are in business, everything sent, everything is in correct spot
    if (buffer[2] == volumeUp) controllers[set*2][9] = true;
    else if (buffer[2] == 0)  controllers[set*2][8] = true;
    for(int j = 0; j <2; j++) 
    {
      for(int i = 0; i < 6; i++) 
      {
        if (i == 4) continue;
        controllers[set*2+j][i] = (buffer[4+j*4] & (1 << i)) > 0;
      }
      
      for(int i = 0; i < 4; i++) 
      {
        if (i == 1) continue;
        controllers[set*2+j][4+i] = (buffer[5+j*4] & (B00000010 << i)) > 0;
      }
    }
  }
}

// checks if a button is pressed and presses/releases it based on state
void writeButton(int i, int j) 
{
  int oldState = fixers[i][j];
  if (checkButtonInput(i,j)) 
  {
    if (oldState == 1) Keyboard.press(keyCodes[i][j]);
    return;
  }
  if (oldState == 3) Keyboard.release(keyCodes[i][j]);
}

// iterates over buttons on controller and writes value
void buttonsWriter(int i)
{
  for(int j = 0; j < 8; j++) 
  {
    writeButton(i,j);
  }
}

// checks if the switch is in a position and timing to change volume
void writeVolume(int i) 
{
  if (!controllers[0][8+i] && fixers[0][8+i] < 2)
  {
    fixers[0][8+i] = 0;
    return;
  }
    
  switch(fixers[0][8+i]) 
  {
  case 0: // Unpressed
    fixers[0][8+i]++;
    break;
  case 1:
    fixers[0][8+i]++;
    break;
  case 2:
    timers[0][8+i] = millis();
    fixers[0][8+i]++;
    Consumer.write(cons[i]);
    break;
  case 3: // pressed for 1/60th and above.
    if (millis() < timers[0][8+i]) timers[0][8+i] = 0;
    if (millis() - timers[0][8+i] >= 250) fixers[0][8+i] = 0;
    break;
  default:
    break;
  }
}

// iterates over the two volume switches and deals with them
void volumeWriter() 
{
  for (int i = 0; i < 2; i++)
  {
    writeVolume(i);
  }
}

void loop() 
{
  clearControls();
  readControls(0); // p1 and 2
  readControls(1); // p3 and 4
  
  for(int i = 0; i < 4; i++) 
  {
    buttonsWriter(i);
  }
    
  // volume checks
  volumeWriter();
  
  if (fixers[0][5] > 1) 
  {
    if (!check_exit) 
    {
      check_exit = true;
      exit_time = millis();
    }
    if (millis() < exit_time) exit_time = 0;
    if ((millis()-exit_time >= 5000)) Keyboard.write(KEY_ESC);
    
    return;
  }
  check_exit = false;
}

