#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "MAX30100_PulseOximeter.h"

// Define OLED parameters
#define SCREEN_WIDTH 96
#define SCREEN_HEIGHT 16
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET); //Interface to the OLED display
PulseOximeter pox; // Interface to the sensor

const byte greenLEDPin = A2; // Pin for the green LED
const byte buttonAPin = A0; // Pin for button A
const byte buttonBPin = A1; // Pin for button B
const byte SCLPin = A5; // ACL pin for display
const byte SDAPin = A4; // SDA pin for display

int BPM = 0; // Heartbeats per min
int displayState = 0; // State of the display. O is no display, 1 is BPM, 2 is SpO2 and 3 is both
int poxState = -1; // Determines if the pox is on (1) or off (-1)
int buttonAState = 1; // Determines if button A is pressed down or not
int lastButtonAState = 1; // The last state button A was in
int buttonBState = 1; // Determines if button A is pressed down or not
int lastButtonBState = 1; // The last state button A was in

int greenLEDState = LOW; // State of the LED

long reportingPeriod = 5000; // Time between information being printed
long displayPeriod = 1000; // Time after the display changes text to when it starts displaying measurement results
long lastReportTime = 0; // Time since last report
long displayTimerStart = 0; // Display timer start time
long lastDebounceTimeA = 0; // Last time the output pin was toggled for button A
long lastDebounceTimeB = 0; // Last time the output pin was toggled for button B
long debounceDelay = 50; // Debounce time to mitigate button noise

void onBeatDetected();
void setPOXState();
void setDisplayState();
void printPOXResults();
void poxOffDisplay(void);
void poxOnDisplay(void);
void poxMeasurementDisplay(void);


void setup() 
{
  pinMode(buttonAPin, INPUT); // Initialise button A pin
  pinMode(buttonBPin, INPUT); // Initialise button B pin
  pinMode(greenLEDPin, OUTPUT); // Initialise green LED pin

  digitalWrite(greenLEDPin, greenLEDState); // Set the state of the green LED

  Serial.begin(9600);

  // Initialise OLED display
  display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS); // Generate display voltage from 3.3V internally

  poxOffDisplay();

  // Initialise POX
  pox.begin();

  pox.setOnBeatDetectedCallback(onBeatDetected); // Register the callback for the beat detection function
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

  if (poxState == 1) {
    setDisplayState(); // Set the state of the display
  }

  printPOXResults(); // Print the POX results on the serial monitor

  if (poxState == 1) {
  poxMeasurementDisplay(); // Set the pox display state
  }
}



// Set the state of the POX and the green LED
void setPOXState()
{
  digitalWrite(greenLEDPin, greenLEDState); // Set the state of the green LED

  int reading = digitalRead(buttonAPin);

  if (reading != lastButtonAState) { // If the button state changes
    lastDebounceTimeA = millis(); // reset debouncing timer
  }

  if ((millis() - lastDebounceTimeA) > debounceDelay) // If a reading has been around for an amount of time equal to debounce delay has passed, assume it is correct
  {
    if (reading != buttonAState) // If the button state has changed
    {
      buttonAState = reading;

      if (buttonAState == LOW); // If the button is being pressed
      {
        if (lastButtonAState == 1) // If the last state was off
        {
          poxState = -poxState;
          if (greenLEDState == LOW) { // Turn on/off LED and change display depending on which state the green LED is in
            greenLEDState = HIGH; // If LED is off, turn it on
            displayState = 0; // Set display to the default on state
            poxOnDisplay();
          }
          else {
            greenLEDState = LOW; // If LED is on, turn it off
            poxOffDisplay();
          }
        }
      }
    }
  }

  lastButtonAState = reading;
}



// Set the state of the display
void setDisplayState()
{
  int reading = digitalRead(buttonBPin);

  if (reading != lastButtonBState) { // If the button state changes
    lastDebounceTimeB = millis(); // reset debouncing timer
  }

  if ((millis() - lastDebounceTimeB) > debounceDelay) // If a reading has been around for an amount of time equal to debounce delay has passed, assume it is correct
  {
    if (reading != buttonBState) // If the button state has changed
    {
      buttonBState = reading;

      if (buttonBState == LOW); // If the button is being pressed
      {
        if (lastButtonBState == 1) // If the last state was off
        {
          displayState += 1;
          if (displayState > 3) // If display state is greater than 3, reset it to BPM state
          {
            displayState = 1;
          }    
          display.clearDisplay();
          displayTimerStart = millis(); // Start the timer to change the display
        }
      }
    }
  }

  lastButtonBState = reading;
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



// Create display for when pox is off
void poxOffDisplay(void)
{
  display.clearDisplay();

  // Set display parameters
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0); // Offset text so it starts at the left of the display
  display.println("POX OFF");

  display.display(); // Display text
}



// Create display for when display is on but not displaying measurements
void poxOnDisplay(void)
{
  display.clearDisplay();

  // Set display parameters
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0); // Offset text so it starts at the left of the display
  display.println("POX ON");

  display.display(); // Display text
}



// Create display for when pox is displaying measurements
void poxMeasurementDisplay()
{
  display.clearDisplay();

  // Set display parameters
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0); // Offset text so it starts at the left of the display
  

  if (displayState == 1) { // Display BPM
    if ((millis() - displayTimerStart) <= displayPeriod) { // Print text when display period not elapsed
      display.println("BPM");
      display.display();
    }

    if ((millis() - displayTimerStart) >= displayPeriod) { // Print results when display period elapsed
      display.println(pox.getHeartRate());
      display.display();
    }
  }

  if (displayState == 2) { // Display SpO2
    if ((millis() - displayTimerStart) <= displayPeriod) { // Print text when display period not elapsed
      display.println("SpO2");
      display.display();
    }

    if ((millis() - displayTimerStart) >= displayPeriod) { // Print results when display period elapsed
      display.println(pox.getSpO2());
      display.display();
    }
  }

  if (displayState == 3) { // Display BPM and SpO2
    if ((millis() - displayTimerStart) <= displayPeriod) { // Print text when display period not elapsed
      display.println("BPM/SpO2");
      display.display();
    }

    if ((millis() - displayTimerStart) >= displayPeriod) { // Print results when display period elapsed
      display.print(pox.getHeartRate());
      display.print("/"); // Spacing
      display.println(pox.getSpO2());
      display.display();
    }
  }
}