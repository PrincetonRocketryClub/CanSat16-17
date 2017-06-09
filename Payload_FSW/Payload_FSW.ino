#include <Wire.h>
#include <math.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_HMC5883_U.h> // Magnetometer
#include "Adafruit_BME280.h" // Barometer/Altimeter/Thermometer
#include "Adafruit_VC0706.h" // camera
#include <string.h>
#include <SerialFlash.h> // flash
#include <EEPROM.h> // EEPROM built into the teensy

// Defined Constants
#define PITOTPIN A2 // pin for the pitot tube
#define BUZZERPIN 2 // buzzer pin
#define VP A2 // positive side of the voltmeter
#define VLTG_FACTOR 6 // 1 / how much the voltage divider scales the signal by
#define RTC_ADDR 0x68
#define EEPROM_ADDR 0x50
#define DECLINATION 4.2167 // Stephenville, Tx (degrees)
#define SEALEVELPRESSURE_HPA (1013.25)
#define ISTATE 1 // Initializing
#define TSTATE 2 // Transmitting
#define LSTATE 3 // Landed
#define SSTATE 4 // Sampling/Dormant
#define USART_SERIAL Serial1 // the serial port the antenna connects to
#define USART_SERIALEVENT serialEvent1
#define CAM_SERIAL Serial3 // the serial port the camera connects to
#define USB_Serial Serial // The USB serial port
#define V_REF 2.56 // internal reference voltage
#define PKT_CNT_ADDR 10 // (change this address) address in EEPROM of packet count
#define PIC_CNT_ADDR 11 // address in EEPROM of picture count
#define STATE_ADDR 12 // (change this address) address in EEPROM of state
#define ALT_ERROR .2 // How high can the altimeter read if alt actually equals 0?
#define MB_PER_PASCAL .01 // converts Pascals to mb


// Sensor/Global Variable Declarations
Adafruit_HMC5883_Unified mag = Adafruit_HMC5883_Unified(12345); // magnetometer
Adafruit_BME280 bme; // pressure, temperature, altitude sensor
Adafruit_VC0706 cam = Adafruit_VC0706(&CAM_SERIAL); // camera
SerialFlashFile file; // where to write the file

double groundPressure = 1015; // used to calculate altitude
uint16_t jpglen = 0; // bytes of the image left to download
int offset; // pitot tube offset
short state; // current state

void setup() {
  USART_SERIAL.begin(9600);
  USB_Serial.begin(9600);
  CAM_SERIAL.begin(38400);
  Wire.begin(); // I2C bus
  bme.begin(); // Pressure/Altitude/Temperature sensor
  mag.begin(); // mag sensor
  cam.begin(); // camera
  cam.setImageSize(VC0706_640x480); // 640 x 480 pixels

  pinMode(BUZZERPIN, OUTPUT);
  
  state = EEPROM.read(STATE_ADDR); // load state upon power-up
}

void loop() {
  int alt1, alt2; // used in TSTATE
  switch(state){
    case ISTATE: // Initializing
        rst_rtc(); // reset mission time
        EEPROM.write(PKT_CNT_ADDR, 0); // reset packet count
        state = TSTATE; // state <-- transmitting
        EEPROM.write(STATE_ADDR, state);
        break;
    case TSTATE: // transmitting
      USART_SERIAL.println(packet());
      
      alt1 = alt();
      delay(1000);
      alt2 = alt();
      if (abs(alt1 - alt2) < ALT_ERROR) // if alt is constant
      {
        state = LSTATE;
        EEPROM.write(STATE_ADDR, state);
      }
      break;
    case LSTATE: // landed
      audio(); // play a sound
      if (!(EEPROM.read(STATE_ADDR) == ISTATE)) // if state hasn't been set to initializing
        EEPROM.write(STATE_ADDR, ISTATE); // reset state to initializing
      break;
  }

/*  // Download Image to flash
  if (jpglen > 0) {
    // read 64 bytes at a time;
    uint8_t *buffer;
    uint8_t bytesToRead = min(64, jpglen); // change 32 to 64 for a speedup but may not work with all setups!
    buffer = cam.readPicture(bytesToRead);
    imgFile.write(buffer, bytesToRead);

    jpglen -= bytesToRead;
  }
  imgFile.close();
*/}

// Called when Serial recieves data
// Sets state to the specified value
void USART_SERIALEVENT()
{
  String input = USART_SERIAL.readStringUntil('\n');  // read new token
  if (input.equals("initializing"))
  {
    state = ISTATE;
    EEPROM.write(STATE_ADDR, state);
  }
  else if (input.equals("transmitting"))
  {
    state = TSTATE;
    EEPROM.write(STATE_ADDR, state);
  }
  else if (input.equals("landed"))
  {
    state = LSTATE;
    EEPROM.write(STATE_ADDR, state);
  }
//  else if (input.equals("sample"))
//  {
//    state = SSTATE; // becomes dormant
//    EEPROM.write(STATE_ADDR, ISTATE); // enters initializing state upon power-up
//    sampleInitialValues();
//  }
  else if (input.equals("picture")) ;//takePicture(); // take a picture
  else if (input.equals("download"))
  {
    state = SSTATE; // becomes dormant
    //read all pictures from Flash and send them to the proper serial port.  
  }
}

// sample initial values of BME 280 and pitot tube
void sampleInitialValues()
{
  groundPressure = pressure(); // store pressure on the ground
  //offset
}
