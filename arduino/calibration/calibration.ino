#include <EEPROM.h>
#define forceSensorPin                      0
int low;
int high;
float Read;
unsigned long timey;

struct forceCalibration
{
  float slope;
  float zero;
} calibrationData = {0, 0};

long timestamp = millis();
int receiveBuffer;
int dataReceived = 0;
bool printToScreen = true;
String readString;

void setup() {
Serial.begin(57600);
Serial.println("Hello bitches/world");
EEPROM.get(0, calibrationData);
Serial.print("From Flash:\nSlope is ");
Serial.print(calibrationData.slope);
Serial.print(", Zero is ");
Serial.println(calibrationData.zero);
timey = millis();
}

void loop() {
  if(millis() - timey > 800){
      Read = analogRead(forceSensorPin);
      Serial.println(Read);
      timey = millis();}
  if (printToScreen) {
    switch (dataReceived) {
      case 0:
        Serial.println("Enter the 11.5");
        break;
      case 1:
        Serial.println("Enter the 23");
        break;
      case 2:
        Serial.println("Enter the zero");
        break;
      default:
        break;
    }
    printToScreen = false;
  }
  
  while (Serial.available() > 0) {
    receiveBuffer = Serial.parseInt();
    
    if (receiveBuffer == 9999) { // This is the command to write to the EEPROM
      EEPROM.put(0, calibrationData);
      Serial.println("Data Written, bitches!");
      break;
    }
    
    if (receiveBuffer != 0) {
      //%3==0
      switch (dataReceived) {
        case 0:
          low = receiveBuffer;
          break;
        case 1:
          high = receiveBuffer;
          break;
        case 2:
          calibrationData.zero = receiveBuffer;
          dataReceived = 0;
          break;
        default:
          break;
      }
  
      Serial.print("New low ");
      Serial.print(low);
      Serial.print(", New high ");
      Serial.print(high);
      calibrationData.slope = (high - low)/11.5;
      Serial.print(", New slope ");
      Serial.print(calibrationData.slope);
      Serial.print(", New zero ");
      Serial.println(calibrationData.zero);
      printToScreen = true;
      dataReceived++;
      if(dataReceived >2){dataReceived=0;}
    }
    delay(1);
  }
/*
  while (Serial.available()) {
    char c = Serial.read();
    readString += c;
    delay(1);
  }

  if (readString.length() > 0) {
    int n = readString.toInt();
    readString = "";
  }
  */
}
