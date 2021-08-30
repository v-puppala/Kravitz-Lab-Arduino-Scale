
/*
  Simple SD card datalogger
  Records digital pulses on A0 at 20Hz



*/

//Include libraries
#include <SPI.h>
#include <SD.h>
#include <vector>
#include <Wire.h>
#include "Adafruit_VL6180X.h"
#include <vector>
#include<unistd.h>
#include <numeric>
#include "HX711.h"

Adafruit_VL6180X vl = Adafruit_VL6180X();
#define DOUT A1
#define CLK A0
using namespace std;
unsigned long mill = millis();
boolean tared=0;

float lastReading = 0.0;
float scaleChange = 0.0;
float avg=0.0;
vector <float>logger;

float weightbuffer[180];
float lxbuffer[180];
float proxbuffer[180];
float currbuffer[180];
float avgweight_buffer[180];
boolean taredbuffer[180];
//vector<float> weightbuffer;
//vector<float> lxbuffer;
//vector <float> proxbuffer;
//vector<float> currbuffer;
//vector <float> avgweight_buffer;
int buffer1=0;

HX711 scale;
File myFile;
const int chipSelect = 4;
float calibration_factor = 4360;

int counter = 0;
int curr = millis();
double weight = scale.get_units();

//Set variables
// SPI needs a "chip select" pin for each device, in this case the SD card
int record = 0;                                                 // initialize "record" as a variable
char filename[16];


/* Change these values to set the current initial time */
const byte seconds = 0;
const byte minutes = 0;
const byte hours = 16;

/* Change these values to set the current initial date */
const byte day = 15;
const byte month = 6;
const byte year = 15;
// make a "char" type variable called "filename", containing 16 characters

void setup() {
  Serial.print("ello");

  //Set pinModes
  pinMode(13, OUTPUT);                                          //Set pin A0 as an INPUT to read from photoresistor
  pinMode(8, OUTPUT);                                           //Set pin A0 as an INPUT to read from photoresistor

  //Start Serial port
  Serial.begin(9600);

  scale.begin(DOUT,CLK);
  scale.set_scale(calibration_factor);

  //  scale.set_scale();// Open serial communications and wait for port to open:
  scale.tare(); //Reset the scale to 0
  Serial.print("Initializing SD card...");                      // Serial print

  // see if the card is present and can be initialized:

  Serial.println("card initialized.");

  //////////////////////////////////////////////////////////////////////////
  // Generate a unique filename
  // (code below from: https://forum.arduino.cc/index.php?topic=372248.0)

  //////////////////////////////////////////////////////////////////////////
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
     digitalWrite(8, HIGH);
     digitalWrite(13 , HIGH);
     while (1);
  }

  int n = 0;
  snprintf(filename, sizeof(filename), "new%03d.txt", n);      // includes a three-digit sequence number in the file name
  if (SD.exists(filename)) {
    n++;
    snprintf(filename, sizeof(filename), "new%03d.txt", n);
    Serial.print("New file created: ");
    Serial.println(filename);
  }

  Serial.println("Adafruit VL6180x test!");
  if (! vl.begin()) {
    Serial.println("Failed to find sensor");
   // while (1);
 }
  Serial.println("Sensor found!");

//  rtc.begin(); // initialize RTC
//
//  // Set the time
//  rtc.setHours(hours);
//  rtc.setMinutes(minutes);
//  rtc.setSeconds(seconds);
//
//  // Set the date
//  rtc.setDay(day);
//  rtc.setMonth(month);
//  rtc.setYear(year);
}

void loop() {
//
//    // Print date...
//  print2digits(rtc.getDay());
// Serial.print("/");
// print2digits(rtc.getMonth());
//  Serial.print("/");
//  print2digits(rtc.getYear());
//  Serial.print(" ");
//
//  // ...and time
//  print2digits(rtc.getHours());
//  Serial.print(":");
//  print2digits(rtc.getMinutes());
//  Serial.print(":");
//  print2digits(rtc.getSeconds());

  Serial.println();
  tared=0;
  float lux = vl.readLux(VL6180X_ALS_GAIN_5);
  uint8_t range = vl.readRange();

  //Write data to card
  // open the file with the name "filename"
  curr = millis();
  weight = scale.get_units();

  if (curr - mill > 60000 && range > 40) {
    Serial.print("flag");
    if (scaleChange < 10) {
        tared=1;
      scale.tare();
    }
    mill = curr;
    scaleChange = 0;
  }

  if (counter > 10) {
    //calculate moving average
    logger.erase(logger.begin());
     avg = accumulate(logger.begin(), logger.end(), 0.0) / 10;

    //Serial print
    Serial.print("Weight: ");
    Serial.print(weight, 2);
    Serial.print("Tared?:");
    Serial.print(tared);
    Serial.print("  Moving Avg: ");
    Serial.println(avg, 2);
    Serial.print("  Lux: ");
   Serial.print(lux);
    Serial.print("  Range: ");
   Serial.println(range);
   //Serial.print(buffer1);
   Serial.println(buffer1);
  if (buffer1==179){
    File dataFile = SD.open(filename, FILE_WRITE);

    for (int i=0;i<180;i++){
    dataFile.println("");
    dataFile.print(currbuffer[i]);
    dataFile.print(",");
    dataFile.print(weightbuffer[i], 2);
    dataFile.print(",");
    dataFile.print(avgweight_buffer[i], 2);
    dataFile.print(",");
    dataFile.print(lxbuffer[i]);
    dataFile.print(",");
    dataFile.print(proxbuffer[i]);
    dataFile.print(taredbuffer[i]);
    }
        dataFile.close();
//    weightbuffer[180]={0};
//    lxbuffer[180]={0};
//    proxbuffer[180]={0};
//    currbuffer[180]={0};
//    avgweight_buffer[180]={0};
    buffer1=0;
  }
    digitalWrite(8, HIGH);
    delay(50);
    digitalWrite(8, LOW);
    delay(50);
  }

  lastReading = weight;
  scaleChange += abs(weight - lastReading);
  counter++;
  buffer1++;
  logger.push_back(weight);

weightbuffer[buffer1]=weight;
lxbuffer[buffer1]=lux;
proxbuffer[buffer1]=range;
currbuffer[buffer1]=curr;
avgweight_buffer[buffer1]=avg;
taredbuffer[buffer1]=tared;
}

void print2digits(int number) {
  if (number < 10) {
    Serial.print("0"); // print a 0 before if the number is < than 10
  }
  Serial.print(number);
}
