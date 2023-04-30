#include <SPI.h>
#include <SD.h>

const int switchPin = 6; // Digital pin for the switch controlling the motor

const String header[] = {"Battery 1 (V)", "Battery 2 (V)"};
bool headerWritten = false;
const int chipSelect = 10;

//DEFINE SAFE OPERATING LIMITS
const float voltageMin = 7;
const float voltageMax = 9;

const int initialNumBatteries = 2;
int numBatteries = 0;
const int voltagePins[] = {A0, A1}; //Analog pins for taking voltage readings
const float R1 = 10000.0; // R1 resistor value in ohms (change to your actual value)
const float R2 = 10000.0; // R2 resistor value in ohms (change to your actual value)

const int greenLEDs[] = {2, 3}; // GREEN LEDs for batteries
const int redLEDs[] = {4, 5}; // RED LEDs for batteries

const int transistorPins[] = {8, 9}; // Transistor base pins for hooking batteries to the rest of the circuit

String fileName;

//Structure for storing information on each battery
struct BatteryInfo{
  int voltagePin;
  int greenLED;
  int redLED;
  int transistorPin;
};

//Creates an array of battery info struct named operational batteries with a length equal to the initial number of batterie
BatteryInfo operationalBatteries[initialNumBatteries];

//FUNCTION FOR READING VOLTAGE VALUES
float readBatteryVoltage(int pin){
  int analogVoltage = analogRead(pin);
  float voltage = (analogVoltage/ 1023.0) * 5.0; //Convert the reading to voltage (assuming a 5V reference)

  // Apply voltage divider ratio
  float voltageDividerRatio = (R1 + R2) / R2;
  voltage *= voltageDividerRatio;

  return voltage;
}

//FUNTION FOR WRITING TO THE SD CARD
void writeDataToSDCard(String dataString) {
  //OPEN THE FILE
  File dataFile = SD.open(fileName, FILE_WRITE);

  // if the file is available, write to it:
  if (dataFile) {
    // Write header row if the file is empty and the header row has not been written yet
    if (dataFile.size() == 0 && !headerWritten) {
      dataFile.print("Timestamp,");
      for(int i = 0; i < numBatteries; i++){
        dataFile.print(header[i]);
        if (i < numBatteries - 1) {
          dataFile.print(",");
        }
      }
      dataFile.println();
      headerWritten = true;
    }

    // Write data row
    dataFile.print(millis());
    dataFile.print(",");
    dataFile.println(dataString);

    dataFile.close();
    // print to the serial port too:
    Serial.println(dataString);
  }
  // if the file isn't open, pop up an error:
  else {
    Serial.println("error opening " + fileName);
  }
}

//FUNCTION FOR GENERATING SD CARD FILE NAME
String generateIndexedFileName() {
  int fileIndex = 0;
  String baseName = "datalog";
  String fileExtension = ".txt";
  
  while (SD.exists(baseName + String(fileIndex) + fileExtension)) {
    fileIndex++;
  }
  
  return baseName + String(fileIndex) + fileExtension;
}

//MAIN SETUP
void setup() {
  // put your setup code here, to run once:
  pinMode(switchPin, INPUT_PULLUP);
  Serial.begin(9600);
  //pinMode(chipSelect, OUTPUT);

  while(!Serial){
    ; // wait for serial port to connect. needed for native USB port only
  }

  Serial.print("Initialising SD card...");

  if(!SD.begin(chipSelect)){
    Serial.println("initialization failed!");
    while(1);
  }
  Serial.println("initialization done.");

  fileName = generateIndexedFileName();
  Serial.println("Created new file: " + fileName);

  // Check battery voltages and turn on respective LEDs
  for (int i = 0; i < initialNumBatteries; i++) {
    pinMode(greenLEDs[i], OUTPUT);
    pinMode(redLEDs[i], OUTPUT);
    pinMode(transistorPins[i], OUTPUT);

    float voltage = readBatteryVoltage(voltagePins[i]);
    if (voltage >= voltageMin && voltage <= voltageMax) {
      digitalWrite(greenLEDs[i], HIGH);
      digitalWrite(redLEDs[i], LOW);
      digitalWrite(transistorPins[i], HIGH); // Turn on the transistor

      // Add battery to the list of operational batteries
      BatteryInfo batteryInfo = {voltagePins[i], greenLEDs[i], redLEDs[i], transistorPins[i]};
      operationalBatteries[numBatteries] = batteryInfo;
      numBatteries++;
    } else {
      digitalWrite(greenLEDs[i], LOW);
      digitalWrite(redLEDs[i], HIGH);
      digitalWrite(transistorPins[i], LOW); // Turn off the transistor
      Serial.println("Battery " + String(i + 1) + " is faulty.");
    }
  }
}

//MAIN LOOP
void loop() {
  // put your main code here, to run repeatedly:

  // Wait for the switch to be activated
  while (digitalRead(switchPin) == LOW) {
    delay(100);
  }

  //MAKE A STRING FOR ASSEMBLING THE DATA TO LOG
  String dataString="";

  //read the voltage values and append to the string
  float totalVoltage = 0;
  float batteryVoltages[numBatteries];
  for(int i = 0; i < numBatteries; i++){
    float voltage = readBatteryVoltage(operationalBatteries[i].voltagePin);
    dataString += String(voltage);
    totalVoltage +=voltage; //for calculating average voltage
    batteryVoltages[i] = voltage;
    if (i < numBatteries - 1) {
      dataString += ",";
    }
  }
  // Call function to write data to SD card
  writeDataToSDCard(dataString);

  // Calculate average voltage
  float averageVoltage = totalVoltage / numBatteries;

  // Compare individual battery voltage to the average voltage and control transistors accordingly
  for (int i = 0; i < numBatteries; i++) {
    if (batteryVoltages[i] < averageVoltage) {
      digitalWrite(operationalBatteries[i].transistorPin, LOW); // Turn off the transistor (disconnect battery)
    } else {
      digitalWrite(operationalBatteries[i].transistorPin, HIGH); // Turn on the transistor (connect battery)
    }
  }

  // Wait for 1 second before the next iteration
  delay(1000);
}
