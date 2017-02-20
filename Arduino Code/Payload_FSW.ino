#include <Wire.h>
#include <math.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_HMC5883_U.h> // Magnetometer
#include <DHT.h> // Thermometer
#include <DHT_U.h>
#include <SFE_BMP180.h> // Barometer/Altimeter
#include <string.h>

// Defined Constants
#define TEMPPIN 2 // The digital pin on the Arduino that the thermometer connects to
#define PITOTPIN A0 // pin for the pitot tube
#define DECLINATION 12.5 // Princeton, NJ (degrees)
// #define DECLINATION 4.2167 // Stephenville, Tx (degrees)

// Sensor/Global Variable Declarations
Adafruit_HMC5883_Unified mag = Adafruit_HMC5883_Unified(12345); // magnetometer
DHT_Unified dht(TEMPPIN,DHT22);
SFE_BMP180 bmp;
double baseline; // baseline pressure
int offset;
int t_pwr_up; // time of power-up
int pkt_cnt = 0;
String pkt;
int state;
int pic_cnt; // number of pictures taken

void setup() {
  Serial.begin(9600);
  t_pwr_up = rlt(); 
  dht.begin(); // thermometer
  bmp.begin(); // Pressure/Altitude sensor
  mag.begin(); // mag sensor
  baseline = pressure();  

    // Setup for pitot tube (copied from example code)
    int i=0;
    int sum=0;
    offset=0;
    int sensorValue;
    Serial.println("init...");
    for(i=0;i<10;i++)
    {
         sensorValue = analogRead(PITOTPIN)-512;
         sum+=sensorValue;
    }
    offset=sum/10.0;
  
}

void loop() {
  Serial.println(packet());
  
  delay(500);
}

// Reads sensor data and generates a packet String
String packet()
{
  pkt_cnt++; // increment packet count
  
  String p = "4234,GLIDER,"; // team_id,GLIDER
  p += String(rlt() - t_pwr_up) + ","; // MISSION TIME,
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
int rlt()
{
  return 0;
}

// converts int state into a String
String stateToString()
{
    return "STATE";  
}

// voltage in volts
double vltg()
{
    return 0.0;  
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
  return 0.0;
  /*double sensorValue = analogRead(PITOTPIN)-offset; 
  int P=(5*sensorValue)/1024.0 - 2.5;
  P *= 1000; // in Pa

  double speed = sqrt((2/1.225)*P);
  return speed;*/
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
        else Serial.println("error retrieving pressure measurement\n");
      }
      else Serial.println("error starting pressure measurement\n");
    }
    else Serial.println("error retrieving temperature measurement\n");
  }
  else Serial.println("error starting temperature measurement\n");
}
