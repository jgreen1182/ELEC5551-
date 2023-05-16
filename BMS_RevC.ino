#include <SPI.h>
#include <SD.h>

//DEFINE SAFE OPERATING LIMITS
const float voltageMin = 18;
const float voltageMax = 25.2;

//DEFINE PIN CONNECTIONS
const int voltagePins[] = {A0, A1, A2, A3, A4, A5}; //Analog pins for taking voltage readings
const int tempPins[] = {A6, A7, A8, A9, A10, A11}; //Analog pins for taking temperature readings
const int greenLEDs[] = {2, 3, 4, 5, 6, 7}; // GREEN LEDs for batteries
const int voltagePack = A15;

//DEFINE VOLTAGE DIVIDER RATIO
const float R1 = 10.0; // R1 resistor value in ohms (change to your actual value)
const float R2 = 1.0; // R2 resistor value in ohms (change to your actual value)

//FOR COUNTING CONNECTED BATTERIES
const int initialNumBatteries = 6;
int numBatteries = 0;

//FOR SD CARD
String fileName;
const int chipSelect = 49; //chip select for the SD card
bool headerWritten = false; //To check if header has already been written to SD card

//Structure for storing information on each battery
struct BatteryInfo{
  int voltagePin;
  int tempPin;
  int greenLED;
  int redLED;
  int transistorPin;
};

//Creates an array of battery info struct named operational batteries with a length equal to the initial number of batteries
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

//FUNCTION FOR READING TEMPERATURE VALUES
float readBatteryTemperature(int pin){
  int analogTemp = analogRead(pin);
  float tempVoltage = (analogTemp / 1023.0) * 5.0; //Convert the reading to voltage (assuming a 5V reference)
  float temperature = tempVoltage * 100.0; // Convert voltage to temperature in Celsius (for LM35)

  return temperature;
}

//FUNCTION FOR WRITING DATA TO THE SD CARD
void writeDataToSDCard(String dataString) {
  //OPEN THE FILE
  File dataFile = SD.open(fileName, FILE_WRITE);

  // if the file is available, write to it:
  if (dataFile) {
    // Write header row if the file is empty and the header row has not been written yet
    if (dataFile.size() == 0 && !headerWritten) {
      dataFile.print("Timestamp,");

      //Add voltage headers
      for (int i = 0; i < numBatteries; i++) {
        int batteryIndex = operationalBatteries[i].voltagePin - A0 + 1;
        dataFile.print("Battery " + String(batteryIndex) + " (V),");
        if (i < numBatteries - 1) {
          dataFile.print(",");
        }
      }

      //add temperature headers
      for (int j = 0; j < numBatteries; j++) {
        int batteryIndex = operationalBatteries[j].voltagePin - A0 + 1;
        dataFile.print("Battery " + String(batteryIndex) + " (°C)");
        if (j < numBatteries - 1) {
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

  // Check battery voltages and turn on respective LEDs if within safe operating range GREEN and connect to circuit, if not within safe operating range RED and do not connect to circuit
  for (int i = 0; i < initialNumBatteries; i++) {
    pinMode(greenLEDs[i], OUTPUT);

    float voltage = readBatteryVoltage(voltagePins[i]);
    if (voltage >= voltageMin && voltage <= voltageMax) {
      digitalWrite(greenLEDs[i], HIGH);

      // Add battery to the list of operational batteries
      BatteryInfo batteryInfo = {voltagePins[i], tempPins[i], greenLEDs[i], redLEDs[i], transistorPins[i]};
      operationalBatteries[numBatteries] = batteryInfo;
      numBatteries++;
    } else {
      digitalWrite(greenLEDs[i], LOW);
      Serial.println("Battery " + String(i + 1) + " is faulty.");
    }
  }
}


//MAIN LOOP
void loop() {
  // Wait for the switch to be activated
  while (digitalRead(switchPin) == LOW) {
    delay(100);
  }

  //MAKE A STRING FOR ASSEMBLING THE DATA TO LOG
  String dataString="";

  //read the voltage values and append to the string
  float totalVoltage = 0;
  for(int i = 0; i < numBatteries; i++){
    float voltage = readBatteryVoltage(operationalBatteries[i].voltagePin);
    dataString += String(voltage);
    if (i < numBatteries - 1) {
      dataString += ",";
    }
  }

  // Add a comma separator between voltage and temperature readings
  dataString += ",";

  //read the temperature values and append to the string
  for(int i = 0; i < numBatteries; i++){
    float temperature = readBatteryTemperature(operationalBatteries[i].tempPin);
    dataString += String(temperature);
    if (i < numBatteries - 1) {
    dataString += ",";
    }
  }

  //Check for error conditions by looping over batteries
  for(int i = 0; i < numBatteries; i++){
      float voltage_check = readBatteryVoltage(operationalBatteries[i].voltagePin);
      float temperature_check = readBatteryTemperature(operationalBatteries[i].tempPin);
      if(voltage < 18 || voltage > 25.2 || temperature < 0 || temperature > 45){
        digitalWrite(operationalBatteries[i].greenLED, LOW);
        Serial.println("Error: Battery " + String(i + 1) + " has exceeded safe operating conditions.");
        //SEND MAVLINK MESSAGE
      }
  }

  //Read pack voltage and send to flight controller
  float packVoltage = readBatteryVoltage(A15);
  //SEND MAVLINK MESSAGE

  // Call function to write data to SD card
  writeDataToSDCard(dataString);

  // Wait for 1 second before the next iteration
  delay(1000);
}
