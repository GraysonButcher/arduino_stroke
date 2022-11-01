#define percentileoff                         1        //turns percentile changes off (1) or on (0)

//#define fixedvalue                            100       //pull criteron value when percentile is off

#define percentileArraySize                  15      //must be odd number - number of past pulls with which new criterion will be set

#define holdArraySize                       100     //How many previous and consecutive trials are tracked for computing success rate of hold attempts 

#define solenoidOutputPin                   8       // Use the LED pin for output diagnostics

#define rxpin                                9      //
 
#define txpin                                 3     //

#define soleniodOn                          LOW     // Voltage state when solenoid energized

#define soleniodOff                         HIGH      // Voltage state when solenoid NOT nergized

#define heartbeatTimeout                    3000     // Time (in ms) between heartbeat messages indicating micro is still alive

#define NumberOfCharactersInReport          40       // Size of the report going back to the PC

#define handshake                           0        // Requires a full handshake between PC and micro to start code execution

#define InitialMasterDebug                  0        // Initial state of the master debug flag (used to capture debug on micro bootup)

#define initialFilterTimeout                100      // Initial value used for the debounce filter timeout

#define forceSensorPin                      0

#define hysteresis                          1        // Must be an even number

#define pullThreshold                       5       // Threshold of force sensor that initiates a pull. This number should never be set higher than the activationThreshold variable on the main page. 

#define pullThresholdhigh                   pullThreshold + hysteresis

#define pullThresholdlow                    pullThreshold - hysteresis

#define highThreshold                       (activationThreshold + hysteresis)

#define lowThreshold                        (activationThreshold - hysteresis)

#define autoshaping                          0      //1 8op8for yes, 0 for no

#define minRandomValue                        30000      //minimum time between autoshaping random firings

#define maxRandomValue                        60000      //max time between autoshaping random firings

/*******************************************
* DO NOT ADJUST THE BELOW VARIABLES WITHOUT PERMISSION!!!! 
* Also note the below will not be reported to the logs. 
*******************************************/

#define heartbeatCharacter                   "h"   // Character sent from micro indicating the micro is still alive

#define startChar                            's'     // Start byte indicating an adjustment of the switch debounce timeout

#define endChar                              'e'     // End byte indicating an adjustment of the switch debounce timeout

#define ridiculousFilterLimit                3000    // Limit of filter timing (in ms) beyond which micro will reject

#define timeoutRejectedChar                  "c"     // If the timeout limit is exceeded, micro tells PC with this character, short for 'crappy'

#define timeoutAcceptedChar                  "a"     // if the new timeout is accepted, micro sends this back to PC

#define baudRate                             9600   // Baud to be used in serial communication with PC

// PC COMMUNICATION VARIABLES
#define PCReadyCharacter                     'r'     // Character the micro is waiting for from PC to start data collection

#define MasterDebugCharacter                 'd'     // Character that turns on and off the MASTERDEBUG flag used to puke debug messages out the serial port

#define MicroStartCharacter                  'b'     // Character transmitted when micro begins execution
