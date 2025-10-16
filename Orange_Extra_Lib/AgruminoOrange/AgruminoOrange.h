/*
Custom Orange library gabriele.foddis@lifely.cc
last update 02/2023
*/

#ifndef AgruminoOrange_h
#define AgruminoOrange_h

#include "Arduino.h"

class AgruminoOrange {

  public:
    // Constructor
    AgruminoOrange();
    void setup();
    void deepSleepSec(unsigned int sec);
    
    // Public methods GPIO
    void turnWateringOn();
    void turnWateringOff(); // Default Off
    boolean isButtonPressed();
    boolean isAttachedToUSB(); 
    boolean isBatteryCharging();
    boolean isBoardOn();
    void turnBoardOn(); // Also call initBoard()
    void turnBoardOff(); 
    float readBatteryVoltage(); 
    unsigned int readBatteryLevel();
    
    // Public methods I2C
    float readTempC();
    float readTempF();
    void turnLedOn();
    void turnLedOff(); // Default Off
    boolean isLedOn();
    void toggleLed();
    unsigned int readSoil();
    unsigned int readSoilRaw();
    void calibrateSoilWater();
    void calibrateSoilAir();
    void calibrateSoilWater(unsigned int rawValue);
    void calibrateSoilAir(unsigned int rawValue);
    float readLux();
    
    void setSoilSensorVref(unsigned int vref);
 
  private:
    // Private methods
    void setupGpioModes();
    void printLogo();
    void initBoard();
    void initWire();
    void initTempSensor();
    void initSoilSensor();
    void initLuxSensor();
    float readBatteryVoltageSingleShot(); 
    boolean checkBattery();
};

#endif

