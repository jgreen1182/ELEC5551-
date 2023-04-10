
#include <SPI.h>
#include <SD.h>

const String header[] = {"Battery 1 (V)", "Battery 2 (V)"};
bool headerWritten = false;
const int chipSelect = 10;

const int numBatteries = 2;
const int voltagePins[] = {A0, A1}; //Analog pins for taking voltage readings
const float R1 = 10000.0; // R1 resistor value in ohms (change to your actual value)
const float R2 = 10000.0; // R2 resistor value in ohms (change to your actual value)

//FUNCTION FOR READING VOLTAGE VALUES
float readBatteryVoltage(int pin){
  int analogVoltage = analogRead(pin);
  float voltage = (analogVoltage/ 1023.0) * 5.0; //Convert the reading to voltage (assuming a 5V reference)

  // Apply voltage divider ratio
  float voltageDividerRatio = (R1 + R2) / R2;
  voltage *= voltageDividerRatio;

  return voltage;
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

  // if(SD.exists("data.txt")){
  //   Serial.println("data.txt exists.");
  // } else{
  //   Serial.println("data.txt doesn't exist.");
  // }

//   //open a new file and immediateley close it
//   Serial.println("Creating data.txt...");
//   myFile = SD.open("data.txt", FILE_WRITE);
//   myFile.write("Hello");
//   Serial.println("Writing to file successful");
//   myFile.close();
}

void loop() {
  // put your main code here, to run repeatedly:

  //MAKE A STRING FOR ASSEMBLING THE DATA TO LOG
  String dataString="";

  //read the voltage values and append to the string
  for(int i = 0; i < numBatteries; i++){
    float voltage = readBatteryVoltage(i);
    dataString += String(voltage);
    if (i < 2) {
      dataString += ",";
    }
  }

  //OPEN THE FILE
  File dataFile = SD.open("datalog.txt", FILE_WRITE);

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
    Serial.println("error opening datalog.txt");
  }

  delay(1000);
}
