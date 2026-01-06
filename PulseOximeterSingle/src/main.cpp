int inPin = 10; // Pin for the voltage reading
int BPM = 0; // Heartbeats per min

long currentTimerCount = 0; // Current time
long previousTimerCount = 0; // Time at last beat

float thresholdVoltage = 5e-3; // Threshold voltage to count heart rate


void setup() {
  Serial.begin(9600);
  delay(500); // Delay for 500 ms so the serial monitor works properly

  Serial.println("Initialised Single-Wavelength Pulse Oximeter");
}
      

void loop() 
{
  float Voltage = analogRead(inPin); //Read the voltage
  if (Voltage >= thresholdVoltage) // When threshold voltage is reached, calculate BPM
  {
    currentTimerCount = millis(); // Set current timer count
    float timeDifference = (currentTimerCount - previousTimerCount) / 60000; // Time difference between previous and current count in minutes

    BPM = round(1 / timeDifference);

    previousTimerCount = millis(); // Set the previous timer count to the current time AFTER the BPM is calculated

    Serial.println("BPM: " + BPM);
  }
}


void calculateAverageVoltage()
{

}


void detectVoltageSpikes()
{

}


void calculateBPM()
{

}