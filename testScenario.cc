#include "testScenario.h"

void testScenario::fillVectors(d2dOptimizer *opt, int numChannels, int numDevices, int numContent) {
	for (int c=0; c<numChannels; c++) {
		opt->channelVector.push_back(new channel(c));
	}
	
	for (int d=0; d<numDevices; d++) {
		opt->deviceVector.push_back(new device(d));
	}
	
	for (int n=0; n<numContent; n++) {
		opt->contentVector.push_back(new content(n,1));
	}
}

void testScenario::setupScenario(int scenario, d2dOptimizer *opt) {
	
	int numChannels = 0;
	int numDevices  = 0;
	int numContent  = 0;
	
	switch(scenario) {
		case TEST_RELAY:
			cout << "TEST_RELAY" << endl;
			
			numChannels = 2;
			numDevices  = 3;
			numContent  = 1;
			
			fillVectors(opt, numChannels, numDevices, numContent);
			opt->allocateParameters();
			
			opt->S[0] = 1;
			
			opt->P[0][0] = 1;
			opt->I[1][0] = 1;
			opt->I[2][0] = 1;
			
			opt->W[0][0] = 1;
			opt->W[1][0] = 1;
			opt->W[1][1] = 1;
			opt->W[2][1] = 1;
			
			opt->U[0][1][0] = 1;
			opt->U[1][2][0] = 1;
			
			opt->L[0][1][0] = 1;
			opt->L[1][2][1] = 1;
			
			opt->settings.numTimeSlots 		= 10;
			opt->settings.tau 				= 10.0;
			opt->settings.kappa_startAP		= 1;
			opt->settings.kappa_startClient	= 1;

			break;
		case TEST_FREQ_DIVERSITY:
			cout << "TEST_FREQ_DIVERSITY" << endl;
			
			numChannels = 2;
			numDevices  = 4;
			numContent  = 2;
			
			fillVectors(opt, numChannels, numDevices, numContent);
			opt->allocateParameters();
			
			opt->S[0] = 5;
			opt->S[1] = 7;
			
			// dev 0 -> 1 content 0 chan 0
			// dev 2 -> 3 content 1 chan 1
			opt->P[0][0] = 1;
			opt->I[1][0] = 1;
			
			opt->P[2][1] = 1;		
			opt->I[3][1] = 1;
			
			for (int d=0; d<numDevices; d++) {
				for (int c=0; c<numChannels; c++) {
					opt->W[d][c] = 1;
				}
			}
			
			opt->U[0][1][0] = 1;
			opt->U[2][3][1] = 1;
			
			opt->L[0][1][0] = 1;
			opt->L[0][1][1] = 2;
			opt->L[2][3][0] = 1;
			opt->L[2][3][1] = 3;
			
			opt->settings.numTimeSlots 		= 10;
			opt->settings.tau 				= 10.0;
			opt->settings.kappa_startAP		= 1;
			opt->settings.kappa_startClient	= 1;
		
			break;
		case TEST_ONLY_CONTENT_ONCE:
			cout << "TEST_ONLY_CONTENT_ONCE" << endl;
		
			numChannels = 1;
			numDevices  = 3;
			numContent  = 1;
			
			fillVectors(opt, numChannels, numDevices, numContent);
			opt->allocateParameters();
			
			opt->S[0] = 1;
			
			opt->P[0][0] = 1;
			opt->P[1][0] = 1;
			opt->I[2][0] = 1;
			
			opt->W[0][0] = 1;
			opt->W[1][0] = 1;
			opt->W[2][0] = 1;
			
			opt->U[0][2][0] = 1;
			opt->U[1][2][0] = 1;
			
			opt->L[0][2][0] = 1;
			opt->L[1][2][1] = 1;
			
			opt->settings.numTimeSlots 		= 10;
			opt->settings.tau 				= 10.0;
			opt->settings.kappa_startAP		= 1;
			opt->settings.kappa_startClient	= 1;

			break;
		case TEST_START_AP_CLIENT:
			cout << "TEST_START_AP_CLIENT" << endl;
		
			numChannels = 1;
			numDevices  = 2;
			numContent  = 1;
			
			fillVectors(opt, numChannels, numDevices, numContent);
			opt->allocateParameters();
			
			opt->S[0] = 1;
			
			opt->P[0][0] = 1;
			opt->I[1][0] = 1;
			
			opt->W[0][0] = 1;
			opt->W[1][0] = 1;
			
			opt->U[0][1][0] = 1;
			
			opt->L[0][1][0] = 1;
			
			opt->START_Idle[0] = 0;
			opt->START_Idle[1] = 0;
			
			opt->START_AP[0][0] 	= 1;
			opt->START_Client[1][0] = 1;
			
			
			opt->settings.numTimeSlots 		= 10;
			opt->settings.tau 				= 10.0;
			opt->settings.kappa_startAP		= 1;
			opt->settings.kappa_startClient	= 1;
			
			break;
			
		case TEST_START_AP_CLIENT_AP:
			cout << "TEST_START_AP_CLIENT_AP" << endl;
			
			numChannels = 1;
			numDevices  = 3;
			numContent  = 1;
			
			fillVectors(opt, numChannels, numDevices, numContent);
			opt->allocateParameters();
			
			opt->S[0] = 3;
			
			opt->P[1][0] = 1;
			opt->I[2][0] = 1;
			
			opt->W[0][0] = 1;
			opt->W[1][0] = 1;
			opt->W[2][0] = 1;
			
			opt->U[1][2][0] = 1;
			
			opt->L[1][2][0] = 1;
			
			opt->START_Idle[0] = 0;
			opt->START_Idle[1] = 0;
			
			opt->START_AP[0][0] 	= 1;
			opt->START_Client[1][0] = 1;
			
			
			opt->settings.numTimeSlots 		= 10;
			opt->settings.tau 				= 10.0;
			opt->settings.kappa_startAP		= 1;
			opt->settings.kappa_startClient	= 1;
			
			break;
			
		case TEST_NO_COMMON_CHANNEL:
			cout << "TEST_NO_COMMON_CHANNEL" << endl;
			numChannels = 2;
			numDevices  = 2;
			numContent  = 1;
			
			fillVectors(opt, numChannels, numDevices, numContent);
			opt->allocateParameters();
			
			opt->S[0] = 1;
			
			opt->P[0][0] = 1;
			opt->I[1][0] = 1;
			
			opt->W[0][0] = 1;
			opt->W[1][1] = 1;
			
			opt->U[0][1][0] = 1;
			
			opt->L[0][1][0] = 1;
			opt->L[0][1][1] = 1;
						
			opt->settings.numTimeSlots 		= 10;
			opt->settings.tau 				= 10.0;
			opt->settings.kappa_startAP		= 1;
			opt->settings.kappa_startClient	= 1;
			
			break;
			
		case TEST_LONG_KAPPA_START:
			cout << "TEST_LONG_KAPPA_START" << endl;
		
			numChannels = 1;
			numDevices  = 2;
			numContent  = 1;
			
			fillVectors(opt, numChannels, numDevices, numContent);
			opt->allocateParameters();
			
			opt->S[0] = 1;
			
			opt->P[0][0] = 1;
			opt->I[1][0] = 1;
			
			opt->W[0][0] = 1;
			opt->W[1][0] = 1;
			
			opt->U[0][1][0] = 1;
			
			opt->L[0][1][0] = 1;
			
			opt->settings.numTimeSlots 		= 10;
			opt->settings.tau 				= 10.0;
			opt->settings.kappa_startAP		= 20;
			opt->settings.kappa_startClient	= 1;
			break;
		
		case TEST_RANDOM:
		{
			cout << "TEST_RANDOM" << endl;
			
			long seed = time(NULL);
			srand (seed);
			cout << "Seed " << seed << endl;
		
			numChannels = 1 + (rand() % 3);
			numDevices  = 2 + (rand() % 5);
			numContent  = 1 + (rand() % 5);

			fillVectors(opt, numChannels, numDevices, numContent);
			opt->allocateParameters();
				
			for (int n=0; n<numContent; n++) {
				opt->S[n] =  1 + (rand() % 3);
			}
			
			for (int dev=0; dev<numDevices; dev++) {
				if ((rand() % 100) > 60) {
					opt->P[dev][rand() % numContent] = 1;
				}
				if ((rand() % 100) > 50) {
					opt->I[dev][rand() % numContent] = 1;
				}
				if ((rand() % 100) > 20) {
					opt->W[dev][rand() % numChannels] = 1;
				}
			}
			
			
			for (int d_prv=0; d_prv<numDevices; d_prv++) {
				for (int d_cns=0; d_cns<numDevices; d_cns++) {
					for (int n=0; n<numContent; n++) {
						if ((rand() % 100) > 30) {
							opt->U[d_prv][d_cns][n] =  1 + (rand() % 10);
						}
						
					}	
					for (int c=0; c<numChannels; c++) {
						if ((rand() % 100) > 20) {
							opt->L[d_prv][d_cns][c] =  1 + (rand() % 3);
						}	
					}
				}
			}
						
			opt->settings.numTimeSlots 		= 15;
			opt->settings.tau 				= 10.0;
			opt->settings.kappa_startAP		= 3;
			opt->settings.kappa_startClient	= 2;	
		}
			break;
		case TEST_INTERFERENCE:
			//device 1 sends to device 2 on channel 1 and creates interference on c2 so that d2 and d3 cannot exchange
			cout << "TEST_INTERFERENCE" << endl;
		
			numChannels = 2;
			numDevices  = 4;
			numContent  = 2;
			
			fillVectors(opt, numChannels, numDevices, numContent);
			opt->allocateParameters();
			
			opt->S[0] = 4;
			//with size 2 it schedules it after tx of content 0 from d1->d2
			//with size 3 it is not possible to schedule both exchanges in a superslot of k=10
			opt->S[1] = 2;
			
			opt->P[0][0] = 1;
			opt->I[1][0] = 1;
			opt->P[2][1] = 1;
			opt->I[3][1] = 1;	
		
			opt->W[0][0] = 1;
			opt->W[1][0] = 1;
			opt->W[2][1] = 1;
			opt->W[3][1] = 1;
			
			opt->U[0][1][0] = 5;
			opt->U[2][3][1] = 1;
			
			opt->L[0][1][0] = 1;
			opt->L[2][3][1] = 1;
			
			opt->X[0][0][3][1] = 1;
			
			opt->settings.numTimeSlots 		= 10;
			opt->settings.tau 				= 10.0;
			opt->settings.kappa_startAP		= 1;
			opt->settings.kappa_startClient	= 1;
			break;
		case TEST_NO_EXCHANGE_IF_ALREADY_AVAIL:
			cout << "TEST_NO_EXCHANGE_IF_ALREADY_AVAIL" << endl;
			
			numChannels = 1;
			numDevices  = 2;
			numContent  = 1;
			
			fillVectors(opt, numChannels, numDevices, numContent);
			opt->allocateParameters();
			
			opt->S[0] = 1;
			
			opt->P[0][0] = 1;
			opt->I[0][0] = 1;
			opt->P[1][0] = 1;
			
			opt->W[0][0] = 1;
			opt->W[1][0] = 1;
			
			opt->U[1][0][0] = 1;
			
			opt->L[1][0][0] = 1;
			
			opt->settings.numTimeSlots 		= 10;
			opt->settings.tau 				= 10.0;
			opt->settings.kappa_startAP		= 1;
			opt->settings.kappa_startClient	= 1;
			
			break;
		default:
			cerr << "Unknown test scenario " << scenario << endl;
			exit (EXIT_FAILURE);
			break;
	}
	
}