#include <Arduino.h>
#include <Wire.h>

#include <movingAvg.h>

movingAvg pox(200); // Creates a moving average for the POX with a period of 200 ms

const byte inPin = A1; // Pin for the voltage reading

int beatDetected = 0; // bool which switches to 1 when a heartbeat is detected
int BPM = 0; // Heartbeats per min
int V0 = 0; // Past 5 voltage readings
int V1 = 0;
int V2 = 0;
int V3 = 0;
int V4 = 0;

long currentTimerCount = 0; // Current time
long previousTimerCount = 0; // Time at last beat

float thresholdVoltage = 0.01; // Threshold voltage in volts to trigger heart beat count 
float Voltage = 0; // Voltage
float avgVoltage = 0; // Rolling average voltage

float calculateAverageVoltage();
int detectVoltageSpikes();
void calculateBPM();

void setup() 
{
  pinMode(inPin, INPUT_PULLUP); // Initialise voltage reading pin

  Serial.begin(9600);
  delay(2000); // Delay for 2s so the serial monitor works properly

  Serial.println("Initialised Single-Wavelength Pulse Oximeter");
}
      

void loop() 
{
  calculateAverageVoltage();

  detectVoltageSpikes();

  calculateBPM();
  Serial.println(avgVoltage);
}


// Calculate the rolling average voltage
float calculateAverageVoltage()
{
  Voltage = analogRead(inPin);
  avgVoltage = pox.reading(Voltage); // Find moving average voltage

  return avgVoltage;
}


// Detect if there are 5 voltage spikes in a row
int detectVoltageSpikes()
{
  // A shitty way of storing the past 5 results
  if ((millis() % 5) == 0) {
    V0 = Voltage;
  }
  else if ((millis() % 5) == 1) {
    V1 = Voltage;
  }
  else if ((millis() % 5) == 2) {
    V2 = Voltage;
  }
  else if ((millis() % 5) == 3) {
    V3 = Voltage;
  }
  else if ((millis() % 5) == 4) {
    V4 = Voltage;
  }

  // if all values are greater than threshold voltage, count heartbeat. This stops false positives from noise
  if (V0 >= thresholdVoltage & V1 >= thresholdVoltage & V2 >= thresholdVoltage & V3 >= thresholdVoltage & V4 >= thresholdVoltage) {
    beatDetected = 1;
  }

  return beatDetected;
}


// Calculate BPM and print values
void calculateBPM()
{
  if (beatDetected == 1) 
  {
    currentTimerCount = millis(); // Set current timer count
    float timeDifference = (currentTimerCount - previousTimerCount) / 60000; // Time difference between previous and current count in minutes

    BPM = round(1 / timeDifference);

    previousTimerCount = millis(); // Set the previous timer count to the current time AFTER the BPM is calculated
    beatDetected = 0;

    Serial.println("heartbeat detected");
    Serial.println("BPM: " + BPM);
    Serial.println("------");
  }
}