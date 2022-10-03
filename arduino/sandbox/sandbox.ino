#include <EEPROM.h>
#include "sandbox.h"

struct receivedData
{
  char tagId[18]; // 17 digit RFID code
  int forceThreshold; // 5 digits
  int handlePosition; // 5 digits
} configData = {0, 0, 0};

int testHistory[100];
int sizeoftestHistory = sizeof(testHistory) / sizeof(int);

struct forceCalibration
{
  int slope;
  int zero;
} calibrationData = {0, 0};

const int numChars = 400;
char receiveBuffer[numChars];

boolean newData = false;
long previousHeartbeat = 0;
long previousRequestTimer = 0;
long receiveTimer = 0;
int dataCtr = 0;
int writeCtr = 0;
int requestWriteCtr = 0;
boolean writeToFlashNeeded = 0;
int ledState = 1;
boolean recvInProgress = false;
int ndx = 0;

void setup() {
  Serial.begin(9600);
  for (int i = 0; i < sizeoftestHistory; i++) {
    testHistory[i] = i & 1;
  }
//    EEPROM.get(0, calibrationData);
//    Serial.print("From Flash: Slope is ");
//    Serial.println(calibrationData.slope);
//    Serial.print("Zero is ");
//    Serial.println(calibrationData.zero);
  Serial.println("<Ready bitches>");
  pinMode(13, OUTPUT);
  digitalWrite(13, ledState);
}

void loop() {
  recvWithStartEndMarkers();
  showNewData();
  //writeToFlash();
  
  if ((!recvInProgress) && (millis() - previousRequestTimer > requestInterval)) {
    switch (requestWriteCtr & 1) {
      case 0:
        requestData();
        break;

      case 1:
        writeData();
        break;

      default:
        break;
    }
    requestWriteCtr++;
    previousRequestTimer = millis();
  }
  
  if (millis() - previousHeartbeat > heartbeatInterval) {
    Serial.println('h');
    requestStim();
    previousHeartbeat = millis();
  }
}

void requestStim() {
  Serial.println("<stimulate>");
}

void writeToFlash() {
  if (writeToFlashNeeded == 1) {
    /*
     * NOTE - EEPROM can only be written to about 100,000 times in its whole life. Use writes sparingly!
     */
    Serial.println("Writing to flash.");
    EEPROM.put(0, calibrationData);
    writeToFlashNeeded = 0;
  }
}

void requestData() {
  /*
   * NOTE - This is for testing the pyhon code and isn't useful in normal operation
   */
  switch (dataCtr & 1) {
    case 0:
      Serial.println("<request12345678901234567>"); // should be valid
      break;

    case 1:
      //Serial.println("<request12345678911234567>"); // should be invalid
      //Serial.println("<request11111111111111111>"); // request the values written previously
      Serial.println("<request96847000010332304>"); // request a value already in the data file
      break;

    default:
      break;
  }
  dataCtr++;
}

void writeData() {
  /*
   * NOTE - This is for testing the pyhon code and isn't useful in normal operation
   */
  String line;
  String counter = String(writeCtr);
  switch (writeCtr & 1) {
    case 0:
      line = "<write11111111111111111," + counter;
      line = line + ",4";
      break;

    case 1:
      line = "<write22222222222222222," + counter;
      line = line + ",4";
      break;

    default:
      break;
  }

  for (int i = 0; i < sizeoftestHistory; i++) {
    line = line + "," + testHistory[i];
  }

  line = line + ">";  
  
  Serial.println(line);
  writeCtr++;
}

void recvWithStartEndMarkers() {
  char startMarker = '<';
  char endMarker = '>';
  char rc;

  while (Serial.available() > 0 && newData == false) {
    rc = Serial.read();

    if (recvInProgress == true) {
      if (rc != endMarker) {
        receiveBuffer[ndx] = rc;
        ndx++;
        if (ndx >= numChars) {
          ndx = numChars - 1;
        }
      }
      else {
        receiveBuffer[ndx] = '\0';
        recvInProgress = false;
        ndx = 0;
        newData = true;
      }
      if (millis() - receiveTimer > receivetimeout) {
        receiveBuffer[ndx] = '\0';
        recvInProgress = false;
        ndx = 0;
        newData = false;
      }
    }

    else if (rc == startMarker) {
      recvInProgress = true;
      receiveTimer = millis();
    }
  }
}

void parseData() {
  int commaCount = 0;
  for (int i = 1; i < sizeof(receiveBuffer) - 1; i++) {
    if (receiveBuffer[i] == ',') {
      commaCount++;
    }
  }

  if (commaCount == 2) {
    char * strtokIndx; // this is used by strtok() as an index

    strtokIndx = strtok(receiveBuffer,","); // get the first part - the string
    strcpy(configData.tagId, strtokIndx); // copy it to tagId
    
    strtokIndx = strtok(NULL, ","); // this continues where the previous call left off
    configData.forceThreshold = atoi(strtokIndx);
  
    strtokIndx = strtok(NULL, ","); // this continues where the previous call left off
    configData.handlePosition = atoi(strtokIndx);
  }

  if (commaCount == 1) {
    Serial.println("Force Calibration Data Received.");
    char * strtokIndx; // this is used by strtok() as an index

    strtokIndx = strtok(receiveBuffer,",");
    calibrationData.slope = atoi(strtokIndx);
    
    strtokIndx = strtok(NULL, ",");
    calibrationData.zero = atoi(strtokIndx);
    writeToFlashNeeded = 1;
  }

}

void showNewData() {
  if (newData == true) {
    Serial.print("Micro received: ");
    Serial.println(receiveBuffer);
    parseData();
    Serial.print("Tag ID is ");
    Serial.println(configData.tagId);
    Serial.print("Force Threshold is ");
    Serial.println(configData.forceThreshold);
    Serial.print("Handle Position is ");
    Serial.println(configData.handlePosition);
//    Serial.print("Slope is ");
//    Serial.println(calibrationData.slope);
//    Serial.print("Zero is ");
//    Serial.println(calibrationData.zero);
    newData = false;
  }
}
