// reads button values from serial buffer for controller j
void readButtonsFromController(int j, byte *buffer) 
{
  for(int i = 0; i < 6; i++) {
    controllers[j][6+i] = (buffer[5+j*4] & (1 << i)) > 0;
    if (i == 4) continue;
    controllers[j][i] = (buffer[4+j*4] & (1 << i)) > 0;
  }
}

// reads all the buttons
void readButtons(byte *buffer)
{
  for(int j = 0; j <2; j++) {
    readButtonsFromController(j, buffer);
  }
  if (buffer[2] == volumeUp) controllers[0][4] = true;
  else if (buffer[2] == 0)  controllers[0][12] = true;
}

// reads both controllers over the serial stream
void readInput()
{
  byte loopCount = 0;
  byte buffer[18];
  
  portOne.listen();
  portOne.write(sendSignal, 3);
  
  delayMicroseconds(1563); // time it takes to send 18 bytes 8n1 on 115200
  while(portOne.available() > 0) {
    buffer[loopCount] = portOne.read();
    loopCount++;
  }
  
  if (loopCount == 18 && buffer[0] == 0xA7 && buffer[1] == 0x10 && buffer[3] == 0x01 && buffer[7] == 0x02) {
  // we are in business, everything sent, everything is in correct spot
    readButtons(buffer);
  }
}
