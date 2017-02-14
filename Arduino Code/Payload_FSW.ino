#include <math.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <Adafruit_HMC5883_U.h> // Magnetometer
#include <DHT.h> // Thermometer
#include <DHT_U.h>
#include <SFE_BMP180.h> // Barometer/Altimeter

// Defined Constants
#define PI 3.1415926536
#define TEMPPIN 2 // The digital pin on the Arduino that the thermometer connects to
#define DECLINATION 12.5 // Princeton, NJ (degrees)
// #define DECLINATION 4.2167 // Stephenville, Tx (degrees)

// Sensor/Global Variable Declarations
Adafruit_HMC5883_Unified mag = Adafruit_HMC5883_Unified(12345); // magnetometer
DHT_Unified dht(TEMPPIN,DHT22);
SFE_BMP180 bmp;
double baseline; // baseline pressure
int offset;

void setup() {
  Serial.begin(9600); 
  dht.begin(); // thermometer
  bmp.begin(); // Pressure/Altitude sensor
  baseline = pressure();  

    // Setup for pitot tube (copied from example code)
    int i=0;
    int sum=0;
    offset=0;
    Serial.println("init...");
    for(i=0;i<10;i++)
    {
         sensorValue = analogRead(sensorPin)-512;
         sum+=sensorValue;
    }
    offset=sum/10.0;
  
}

void loop() {
  
  
}

// Sample time, altitude, pressure, speed, temp, voltage, heading, camera
// in degrees
double heading() {
  // Step 1: Read magnetometer data
  sensors_event_t e = mag.getEvent(&e);
  double x = e.magnetic.x;
  double y = e.magnetic.y;
  double z = e.magnetic.z;

  // Step 2: Calculate the heading
  double heading = atan2(y, x) * PI/180; // Find heading in degrees
  heading += DECLINATION;
  
  if (heading < 0)  heading += 360; // Make sure 0 < heading < 360
  else if (heading > 360) heading -= 360;

  return heading;
}

// in degrees Celsius
double temp() {
  sensors_event_t e = dht.temperature().getEvent(&e);
  return e.temperature;  
}

// relative altitude (in meters)
double alt(double P) {
  return bmp.altitude(P, baseline);
}

// in m/s
double speed(){
  
  
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

  status = pressure.startTemperature();
  if (status != 0)
  {
    // Wait for the measurement to complete:

    delay(status);

    // Retrieve the completed temperature measurement:
    // Note that the measurement is stored in the variable T.
    // Use '&T' to provide the address of T to the function.
    // Function returns 1 if successful, 0 if failure.

    status = pressure.getTemperature(T);
    if (status != 0)
    {
      // Start a pressure measurement:
      // The parameter is the oversampling setting, from 0 to 3 (highest res, longest wait).
      // If request is successful, the number of ms to wait is returned.
      // If request is unsuccessful, 0 is returned.

      status = pressure.startPressure(3);
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

        status = pressure.getPressure(P,T);
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
