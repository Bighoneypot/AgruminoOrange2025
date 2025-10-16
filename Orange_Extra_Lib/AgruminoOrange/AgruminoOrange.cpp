/*
  Agrumino.cpp - Library for Agrumino board - Version 0.4 for Board RV4A
  Created by giuseppe.broccia@lifely.cc on October 2017.
  Updated on March 2018

  For details @see AgruminoOrange.h
*/

#include "AgruminoOrange.h"
#include <Wire.h>
#include "libraries/MCP9800/MCP9800.cpp"
#include "libraries/MCP3221/MCP3221.cpp"

// PINOUT Agrumino        Implemented
#define PIN_SDA          2 // [X] BOOT: Must be HIGH at boot
#define PIN_SCL         14 // [X] 
#define PIN_LED         12 // [X] 
#define PIN_BTN_S1       4 // [X] Same as Internal WT8266 LED
#define PIN_USB_DETECT   5 // [X] 
#define PIN_MOSFET      15 // [X] BOOT: Must be LOW at boot
#define PIN_BATT_STAT   13 // [X] 
#define PIN_LEVEL        0 // [ ] BOOT: HIGH for Running and LOW for Program

// Addresses I2C sensors       Implemented
#define I2C_ADDR_SOIL      0x4D // [X] TODO: Save air and water values to EEPROM
#define I2C_ADDR_LUX       0x44 // [X] 
#define I2C_ADDR_TEMP      0x48 // [X] 
#define DELAY_ADDR_LUX      110 // delay for new Lux sensor

////////////
// CONFIG //
////////////

// Soil

///this value are only for regular device
#define DEFAULT_MV_SOIL_RAW_AIR      1899 // Millivolt reading of a Lifely Orange  Capacitive, sensor in the air 12/02/2024 Gabriel
#define DEFAULT_MV_SOIL_RAW_WATER    1350 // Millivolt reading of a Lifely R4 Capacitive Flat, sensor immersed by signed level 12/02/2024 Gabriel

///this value are only for long device
//#define DEFAULT_MV_SOIL_RAW_AIR      1980 // Millivolt reading of a Lifely Orange  Capacitive, sensor in the air 12/02/2024 Gabriel
//#define DEFAULT_MV_SOIL_RAW_WATER    1290 // Millivolt reading of a Lifely R4 Capacitive Flat, sensor immersed by signed level 12/02/2024 Gabriel
// Battery
#define BATTERY_MV_LEVEL_0           2700 // Voltage in millivolt of a fully discharged battery value min to correct response is 2.7
#define BATTERY_MV_LEVEL_100         3000 // Voltage in millivolt of a fully charged battery
#define BATTERY_VOLT_DIVIDER_Z1      1800 // Value of the Z1(R25) resistor in the voltage divider used for read the batt voltage
#define BATTERY_VOLT_DIVIDER_Z2       424 // Value of the Z2(R26) resistor (470 original). Adjusted considering the ADC internal resistance
#define BATTERY_VOLT_SAMPLES           20 // Number of reading (samples) needed to calculate the battery voltage

///////////////
// Variables //
///////////////

MCP9800 mcpTempSensor;
MCP3221 mcpSoilSensor(I2C_ADDR_SOIL);
unsigned int millivoltSoilRawAir = DEFAULT_MV_SOIL_RAW_AIR;
unsigned int millivoltSoilRawWater = DEFAULT_MV_SOIL_RAW_WATER;

/////////////////
// Constructor //
/////////////////

AgruminoOrange::AgruminoOrange() {
}

void AgruminoOrange::setup() {
  setupGpioModes();
  printLogo();
  // turnBoardOn(); // Decomment to have the board On by Default
}

void AgruminoOrange::deepSleepSec(unsigned int sec) {
  if (sec > 4294) {
    // ESP.deepSleep argument is an unsigned int, so the max allowed walue is 0xffffffff (4.294.967.295).
    sec = 4294;
    Serial.println("Warning: deepSleep can be max 4294 sec (~71 min). Value has been constrained!");
  }

  Serial.print("\nGoing to deepSleep for ");
  Serial.print(sec);
  Serial.println(" seconds... (ー。ー) zzz\n");
  ESP.deepSleep(sec * 1000000L); // microseconds
  // deepSleep is not executed istantly to this delay fix the issue
  delay(1000);
}

/////////////////////////
// Public methods GPIO //
/////////////////////////

boolean AgruminoOrange::isAttachedToUSB() {
  return digitalRead(PIN_USB_DETECT) == LOW;
}

// Return true if the battery is in charging state
boolean AgruminoOrange::isBatteryCharging() {
  return digitalRead(PIN_BATT_STAT) == LOW;
}

boolean AgruminoOrange::isButtonPressed() {
  return digitalRead(PIN_BTN_S1) == LOW;
}

boolean AgruminoOrange::isBoardOn() {
  return digitalRead(PIN_MOSFET) == HIGH;
}

void AgruminoOrange::turnBoardOn() {
  if (!isBoardOn()) {
    digitalWrite(PIN_MOSFET, HIGH);
    delay(5); // Ensure that the ICs are booted up properly
    initBoard();
    // checkBattery(); TODO!!!
  }
}

void AgruminoOrange::turnBoardOff() {
  digitalWrite(PIN_MOSFET, LOW);
}

void AgruminoOrange::turnWateringOn() {
  // Not available on orange
  // digitalWrite(PIN_PUMP, HIGH);
}

void AgruminoOrange::turnWateringOff() {
  // Not available on orange
  // digitalWrite(PIN_PUMP, LOW);
}

////////////////////////
// Public methods I2C //
////////////////////////

float AgruminoOrange::readTempC() {
  return mcpTempSensor.readCelsiusf();
}

float AgruminoOrange::readTempF() {
  return mcpTempSensor.readFahrenheitf();
}

void AgruminoOrange::turnLedOn() {
  digitalWrite(PIN_LED, HIGH);
}

void AgruminoOrange::turnLedOff() {
  digitalWrite(PIN_LED, LOW);
}

boolean AgruminoOrange::isLedOn() {
  return digitalRead(PIN_LED) == HIGH;
}

void AgruminoOrange::toggleLed() {
  digitalWrite(PIN_LED, !digitalRead(PIN_LED));
}

void AgruminoOrange::calibrateSoilWater() {
  millivoltSoilRawWater = readSoilRaw();
}

void AgruminoOrange::calibrateSoilAir() {
  millivoltSoilRawAir = readSoilRaw();
}

void AgruminoOrange::calibrateSoilWater(unsigned int rawValue) {
  millivoltSoilRawWater = rawValue;
}

void AgruminoOrange::calibrateSoilAir(unsigned int rawValue) {
  millivoltSoilRawAir = rawValue;
}

unsigned int AgruminoOrange::readSoil() {
  unsigned int soilRaw = readSoilRaw();
  soilRaw = constrain(soilRaw, millivoltSoilRawWater, millivoltSoilRawAir);
  return map(soilRaw, millivoltSoilRawAir, millivoltSoilRawWater, 0, 100);
}

float AgruminoOrange::readLux() {
  // Logic for Light-to-Digital Output Sensor ISL29003
  Wire.beginTransmission(I2C_ADDR_LUX);
  Wire.write(0x02); // Data registers are 0x02->LSB and 0x03->MSB
  Wire.endTransmission();
  Wire.requestFrom(I2C_ADDR_LUX, 2); // Request 2 bytes of data
  unsigned int data;
  if (Wire.available() == 2) {
    byte lsb = Wire.read();
    byte  msb = Wire.read();
    data = (msb << 8) | lsb;
  } else {
    Serial.println("readLux Error!");
    return 0;
  }
  // Convert the data from the ADC to lux
  // 0-64000 is the selected range of the ALS (Lux)
  // 0-65536 is the selected range of the ADC (16 bit)
  return (64000.0 * (float) data) / 65536.0;
}

float AgruminoOrange::readBatteryVoltage() {
  float voltSum = 0.0;
  for (int i = 0; i < BATTERY_VOLT_SAMPLES; ++i) {
    voltSum += readBatteryVoltageSingleShot();
  }
  float volt = voltSum / BATTERY_VOLT_SAMPLES;
  // Serial.println("readBatteryVoltage: " + String(volt) + " V");
  return volt;
}

unsigned int AgruminoOrange::readBatteryLevel() {
  float voltage = readBatteryVoltage();
  unsigned int milliVolt = (int) (voltage * 1000.0);
  milliVolt = constrain(milliVolt, BATTERY_MV_LEVEL_0, BATTERY_MV_LEVEL_100);
  return map(milliVolt, BATTERY_MV_LEVEL_0, BATTERY_MV_LEVEL_100, 0, 100);
}

/////////////////////
// Private methods //
/////////////////////

void AgruminoOrange::initTempSensor() {
  Serial.print("initTempSensor   → ");
  boolean success = mcpTempSensor.init(true);
  if (success) {
    mcpTempSensor.setResolution(MCP_ADC_RES_11); // 11bit (0.125c)
    mcpTempSensor.setOneShot(true);
    Serial.println("OK");
  } else {
    Serial.println("FAIL!");
  }
}

void AgruminoOrange::initSoilSensor() {
  Serial.print("initSoilSensor   → ");
  byte response = mcpSoilSensor.ping();
  if (response == 0) {
    mcpSoilSensor.reset();
    mcpSoilSensor.setSmoothing(EMAVG);
    mcpSoilSensor.setVref(3300); // This will make the reading of the MCP3221 voltage accurate.
    Serial.println("OK");
  } else {
    Serial.println("FAIL!");
  }
}

void AgruminoOrange::setSoilSensorVref(unsigned int vref) {
    mcpSoilSensor.setVref(vref);
}

void AgruminoOrange::initLuxSensor() {
  // Logic for Light-to-Digital Output Sensor ISL29003
  Serial.println("Lux Ver. " + String(DELAY_ADDR_LUX));
  Serial.print("initLuxSensor    → ");
  Wire.beginTransmission(I2C_ADDR_LUX);
  delay(DELAY_ADDR_LUX);
  byte result = Wire.endTransmission();
  if (result == 0) {
    Wire.beginTransmission(I2C_ADDR_LUX);
    Wire.write(0x00); // Select "Command-I" register
    Wire.write(0xA0); // Select "ALS continuously" mode
    Wire.endTransmission();
    Wire.beginTransmission(I2C_ADDR_LUX);
    Wire.write(0x01); // Select "Command-II" register
    Wire.write(0x03); // Set range = 64000 lux, ADC 16 bit
    Wire.endTransmission();
    Serial.println("OK");
  } else {
    Serial.println("FAIL!");
  }
}

unsigned int AgruminoOrange::readSoilRaw() {
  return mcpSoilSensor.getVoltage();
}

float AgruminoOrange::readBatteryVoltageSingleShot() {
  float z1 = (float) BATTERY_VOLT_DIVIDER_Z1;
  float z2 = (float) BATTERY_VOLT_DIVIDER_Z2;
  float vread = (float) analogRead(A0) / 1024.0; // RAW Value from the ADC, Range 0-1V
  float volt = ((z1 + z2) / z2) * vread;
  return volt;
}

void AgruminoOrange::initWire() {
  Wire.begin(PIN_SDA, PIN_SCL);
}

void AgruminoOrange::initBoard() {
  initWire();
  initLuxSensor();  // Boot time depends on the selected ADC resolution (16bit first reading after ~90ms)
  initSoilSensor(); // First reading after ~30ms
  initTempSensor(); // First reading after ~?ms
  delay(DELAY_ADDR_LUX); // Ensure that the ICs are init properly
}

void AgruminoOrange::setupGpioModes() {
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_BTN_S1, INPUT_PULLUP);
  pinMode(PIN_USB_DETECT, INPUT);
  pinMode(PIN_MOSFET, OUTPUT);
  pinMode(PIN_BATT_STAT, INPUT);
}

void AgruminoOrange::printLogo() {
  Serial.println("\n\n\n");
  Serial.println(" ____________________________________________");
  Serial.println("/\\      _                       _            \\");
  Serial.println("\\_|    /_\\  __ _ _ _ _  _ _ __ (_)_ _  ___   |");
  Serial.println("  |   / _ \\/ _` | '_| || | '  \\| | ' \\/ _ \\  |");
  Serial.println("  |  /_/ \\_\\__, |_|  \\_,_|_|_|_|_|_||_\\___/  |");
  Serial.println("  |        |___/           ORANGE By Lifely  |");
  Serial.println("  |  ________________________________________|_");
  Serial.println("  \\_/_________________________________________/");
  Serial.println("");
}
