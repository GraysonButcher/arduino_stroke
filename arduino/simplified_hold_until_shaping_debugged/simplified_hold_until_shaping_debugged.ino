#include "rat.h"
#include "ArduinoSort.h"
#include <SoftwareSerial.h>
#include <EEPROM.h>



//Python communication stuff:
// Using a dictionary here is too memory-heavy.  Using array of structs instead.
struct configData
{
  char tagId[18]; // 17 digit RFID code
  float forceThreshold; // 5 digits; Not sure if I can change this to activationThrehsold, or if I need to keep it as forceThreshold
  float handlePosition; // 5 digits; I would change this to float holdCriterion (could it be the same as the one in this file or no?)
  //char animalHoldArray[100];
  //float animalHoldArray //100 digits
} configData = {0, 0, 0}; //Im guessing this would change to configData = {0,0,0,0};

struct forceCalibration
{
  float slope;
  float zero;
} calibrationData = {0, 0};

const int numChars = 50;
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
int evoodd = 0;
int animalin = 0;
int prevanimalin = 0;
int newanimal = 0;
unsigned long checkgunk = millis();


//ORT microswitch stuff:
uint8_t microswitchRFID = A5;
int currentSwitch = 0;
int lastSwitch = 0;

//Training stuff:

int activationThreshold = 7;
int minthreshold = 5;
int pullInitiated = 0;
int thisPull = 0;
int pullBuffer[percentileArraySize];
int initialForceCri = 5;
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
int animalID[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int currentID[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int animaldetected = 0;
String notes;


//hold stuff
unsigned long  holdTimerStart;
int timingHold;
int holdMet;
int holdCriterion = 330;
unsigned long responseStart;
unsigned long responseDuration;
int responseBegan;
int firstInstance = 0;
int holdMetThisPull = 0;
int thisHold;
int holdArray[holdArraySize];
int holdPlace = 110;
int newThreshold;
int fixedvalue = 100;
int holdStep[] = {7, 15, 25, 40, 65, 100};
int holdStepSize = 6;
int durationStep[] = {0, 330, 660, 1000};
int durationStepSize = 4;


//stim assignments for each animal - add ID numbers for anyone who needs stimulation
int ratone[] = {7, 6, 8, 47, 0, 0, 0, 0, 1, 0, 3, 9, 0, 3, 5, 4};
int rattwo[] = {7, 6, 8, 47, 0, 0, 0, 0, 1, 0, 3, 3, 2, 0, 2, 2};
int ratthree[] = {7, 6, 8, 47, 0, 0, 0, 0, 1, 0, 3, 3, 2, 1, 0, 0};
int ratfour[] = {7, 6, 8, 47, 0, 0, 0, 0, 1, 0, 3, 8, 9, 8, 0, 0};
int stimtoggle = 1; // contingent stimulation on(1) or off(0) - turns on and off in void rfidread()  and off when ort door opens

//stim assignments for animals that need to be on fixed value 120 when percentile is on
int thisratfinalcriterion = 0;
int ratfixedone[] = {7, 6, 8, 47, 0, 0, 0, 0, 1, 0, 3, 3, 1, 7, 3, 0};
int ratfixedtwo[] = {7, 6, 8, 47, 0, 0, 0, 0, 1, 0, 3, 3, 1, 7, 3, 0};

void transmitHeartbeat()
{
  Serial.println(heartbeatCharacter);
  Serial.print("Time Elapsed: ");
  Serial.println(millis() - solenoidTimer);
  Serial.print("Next Random Fire: ");
  Serial.println(nextRandom);
  Serial.print("Force Reading: ");
  Serial.println(forceVal);
  //  Serial.print("Current Percentile Array:");
  //  for (int i = 0; i < percentileArraySize; i++) {
  //    Serial.print(pullBuffer[i]);
  //    Serial.print(",");
  //  }
  //  Serial.print("\n");
  Serial.print("Current Hold Array:");
  for (int h = 0; h < holdArraySize; h++) {
    Serial.print(holdArray[h]);
    Serial.print(",");
  }
  Serial.print("\n");
  Serial.print("Current Activation Threshold:");
  Serial.println(activationThreshold);

  heartbeatTimer = millis();
}


void rfidread()
{
  if (content.length() >= 16) {
    content = "";
  }
  if (millis() - checkgunk > 500) {
    content = "";
  }
  while (serial_lcd.available())
  {
    //Serial.print("checking");
    character = serial_lcd.read();
    //Serial.print(character);
    //Serial.println(character);
    //Serial.println(content.length());
    content.concat(character); //Attaches the parameter to a String
    checkgunk = millis();
    if (firsttime < 3) {
      content
        = "";
      firsttime
        = firsttime + 1;
    }
  }
  if (content.length() >= 16) {
    for (int i = 0; i < 16; i++) {
      char temp = content.charAt(i);
      int temp2 = temp - '0';
      animalID[i] = temp2;
    }
    evoodd = evoodd + 1;
    animaldetected = 1;

    //            Serial.println(evoodd);
  }
  else {
    oldval2 = content.length();
    //Serial.print("double");
  }
  boolean same = true;
  for (int k = 0; k < 16; ++k)
  { if (animalID[k] != currentID[k]) {
      same = false;
    }
  }
  //start detect animal ID for stim

  int yepstim = 0;
  boolean stimIDdetected = true;
  for (int m = 0; m < 16; ++m)
  { if (animalID[m] != ratone[m]) {
      stimIDdetected = false;
    }
  }
  if (stimIDdetected == true) {
    yepstim = 1;
  }

  stimIDdetected = true;
  for (int m = 0; m < 16; ++m)
  { if (animalID[m] != rattwo[m]) {
      stimIDdetected = false;
    }
  }
  if (stimIDdetected == true) {
    yepstim = 1;
  }

  stimIDdetected = true;
  for (int m = 0; m < 16; ++m)
  { if (animalID[m] != ratthree[m]) {
      stimIDdetected = false;
    }
  }
  if (stimIDdetected == true) {
    yepstim = 1;
  }

  stimIDdetected = true;
  for (int m = 0; m < 16; ++m)
  { if (animalID[m] != ratfour[m]) {
      stimIDdetected = false;
    }
  }
  if (stimIDdetected == true) {
    yepstim = 1;
  }


  if ((animaldetected == 1) && (yepstim == 0)) {
    stimtoggle = 0;
    notes = "stimulation off";
    sendData();
  }
  if ((animaldetected == 1) && (yepstim == 1)) {
    stimtoggle = 1;
    notes = "stimulation on";
    sendData();
  }

  //end detect animal ID for stim
  //start detect animal on fixed 120

  boolean  makeitfixed = true;
  int fixedmark = 0;
  for (int m = 0; m < 16; ++m)
  { if (animalID[m] != ratfixedone[m]) {
      makeitfixed = false;
    }
  }
  if (makeitfixed == true) {
    fixedmark = 1;
  }
  for (int m = 0; m < 16; ++m)
  { if (animalID[m] != ratfixedtwo[m]) {
      makeitfixed = false;
    }
  }
  if (makeitfixed == true) {
    fixedmark = 1;
  }

  if ((fixedmark == 1) && (animaldetected == 1)) {
    initialForceCri = 120;
    for (int i = 0; i < percentileArraySize; ++i) {
      pullBuffer[i] = initialForceCri;
    }
    thisratfinalcriterion = 1;
    notes = "Rat scheduled for Fixed 120g value";
    sendData();
  }
  if ((fixedmark == 0) && (animaldetected == 1)) {
    thisratfinalcriterion = 0;
  }

  //end detect animal ID on fixed 120

  if ((animaldetected == 1) && (same == true)) {
    notes = "RFID pass same animal";
    sendData();
    animaldetected = 0;
  }

  if ((animaldetected == 1) && (same == false))
  {
    notes = "Animal Entering";
    sendData();
    for (int i = 0; i < 16; ++i) {
      currentID[i] = animalID[i];
    }
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
  if ((forceVal > pullThresholdhigh) && (forceValprev > pullThresholdhigh) && (firstInstance == 0)) {
    //serial prints the beginning of the pull and continued pull that is above "pullThreshold". It is not necessarily the first moment of the pull...
    //would need to start the "response" timer here
    //Serial.println("pull initiation");
    pullInitiated = 1;
    transmitData = 1;
    firstInstance = 1;
    //this conditional makes sure only the peak of this response stream is stored as representing "this pull." so if the incoming values are 9, 10, 9, 8, 11, 3, 2 (continuously), it will say thisPull = 11.
    if (forceVal > thisPull) {
      thisPull = forceVal;
    }

    responseStart = millis();
    //Serial.print("response started at: ");
    // Serial.println(responseStart);

  }

  if ((forceVal > pullThresholdhigh) && (forceValprev > pullThresholdhigh) && (firstInstance == 1)) {
    //serial prints the beginning of the pull and continued pull
    //would need to start the "response" timer here
    pullInitiated = 1;
    transmitData = 1;
    if (forceVal > thisPull) {
      thisPull = forceVal;
    }
  }





  if ((forceVal < pullThresholdlow) && (pullInitiated == 1)) {
    pullInitiated = 0;
    firstInstance = 0;

    responseDuration =  millis() - responseStart;
    //    Serial.print("current millis is: " );
    //    Serial.println(millis());
    //Serial.print("response duration is: ");
    //Serial.println(responseDuration);
    //yes, this records the last pull, so the timer could stop here. For some reason, it adds one more force report after the response ceases, but only if the force criterion is low.
    //Serial.println("is this when the pull stops?");
    //    Serial.print("bufferPlace");
    //    Serial.println(bufferPlace);
    //    Serial.print("thisPull");
    //    Serial.println(thisPull);

    if ((percentileoff == 0) && (thisratfinalcriterion == 0)) {
      if (bufferPlace < 15) {
        pullBuffer[bufferPlace] = thisPull;
        thisPull = 0;
        bufferPlace = bufferPlace + 1;
      }
      else {
        bufferPlace = 0;
        pullBuffer[bufferPlace] = thisPull;
        thisPull = 0;
        bufferPlace = bufferPlace + 1;
      }
    }
    if ((responseDuration >= holdCriterion) && (holdMetThisPull == 1)) {
      thisHold = 2;
      notes = "successful hold"; 


    }
    else if ((responseDuration < holdCriterion) || (holdMetThisPull == 0)) {
      thisHold = 1;
      notes = "unsuccessful hold"; 

    }
    if (holdPlace < holdArraySize) {
      holdArray[holdPlace] = thisHold;
      holdPlace = holdPlace + 1;
    }
    else if (holdPlace >= holdArraySize) {
      //i want the 11th response to.... replace the 10th [9] position in the hold
      //i want the old 10h [9] place one to move to the 9th [8] place.... etc. And the 1st place one to get removed. But I need to do this starting with moving the 2nd place to the 1st, then the 3rd place to the second, in that order. This ends with moving the 10th to the 9th slot
      for (int g = 0; g < holdArraySize; g++) {
        holdArray[g] = holdArray[g + 1];
      }
      holdArray[holdArraySize - 1] = thisHold;
    }
    thisHold = 0;
    holdMetThisPull = 0;
    //    cumulativeAttempts++;
    //Serial.print("cumulative attempts is: ");
    //Serial.println(cumulativeAttempts);
    //    attemptsThisSession++;
    //Serial.print("attempts this session are: ");
    //Serial.println(attemptsThisSession);
  }

  if ((forceVal > highThreshold) && (forceTriggered == 0))
  {
    //This records the first moment a rat meets the pull force criterion
    //Serial.println("force triggered");
    transmitData = 1;
    forceTriggered = 1;
    forceReporter = millis();
    holdTimerStart = millis();
    //Serial.print("holdTimerStart");
    //Serial.println(holdTimerStart);
    timingHold = 1;




  }
  else if ((forceVal > highThreshold) && (timingHold == 1))
  {
    //    Serial.println("above criterion hold still occurring");
    //    Serial.print("hold timer is: ");
    //    Serial.println(holdTimerStart);
    //    Serial.print("millis is: ");
    //    Serial.println(millis());

    if ((millis() - holdTimerStart > holdCriterion) && (holdMet == 0)) {
      notes = "Hold Duration Met";
      //Serial.println("HOLD CRITERION MET");
      holdMet = 1;
      holdMetThisPull = 1;
      fireSolenoid();
      int responsetwoDuration =  millis() - responseStart;
      //Serial.print("response 2 duration is: ");
      //Serial.print(responsetwoDuration);
      if (stimtoggle == 1) {
        Serial.println("<stimulate>");
      }
    }
  }

  else if ((forceTriggered == 1) && (forceVal < lowThreshold))
  {
    transmitData = 1;
    forceTriggered = 0;
    timingHold = 0;
    holdMet = 0;
    holdTimerStart = 0;
    //Serial.println("holdMet returned to 0");
    //reported when the criterial part of the pull ends
  }
}

//this function evaluate the average of the hold array. You can use this to make moving to a new force criterion contingent on some level of success over the array of previous trials.
//void evaluateShapingStep() {
//  float holdArrayAverage = 0.00;
//  float arraySum = 0.00;
//  for (int d = 0; d < holdArraySize; d++) {
//    arraySum += holdArray[d];
//  }
//  holdArrayAverage = (arraySum / holdArraySize);
//  //Serial.println(holdArrayAverage);
//  if (holdArrayAverage >= 1.8) { //should be able to switch this conditionals to JUST being that the 1.8 is met.
//    for (int h = 0; h < holdArraySize; h++) {
//      holdArray[h] = 0;
//    }
//    if (holdCriterion == 1000) {
//      for (int g = 0; g < holdStepSize; g ++) {
//        if (activationThreshold == holdStep[g] && activationThreshold != 100) {
//          activationThreshold = holdStep[g + 1];
//          Serial.print("hold force threshold met, moving on to");
//    Serial.println(activationThreshold);
//          break;
//        }
//      }
//    
//    }
//
//    for (int k = 0; k < durationStepSize; k ++) {
//      if (holdCriterion == durationStep[k] && holdCriterion != 1000) {
//        holdCriterion = durationStep[k + 1];
//        Serial.print("hold DURATION met, moving on to ");
//    Serial.println(holdCriterion);
//        break;
//      }
//    }
//    
//
//
//    for (int x = 0; x < holdArraySize; x++) {
//      holdArray[x] = 0;
//    }
//  }
//}

void fireSolenoid()
{
  Serial.println (";;;;;Firing Solenoid");
  digitalWrite(solenoidOutputPin, soleniodOn);
  delay(5);
  digitalWrite(solenoidOutputPin, soleniodOff);
  solenoidTimer = millis();
}

void sendData()
{
  Serial.print(";Force Reading: ");
  Serial.print(forceVal);
  Serial.print(";Force Criterion is: ");
  Serial.print(activationThreshold);
  Serial.print(";Hold Criterion is: ");
  Serial.print(holdCriterion);
  Serial.print("; Criterion met?: ");
  Serial.print(forceTriggered);
  Serial.print("; Animal ID: ");
  for (int i = 0; i < 16; ++i) {
    Serial.print(animalID[i]);
  }
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
       NOTE - EEPROM can only be written to about 100,000 times in its whole life. Use writes sparingly!
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

    strtokIndx = strtok(receiveBuffer, ","); // get the first part - the string
    strcpy(configData.tagId, strtokIndx); // copy it to tagId

    strtokIndx = strtok(NULL, ","); // this continues where the previous call left off
    configData.forceThreshold = atoi(strtokIndx); //I'm guessing this needs to be changed for the activationThreshold or whatever.

    strtokIndx = strtok(NULL, ","); // this continues where the previous call left off
    configData.handlePosition = atoi(strtokIndx); //I'm guessing this needs to be changed for hold duration.

    //I'm guessing we would need one more, maybe configData.animalHoldArray = atoi(strtokIndx);  //which would be the record of the array. In damon's code, we would at least have to add one more \d{100} or whatever.
  }

//    if (commaCount >= 2) {
//    char *dataa[103];
//    char *ptr = NULL;
//    byte index = 0;
//    ptr = strtok(receiveBuffer, ",");
//    while (ptr != NULL)
//    {
//      dataa[index] = ptr;
//      index++;
//      ptr = strtok(NULL, ",");
//    }
//    for (int s = 0; s < index; s++)
//    {
//      switch (s)
//      {
//        case 0:
//          strcpy(configData.tagId, dataa[s]);
//          break;
//        case 1:
//          configData.forceThreshold = atoi(dataa[s]);
//          break;
//        case 2:
//          configData.handlePosition = atoi(dataa[s]);
//          break;
//        default:
//          holdArray[s-3] = atoi(dataa[s]);
//          break;
//      }
//    }
//  }

  if (commaCount == 1) {
    Serial.println("Force Calibration Data Received.");
    char * strtokIndx; // this is used by strtok() as an index

    strtokIndx = strtok(receiveBuffer, ",");
    calibrationData.slope = atoi(strtokIndx);

    strtokIndx = strtok(NULL, ",");
    calibrationData.zero = atoi(strtokIndx);
    writeToFlashNeeded = 1;
  }
  initialForceCri = configData.forceThreshold;
  if (percentileoff == 1) {
    initialForceCri = fixedvalue;
  }
  if (thisratfinalcriterion == 1) {
    initialForceCri = 120;
  }
  //if rat is supposed to be at fixed 120 even with the percentile on for others:
  //int donothing = 0;
  //        for (int m = 0; m < 16; ++m)
  //  { if (animalID[m] != ratfixedone[m]) {
  //      donothing = 1;
  //      Serial.println("notafixed");
  //    }}
  //            for (int m = 0; m < 16; ++m)
  //  { if (animalID[m] != ratfixedtwo[m]) {
  //      donothing = 1;
  //            Serial.println("notafixed");
  //    }}

  //    if (donothing == 0){
  //        initialForceCri = 120;
  //        thisratfinalcriterion = 1;
  //        Serial.println(thisratfinalcriterion);
  //    }
  //    else {thisratfinalcriterion = 0;}
  Serial.print("initialForceCrigotchanged:");
  Serial.println(initialForceCri);
  for (int i = 0; i < percentileArraySize; ++i) {
    pullBuffer[i] = initialForceCri;
  }
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
    Serial.print("Tag ID is: ");
    Serial.println(configData.tagId);
    Serial.print("Force Threshold is: "); //this would need to be changed for activationThreshold
    Serial.println(configData.forceThreshold);
    Serial.print("Handle Position is: "); //this would need to be changed for hold duration
    Serial.println(configData.handlePosition);
    //we would also add the third one for reporting the array, right?
    Serial.print("Slope is: ");
    Serial.println(calibrationData.slope);
    Serial.print("Zero is: ");
    Serial.println(calibrationData.zero);
    minthreshold = configData.forceThreshold - 20;
    newData = false;
  }
}

void writeEnding()
{
  int add = (activationThreshold - configData.forceThreshold);
  if (add < 0) {
    add = 0;
  }
  Serial.print("<write");
  for (int i = 0; i < 16; ++i) {
    Serial.print(animalID[i]);
  }
  Serial.print(",");
  if (percentileoff == 1) {
    //Serial.print(fixedvalue); //commenting out because this was what was needed for the traditional fixed value.
    Serial.print(activationThreshold);
    Serial.print(",");
    Serial.print(holdCriterion);
    for (int v = 0; v < holdArraySize; v++) {
      Serial.print(",");
      Serial.print(holdArray[v]);
    }


  }
  else {
    Serial.print(configData.forceThreshold + add);
  }

  Serial.println(">");
}

void checkdoor()
{ currentSwitch = digitalRead(microswitchRFID);
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
      stimtoggle = 0;
      notes = "stimulation off";
      sendData();
      for (int i = 0; i < 16; i++) {
        animalID[i] = 0;
      }
    }
  }
}

void checkforreset ()
{
  //  Serial.print(animalin);
  //  Serial.print(";");
  //    Serial.print(prevanimalin);
  //      Serial.print(";");
  //      Serial.println(newanimal);
  if ((animalin == 1) && (prevanimalin == 0))
  { //Serial.println("gothere");
    restart();
    prevanimalin = 1;
    newanimal = 0;
  }
  if ((animalin == 1) && (newanimal == 1))
  { restart();
    newanimal = 0;
  }
}

void restart() //I feel like we may need to change something here as well.
{
  //configData.forceThreshold = 5;
  Serial.print("<request");
  for (int i = 0; i < 16; ++i) {
    Serial.print(animalID[i]);
  }
  Serial.println(">");

  //initialForceCri = configData.forceThreshold; //this is the version that worked with fixed value, commented out for my hold version
  //  if (percentileoff == 1) {
  //    initialForceCri = fixedvalue;
  //    Serial.print("fixedValueMode ");
  //  }
  if (thisratfinalcriterion == 1) {
    initialForceCri = 120;
    Serial.print("finalValue120Mode");
  }
  Serial.print("initialForceCri:");
  Serial.println(initialForceCri);
  //  attemptsThisSession = 0;

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

  if (thisratfinalcriterion == 1) {
    for (int i = 0; i < percentileArraySize; ++i) {
      pullBuffer[i] = 120;
    }
  }
  if (percentileoff == 0) {
    for (int i = 0; i < percentileArraySize; ++i) {
      pullBuffer[i] = initialForceCri;
    }
  }
  else {
    for (int i = 0; i < percentileArraySize; ++i) {
      pullBuffer[i] = fixedvalue;
    }
  }
  pinMode(microswitchRFID, INPUT);
  digitalWrite(microswitchRFID, HIGH);
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
  forceTriggered = 0;
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

  //int responseAttempts[holdArraySize]
  //for (int z = 0; z < holdArraySize; z++){
  //  responseAttempts[z] =
  //}


  int pullTemp[percentileArraySize];
  for (int i = 0; i < percentileArraySize; ++i) {
    pullTemp[i] = pullBuffer[i];
  }

  sortArray(pullTemp, percentileArraySize);

  //activationThreshold = pullTemp[(percentileArraySize - 1) / 2];  //THIS WAS COMMENTED OUT becauase it was overwriting my updating of the activationthreshold.
  //  if (activationThreshold < minthreshold) {
  //    Serial.println("activation threshold is less than minthreshold");
  //    activationThreshold = minthreshold;
  //  }

  forceValprev = forceVal;
  delay(1);
  forceValraw = analogRead(forceSensorPin);
  forceVal = (forceValraw - calibrationData.zero) / calibrationData.slope;

  evaluateInputStates();

  //trying to program moving to next force step for an animal here...
//  evaluateShapingStep();



  if ((forceTriggered) && (forceReporter - millis() > 100))
  {
    //Serial.println("when does this happen");
    //I have no idea what this part is adding to the code. I commented it out and it still seemed to continuously record behavior.
    transmitData = 1;
  }

  if (millis() - heartbeatTimer > heartbeatTimeout)
  {
    transmitHeartbeat();
  }

  if (transmitData)
  {
    //sending data continuously
    //Serial.println("sending data");
    //this seems to be responsible for actually sending the data.
    sendData();
    transmitData = 0;
  }

  if (autoshaping && animalin)
  {
    //Serial.println("autoshaping?");
    randomFire();
  }
}
