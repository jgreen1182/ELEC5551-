#include <SPI.h>
#include <SD.h>

const String header[] = {"Battery 1 (V)", "Battery 2 (V)"};
bool headerWritten = false;
const int chipSelect = 10;

const int numBatteries = 2;
const int voltagePins[] = {A0, A1}; //Analog pins for taking voltage readings
const float R1 = 10000.0; // R1 resistor value in ohms (change to your actual value)
const float R2 = 10000.0; // R2 resistor value in ohms (change to your actual value)

String fileName;

//FUNCTION FOR READING VOLTAGE VALUES
float readBatteryVoltage(int pin){
  int analogVoltage = analogRead(pin);
  float voltage = (analogVoltage/ 1023.0) * 5.0; //Convert the reading to voltage (assuming a 5V reference)

  // Apply voltage divider ratio
  float voltageDividerRatio = (R1 + R2) / R2;
  voltage *= voltageDividerRatio;

  return voltage;
}

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

String generateIndexedFileName() {
  int fileIndex = 0;
  String baseName = "datalog";
  String fileExtension = ".txt";
  
  while (SD.exists(baseName + String(fileIndex) + fileExtension)) {
    fileIndex++;
  }
  
  return baseName + String(fileIndex) + fileExtension;
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(chipSelect, OUTPUT);

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
}

void loop() {
  // put your main code here, to run repeatedly:

  //MAKE A STRING FOR ASSEMBLING THE DATA TO LOG
  String dataString="";

  //read the voltage values and append to the string
  for(int i = 0; i < numBatteries; i++){
    float voltage = readBatteryVoltage(voltagePins[i]);
    dataString += String(voltage);
    if (i < numBatteries - 1) {
      dataString += ",";
    }
  }

  // Call function to write data to SD card
  writeDataToSDCard(dataString);

  delay(1000);
}
