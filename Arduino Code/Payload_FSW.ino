#include <Wire.h>
#include <math.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_HMC5883_U.h> // Magnetometer
#include <DHT.h> // Thermometer
#include <DHT_U.h>
#include <SFE_BMP180.h> // Barometer/Altimeter
#include <string.h>

// Defined Constants
#define TEMPPIN 2 // digital pin that the thermometer connects to
#define PITOTPIN A4 // pin for the pitot tube
#define BUZZERPIN 3 // buzzer pin
#define VP A2 // positive side of the voltmeter
#define VN A3 // negative side of the voltmeter
#define RTC_ADDRESS 0x68
#define EEPROM_ADDRESS 0x50
#define DECLINATION 12.5 // Princeton, NJ (degrees)
// #define DECLINATION 4.2167 // Stephenville, Tx (degrees)
#define DSTATE 0 // Dormant
#define TSTATE 1 // Transmitting
#define LSTATE 2 // Landed
#define SERIAL Serial // Which Serial port we're using
#define SERIALEVENT serialEvent
#define BAUD 9600 // for USB Serial
#define V_REF 2.56 // internal reference voltage

// Sensor/Global Variable Declarations
Adafruit_HMC5883_Unified mag = Adafruit_HMC5883_Unified(12345); // magnetometer
DHT_Unified dht(TEMPPIN,DHT22);
SFE_BMP180 bmp;
double baseline; // baseline pressure
int offset;
int t_pwr_up; // time of power-up
int pkt_cnt = 0;
String pkt;
short state = TSTATE; // Change to DSTATE after testing
int pic_cnt; // number of pictures taken

void setup() {
  rst_rtc(); // reset the rtc to 0
  t_pwr_up = rtc(); // store the time of power-up
  dht.begin(); // thermometer
  bmp.begin(); // Pressure/Altitude sensor
  mag.begin(); // mag sensor
  baseline = pressure();  

  // Setup for pitot tube (copied from example code)
  int i=0;
  int sum=0;
  offset=0;
  int sensorValue;
  for(i=0;i<10;i++)
  {
       sensorValue = analogRead(PITOTPIN)-512;
       sum+=sensorValue;
  }
  offset=sum/10.0;
}

void loop() {
  switch(state){
    case DSTATE: // dormant
        break;
    case TSTATE: // transmitting
      SERIAL.println(packet());
      delay(500);
      break;
    case LSTATE: // landed
      audio(); // play a sound
      break;
  }
}

// This method is written to make testing easier.
// It should be rewritten after testing is done.
// Called when Serial recieves data
// Sets state to the specified value
void SERIALEVENT()
{
  String input = SERIAL.readStringUntil('\n');  // read new token
  if (input == "Dormant")  state = DSTATE;
  else if (input == "Transmitting")  state = TSTATE;
  else if (input == "Landed")  state = LSTATE;
}

// Reads sensor data and generates a packet String
String packet()
{
  pkt_cnt++; // increment packet count
  
  String p = "4234,GLIDER,"; // team_id,GLIDER
  p += String(rtc() - t_pwr_up) + ","; // MISSION TIME,
  p += String(pkt_cnt) + ",";
  int P = pressure(); // read pressure
  p += String(alt(P)) + ",";
  p += String(P) + ",";
  p += String(spd()) + ",";
  p += String(temp()) + ",";
  p += String(vltg()) + ",";
  p += String(heading()) + ",";
  p += stateToString() + ",";
  p += String(pic_cnt);
  
  return p;
}

// real-time clock reading in seconds
int rtc()
{
  // Step 1: Set address pointer to the first register
  Wire.beginTransmission(RTC_ADDRESS);
  Wire.write((byte)0x00);
  Wire.endTransmission();

  // Step 2: Convert seconds, minutes, and hours to seconds
  Wire.requestFrom(RTC_ADDRESS, 7);
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

// reset the rtc to 0
// Adapted from example code
void rst_rtc()
{
  byte zero = 0x00;
  Wire.beginTransmission(RTC_ADDRESS);
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

// Read from EEPROM
byte eeRead(unsigned int address)
{
  // Set pointer to address
  Wire.beginTransmission(EEPROM_ADDRESS);
  Wire.write(address >> 8); // Most significant byte
  Wire.write(address & 0xFF); // Least significant byte
  Wire.endTransmission();

  Wire.requestFrom(EEPROM_ADDRESS, 1);
  return Wire.read();
}

// Write to EEPROM
void eeWrite(unsigned int address, byte data)
{
  Wire.beginTransmission(EEPROM_ADDRESS);
  Wire.write(address >> 8); // Most significant byte
  Wire.write(address & 0xFF); // Least significant byte
  Wire.write(data);
  Wire.endTransmission();

  delay(6);
}

// converts state into a String
String stateToString()
{
  switch(state){
    case DSTATE:
      return "Dormant";
      break;
    case TSTATE:
      return "Transmitting";
      break;
    case LSTATE:
      return "Landed";
      break;  
  }
}

// voltage in volts
double vltg()
{
    return (analogRead(VP) - analogRead(VN)) * V_REF / 1023; // OP_VLTG/1023 converts to volts  
}

// Sample time, altitude, pressure, speed, temp, voltage, heading, camera
// in degrees
double heading() {
  // Step 1: Read magnetometer data
  sensors_event_t e;
  mag.getEvent(&e);
  double x = e.magnetic.x;
  double y = e.magnetic.y;
  double z = e.magnetic.z;

  // Step 2: Calculate the heading
  double heading = atan2(y, x) * 180/PI; // Find heading in degrees
  heading += DECLINATION;
  
  if (heading < 0)  heading += 360; // Make sure 0 < heading < 360
  else if (heading > 360) heading -= 360;

  return heading;
}

// in degrees Celsius
double temp() {
  sensors_event_t e;
  dht.temperature().getEvent(&e);
  return e.temperature;  
}

// relative altitude (in meters)
double alt(double P) {
  return bmp.altitude(P, baseline);
}

// in m/s
double spd(){
  double sensorValue = analogRead(PITOTPIN)-offset; 
  int P=(5*sensorValue)/1024.0 - 2.5;
  P *= 1000; // in Pa

  double speed = sqrt((2/1.225)*P);
  return speed;
}

// in mb
// Copied from example code
double pressure()
{
  char status;
  double T,P,p0,a;

  // You must first get a temperature measurement to perform a pressure reading.
  
  // Start a temperature measurement:
  // If request is successful, the number of ms to wait is returned.
  // If request is unsuccessful, 0 is returned.

  status = bmp.startTemperature();
  if (status != 0)
  {
    // Wait for the measurement to complete:

    delay(status);

    // Retrieve the completed temperature measurement:
    // Note that the measurement is stored in the variable T.
    // Use '&T' to provide the address of T to the function.
    // Function returns 1 if successful, 0 if failure.

    status = bmp.getTemperature(T);
    if (status != 0)
    {
      // Start a pressure measurement:
      // The parameter is the oversampling setting, from 0 to 3 (highest res, longest wait).
      // If request is successful, the number of ms to wait is returned.
      // If request is unsuccessful, 0 is returned.

      status = bmp.startPressure(3);
      if (status != 0)
      {
        // Wait for the measurement to complete:
        delay(status);

        // Retrieve the completed pressure measurement:
        // Note that the measurement is stored in the variable P.
        // Use '&P' to provide the address of P.
        // Note also that the function requires the previous temperature measurement (T).
        // (If temperature is stable, you can do one temperature measurement for a number of pressure measurements.)
        // Function returns 1 if successful, 0 if failure.

        status = bmp.getPressure(P,T);
        if (status != 0)
        {
          return(P);
        }
      }
    }
  }
}

// Plays a sound
void audio()
{
  int f, d;

  // Loop through all the Serial input
  while(SERIAL.peek() != -1){
      f = SERIAL.parseInt(); // frequency
      d = SERIAL.parseInt(); // duration
      
      tone(BUZZERPIN, f, d); // play the sound
      delay(d);
  }  
  if(SERIAL.peek() == -1) tone(BUZZERPIN, 4000, 100);
}
