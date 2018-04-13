#ifndef TESTSCENARIO_H
#define TESTSCENARIO_H

#include "d2dOptimizer.h"
#include <stdlib.h>
#include <time.h> 

#define TEST_NONE 				0

// 0 -> 1 -> 2
// 3 devices, device 0 has content, devices 1 and 2 are interested in
// device 0 is capable of using channel 0, devices 1 channel 0,1 and device 2 channel 1
#define TEST_RELAY 				1

// 0-> 1 on c0
// 2-> 3 on c1
#define TEST_FREQ_DIVERSITY		2

//two devices are provider of content n, third device is interested in this content
#define TEST_ONLY_CONTENT_ONCE 	3

// two devices are already in client and AP mode
#define TEST_START_AP_CLIENT 4

// two devices are already in client and AP mode in the same network
//third device also AP mode has content client is interested in
#define TEST_START_AP_CLIENT_AP 5

//two devices, the intersection of both channel sets is empty
//no exchange should be scheduled
#define TEST_NO_COMMON_CHANNEL 6

//time to setup an AP is longer the number of time slots in a super slots
//testing loop boundaries
#define TEST_LONG_KAPPA_START 7

//generate random scenarios
#define TEST_RANDOM 8

//Interference test
//device 1 sends to device 2 on channel 1 and creates interference on c2 so that d2 and d3 cannot exchange
#define TEST_INTERFERENCE 9

//no exchange should occur if a device has already the content initially
#define TEST_NO_EXCHANGE_IF_ALREADY_AVAIL 10

class d2dOptimizer;
class testScenario {
	
	public:
		static void setupScenario(int scenario, d2dOptimizer *opt);
		static void fillVectors(d2dOptimizer *opt, int numChannels, int numDevices, int numContent);
};

#endif