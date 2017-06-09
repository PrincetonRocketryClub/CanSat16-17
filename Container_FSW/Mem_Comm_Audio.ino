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

// Plays a sound
void audio()
{
  tone(BUZZERPIN, 440, 1000);
  delay(1000);
}
