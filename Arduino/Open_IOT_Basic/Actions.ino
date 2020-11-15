void ping()
{
  server.send(200, "text/html", "OK");
}

void power()
{
  if (isRunning)
  {
    turnOff();
  }
  else
  {
    turnOn();
  }
}

void headlight()
{
  if (headlightStatus)
  {
    disableHeadlight();
  }
  else
  {
    enableHeadlight();
  }
}

void flash()
{
  if (flashStatus)
  {
    disableFlash();
  }
  else
  {
    enableFlash();
  }
}

void setUnitSystem()
{
  if (isMetric)
  {
    setMi();
  }
  else
  {
    setKm();
  }
}

void toggleBit4()
{
  if (isBit4)
  {
    disableMysteryBit4();
  }
  else
  {
    enableMysteryBit4();
  }
}

void toggleBit7()
{
  if (isBit7)
  {
    disableMysteryBit7();
  }
  else
  {
    enableMysteryBit7();
  }
}

void toggleBit8()
{
  if (isBit8)
  {
    disableMysteryBit8();
  }
  else
  {
    enableMysteryBit8();
  }
}

void turnOff()
{
  isRunning = false;
  okaiPacket[3] = okaiPacket[3] & B11111110;
  reCalculateCommand();
  sendCommand();
  delay(1000);
  sendCommand();
  goHome();
  tone2(D5, 1046, 50);
  tone2(D5, 785, 50);
  tone2(D5, 523, 50);
}

void turnOn()
{
  isRunning = true;
  okaiPacket[3] = okaiPacket[3] | B00000001;
  reCalculateCommand();
  sendCommand();
  delay(1000);
  sendCommand();
  goHome();
  tone2(D5, 523, 50);
  tone2(D5, 785, 50);
  tone2(D5, 1046, 50);
}

void enableFlash()
{
  flashStatus = true;
  okaiPacket[3] = okaiPacket[3] | B00000010;
  reCalculateCommand();
  sendCommand();
  goHome();
}

void disableFlash()
{
  flashStatus = false;
  okaiPacket[3] = okaiPacket[3] & B11111101;
  reCalculateCommand();
  sendCommand();
  goHome();
}

void enableHeadlight()
{
  headlightStatus = true;
  okaiPacket[3] = okaiPacket[3] | B00000100;
  reCalculateCommand();
  sendCommand();
  goHome();
  tone2(D5, 1046, 20);
}

void disableHeadlight()
{
  headlightStatus = false;
  okaiPacket[3] = okaiPacket[3] & B11111011;
  reCalculateCommand();
  sendCommand();
  goHome();
  tone2(D5, 523, 20);
}

void enableMysteryBit4()
{
  isBit4 = true;
  okaiPacket[3] = okaiPacket[3] | B00001000;
  reCalculateCommand();
  sendCommand();
  goHome();
}

void disableMysteryBit4()
{
  isBit4 = false;
  okaiPacket[3] = okaiPacket[3] & B11110111;
  reCalculateCommand();
  sendCommand();
  goHome();
}

void setKm()
{
  isMetric = true;
  okaiPacket[3] = okaiPacket[3] | B00010000;
  reCalculateCommand();
  sendCommand();
  goHome();
}

void setMi()
{
  isMetric = false;
  okaiPacket[3] = okaiPacket[3] & B11101111;
  reCalculateCommand();
  sendCommand();
  goHome();
}

void enableFastAcceleration()
{
  okaiPacket[3] = okaiPacket[3] | B00100000;
  reCalculateCommand();
  sendCommand();
  //server.send(200, "text/html", goHome());
}

void disableFastAcceleration()
{
  okaiPacket[3] = okaiPacket[3] & B11011111;
  reCalculateCommand();
  sendCommand();
  //server.send(200, "text/html", goHome());
}

void enableMysteryBit7()
{
  isBit7 = true;
  okaiPacket[3] = okaiPacket[3] | B01000000;
  reCalculateCommand();
  sendCommand();
  goHome();
}

void disableMysteryBit7()
{
  isBit7 = false;
  okaiPacket[3] = okaiPacket[3] & B10111111;
  reCalculateCommand();
  sendCommand();
  goHome();
}

void enableMysteryBit8()
{
  isBit8 = true;
  okaiPacket[3] = okaiPacket[3] | B10000000;
  reCalculateCommand();
  sendCommand();
  goHome();
}

void disableMysteryBit8()
{
  isBit8 = false;
  okaiPacket[3] = okaiPacket[3] & B01111111;
  reCalculateCommand();
  sendCommand();
  goHome();
}

void setDefaultConfig()
{
  okaiPacket[4] = okaiMaxSpeed;
  okaiPacket[3] = okaiDefaultConfig;
  reCalculateCommand();
  goHome();
  isRunning = true;
  headlightStatus = false;
  flashStatus = false;
  isMetric = false;
  isBit4 = false;
  isBit7 = true;
  isBit8 = true;
}

void reboot()
{
  turnOff();
  delay(500);
  turnOn();
  delay(500);
  server.send(200, "text/html", SendHTML());
}

void tone2(uint8_t _pin, unsigned int frequency, unsigned long duration) {
  pinMode (_pin, OUTPUT );
  analogWriteFreq(frequency);
  analogWrite(_pin, 500);
  delay(duration);
  analogWrite(_pin, 0);
}
