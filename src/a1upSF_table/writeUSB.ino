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

const int hatValues[] = {180, 270, 0, 90};

// gets the hat angle of controller i
int getHatValue(int i)
{
  int hatValue = 0;
  int directionsPressed = 0;
  bool upIncluded = false;
  
  for(int j = 0; j < 4; j++)
  {
    if (!checkButtonInput(i,j)) continue;
    if (j == 2) upIncluded = true;
    directionsPressed++;
    hatValue += hatValues[j];
  }
  
  if (directionsPressed == 0) return -1;
  hatValue /= directionsPressed;
  if (upIncluded && hatValue == 135) hatValue = 315;
  return hatValue;
}

int previoushats[2] = {-1,-1};

// writes HatValue to controller i
void writeHatValue(int i) 
{
  int hatValue = getHatValue(i);
  
  if (hatValue != previoushats[i]) 
  {
    Joystick[i].setHatSwitch(0,hatValue);
    previoushats[i] = hatValue;
  }
}

// checks button j on controller i and writes it to usb.
void writeButton(int i, int j)
{
  int oldState = fixers[i][j+5];
  if (checkButtonInput(i, j+5))
  {
    if (oldState == 1) Joystick[i].pressButton(j);
    return;
  }
  if (oldState == 3) Joystick[i].releaseButton(j);
}

// iterates over all buttons on controller i and writes them to the usb.
void writeAllButtons(int i)
{
  for(int j = 0; j < 7; j++)
  {
    writeButton(i, j);
  }
}

// iterates over controllers writing Joystick data
void writeJoysticks() 
{
  for(int i = 0; i < 2; i++) 
  {
    writeHatValue(i);
    
    writeAllButtons(i);
  }
}

// checks the volume switch and adjust volume accordingly
void writeVolumeSwitch()
{
  int oldState = fixers[0][4];
  
  if (checkButtonInput(0,4))
  {
    if (oldState == 1) Keyboard.write(MEDIA_VOLUME_DOWN);
  
  oldState = fixers[0][12];
  
  if (checkButtonInput(0, 12))
  {
    if (oldState == 1) Keyboard.write(MEDIA_VOLUME_UP);
  }
}
