#include "rat.h"
#include "ArduinoSort.h"
#include <SoftwareSerial.h>
#include <EEPROM.h>



//Python communication stuff: 
// Using a dictionary here is too memory-heavy.  Using array of structs instead.
struct configData
{
  char tagId[18]; // 17 digit RFID code
  float forceThreshold; // 5 digits
  float handlePosition; // 5 digits
} configData = {0,0,0};

struct forceCalibration
{
  float slope;
  float zero;
} calibrationData = {0, 0};

const byte numChars = 50;
char receiveBuffer[numChars];

boolean newData = false;
long previousHeartbeat = 0;
int heartbeatInterval = 2000;
long previousRequestTimer = 0;
int requestInterval = 5000;
long receiveTimer = 0;
int receivetimeout = 1000; // Time to successfully send a fully-formed serial message before it bails.
boolean writeToFlashNeeded = 0;

//RFID stuff:
String content = "";
char character;
int firsttime = 0;
SoftwareSerial serial_lcd(rxpin, txpin); // new serial port on pins 2 and 3
int oldval2;
int evoodd= 0;
int animalin = 0;
int prevanimalin = 0;
int newanimal = 0;
unsigned long checkgunk = millis();


//ORT microswitch stuff: 
uint8_t microswitchRFID = A5;
int currentSwitch = 0;
int lastSwitch = 0; 

//Training stuff:

int activationThreshold = 5;
int pullInitiated = 0;
int thisPull = 0;
int pullBuffer[percentileArraySize];
int initialForceCri = 10;
int bufferPlace = 0;
byte transmitData = 0;
bool forceTriggered = 0; 
unsigned long heartbeatTimer = millis();
unsigned long solenoidTimer = millis();
unsigned long autoshapingTimer = millis();
unsigned long forceReporter = 0;
bool MASTERDEBUG = InitialMasterDebug;
//bool lightActive = 1;
bool autoshapingTrigger = 0;
float forceVal = 0;
float forceValprev = 0;
float forceValraw = 0;
long nextRandom;
int animalID[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
int currentID[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
int animaldetected = 0;
String notes;

void transmitHeartbeat()
{
  Serial.println(heartbeatCharacter);
  Serial.print("Time Elapsed: ");
  Serial.println(millis() - solenoidTimer);
  Serial.print("Next Random Fire: ");
  Serial.println(nextRandom);
  Serial.print("Force Reading: ");
  Serial.println(forceVal);
  Serial.print("Current Percentile Array:");
  for (int i = 0; i < percentileArraySize; i++){
    Serial.print(pullBuffer[i]);
    Serial.print(",");
  }
  Serial.print("\n");
  Serial.print("Current Activation Threshold:");
  Serial.println(activationThreshold);
  heartbeatTimer = millis();
}

void rfidread()
{
if (content.length() >=16){
  content = "";
}
if (millis()-checkgunk > 500){
  content = "";
}
while(serial_lcd.available())
{
//Serial.print("checking");
character = serial_lcd.read();
//Serial.println(character);
//Serial.println(content.length());
content.concat(character); //Attaches the parameter to a String
checkgunk = millis();
if(firsttime<3){
content
 = "";   
firsttime
 = firsttime+1;  
} 
}
if (content.length()>=16){
    for(int i=0;i<16;i++){
      char temp = content.charAt(i);
      int temp2 = temp - '0';
    animalID[i] = temp2;
  }
    evoodd=evoodd + 1;
    animaldetected = 1;
           
//            Serial.println(evoodd);
}
else {    
oldval2=content.length();
  //Serial.print("double"); 
}
   boolean same = true;  
 for (int k = 0; k < 16; ++k)
 { if (animalID[k] != currentID[k]){
    same = false;
  }
  }
  if ((animaldetected == 1)&&(same == true)){
    notes = "RFID pass same animal";
    sendData();
    animaldetected = 0;}
    
  if ((animaldetected == 1)&&(same == false))
 {
      notes = "Animal Entering";
     sendData();
      for (int i = 0; i < 16; ++i){
      currentID[i] = animalID[i];}
      animaldetected = 0;
      newanimal = 1;
 }
}

void randomFire ()
{
  if (millis() - solenoidTimer > nextRandom)
  {
    fireSolenoid();
    autoshapingTimer = millis();
    autoshapingTrigger = 1;
    nextRandom = random(minRandomValue, maxRandomValue);
  }
  
  if ((autoshapingTrigger) && millis() - autoshapingTimer > 200)
  {
    digitalWrite(solenoidOutputPin, soleniodOff);
    autoshapingTrigger = 0;
  }
}

void evaluateInputStates()
{
   if ((forceVal > pullThresholdhigh)&&(forceValprev > pullThresholdhigh)){
    pullInitiated = 1;
    transmitData = 1;
        if (forceVal > thisPull){
     thisPull = forceVal;}
  }
  if ((forceVal < pullThresholdlow)&&(pullInitiated == 1)){
    pullInitiated = 0;
      if (bufferPlace < 15){
      pullBuffer[bufferPlace] = thisPull;
      thisPull = 0;
      bufferPlace = bufferPlace + 1;}
      else{
      bufferPlace = 0;
      pullBuffer[bufferPlace] = thisPull;
      thisPull = 0;
      bufferPlace = bufferPlace + 1;
      }
  }
  if ((forceVal > highThreshold)&&(forceTriggered==0))
  { 
    fireSolenoid();
    Serial.println("<stimulate>");
    transmitData = 1;
    forceTriggered = 1;
    forceReporter = millis();
  }
  else if ((forceTriggered == 1) && (forceVal < lowThreshold))
    {  
      transmitData = 1;
      forceTriggered = 0;
    }
}

void fireSolenoid()
{
    Serial.println ("Firing Solenoid");
    digitalWrite(solenoidOutputPin, soleniodOn);
    delay(5);
    digitalWrite(solenoidOutputPin, soleniodOff);
    solenoidTimer = millis();
}

void sendData()
{
  Serial.print("Force Reading: ");
  Serial.print(forceVal);
  Serial.print("; Criterion met?: ");
  Serial.print(forceTriggered);
  Serial.print("; Animal ID: ");
   for (int i = 0; i < 16; ++i){
      Serial.print(animalID[i]);}
  Serial.print("; Notes: ");
  Serial.print(notes);
  Serial.print("\n");
  notes = "";
  }

void setInputPinPullups() // AB - i'm not sure what this one is, either.  
{
  #if defined (__AVR_ATmega328P__)
    byte inputPins = 13;
  #elif defined (__AVR_ATmega1280__)
    byte inputPins = 52;
  #endif
}
void writeToFlash() {
  if (writeToFlashNeeded == 1) {
    /*
     * NOTE - EEPROM can only be written to about 100,000 times in its whole life. Use writes sparingly!
     */
//    Serial.println("Writing to flash.");
  //  EEPROM.put(0, calibrationData);
    writeToFlashNeeded = 0;
  }
}

void recvWithStartEndMarkers() {
  static boolean recvInProgress = false;
  static byte ndx = 0;
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
    Serial.println("Configuation Data Received.");
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
  initialForceCri = configData.forceThreshold; 
  Serial.print("initialForceCrigotchanged:");
  Serial.println(initialForceCri);
        for (int i = 0; i < percentileArraySize; ++i){
      pullBuffer[i] = initialForceCri;}
//        Serial.println("percentileafterdatarecieveis:");
//   for (int i = 0; i < percentileArraySize; i++){
//    Serial.print(pullBuffer[i]);
//}
}

void showNewData() {
  if (newData == true) {
    Serial.print("This just in: ");
    Serial.println(receiveBuffer);
    parseData();
    Serial.print("Tag ID is ");
    Serial.println(configData.tagId);
    Serial.print("Force Threshold is ");
    Serial.println(configData.forceThreshold);
    Serial.print("Handle Position is ");
    Serial.println(configData.handlePosition);
    Serial.print("Slope is ");
    Serial.println(calibrationData.slope);
    Serial.print("Zero is ");
    Serial.println(calibrationData.zero);
    newData = false;
  }
}

void writeEnding()
{
      int add = (activationThreshold - configData.forceThreshold)/2;
      Serial.print("<write");
     for (int i = 0; i < 16; ++i){
      Serial.print(animalID[i]);}
      Serial.print(",");
      Serial.print(configData.forceThreshold + add);
      Serial.println(",0>");
}

void checkdoor()
{currentSwitch = digitalRead(microswitchRFID);
//Serial.println(currentSwitch);
  prevanimalin = animalin;
  if (currentSwitch != lastSwitch) {
  //if the state has changed, increment the counter
  //Serial.println ("switch changed");
    if (currentSwitch == LOW) {
      //if the current state is HIGH then the button went from off to on:
        animalin = 1;
        lastSwitch = currentSwitch;
        notes = "animal in";
        sendData();
    }
    else {
      animalin = 0;
      lastSwitch = currentSwitch;
      notes = "animal out";
      writeEnding();
      sendData();
    }}
    }

void checkforreset ()
{
//  Serial.print(animalin);
//  Serial.print(";");
//    Serial.print(prevanimalin);
//      Serial.print(";");
//      Serial.println(newanimal);
 if ((animalin == 1) && (prevanimalin == 0))
    {//Serial.println("gothere");
    restart();
    prevanimalin = 1;
      newanimal = 0;
    }
 if ((animalin == 1) && (newanimal==1))
     {restart ();
     newanimal = 0;
  }
}

void restart()
{
  //configData.forceThreshold = 5;
      Serial.print("<request");
     for (int i = 0; i < 16; ++i){
      Serial.print(animalID[i]);}
      Serial.println(">");
      initialForceCri = configData.forceThreshold;
      Serial.print("initialForceCri:");
      Serial.println(initialForceCri);
      notes = "restart";
      sendData();

      nextRandom = random(minRandomValue, maxRandomValue);
      if (autoshaping)
        {
        autoshapingTimer = millis();
        autoshapingTrigger = 1;

}
//        Serial.println("percentileafterrestartis:");
//   for (int i = 0; i < percentileArraySize; i++){
//    Serial.print(pullBuffer[i]);
//}
}

//void checkforslowasspython ()
//{
//  newpythonnews = configData.forceThreshold;
//  if (newpythonnews != oldpythonnews) 
//  {
//    initialForceCri = configData.forceThreshold; 
//    oldpythonnews = newpythonnews;
//  }
//}

void setup()
{
      for (int i = 0; i < percentileArraySize; ++i){
      pullBuffer[i] = initialForceCri;}
  pinMode(microswitchRFID,INPUT);
 digitalWrite(microswitchRFID,HIGH);
 Serial.begin(baudRate);
  serial_lcd.begin(baudRate);
  serial_lcd.println("ST2<crn>\r");
  setInputPinPullups();
  EEPROM.get(0, calibrationData);
  pinMode(solenoidOutputPin, OUTPUT);
  digitalWrite(solenoidOutputPin, soleniodOff);
  #if handshake
    for (;;) //AB i can tell what this function is doing but i don't know what (;;) means
    {
      if (Serial.available()) //AB see if the pc is ready -break only once the determination is made, delay 100ms before checking again if it is not made
      {
        byte startExecution = 0;
        byte data = Serial.read();
        switch (data)
        {
          case PCReadyCharacter:
            startExecution = 1;
            break;
          case MasterDebugCharacter:
            startExecution = 1;
            MASTERDEBUG = 1;
            break;
        }
        if (startExecution) break;
      }
      delay(100);    
    }
  #endif
  Serial.println(MicroStartCharacter); // message to PC to get a new data file ready
    Serial.print("From Flash: Slope is ");
    Serial.println(calibrationData.slope);
    Serial.print("Zero is ");
    Serial.println(calibrationData.zero);
  //  Serial.println("<Ready bitches>");
  //animaldetected = 1;//this is just for testing - makes it so that the code starts with the introduction of an animal
  //restart();
  forceTriggered=0;
  Serial.println("exiting setup");
}

void loop()
{  
  //Serial.println(forceVal);
  rfidread();
  checkdoor();
  checkforreset();
    recvWithStartEndMarkers();
    showNewData();
    writeToFlash();
//checkforslowasspython();
    
    if (millis() - previousHeartbeat > heartbeatInterval) {
      Serial.println('h');
      previousHeartbeat = millis();
    }
      //Debug script:
      //Serial.print("Animal:");
      //for (int i = 0; i < 17; ++i){
      //Serial.print(animalID[i]);}
      //Serial.print("\n");
      //Serial.print("Current:");
      //for (int i = 0; i < 17; ++i){
      //Serial.print(currentID[i]);}
      //Serial.print("\n");
      
  //animaldetected = 1; //this will be what rfid() returns
  //int animalIDincoming[] = {3,4,9,5,9,4,8,2,0,6,6,8,7,4,2,1,0}; //this will actually be what the rfidread() passes on
  //    for (int i = 0; i < 17; ++i){
  //    animalID[i] = animalIDincoming[i];}

  

  int pullTemp[percentileArraySize];
  for (int i = 0; i < percentileArraySize; ++i){
  pullTemp[i] = pullBuffer[i];}
  
  sortArray(pullTemp, percentileArraySize);

  activationThreshold = pullTemp[(percentileArraySize-1)/2];
  forceValprev = forceVal;
  delay(1);
  forceValraw = analogRead(forceSensorPin);
  forceVal = (forceValraw - calibrationData.zero)/calibrationData.slope; 
  evaluateInputStates();
  
  if ((forceTriggered) && (forceReporter - millis() > 100))
  {
    transmitData = 1;
  }

  if (millis() - heartbeatTimer > heartbeatTimeout)
  {
    transmitHeartbeat();
  }

  if (transmitData)
  {
    sendData();
    transmitData = 0;
  }
  
  if (autoshaping && animalin)
  {
    randomFire();
  }
}
