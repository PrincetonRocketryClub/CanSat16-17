// Reads sensor data and generates a packet String
String packet()
{

  EEPROM.write(PKT_CNT_ADDR, EEPROM.read(PKT_CNT_ADDR) + 1); // increment packet count
  
  String p = "4234,CONTAINER,"; // team_id,CONTAINER
  p += String(rtc()) + ","; // MISSION TIME,
  p += String(EEPROM.read(PKT_CNT_ADDR)) + ",";
  p += String(alt()) + ",";
  p += String(temp()) + ",";
  p += String(vltg()) + ",";
  p += stateToString();
  
  return p;
}

// real-time clock reading in seconds
int rtc()
{
  // Step 1: Set address pointer to the first register
  Wire.beginTransmission(RTC_ADDR);
  Wire.write(0x00);
  Wire.endTransmission();

  // Step 2: Convert seconds, minutes, and hours to seconds
  Wire.requestFrom(RTC_ADDR, 7);
  int time = bcdToDec(Wire.read()); // read seconds
  time += 60 * bcdToDec(Wire.read()); // read minutes
  time += 3600 * bcdToDec(Wire.read()); // read hours
  Wire.read(); Wire.read(); Wire.read(); Wire.read();
  
  return time; // time is the time since power-up in seconds
}

// Converts binary coded decimals to decimals
int bcdToDec(byte value)
{
  return (value / 16 * 10) + (value % 16);
}

// converts state into a String
String stateToString()
{
  switch(state){
    case ISTATE:
      return "Initializing";
      break;
    case TSTATE:
      return "Transmitting";
      break;
    case RSTATE:
      return "Releasing";
      break;
    case LSTATE:
      return "Landed";
      break;  
  }
  return ""; // default
}

// voltage in volts
double vltg()
{
  analogReference(INTERNAL);
  int voltage = analogRead(VP) * (V_REF / 1023.0) * (VLTG_FACTOR); // Read value, convert to volts, scale by voltage divider factor
  analogReference(DEFAULT);
  return voltage;
}

// in degrees Celsius
double temp() {
  return bme.readTemperature();
}

// relative altitude (in meters)
double alt() {
  return bme.readAltitude(groundPressure / MB_PER_PASCAL); // convert pressure to Pascals
}
