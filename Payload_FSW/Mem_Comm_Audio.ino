// reset the rtc to 0s
// Adapted from example code
void rst_rtc()
{
  byte zero = 0x00;
  Wire.beginTransmission(RTC_ADDR);
  Wire.write(zero); // sets the address pointer to zero

  Wire.write(zero); // set successive registers to 0
  Wire.write(zero);
  Wire.write(zero);
  Wire.write(zero);
  Wire.write(zero);
  Wire.write(zero);
  Wire.write(zero);

  Wire.endTransmission();
}

/*// Read from flash
byte flashRead(unsigned int address)
{
  // Set pointer to address
  Wire.beginTransmission(EEPROM_ADDR);
  Wire.write(address >> 8); // Most significant byte
  Wire.write(address & 0xFF); // Least significant byte
  Wire.endTransmission();

  Wire.requestFrom(EEPROM_ADDR, 1);
  return Wire.read();
}

// Write to flash
void flashWrite(unsigned int address, byte data)
{
  Wire.beginTransmission(EEPROM_ADDR);
  Wire.write(address >> 8); // Most significant byte
  Wire.write(address & 0xFF); // Least significant byte
  Wire.write(data);
  Wire.endTransmission();

  delay(6);
}*/

// Plays a sound
void audio()
{
  tone(BUZZERPIN, 4000, 100);
}

// Takes a picture and transfers it to the flash chip
/*public void takePicture()
{
  cam.takePicture();  
  
}*/
