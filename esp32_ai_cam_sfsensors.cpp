/*
  esp32-ai-cam

    by James Zahary    jamzah.plc@gmail.com

    https://github.com/jameszah/ESP32-AI_CAM

    jameszah/ESP32-AI_CAM is licensed under the
    GNU General Public License v3.0

*/

#include <Wire.h>

#include "SparkFunBME280.h"
#include <SparkFunTSL2561.h>
#include <ADXL345.h>

BME280 sf_bme; //Uses default I2C address 0x77
SFE_TSL2561 sf_tsl;
ADXL345 sf_adxl;

#define I2C_SDA 0
#define I2C_SCL 13

boolean gain;     // Gain setting, 0 = X1, 1 = X16;
unsigned int ms;  // Integration ("shutter") time in milliseconds

void sf_setup() {
  Serial.println("\nI2C Scanner");

  Wire.begin(I2C_SDA, I2C_SCL);
  byte error, address;
  int nDevices;

  Serial.println("Scanning...");

  nDevices = 0;
  for (address = 1; address < 127; address++ )
  {
    // The i2c_scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0)
    {
      Serial.print("I2C device found at address 0x");
      if (address < 16)
        Serial.print("0");
      Serial.print(address, HEX);
      Serial.println("  !");
      nDevices++;
    }
    else if (error == 4)
    {
      Serial.print("Unknown error at address 0x");
      if (address < 16)
        Serial.print("0");
      Serial.println(address, HEX);
    }
  }
  if (nDevices == 0)
    Serial.println("No I2C devices found\n");
  else
    Serial.println("done\n");

  delay(100);
  Serial.println("BME  section ");
  sf_bme.setI2CAddress(0x76);
  if (sf_bme.beginI2C() == false) Serial.println("Sensor A connect failed");

  Serial.print("Humidity: ");
  Serial.print(sf_bme.readFloatHumidity(), 1);

  Serial.print(" Pressure: ");
  Serial.print(sf_bme.readFloatPressure(), 0);

  Serial.print(" Temp: ");
  Serial.println(sf_bme.readTempC(), 2);

  Serial.println("TSL  section ");
  sf_tsl.begin();
  unsigned char ID;

  if (sf_tsl.getID(ID)) {
    //Serial.print("Got factory ID: 0X");
    //Serial.print(ID, HEX);
    //Serial.println(", should be 0X5X");
  } else {
    byte error = sf_tsl.getError();
    Serial.println(error);
  }

  // If gain = false (0), device is set to low gain (1X)
  // If gain = high (1), device is set to high gain (16X)

  gain = 0;

  // If time = 0, integration will be 13.7ms
  // If time = 1, integration will be 101ms
  // If time = 2, integration will be 402ms
  // If time = 3, use manual start / stop to perform your own integration

  unsigned char time = 2;

  // setTiming() will set the third parameter (ms) to the
  // requested integration time in ms (this will be useful later):

  Serial.println("Set timing...");
  sf_tsl.setTiming(gain, time, ms);

  // To start taking measurements, power up the sensor:

  //Serial.println("Powerup...");
  sf_tsl.setPowerUp();

  unsigned int data0, data1;

  if (sf_tsl.getData(data0, data1)) {
    // getData() returned true, communication was successful

    //Serial.print("data0: "); Serial.print(data0);
    //Serial.print(" data1: "); Serial.print(data1);

    // To calculate lux, pass all your settings and readings
    // to the getLux() function.

    // The getLux() function will return 1 if the calculation
    // was successful, or 0 if one or both of the sensors was
    // saturated (too much light). If this happens, you can
    // reduce the integration time and/or gain.
    // For more information see the hookup guide at: https://learn.sparkfun.com/tutorials/getting-started-with-the-tsl2561-luminosity-sensor

    double lux;    // Resulting lux value
    boolean good;  // True if neither sensor is saturated

    // Perform lux calculation:

    good = sf_tsl.getLux(gain, ms, data0, data1, lux);

    // Print out the results:

    Serial.print(" lux: ");
    Serial.print(lux);
    if (good) Serial.println(" (good)"); else Serial.println(" (BAD)");
  }

  Serial.println("ADXL section ");

  sf_adxl.powerOn();

  //set activity/ inactivity thresholds (0-255)
  
  //sf_adxl.setActivityThreshold(75); //62.5mg per increment
  sf_adxl.setActivityThreshold(55); //62.5mg per increment
  sf_adxl.setInactivityThreshold(75); //62.5mg per increment
  sf_adxl.setTimeInactivity(10); // how many seconds of no activity is inactive?

  //look of activity movement on this axes - 1 == on; 0 == off
  sf_adxl.setActivityX(1);
  sf_adxl.setActivityY(1);
  sf_adxl.setActivityZ(1);

  //look of inactivity movement on this axes - 1 == on; 0 == off
  sf_adxl.setInactivityX(1);
  sf_adxl.setInactivityY(1);
  sf_adxl.setInactivityZ(1);

  //look of tap movement on this axes - 1 == on; 0 == off
  sf_adxl.setTapDetectionOnX(1);
  sf_adxl.setTapDetectionOnY(1);
  sf_adxl.setTapDetectionOnZ(1);

  //set values for what is a tap, and what is a double tap (0-255)
  sf_adxl.setTapThreshold(50); //62.5mg per increment
  sf_adxl.setTapDuration(15); //625us per increment
  sf_adxl.setDoubleTapLatency(80); //1.25ms per increment
  sf_adxl.setDoubleTapWindow(200); //1.25ms per increment

  //set values for what is considered freefall (0-255)
  sf_adxl.setFreeFallThreshold(7); //(5 - 9) recommended - 62.5mg per increment
  sf_adxl.setFreeFallDuration(45); //(20 - 70) recommended - 5ms per increment

  //setting all interrupts to take place on int pin 1
  //I had issues with int pin 2, was unable to reset it
  sf_adxl.setInterruptMapping( ADXL345_INT_SINGLE_TAP_BIT,   ADXL345_INT2_PIN );
  sf_adxl.setInterruptMapping( ADXL345_INT_DOUBLE_TAP_BIT,   ADXL345_INT1_PIN );
  sf_adxl.setInterruptMapping( ADXL345_INT_FREE_FALL_BIT,    ADXL345_INT1_PIN );
  sf_adxl.setInterruptMapping( ADXL345_INT_ACTIVITY_BIT,     ADXL345_INT1_PIN );
  sf_adxl.setInterruptMapping( ADXL345_INT_INACTIVITY_BIT,   ADXL345_INT1_PIN );

  //register interrupt actions - 1 == on; 0 == off
  sf_adxl.setInterrupt( ADXL345_INT_SINGLE_TAP_BIT, 1);
  sf_adxl.setInterrupt( ADXL345_INT_DOUBLE_TAP_BIT, 0);
  sf_adxl.setInterrupt( ADXL345_INT_FREE_FALL_BIT,  0);
  sf_adxl.setInterrupt( ADXL345_INT_ACTIVITY_BIT,   1);
  sf_adxl.setInterrupt( ADXL345_INT_INACTIVITY_BIT, 0);


  int x, y, z;
  sf_adxl.readXYZ(&x, &y, &z); //read the accelerometer values and store them in variables  x,y,z
  // Output x,y,z values
  Serial.print("values of X , Y , Z: ");
  Serial.print(x);
  Serial.print(" , ");
  Serial.print(y);
  Serial.print(" , ");
  Serial.println(z);

  double xyz[3];
  double ax, ay, az;
  sf_adxl.getAcceleration(xyz);
  ax = xyz[0];
  ay = xyz[1];
  az = xyz[2];
  Serial.print("X=");
  Serial.print(ax);
  Serial.println(" g");
  Serial.print("Y=");
  Serial.print(ay);
  Serial.println(" g");
  Serial.print("Z=");
  Serial.print(az);
  Serial.println(" g");
  Serial.println("**********************");
}

void en_inter(){
    //register interrupt actions - 1 == on; 0 == off
  sf_adxl.setInterrupt( ADXL345_INT_SINGLE_TAP_BIT, 1);
  sf_adxl.setInterrupt( ADXL345_INT_DOUBLE_TAP_BIT, 0);
  sf_adxl.setInterrupt( ADXL345_INT_FREE_FALL_BIT,  0);
  sf_adxl.setInterrupt( ADXL345_INT_ACTIVITY_BIT,   1);
  sf_adxl.setInterrupt( ADXL345_INT_INACTIVITY_BIT, 0);
}

void dis_inter(){
    //register interrupt actions - 1 == on; 0 == off
  sf_adxl.setInterrupt( ADXL345_INT_SINGLE_TAP_BIT, 0);
  sf_adxl.setInterrupt( ADXL345_INT_DOUBLE_TAP_BIT, 0);
  sf_adxl.setInterrupt( ADXL345_INT_FREE_FALL_BIT,  0);
  sf_adxl.setInterrupt( ADXL345_INT_ACTIVITY_BIT,   0);
  sf_adxl.setInterrupt( ADXL345_INT_INACTIVITY_BIT, 0);
}


float get_bme_temperature() {
  float good = sf_bme.readTempC();
  Serial.printf("Temp is %05.1f\n",good);
  return (good);
}
float get_bme_pressure() {
  float good = sf_bme.readFloatPressure()/1000;
  Serial.printf("Pressure is %05.1f\n",good);
  return (good);
}
float get_bme_humidity() {
  float good = sf_bme.readFloatHumidity();
  Serial.printf("Humid is %05.1f\n",good);
  return (good);
}
float get_tsl_lux() {
  unsigned int data0, data1;

  if (sf_tsl.getData(data0, data1)) {

    double lux;    // Resulting lux value
    boolean good;  // True if neither sensor is saturated

    // Perform lux calculation:

    good = sf_tsl.getLux(gain, ms, data0, data1, lux);
    Serial.printf("Lux is %05.1f\n",lux);

    return (lux);
  }
}

void ADXL_ISR() {
  
  // getInterruptSource clears all triggered actions after returning value
  // Do not call again until you need to recheck for triggered actions
  byte interrupts = sf_adxl.getInterruptSource();
  
  // Free Fall Detection
  if(sf_adxl.triggered(interrupts, ADXL345_FREE_FALL)){
    Serial.println("*** FREE FALL ***");
    //add code here to do when free fall is sensed
  } 
  
  // Inactivity
  if(sf_adxl.triggered(interrupts, ADXL345_INACTIVITY)){
    Serial.println("*** INACTIVITY ***");
     //add code here to do when inactivity is sensed
  }
  
  // Activity
  if(sf_adxl.triggered(interrupts, ADXL345_ACTIVITY)){
    Serial.println("*** ACTIVITY ***"); 
     //add code here to do when activity is sensed
  }
  
  // Double Tap Detection
  if(sf_adxl.triggered(interrupts, ADXL345_DOUBLE_TAP)){
    Serial.println("*** DOUBLE TAP ***");
     //add code here to do when a 2X tap is sensed
  }
  
  // Tap Detection
  if(sf_adxl.triggered(interrupts, ADXL345_SINGLE_TAP)){
    Serial.println("*** TAP ***");
     //add code here to do when a tap is sensed
  } 
}
