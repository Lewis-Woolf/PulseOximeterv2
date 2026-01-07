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
const byte SCLPin = A5; // ACL pin for display
const byte SDAPin = A4; // SDA pin for display

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
//void startupDisplay(void);

void setup() 
{
  pinMode(buttonAPin, INPUT); // Initialise button A pin
  pinMode(greenLEDPin, OUTPUT); // Initialise green LED pin
  pinMode(SCLPin, OUTPUT); // Initialise SCL pin
  pinMode(SDAPin, OUTPUT); // Initialise SDA pin

  digitalWrite(greenLEDPin, greenLEDState); // Set the state of the green LED

  Serial.begin(9600);
  delay(5000); // Delay by 5s

  // Initialise POX
  Serial.println("Initialising Dual-Wavelength Pulse Oximeter...");
  if (!pox.begin()) {
    Serial.println("FAILURE");
  }
  else {
    Serial.println("SUCCESS");
  }

  pox.setOnBeatDetectedCallback(onBeatDetected); // Register the callback for the beat detection function

  //Initialise OLED 
  Serial.println("Initialising OLED Display...");
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) { // Generate display voltage from 3.3V internally
    Serial.println(F("FAILURE"));
  }
  else {
    Serial.println("SUCCESS");
  }

  //startupDisplay();

  Serial.println("Press button A to turn POX on/off");
  Serial.println("Press button B to switch display");
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

  int reading = digitalRead(buttonAPin);

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

/*
// Create starting display
void startupDisplay(void)
{
  display.clearDisplay();

  // Set display parameters
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0); // Start at top left corner
  display.println("Pulse Oximeter");

  display.display(); // Display text
  delay(3000);
  display.clearDisplay(); // Clear initial display after delay
}
*/