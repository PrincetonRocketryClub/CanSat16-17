#include <Wire.h> // I2C
#include <Adafruit_BME280.h> // Barometer/Altimeter/Thermometer
#include <EEPROM.h> // EEPROM built into the teensy

#define PKT_CNT_ADDR 10 // address of packet count in EEPROM
#define STATE_ADDR 11 // address in EEPROM of state
#define RTC_ADDR 0x68 // Wire id of RTC
#define VP A2 // positive side of the voltmeter
#define BUZZERPIN 17 // buzzer pin
#define USART_SERIAL Serial2 // Which Serial port we're using
#define USART_SERIALEVENT serialEvent2
#define NICR_PORT1 6 // The port controlling NiCr wire 1
#define NICR_PORT2 7
#define NICR_PORT3 8

#define ISTATE 1 // Initializing
#define TSTATE 2 // Transmitting
#define RSTATE 3 // Releasing
#define LSTATE 4 // Landed
#define DSTATE 5 // Dormant

#define NICR_TIME 20 // How long to heat NiCr wire
#define VLTG_FACTOR 6 // 1 / how much the voltage divider scales the signal by
#define V_REF 2.56 // internal reference voltage
#define ALT_ERROR 10 // How high can the altimeter read if alt actually equals 0?
#define MB_PER_PASCAL .01 // converts Pascals to mb
#define RELEASE_ZONE_ERROR 60 // release glider when alt is 400 + or - RELEASE_ZONE_ERROR

Adafruit_BME280 bme; // Barometer/Altimeter/Thermometer

double groundPressure; // used to calculate altitude
short state; // current state
int rStartTime; // time at which release sequence started

void setup() {
  USART_SERIAL.begin(9600);
  bme.begin();
  Wire.begin();
  
  pinMode(BUZZERPIN, OUTPUT);
  pinMode(NICR_PORT1, OUTPUT);
  pinMode(NICR_PORT2, OUTPUT);
  pinMode(NICR_PORT3, OUTPUT);

  state = EEPROM.read(STATE_ADDR); // load state upon power-up
}

void loop() {
  int alt1, alt2; // used in TSTATE and DSTATE
  int rTime; // time since starting release sequence
  switch(state){
    case ISTATE: // Initializing
        groundPressure = bme.readPressure() * MB_PER_PASCAL; // store pressure on the ground
        rst_rtc(); // reset mission time
        EEPROM.write(PKT_CNT_ADDR, 0); // reset packet count
        state = TSTATE; // state <-- transmitting
        EEPROM.write(STATE_ADDR, state);
        break;
    case TSTATE: // transmitting
      USART_SERIAL.println(packet());
      delay(1000);
    
      alt1 = alt();
      if (alt1 > 400 - RELEASE_ZONE_ERROR && alt1 < 400 + RELEASE_ZONE_ERROR)
      {
        state = RSTATE; // start release sequence
        EEPROM.write(STATE_ADDR, state);
        rStartTime = rtc();
      }
      break;
    case RSTATE:
      USART_SERIAL.println(packet());
      delay(1000);
      rTime = rtc() - rStartTime;

      digitalWrite(NICR_PORT1, LOW);
      digitalWrite(NICR_PORT2, LOW);
      digitalWrite(NICR_PORT3, LOW);
      if (rTime < NICR_TIME)
        digitalWrite(NICR_PORT1, HIGH);
      if (2 < rTime && rTime < NICR_TIME + 2)
        digitalWrite(NICR_PORT2, HIGH);
      if (4 < rTime && rTime < NICR_TIME + 4)
        digitalWrite(NICR_PORT3, HIGH);
      if (NICR_TIME + 6 < rTime) // enter release state 2 seconds after releasing the glider
      {
        state = DSTATE;
        EEPROM.write(STATE_ADDR, state);
      }
      break;
    case DSTATE:
      alt1 = alt();
      delay(1000);
      alt2 = alt();
      if (abs(alt1) < ALT_ERROR && abs(alt1 - alt2) < ALT_ERROR) // if alt is close to zero and constant
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
}

// This method is written to make testing easier.
// It should be rewritten after testing is done.
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
  else if (input.equals("releasing")) // Release Override Command
  {
    state = RSTATE;
    EEPROM.write(STATE_ADDR, state);
    rStartTime = rtc(); // reset start time
  }
}
