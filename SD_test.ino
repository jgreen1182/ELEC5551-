#include <SPI.h>
#include <SD.h>

const int chipSelect = 10;

File myFile;

void setup(){
  //Open serial communications and wait for port to open
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

  if(SD.exists("data.txt")){
    Serial.println("data.txt exists.");
  } else{
    Serial.println("data.txt doesn't exist.");
  }

  //open a new file and immediateley close it
  Serial.println("Creating data.txt...");
  myFile = SD.open("data.txt", FILE_WRITE);
  myFile.write("Hello");
  Serial.println("Writing to file successful");
  myFile.close();
}

void loop(){}
