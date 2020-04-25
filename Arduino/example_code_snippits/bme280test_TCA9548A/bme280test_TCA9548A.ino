/***************************************************************************
The device's I2C address is either 0x76 or 0x77.

 ***************************************************************************/

#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#define BME_SCK 13
#define BME_MISO 12
#define BME_MOSI 11
#define BME_CS 10

#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BME280 bme1; // I2C
Adafruit_BME280 bme2; // I2C

#define TCAADDR 0x70
void tcaselect(uint8_t i) {
  if (i > 7) return;
 
  Wire.beginTransmission(TCAADDR);
  Wire.write(1 << i);
  Wire.endTransmission();  
}

unsigned long delayTime;

void setup() {
    Serial.begin(9600);
    while(!Serial);    // time to get serial running
    //Serial.println(F("BME280 test"));

    unsigned status;
     tcaselect(0);
     delay(10);

    if (! bme1.begin(0x77, &Wire)) {
        Serial.println("Could not find a valid BME280 sensor, check wiring! for 0x77");
        while (1);
    } else //Serial.println("started#1");

     tcaselect(4);
     delay(10);

    if (! bme1.begin(0x77, &Wire)) {
        Serial.println("Could not find a valid BME280 sensor, check wiring! for 0x77");
        while (1);
    } else //Serial.println("started#5");

    Serial.println();
}


void loop() { 
    printValues();
    //delay(1000);
}


void printValues() {

  tcaselect(0);
  delay(10);
  //Serial.print("Temperature#1 = ");
  //Serial.print(bme1.readTemperature());
  //Serial.println(" *C");
  //Serial.print("Pressure#1 = ");
  Serial.println((bme1.readPressure() - 103125));
  //Serial.println(" Pa");
  //Serial.print("Humidity#1 = ");
  //Serial.print(bme1.readHumidity());
  //Serial.println(" %");
  //Serial.println();
  
//  delay(5);
//  tcaselect(4);
//  Serial.print("Temperature#5 = ");
//  Serial.print(bme1.readTemperature());
//  Serial.println(" *C");
//  Serial.print("Pressure#5 = ");
//  Serial.print(bme1.readPressure());
//  Serial.println(" Pa");
//  Serial.print("Humidity#5 = ");
//  Serial.print(bme1.readHumidity());
//  Serial.println(" %");
//  Serial.println();
//  Serial.println(); 
}
