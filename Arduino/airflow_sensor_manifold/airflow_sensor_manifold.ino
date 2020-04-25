/*
 * 
 * https://github.com/hydronics2/COVID-19-Airflow-Sensor-AFH55M12
 * 
 * PCB accepts dual footprint feather ESP32 and Adafruit ItsyBitsy M0
 * 
 * 
 * //4/15/20 - added SLM output
 * //4/21/20 - adjusted with Patrick's curve fit calbration equation.
 * //4/21/20 - added one ms to the volume calculation
 * 4/22/20 - updated with 2.8" display and teensy4.0
 * 4/23/20 - updated new SLM and volume and pressure


*/

#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h> // https://github.com/adafruit/Adafruit_Sensor
#include <Adafruit_BME280.h> //https://github.com/adafruit/Adafruit_BME280_Library

//#include <Adafruit_GFX.h>  // https://github.com/adafruit/Adafruit-GFX-Library
//#include <Adafruit_SSD1306.h>  // https://github.com/adafruit/Adafruit_SSD1306
//#include <avr/dtostrf.h> //for converting from an integer to a string for the display
#include "ILI9341_t3.h"


#define LTBLUE    0xB6DF
#define LTTEAL    0xBF5F
#define LTGREEN   0xBFF7
#define LTCYAN    0xC7FF
#define LTRED     0xFD34
#define LTMAGENTA 0xFD5F
#define LTYELLOW  0xFFF8
#define LTORANGE  0xFE73
#define LTPINK    0xFDDF
#define LTPURPLE  0xCCFF
#define LTGREY    0xE71C

#define BLUE      0x001F
#define TEAL      0x0438
#define GREEN     0x07E0
#define CYAN      0x07FF
#define RED       0xF800
#define MAGENTA   0xF81F
#define YELLOW    0xFFE0
#define ORANGE    0xFC00
#define PINK      0xF81F
#define PURPLE    0x8010
#define GREY      0xC618
#define WHITE     0xFFFF
#define BLACK     0x0000

#define DKBLUE    0x000D
#define DKTEAL    0x020C
#define DKGREEN   0x03E0
#define DKCYAN    0x03EF
#define DKRED     0x6000
#define DKMAGENTA 0x8008
#define DKYELLOW  0x8400
#define DKORANGE  0x8200
#define DKPINK    0x9009
#define DKPURPLE  0x4010
#define DKGREY    0x4A49



Adafruit_BME280 bme1; // I2C pressure sensor under the display
long atmosphericPressure = 101491; //this bounces around a bit

//pinouts for Teensy
#define TFT_CS 10
#define TFT_DC 9
#define TFT_RST 255 // RST can be set to -1 if you tie it to Arduino's reset
const int errorLED = 1; //teensy 4.0
const int userLED = 0; //teensy 4.0
const int buzzer = 23; //teensy 4.0
const int encoder0PinA = 20;  //teensy
const int encoder0PinB = 21;  //teensy
const int encoderButton = 22; //teensy
const int userButton = 5; //teensy 4.0 .. need to use pinMode(userButton, INPUT_PULLUP);
const int flowSensorPin = A0;


ILI9341_t3 tft = ILI9341_t3(TFT_CS, TFT_DC);

 
int flowSensorValue = 0;   

const int sizeBreath = 300;
int arrayBreath[sizeBreath];
int incrementBreath = 0;
int incrementGraphExtents = 0;

int arraySLM[sizeBreath];
long arrayVolume[sizeBreath];
int arrayPressure[sizeBreath];

boolean startBreathFlag = false;
boolean breathFlag = false;
int flowSensorThreshold = 1824; //temporary number
int averageRawSensorValue = 0;
long lastTimeSense;
int senseInterval = 20;
int updateDisplay = 40;
long lastTimeUpdateDisplay;

int flowSensorCalibration = 162; //actually number is 0.0162 converts delta ADC into SLM
double timePeriod;

int pressureReading1 = 0;

double a1, b1, c1, d1, r2, r1, vo, tempC, tempF, tempK, ox, oy;

//encoder variables
int encoder0Pos = 0;
int encoder0PinALast = LOW;
int n = LOW;


#define TCAADDR 0x70
void tcaselect(uint8_t i) {
  if (i > 7) return;
 
  Wire.beginTransmission(TCAADDR);
  Wire.write(1 << i);
  Wire.endTransmission();  
}

void setup()  // Start of setup
{        
  //while(!Serial){};
  Serial.begin(115200);
  analogReadResolution(12); //required for SAMD21 and teensy
  pinMode(flowSensorPin, INPUT);

  tft.begin();
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextColor(ILI9341_YELLOW);
  tft.setTextSize(2);
  tft.setRotation(1);
  //tft.println("Waiting for Arduino Serial Monitor...");
  
  
  pinMode(errorLED, OUTPUT);
  digitalWrite(errorLED, LOW);
  pinMode(userLED, OUTPUT);
  digitalWrite(userLED, LOW);
  pinMode(buzzer, OUTPUT);
  pinMode(userButton, INPUT);
  pinMode(userButton, INPUT_PULLUP);
  pinMode (encoder0PinA, INPUT);
  pinMode (encoder0PinB, INPUT);
  pinMode(encoderButton, INPUT);
  digitalWrite(encoderButton, HIGH); //enable pullup
  
  timePeriod = (double)senseInterval/1000; //time period in seconds
  Serial.print("sensing time period(seconds)= ");
  Serial.println(timePeriod);  
  delay(100);  // This delay is needed to let the display to initialize

  tcaselect(0);
  if (! bme1.begin(0x77, &Wire)) //this is the sensor under the display
  {
    Serial.println("Could not find a valid BME280 sensor, check wiring! for 0x77");
    while (1);
  }else Serial.println("started pressure sensor BME#1");

  pressureReading1 = bme1.readPressure(); //ok lets take a pressure reading now that there is flow
  Serial.print("pressure: ");
  Serial.println(pressureReading1);
  atmosphericPressure = pressureReading1;
}  


void loop()  // Start of loop
{
  long now = millis();
  if(now - lastTimeSense > senseInterval)
  {
    float SLM = 0; //Standard Liters per minute (flow rate)
    flowSensorValue = analogRead(flowSensorPin);
    //Serial.println(flowSensorValue);
    lastTimeSense = now;
    if(flowSensorValue > flowSensorThreshold)
    {
      if(startBreathFlag == false)
      {
        startBreathFlag = true;
        
        incrementBreath = 0;
      }else
      {
        SLM = constrain(17.816955 - .029497326 * flowSensorValue + 1.231863E-5 * sq(flowSensorValue),0,100);
        //float change = (constrain(17.816955-.029497326*arrayBreath[i]+1.231863E-5*sq((float)arrayBreath[i]),0,100))*(senseInterval+1)*1000/60000; //converts SLM to mL
        Serial.print("time: ");
        Serial.print(now);
        Serial.print(" ms, SLM: ");
        Serial.println(SLM);
        float pressure = bme1.readPressure(); //ok lets take a pressure reading now that there is flow
        pressure = (pressure - atmosphericPressure)/98.07; //convert Pascals to cm of H2O
        Serial.print("cm of H2O: ");
        Serial.println(pressure);        
        arrayPressure[incrementBreath] = (int)(pressure * 100); //package as integer
        

        
        digitalWrite(userLED, HIGH);     
        if(incrementBreath == 20) //debounce breaths... needs to be atleast 20 x 20ms or 400ms long
        {
          breathFlag = true;
          //displayBreath();
          digitalWrite(errorLED, HIGH);
          analogWrite(buzzer, 5);
        }
        arraySLM[incrementBreath] = SLM * 100; //package SLM as integer
        
        if(incrementBreath > 0)
        {
          arrayVolume[incrementBreath] = arrayVolume[incrementBreath - 1] + SLM * 100 *(senseInterval+1)*1000/60000; //converts SLM to mL
        }else arrayVolume[0] = SLM * 100 *(senseInterval+1)*1000/60000; //converts SLM to mL
        incrementBreath++;
        if(incrementBreath == sizeBreath){ //just for safety... should never rollover
          incrementBreath = 0;      
        }
      }
    }else 
    {
      digitalWrite(userLED, LOW);
      digitalWrite(errorLED, LOW);
      analogWrite(buzzer, 0);
      if(breathFlag) //a breath just finished... calculate the volume
      {        
        printResults();        
      }
      startBreathFlag = false;
      breathFlag = false;
    }    
  }

 //example encoder code
  n = digitalRead(encoder0PinA);
  if ((encoder0PinALast == LOW) && (n == HIGH)) {
    if (digitalRead(encoder0PinB) == LOW) {
      encoder0Pos--;
    } else {
      encoder0Pos++;
    }
    Serial.print (encoder0Pos);
    Serial.print ("/");
  }
  encoder0PinALast = n;

  if(digitalRead(userButton) == LOW) Serial.println("button Down");
}


void printResults()
{
  Serial.println();
  Serial.println("average raw sensor value is currently not used in curve-fit equation... but printed here for reference");
  Serial.print("average sensor value: ");
  Serial.println(averageRawSensorValue);
  Serial.print("volume: ");
  Serial.println(arrayVolume[incrementBreath-1]/100);

  int highestSlmValue = 0;
  for(int i=0; i < incrementBreath-1; i++) //evaluate all the flow readings we received over the breath
  {
    if(arraySLM[i] > highestSlmValue) highestSlmValue = arraySLM[i];
  }
  incrementGraphExtents = incrementBreath;
  incrementBreath = 0;

  int yAxisExtents = 50;
  int yAxisIncrements = 5;
  if(highestSlmValue/100 > 50) 
  {
    yAxisExtents = 100;
    yAxisIncrements = 10;
  }
  tft.fillScreen(BLACK);
  boolean display1 = true;
  for(int x = 0; x < incrementGraphExtents-1; x++)
  {
    Graph(tft, x, arraySLM[x]/100, 50 , 200, 280, 190, 0, incrementGraphExtents + 20, 10, 0, yAxisExtents, yAxisIncrements, "SLM", " ", "  ", DKBLUE, RED, GREEN, WHITE, BLACK, display1 );
  }
  delay(2000);
  int yAxisExtents2 = arrayVolume[incrementGraphExtents-2]/100 + 100;
  Serial.println(yAxisExtents2);
  tft.fillScreen(BLACK);
  boolean display2 = true;
  for(int x = 0; x < incrementGraphExtents-1; x++)
  {
    Graph(tft, x, arrayVolume[x]/100, 50 , 200, 280, 190, 0, incrementGraphExtents + 20, 10, 0, yAxisExtents2, 100, "Volume (mL)", " ", "  ", DKBLUE, RED, GREEN, WHITE, BLACK, display2 );    
  }
  delay(2000);


  tft.fillScreen(BLACK);
  boolean display3 = true;
  for(int x = 0; x < incrementGraphExtents-1; x++)
  {
    Graph(tft, x, arrayPressure[x]/100, 50 , 200, 280, 190, 0, incrementGraphExtents + 20, 10, -5, 20, 2, "Pressure (cm)", " ", "  ", DKBLUE, RED, GREEN, WHITE, BLACK, display3 );    
  }  
}  


/*
  function to draw a cartesian coordinate system and plot whatever data you want
  just pass x and y and the graph will be drawn

  huge arguement list
  &d name of your display object
  x = x data point
  y = y datapont
  gx = x graph location (lower left)
  gy = y graph location (lower left)
  w = width of graph
  h = height of graph
  xlo = lower bound of x axis
  xhi = upper bound of x asis
  xinc = division of x axis (distance not count)
  ylo = lower bound of y axis
  yhi = upper bound of y asis
  yinc = division of y axis (distance not count)
  title = title of graph
  xlabel = x asis label
  ylabel = y asis label
  gcolor = graph line colors
  acolor = axi ine colors
  pcolor = color of your plotted data
  tcolor = text color
  bcolor = background color
  &redraw = flag to redraw graph on fist call only
*/


void Graph(ILI9341_t3 &d, double x, double y, double gx, double gy, double w, double h, double xlo, double xhi, double xinc, int ylo, int yhi, double yinc, String title, String xlabel, String ylabel, unsigned int gcolor, unsigned int acolor, unsigned int pcolor, unsigned int tcolor, unsigned int bcolor, boolean &redraw) {

  double ydiv, xdiv;
  // initialize old x and old y in order to draw the first point of the graph
  // but save the transformed value
  // note my transform funcition is the same as the map function, except the map uses long and we need doubles
  //static double ox = (x - xlo) * ( w) / (xhi - xlo) + gx;
  //static double oy = (y - ylo) * (gy - h - gy) / (yhi - ylo) + gy;
  double i;
  double temp;
  int rot, newrot;

  if (redraw == true) {

    redraw = false;
    ox = (x - xlo) * ( w) / (xhi - xlo) + gx;
    oy = (y - ylo) * (gy - h - gy) / (yhi - ylo) + gy;
    // draw y scale
    for ( i = ylo; i <= yhi; i += yinc) {
      // compute the transform
      temp =  (i - ylo) * (gy - h - gy) / (yhi - ylo) + gy;

      if (i == 0) {
        d.drawLine(gx, temp, gx + w, temp, acolor);
      }
      else {
        d.drawLine(gx, temp, gx + w, temp, gcolor);
      }

      d.setTextSize(1);
      d.setTextColor(tcolor, bcolor);
      d.setCursor(gx - 40, temp);
      // precision is default Arduino--this could really use some format control
      d.println((int)i); //changed from float to int as don't need to print 0.00
    }
    // draw x scale
    for (i = xlo; i <= xhi; i += xinc) {

      // compute the transform

      temp =  (i - xlo) * ( w) / (xhi - xlo) + gx;
      if (i == 0) {
        d.drawLine(temp, gy, temp, gy - h, acolor);
      }
      else {
        d.drawLine(temp, gy, temp, gy - h, gcolor);
      }

      d.setTextSize(1);
      d.setTextColor(tcolor, bcolor);
      d.setCursor(temp, gy + 10);
      // precision is default Arduino--this could really use some format control
      d.println((int)i);  //changed from float to int as don't need to print 0.00
    }

    //now draw the labels
    d.setTextSize(2);
    d.setTextColor(tcolor, bcolor);
    d.setCursor(gx , gy - h - 30);
    d.println(title);

    d.setTextSize(1);
    d.setTextColor(acolor, bcolor);
    d.setCursor(gx , gy + 20);
    d.println(xlabel);

    d.setTextSize(1);
    d.setTextColor(acolor, bcolor);
    d.setCursor(gx - 30, gy - h - 10);
    d.println(ylabel);


  }

  //graph drawn now plot the data
  // the entire plotting code are these few lines...
  // recall that ox and oy are initialized as static above
  x =  (x - xlo) * ( w) / (xhi - xlo) + gx;
  y =  (y - ylo) * (gy - h - gy) / (yhi - ylo) + gy;
  d.drawLine(ox, oy, x, y, pcolor);
  d.drawLine(ox, oy + 1, x, y + 1, pcolor);
  d.drawLine(ox, oy - 1, x, y - 1, pcolor);
  ox = x;
  oy = y;

}
