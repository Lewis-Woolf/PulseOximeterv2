#include <Arduino.h>
#include <Wire.h>
// Make sure to install the MAX30100lib by OXullo Intersecans
#include "MAX30100_PulseOximeter.h"

PulseOximeter pox; // High level interface to the sensor

const byte greenLEDPin = A2; // Pin for the green LED
const byte buttonPin = A1; // Pin for button B

int BPM = 0; // Heartbeats per min
int poxState = -1; // Determines if the pox is on (1) or off (-1)
int buttonState = 1; // Determines if the button is pressed down or not
int lastButtonState = 1; // The last state the button was in
int greenLEDState = LOW; // State of the LED

long reportingPeriod = 5000; // Time between information being printed
long lastReportTime = 0; // Time since last report
long lastDebounceTime = 0; // Last time the output pin was toggled
long debounceDelay = 50; // Debounce time to mitigate button noise

void printPOXResults();
void setPOXState();
void onBeatDetected();

void setup() 
{
  pinMode(buttonPin, INPUT); // Initialise button A pin
  pinMode(greenLEDPin, OUTPUT); // Initialise green LED pin

  Serial.begin(9600);
  delay(2000); // Delay for 2s so the serial monitor works properly

  Serial.println("Initialising Dual-Wavelength Pulse Oximeter...");

  // Check that the pulse oximeter is working
  if (!pox.begin()) {
    Serial.println("FAILURE");
  }
  else {
    Serial.println("SUCCESS");
    Serial.println("Press button B to turn POX on/off");
  }

  // Register the callback for the beat detection function
  pox.setOnBeatDetectedCallback(onBeatDetected);
}


// Print text when beat detected
void onBeatDetected()
{
  if (poxState == 1)
  {
    Serial.println("Beat detected");
  }
}


void loop() 
{
  pox.update(); // Update POX

  setPOXState(); // Set the state of the POX and the green LED

  printPOXResults(); // Print the POX results on the serial monitor
}


// Set the state of the POX and the green LED
void setPOXState()
{
  digitalWrite(greenLEDPin, greenLEDState); // Set the state of the green LED

  int reading = digitalRead(buttonPin);

  if (reading != lastButtonState) { // If the button state changes
    lastDebounceTime = millis(); // reset debouncing timer
  }

  if ((millis() - lastDebounceTime) > debounceDelay) // If a reading has been around for an amount of time equal to debounce delay has passed, assume it is correct
  {
    if (reading != buttonState) // If the button state has changed
    {
      buttonState = reading;

      if (buttonState == LOW); // If the button is being pressed
      {
        if (lastButtonState == 1) // If the last state was off
        {
          poxState = -poxState;
          if (greenLEDState == LOW) { // If LED is off, turn it on
            greenLEDState = HIGH;
          }
          else { // If LED is on, turn it off
            greenLEDState = LOW;
          }
        }
      }
    }
  }

  lastButtonState = reading;
}


// Print the POX results on the serial monitor
void printPOXResults()
{
  if (poxState == 1)
  {
    // Print heart rate and oxidation levels every reporting period
    if (millis() - lastReportTime > reportingPeriod)
    {
      Serial.println("--------------"); // Spacing
      Serial.print("BPM: "); // Print heart rate
      Serial.println(pox.getHeartRate());
      Serial.print("SpO2: "); // Print oxidation levels
      Serial.print(pox.getSpO2());
      Serial.println("%");
      Serial.println("--------------"); // Spacing

      lastReportTime = millis(); // Update report time so loop only runs once every reporting period
    }
  }
}
