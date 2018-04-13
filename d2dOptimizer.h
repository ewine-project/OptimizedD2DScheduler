#ifndef D2DOPTIMIZER_H
#define D2DOPTIMIZER_H

#include <iostream>
#include <fstream>
#include <algorithm>
#include <cstdlib>
#include <vector>
#include <deque>
#include <sstream>
#include <math.h>  
#include <string> 
#include <stdint.h>

#include "gurobi_c++.h"

#define STATE_IDLE 0
#define STATE_STARTAP 1
#define STATE_STARTCLIENT 2

using namespace std;

struct channel {
	uint64_t id;
	
	channel(uint64_t id) {
		this->id = id;
	}
};

struct content {
	uint64_t id;
	uint32_t size;
	
	content(uint64_t id, uint32_t size) {
		this->id	= id;
		this->size 	= size;
	}
};

struct device {
	uint64_t id;
	
	std::vector<channel*> supportedChannelVector;
	std::vector<content*> contentProviderVector;
	std::vector<content*> contentInterestVector;
	
	device(uint64_t id) {
		this->id = id;
	}
	
};

struct d2dSettings {
	// number of time slots per superslot
	int64_t numTimeSlots;

	// length of a superslot in seconds
	double tau;

	//number of time slots required to start an AP
	int32_t kappa_startAP;
	
	//number of time slots required to join a network as client
	int32_t kappa_startClient;
	
	double slotLengthInSeconds() {
		return tau/(double)numTimeSlots;
	}
};

class d2dOptimizer {
	
	public:
		d2dSettings settings;
		bool debug;
		bool debugConstraints;
		bool debugResults;
		
		bool setVariableNames;
		bool setConstraintNames;
		
		GRBEnv *env;
		GRBModel *model;
		
		//Parameters
		int **P;
		int **I;
		int **W;
		long ***L;
		long *S;
		int ****X;
		double *** U;
		
		int *START_Idle;
		int **START_AP;  	
		int **START_Client;  
		
		std::vector<channel*> channelVector;
		std::vector<content*> contentVector;
		std::vector<device*> deviceVector;
		
		//Variables
		GRBVar **S_Idle;
		
		GRBVar ***S_StartAP;
		GRBVar ***S_SwitchAP;
		GRBVar ***S_AP;
	
		GRBVar ***S_StartClient;
		GRBVar ***S_JoinAP;
		GRBVar ***S_Client;
		
		//y[d_prv][d_cns][n][c][t]
		GRBVar *****y;
		
		//p[d][n][t] 
		GRBVar ***p;
		
		// h[d_prv][d_cns][n][c][t]
		GRBVar *****h;
		//r[d][c][t]
		GRBVar ***r;
		//s[d][c][t]
		GRBVar ***s;
			
		d2dOptimizer();
		~d2dOptimizer();
		
		void readBasicParamters();
		content* addContent(uint64_t id, uint32_t size);
		device*  addDevice(uint64_t id);
		channel* addChannel(uint64_t id);
		
		device *getDeviceById(uint64_t id) ;
		int32_t getDeviceIndexById(uint64_t id);

		channel *getChannelById(uint64_t id);
		int32_t getChannelIndexById(uint64_t id);

		content *getContentById(uint64_t id);
		int32_t getContentIndexById(uint64_t id);
		
		void readParameters();
		void allocateParameters();
		void printParameterSimple();
		void printParameter();
		void printResults();
		int32_t optimize();
};

#endif