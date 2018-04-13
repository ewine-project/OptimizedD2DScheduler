#include "testScenario.h"

int main(int argc, char** argv) {
	
	
	int test = TEST_NONE;
	if (argc > 1) {
		test = atoi(argv[1]);
	}
	cout << "Running Test Scenario " << test << endl;

	d2dOptimizer *d2dOpt = new d2dOptimizer();
	
	d2dOpt->settings.numTimeSlots 		= 10;
	d2dOpt->settings.tau 				= 10.0;
	d2dOpt->settings.kappa_startAP		= 3;
	d2dOpt->settings.kappa_startClient	= 2;
	
	switch(test) {
		case TEST_NONE:
			d2dOpt->readBasicParamters();
			d2dOpt->allocateParameters();
			d2dOpt->readParameters();
			break;
		default:
			testScenario::setupScenario(test, d2dOpt);
			
	}		
	d2dOpt->printParameter();
	d2dOpt->optimize();
	
	return 1;
}
