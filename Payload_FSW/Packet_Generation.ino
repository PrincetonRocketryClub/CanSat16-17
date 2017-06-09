// Reads sensor data and generates a packet String
String packet()
{

  EEPROM.write(PKT_CNT_ADDR, EEPROM.read(PKT_CNT_ADDR) + 1); // increment packet count
  
  String p = "4234,GLIDER,"; // team_id,GLIDER
  p += String(rtc()) + ","; // MISSION TIME,
  p += String(EEPROM.read(PKT_CNT_ADDR)) + ",";
  p += String(alt()) + ",";
  p += String(pressure()) + ",";
  p += String(spd()) + ",";
  p += String(temp()) + ",";
  p += String(vltg()) + ",";
  p += String(heading()) + ",";
  p += stateToString() + ",";
  p += String(EEPROM.read(PIC_CNT_ADDR));
  
  return p;
}

// real-time clock reading in seconds
int rtc()
{
  // Step 1: Set address pointer to the first register
  Wire.beginTransmission(RTC_ADDR);
  Wire.write((byte)0x00);
  Wire.endTransmission();

  // Step 2: Convert seconds, minutes, and hours to seconds
  Wire.requestFrom(RTC_ADDR, 7);
  int time = bcdToDec(Wire.read()); // read seconds
  time += 60 * bcdToDec(Wire.read()); // read minutes
  time += 3600 * bcdToDec(Wire.read()); // read hours

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

// Sample time, altitude, pressure, speed, temp, voltage, heading, camera
// in degrees
double heading() {
  // Step 1: Read magnetometer data
  sensors_event_t e;
  mag.getEvent(&e);
  double x = e.magnetic.x;
  double y = e.magnetic.y;
  // double z = e.magnetic.z;

  // Step 2: Calculate the heading
  double heading = atan2(y, x) * 180/PI; // Find heading in degrees
  heading += DECLINATION;
  
  if (heading < 0)  heading += 360; // Make sure 0 < heading < 360
  else if (heading > 360) heading -= 360;

  return heading;
}

// in degrees Celsius
double temp() {
  return bme.readTemperature();
}

// relative altitude (in meters)
double alt() {
  return bme.readAltitude(groundPressure);
}

// in m/s
double spd(){
  double sensorValue = analogRead(PITOTPIN)-offset; 
  int P=(5*sensorValue)/1024.0 - 2.5;
  P = P * 1000; // in Pa

  double speed = sqrt((2/1.225)*P);
  return speed;
}

// in mb
double pressure()
{
  return bme.readPressure() * MB_PER_PASCAL; // convert pressure to mb
}

/*// takes and saves a picture and updates pic count
void takePicture()
{
  cam.takePicture();
  EEPROM.write(PIC_CNT_ADDR, EEPROM.read(PIC_CNT_ADDR) + 1); // increment pic count
  int bytesLeft = cam.frameLength(); // size of picture (in bytes)
  int bytesToRead; // number of bytes to read on each step
  int 
  while (picLength > 0)
  {
    cam.readPicture(bytesToRead);
  }
  
}*/
