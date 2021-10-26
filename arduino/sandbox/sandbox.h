#define heartbeatInterval 2000
#define requestInterval 1100
#define receivetimeout 1000 // Time to successfully send a fully-formed serial message before it bails.

void writeToFlash();

/*
 * Communications Protocol:
 * Start character is <, end character is >
 * 
 * Requests for data from the file are in the form of <request#> 
 *    where "#" is the RFID of the rat
 * 
 * Requests to write the data to the file is in the form of <write#1,#2,#3>
 *    where "#1" is the RFID of the rat,
 *    "#2" is the force threshold
 *    "#3" is the handle position
 */
