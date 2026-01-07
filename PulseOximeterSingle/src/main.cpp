#include <Arduino.h>
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

float thresholdVoltage = 5e-3; // Threshold voltage to count heart rate
float Voltage = 0; // Voltage
float avgVoltage = 0; // Rolling average voltage


void setup() 
{
  pinMode(inPin, INPUT); // Initialise voltage reading pin

  Serial.begin(9600);
  delay(500); // Delay for 500 ms so the serial monitor works properly

  Serial.println("Initialised Single-Wavelength Pulse Oximeter");

  void calculateAverageVoltage();
  void detectVoltageSpikes();
}
      

void loop() 
{
  calculateAverageVoltage();

  detectVoltageSpikes();

  calculateBPM();

  // Debugging
  Serial.print("Voltage: ");
  Serial.println(Voltage);  
  Serial.print("Average Voltage: ");
  Serial.println(avgVoltage);
}


// Calculate the rolling average voltage
void calculateAverageVoltage()
{
  Voltage = analogRead(inPin);
  avgVoltage = pox.reading(Voltage); // Find moving average voltage
}


// Detect if there are 5 voltage spikes in a row
void detectVoltageSpikes()
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

    Serial.println("BPM: " + BPM);
  }
}