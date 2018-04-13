#include "d2dOptimizer.h"

std::vector<std::string> split_string(const std::string& str,
                                      const std::string& delimiter,
									  bool skipMultiple)
{
    std::vector<std::string> strings;

    std::string::size_type pos = 0;
    std::string::size_type prev = 0;
    while ((pos = str.find(delimiter, prev)) != std::string::npos)
    {
		std::string newToken = str.substr(prev, pos - prev);
		if (newToken.length() > 0 && (!skipMultiple || newToken.compare(delimiter) != 0)) {
			strings.push_back(str.substr(prev, pos - prev));
		}
        prev = pos + 1;
    }

    // To get the last substring (or only, if delimiter is not found)
    strings.push_back(str.substr(prev));

    return strings;
}

d2dOptimizer::d2dOptimizer() {

	settings.numTimeSlots 		= 10;
	settings.tau 				= 10.0;
	settings.kappa_startAP		= 3;
	settings.kappa_startClient	= 2;
	
	debug 				= true;
	debugConstraints 	= false;
	debugResults 		= true;
	
	setVariableNames 	= false;
	setConstraintNames 	= false;
	
	P 	= NULL;
	I 	= NULL;
	W	= NULL;
	L	= NULL;
	S	= NULL;
	X	= NULL;
	U	= NULL;

	START_Idle		= NULL;
	START_AP		= NULL;  	
	START_Client	= NULL;
	
	try {
		env = new GRBEnv();
		model = new GRBModel(*env);	
	} catch(GRBException e) {
		cout << "Error code = " << e.getErrorCode() << endl;
		cout << e.getMessage() << endl;
		exit(-1);
	} catch (...) {
		cout << "Error in constructor" << endl;
		exit(-1);
	}
}


d2dOptimizer::~d2dOptimizer() {

	for (uint32_t d = 0; d<deviceVector.size(); d++) {
		if (P != NULL &&
			P[d] != NULL) 
			delete [] P[d];
			
		if (I != NULL && 
			I[d] != NULL) 
			delete []  I[d];
		
		for (uint32_t d_cns = 0; d_cns<deviceVector.size(); d_cns++) {
			if (L != NULL && 
				L[d] != NULL &&
				L[d][d_cns] != NULL) 
				delete []  L[d][d_cns];
			
			if (U != NULL && 
				U[d] != NULL && 
				U[d][d_cns] != NULL) 
				delete []  U[d][d_cns];
		}
		
		if (U != NULL && 
			U[d] != NULL) 
			delete []  U[d];
		if (L != NULL && 
			L[d] != NULL) 
			delete []  L[d];
			
		for (uint32_t c = 0; c<channelVector.size(); c++) {	
			for (uint32_t d_int = 0; d_int<deviceVector.size(); d_int++) {
	
				if (	X != NULL && 
						X[d] != NULL &&
						X[d][c] != NULL &&
						X[d][c][d_int] != NULL) {
					delete [] X[d][c][d_int];
				}
			}

			if (X != NULL && 
				X[d] != NULL &&
				X[d][c] != NULL)
				delete [] X[d][c];
		}
		if (X != NULL && 
			X[d] != NULL)
			delete [] X[d];
			
		if (W != NULL &&
			W[d] != NULL)
			delete [] W[d];
	}
	
	if (P != NULL)
		delete [] P;
	if (I != NULL)
		delete [] I;
	if (W != NULL)
		delete [] W;
	if (S != NULL)
		delete [] S;
	if (L != NULL)
		delete [] L;
	if (U != NULL)
		delete [] U;
	if (X != NULL)
		delete [] X;
	
	for (uint32_t d = 0; d<deviceVector.size(); d++) {
		if (START_AP[d] != NULL) 
			delete [] START_AP[d];

		if (START_Client[d] != NULL) 
			delete [] START_Client[d];
	}
	
	if (START_Idle != NULL)
		delete [] START_Idle;
	if (START_AP != NULL)
		delete [] START_AP;
	if (START_Client != NULL)
		delete [] START_Client;
	
	for (std::vector<channel*>::iterator it = channelVector.begin() ; it != channelVector.end(); ++it) {
		delete (*it);
	} 
	channelVector.clear();
	
	for (std::vector<device*>::iterator it = deviceVector.begin() ; it != deviceVector.end(); ++it) {
		delete (*it);
	} 
	deviceVector.clear();
	
	for (std::vector<content*>::iterator it = contentVector.begin() ; it != contentVector.end(); ++it) {
		delete (*it);
	} 
	contentVector.clear();
	
	delete model;
	delete env;
}

device* d2dOptimizer::getDeviceById(uint64_t id) {	
	for (uint32_t i=0; i<deviceVector.size(); i++) {	
		if (deviceVector.at(i)->id == id) {
			return deviceVector.at(i);
		}
	}
	return NULL;
}

int32_t d2dOptimizer::getDeviceIndexById(uint64_t id) {
	for (uint32_t i=0; i<deviceVector.size(); i++) {
		if (deviceVector.at(i)->id == id) {
			return i;
		}
	}
	return -1;
}

channel* d2dOptimizer::getChannelById(uint64_t id) {
	for (uint32_t i=0; i<channelVector.size(); i++) {
		if (channelVector.at(i)->id == id) {
			return channelVector.at(i);
		}
	}
	return NULL;
}

int32_t d2dOptimizer::getChannelIndexById(uint64_t id) {
	for (uint32_t i=0; i<channelVector.size(); i++) {
		if (channelVector.at(i)->id == id) {
			return i;
		}
	}
	return -1;
}

content* d2dOptimizer::getContentById(uint64_t id) {
	for (uint32_t i=0; i<contentVector.size(); i++) {
		if (contentVector.at(i)->id == id) {
			return contentVector.at(i);
		}
	}
	return NULL;
}

int32_t d2dOptimizer::getContentIndexById(uint64_t id) {
	for (uint32_t i=0; i<contentVector.size(); i++) {
		if (contentVector.at(i)->id == id) {
			return i;
		}
	}
	return -1;
}

content* d2dOptimizer::addContent(uint64_t id, uint32_t size) {
	content *cont = NULL;
	if ((cont = getContentById(id)) == NULL) {
		cont = new content(id, size);
		contentVector.push_back(cont);
	}
	return cont;
}

device* d2dOptimizer::addDevice(uint64_t id) {
	device *dev = NULL;
	if ((dev = getDeviceById(id)) == NULL) {
		dev = new device(id);	
		deviceVector.push_back(dev);
	}
	return dev;
}

channel* d2dOptimizer::addChannel(uint64_t id) {
	channel *chan = NULL;
	if ((chan = getChannelById(id)) == NULL) {
		chan = new channel(id);
		channelVector.push_back(chan);
	}
	return chan;
}

void d2dOptimizer::readBasicParamters() {
	std::string line;
	
	std::string fname = "channels.dat";
	std::ifstream infile(fname.c_str());
	if (!infile.is_open()) {
		cerr << "Cannot open " << fname << endl;
		return;
	}
	while (std::getline(infile, line)) {
		if (line.substr(0,1) == "#") {
			continue;
		}
		std::vector<std::string> tokens = split_string(line, "\t", true);		
		if (tokens.size() != 1) {
			cerr << "wrong number of tokens " << tokens.size() << " in file " << fname <<  endl;
			cerr << line << endl;
			return;
		}
		uint64_t tmpId = atol(tokens.at(0).c_str());
		if (getChannelById(tmpId) != NULL) {
			cerr << "channel id " << tmpId << " not unique " << endl;
			return;
		}
		addChannel(tmpId);
	}
	
	fname = "content.dat";
	std::ifstream contentfile(fname.c_str());
	if (!contentfile.is_open()) {
		cerr << "Cannot open " << fname << endl;
		return;
	}
	while (std::getline(contentfile, line)) {
		if (line.substr(0,1) == "#") {
			continue;
		}
		std::vector<std::string> tokens = split_string(line, "\t", true);		
		if (tokens.size() != 2) {
			for (uint32_t i=0; i<tokens.size(); i++) {
				cout << i << "\t" << tokens.at(i) << endl;
			}
			cerr << "wrong number of tokens " << tokens.size() << " in file " << fname <<  endl;
			cerr << line << endl;
			return;
		}
		uint64_t tmpId = atol(tokens.at(0).c_str());
		if (getContentById(tmpId) != NULL) {
			cerr << "content id " << tmpId << " not unique " << endl;
			return;
		}
		addContent(tmpId, atoi(tokens.at(1).c_str()));
	}
	
	fname = "devices.dat";
	std::ifstream devicefile(fname.c_str());
	if (!devicefile.is_open()) {
		cerr << "Cannot open " << fname << endl;
		return;
	}
	while (std::getline(devicefile, line)) {
		if (line.substr(0,1) == "#") {
			continue;
		}
		std::vector<std::string> tokens = split_string(line, "\t", true);		
		if (tokens.size() != 4) {
			for (uint32_t i=0; i<tokens.size(); i++) {
				cout << i << "\t" << tokens.at(i) << endl;
			}
			cerr << "wrong number of tokens " << tokens.size() << " in file " << fname <<  endl;
			cerr << line << endl;
			return;
		}
		uint64_t tmpId = atol(tokens.at(0).c_str()); 
		if (getDeviceById(tmpId) != NULL) {
			cerr << "device id " << tmpId << " not unique " << endl;
			return;
		}
		device *dev = NULL;
		if ((dev = addDevice(tmpId)) == NULL)
			return;
		
		// #deviceID (list of supported channel ids) (list of available content ids) (list of interested content ids)
		std::vector<std::string> channelTokens = split_string(tokens.at(1), ",", true);	
		for (uint32_t i=0; i<channelTokens.size(); i++) {
			if (channelTokens.at(i).compare("-") == 0) {
				continue;
			}
			uint64_t tmpIChanId = atol(channelTokens.at(i).c_str());
			channel *tmpChannel = getChannelById(tmpIChanId);
			if (tmpChannel == NULL) {
				cerr << "Cannot locate channel with id " << tmpIChanId << " for device " << tmpId << endl;
				return;
			}
			dev->supportedChannelVector.push_back(tmpChannel);
		}
		
		uint64_t tmpContentId;
		content *tmpContent;
		std::vector<std::string> contentTokens = split_string(tokens.at(2), ",", true);	
		for (uint32_t i=0; i<contentTokens.size(); i++) {
			if (contentTokens.at(i).compare("-") == 0) {
				continue;
			}
			tmpContentId = atoi(contentTokens.at(i).c_str());
			tmpContent = getContentById(tmpContentId);
			if (tmpContent == NULL) {
				cerr << "Cannot locate content with id " << tmpContentId << " for device " << tmpId << endl;
				return;
			}
			dev->contentProviderVector.push_back(tmpContent);
		}
		
		contentTokens = split_string(tokens.at(3), ",", true);	
		for (uint32_t i=0; i<contentTokens.size(); i++) {
			if (contentTokens.at(i).compare("-") == 0) {
				continue;
			}
			tmpContentId = atol(contentTokens.at(i).c_str());
			tmpContent = getContentById(tmpContentId);
			if (tmpContent == NULL) {
				cerr << "Cannot locate content with id " << tmpContentId << " for device " << tmpId << endl;
				return;
			}
			dev->contentInterestVector.push_back(tmpContent);
		}
	}
}

void d2dOptimizer::allocateParameters() {
	P = new int*[deviceVector.size()];
	I = new int*[deviceVector.size()];
	W = new int*[deviceVector.size()];
	S = new long[contentVector.size()];
	L = new long**[deviceVector.size()];
	U = new double**[deviceVector.size()];
	X = new int***[deviceVector.size()];
	for (uint32_t d = 0; d<deviceVector.size(); d++) {
		P[d] = new int[contentVector.size()];
		I[d] = new int[contentVector.size()];
		
		for (uint32_t n = 0; n<contentVector.size(); n++) {	
			P[d][n] = 0;
			I[d][n] = 0;	
		}
		
		U[d] = new double*[deviceVector.size()];
		L[d] = new long*[deviceVector.size()];
		for (uint32_t d_cns = 0; d_cns<deviceVector.size(); d_cns++) {
			L[d][d_cns] = new long[channelVector.size()];
			for (uint32_t c = 0; c<channelVector.size(); c++) {	
				L[d][d_cns][c] = 0;
			}
			
			U[d][d_cns] =  new double[contentVector.size()]; 
			for (uint32_t n = 0; n<contentVector.size(); n++) {
				U[d][d_cns][n] = 0.0;
			}
		}
		
		W[d] = new int[channelVector.size()];
		X[d] = new int**[channelVector.size()];
		for (uint32_t c = 0; c<channelVector.size(); c++) {	
			W[d][c] = 0;
			
			X[d][c] = new int*[deviceVector.size()];
			for (uint32_t d_int = 0; d_int<deviceVector.size(); d_int++) {
				X[d][c][d_int] = new int[channelVector.size()];
				
				for (uint32_t c_int = 0; c_int<channelVector.size(); c_int++) {		
					X[d][c][d_int][c_int] = 0;
				}
			}	
		}
	}
	
	START_Idle 		= new int[deviceVector.size()];
	START_AP 		= new int*[deviceVector.size()];
	START_Client	= new int*[deviceVector.size()];
	
	for (uint32_t d = 0; d<deviceVector.size(); d++) {
		
		//put all devices in idle by default
		START_Idle[d] = 1;
		
		START_AP[d] = new int[channelVector.size()];
		for (uint32_t c = 0; c<channelVector.size(); c++) {	
			START_AP[d][c] = 0;
		}
		
		START_Client[d] = new int[deviceVector.size()];
		for (uint32_t d_ap = 0; d_ap<deviceVector.size(); d_ap++) {
			START_Client[d][d_ap] = 0;
		}
	}

}

void d2dOptimizer::readParameters() {
	std::string line;
	std::string fname = "channels.dat";
	
	//cout << endl << "Content " << endl;
	for (uint32_t n=0; n<contentVector.size(); n++) {
		//cout << "S[" << n << "] = " << contentVector.at(n)->size << endl;
		S[n] = contentVector.at(n)->size;
	}
	
	//cout << endl << "Devices " << endl;
	for (uint32_t d=0; d<deviceVector.size(); d++) {

		device *dev = deviceVector.at(d);
		for (uint32_t n=0; n<dev->contentProviderVector.size(); n++) {
			uint32_t contentIndex = getContentIndexById(dev->contentProviderVector.at(n)->id);
			P[d][contentIndex] = 1;
			//cout << "P[" << d << "][" << contentIndex << "]" << endl;
		}
		
		for (uint32_t n=0; n<dev->contentInterestVector.size(); n++) {
			uint32_t contentIndex = getContentIndexById(dev->contentInterestVector.at(n)->id);
			I[d][contentIndex] = 1;
			//cout << "I[" << d << "][" << contentIndex << "]" << endl;
		}
		
		for (uint32_t c=0; c<dev->supportedChannelVector.size(); c++) {
			uint32_t channelIndex = getChannelIndexById(dev->supportedChannelVector.at(c)->id);
			W[d][channelIndex] = 1;
			//cout << "W[" << d << "][" << channelIndex << "]" << endl;
		}
		//cout << endl;
	}

	fname = "interference.dat";
	std::ifstream interference(fname.c_str());
	if (!interference.is_open()) {
		cerr << "Cannot open " << fname << endl;
		return;
	}
	cout << "Interference " << endl;
	while (std::getline(interference, line)) {
		if (line.substr(0,1) == "#") {
			continue;
		}
		std::vector<std::string> tokens = split_string(line, "\t", true);		
		if (tokens.size() !=4 ) {
			for (uint32_t i=0; i<tokens.size(); i++) {
				cout << i << "\t" << tokens.at(i) << endl;
			}
			cerr << "wrong number of tokens " << tokens.size() << " in file " << fname <<  endl;
			cerr << line << endl;
			return;
		}
		//#transmittingDeviceId transmittingChannelId receivingDeviceId interferredChannelIdList
		
		uint32_t txDeviceIndex = getDeviceIndexById(atol(tokens.at(0).c_str()));
		if (txDeviceIndex < 0) {
			cerr << "unknown txDeviceId " << endl;
			cerr << line << endl;
			return;
		}
		uint32_t txChannelIndex = getChannelIndexById(atol(tokens.at(1).c_str()));
		if (txChannelIndex < 0) {
			cerr << "unknown txChannelId " << endl;
			cerr << line << endl;
			return;
		}
		uint32_t rxDeviceIndex = getDeviceIndexById(atol(tokens.at(2).c_str()));
		if (rxDeviceIndex < 0) {
			cerr << "unknown rxDeviceId " << endl;
			cerr << line << endl;
			return;
		}
			
		std::vector<std::string> channelTokens = split_string(tokens.at(3), ",", true);	
		for (uint32_t i=0; i<channelTokens.size(); i++) {
			
			uint32_t rxChannelIndex = getChannelIndexById(atol(channelTokens.at(i).c_str()));
			if (rxChannelIndex < 0) {
				cerr << "unknown rxChannelId " << endl;
				cerr << line << endl;
				return;
			}
			X[txDeviceIndex][txChannelIndex][rxDeviceIndex][rxChannelIndex] = 1;
			
			//cout << "X[" << txDeviceIndex << "][" << txChannelIndex << "][" << rxDeviceIndex << "][" << rxChannelIndex << "]" << endl;
		}
	}
	
	fname = "utility.dat";
	std::ifstream utilityfile(fname.c_str());
	if (!utilityfile.is_open()) {
		cerr << "Cannot open " << fname << endl;
		return;
	}
	//cout << endl << "Utility " << endl;
	while (std::getline(utilityfile, line)) {
		if (line.substr(0,1) == "#") {
			continue;
		}
		// #contentProvider contentConsumer contentId utilityValue
		std::vector<std::string> tokens = split_string(line, "\t", true);		
		if (tokens.size() != 4) {
			for (uint32_t i=0; i<tokens.size(); i++) {
				cout << i << "\t" << tokens.at(i) << endl;
			}
			cerr << "wrong number of tokens " << tokens.size() << " in file " << fname <<  endl;
			cerr << line << endl;
			return;
		}
		uint32_t providerIndex = getDeviceIndexById(atol(tokens.at(0).c_str()));
		if (providerIndex < 0) {
			cerr << "unknown providerId " << endl;
			cerr << line << endl;
			return;
		}
		uint32_t consumerIndex = getDeviceIndexById(atol(tokens.at(1).c_str()));
		if (consumerIndex < 0) {
			cerr << "unknown consumerId " << endl;
			cerr << line << endl;
			return;
		}
		uint32_t contentIndex = getContentIndexById(atol(tokens.at(2).c_str()));
		if (contentIndex < 0) {
			cerr << "unknown contentId " << endl;
			cerr << line << endl;
			return;
		}
		double utility = atof(tokens.at(3).c_str());
		U[providerIndex][consumerIndex][contentIndex] = utility;
		//cout << "U[" << providerIndex << "][" << consumerIndex << "][" << n << "] = " << utility  << endl;
	}
	
	
	fname = "link_speed.dat";
	std::ifstream linkspeedfile(fname.c_str());
	if (!linkspeedfile.is_open()) {
		cerr << "Cannot open " << fname << endl;
		return;
	}
	//cout << endl << "Link Speed " << endl;
	while (std::getline(linkspeedfile, line)) {
		if (line.substr(0,1) == "#") {
			continue;
		}
		// #contentProvider contentConsumer contentId utilityValue
		std::vector<std::string> tokens = split_string(line, "\t", true);		
		if (tokens.size() != 4) {
			for (uint32_t i=0; i<tokens.size(); i++) {
				cout << i << "\t" << tokens.at(i) << endl;
			}
			cerr << "wrong number of tokens " << tokens.size() << " in file " << fname <<  endl;
			cerr << line << endl;
			return;
		}
		// #contentProvider contentConsumer  channelId linkSpeed(not present or 0=no connectivity )
		uint32_t providerIndex = getDeviceIndexById(atol(tokens.at(0).c_str()));
		if (providerIndex < 0) {
			cerr << "unknown providerId " << endl;
			cerr << line << endl;
			return;
		}
		uint32_t consumerIndex = getDeviceIndexById(atol(tokens.at(1).c_str()));
		if (consumerIndex < 0) {
			cerr << "unknown consumerId " << endl;
			cerr << line << endl;
			return;
		}
		uint32_t channelIndex = getChannelIndexById(atol(tokens.at(2).c_str()));
		if (channelIndex < 0) {
			cerr << "unknown channelIndex " << endl;
			cerr << line << endl;
			return;
		}
		uint64_t speed = atol(tokens.at(3).c_str());
		L[providerIndex][consumerIndex][channelIndex] = speed;
		//cout << "L[" << providerIndex << "][" << consumerIndex << "][" << channelIndex << "] = " << speed << endl;
	}
	
	fname = "state.dat";
	std::ifstream statefile(fname.c_str());
	if (!statefile.is_open()) {
		cerr << "Cannot open " << fname << endl;
		return;
	}
	//cout << endl << "State " << endl;
	while (std::getline(statefile, line)) {
		if (line.substr(0,1) == "#") {
			continue;
		}
		
		std::vector<std::string> tokens = split_string(line, "\t", true);		
		if (tokens.size() != 3) {
			for (uint32_t i=0; i<tokens.size(); i++) {
				cout << i << "\t" << tokens.at(i) << endl;
			}
			cerr << "wrong number of tokens " << tokens.size() << " in file " << fname <<  endl;
			cerr << line << endl;
			return;
		}
		// #deviceId deviceState channel/deviceAP
		uint32_t deviceIndex = getDeviceIndexById(atol(tokens.at(0).c_str()));
		if (deviceIndex < 0) {
			cerr << "unknown device id " << endl;
			cerr << line << endl;
			return;
		}
		uint32_t state = atoi(tokens.at(1).c_str());
		if (state < 0 || state > 2) {
			cerr << "unknown state " << state << endl;
			cerr << line << endl;
			return;
		}
		switch (state) {
			case STATE_STARTAP:
			{
				uint32_t channelIndex = getChannelIndexById(atol(tokens.at(2).c_str()));
				if (channelIndex < 0) {
					cerr << "unknown channelIndex " << endl;
					cerr << line << endl;
					return;
				}
				START_AP[deviceIndex][channelIndex] = 1;
				START_Idle[deviceIndex] = 0;
				//cout << "START_AP[" << deviceIndex << "][" << channelIndex << "] = 1 " << endl;
			}
				break;
			case STATE_STARTCLIENT:
			{
				uint32_t deviceAPIndex = getDeviceIndexById(atol(tokens.at(2).c_str()));
				if (deviceAPIndex < 0) {
					cerr << "unknown device id " << endl;
					cerr << line << endl;
					return;
				}
				START_Client[deviceIndex][deviceAPIndex] = 1;
				START_Idle[deviceIndex] = 0;
				//cout << "START_Client[" << deviceIndex << "][" << deviceAPIndex << "] = 1 " << endl;
			}
				break;
			case STATE_IDLE:
				START_Idle[deviceIndex] = 1;
				//cout << "IDLE[" << deviceIndex << "]= 1 " << endl;
				break;
			default:
				cerr << "unknown state " << state <<  endl;
				cerr << line << endl;
				return;
				
		}
		
	}
	cout << endl;
	
	//integrity check
	for (uint32_t d = 0; d<deviceVector.size(); d++) {
		for (uint32_t d_ap = 0; d_ap<deviceVector.size(); d_ap++) {
			if (START_Client[d][d_ap] == 1) {
				bool foundAP = false;
				for (uint32_t c = 0; c<channelVector.size(); c++) {		
					if (START_AP[d_ap][c] == 1) {
						foundAP = true;
						break;
					}
				}
				if (!foundAP) {
					cerr << "device " << d << " is in client mode connected to AP " << d_ap << ", but not in AP mode " << endl;
					cerr << line << endl;
					return;	
				}
			}
		}
	}
}

void d2dOptimizer::printParameterSimple() {
	cout << endl << "Channels " <<  channelVector.size() << endl;
	for (uint32_t c=0; c<channelVector.size(); c++) {
		cout << "\t" << c << " ID " << channelVector.at(c)->id << endl;
	}
	
	cout << endl << "Devices " << deviceVector.size() << endl;
	for (uint32_t d=0; d<deviceVector.size(); d++) {
		cout << "\t" <<  d << " ID " << deviceVector.at(d)->id <<   endl;
	}
	
	cout << endl << "Content " <<  contentVector.size() << endl;
	for (uint32_t c=0; c<contentVector.size(); c++) {
		content *cont = contentVector.at(c);
		cout << "\t" << c << " ID " << cont->id << " Size " << cont->size <<  endl;
	}
}

void d2dOptimizer::printParameter() {
	
	cout << endl << "Channels " <<  channelVector.size() << endl;
	
	cout << endl << "Content " << contentVector.size() << endl;
	for (uint32_t n=0; n<contentVector.size(); n++) {
		cout << "\t" << "S[" << n << "] = " << S[n] << " ID " << contentVector[n]->id << endl;
	}
	
	cout << endl << "Devices " << deviceVector.size() << endl;
	for (uint32_t d=0; d<deviceVector.size(); d++) {
		cout << "Device index " << d << " ID " << deviceVector[d]->id << endl;
		for (uint32_t n=0; n<contentVector.size(); n++) {
			if (P[d][n] == 1) {
				cout << "\tP[" << d << "][" << n << "]" << endl;
			}
		}
		
		for (uint32_t n=0; n<contentVector.size(); n++) {
			if (I[d][n] == 1) {
				cout << "\tI[" << d << "][" << n << "]" << endl;
			}
		}
		
		for (uint32_t c=0; c<channelVector.size(); c++) {
			if (W[d][c] == 1) {				
				cout << "\tW[" << d << "][" << c << "]" << endl;
			}	
		}
		cout << endl;
	}
	
	cout << "Interference " << endl;
	for (uint32_t d_tx=0; d_tx<deviceVector.size(); d_tx++) {
		for (uint32_t c_tx=0; c_tx<channelVector.size(); c_tx++) {
			for (uint32_t d_rx=0; d_rx<deviceVector.size(); d_rx++) {
				for (uint32_t c_rx=0; c_rx<channelVector.size(); c_rx++) {
					if (X[d_tx][c_tx][d_rx][c_rx] == 1) {
						cout << "\t" << "X[" << d_tx << "][" << c_tx << "][" << d_rx << "][" << c_rx << "]" << endl;
					}
				}
			}	
		}
	}
	
	cout << endl << "Utility " << endl;
	for (uint32_t d_prv=0; d_prv<deviceVector.size(); d_prv++) {
		for (uint32_t d_cns=0; d_cns<deviceVector.size(); d_cns++) {
			for (uint32_t n=0; n<contentVector.size(); n++) {
				if (U[d_prv][d_cns][n] > 0) {
					cout << "\t" << "U[" << d_prv << "][" << d_cns << "][" << n << "] = " << U[d_prv][d_cns][n]  << endl;	
				}
			}
		}
	}
		
	cout << endl << "Link Speed " << endl;	
	for (uint32_t d_prv=0; d_prv<deviceVector.size(); d_prv++) {
		for (uint32_t d_cns=0; d_cns<deviceVector.size(); d_cns++) {
			for (uint32_t c=0; c<channelVector.size(); c++) {
				if (L[d_prv][d_cns][c] > 0) {
					cout << "\t" << "L[" << d_prv << "][" << d_cns << "][" << c << "] = " << L[d_prv][d_cns][c] << endl;
				}
			}
		}
	}
	
	cout << endl << "State " << endl;
	for (uint32_t d=0; d<deviceVector.size(); d++) {
		
		if (START_Idle[d] == 1) {
			cout << "\t" << "IDLE[" << d << "]= 1 " << endl;
		}
		for (uint32_t c=0; c<channelVector.size(); c++) {
			if (START_AP[d][c] == 1) {
				cout << "\t" << "START_AP[" << d << "][" <<  c << "] = 1 " << endl;
			}
		}
		for (uint32_t d_ap=0; d_ap<deviceVector.size(); d_ap++) {
			if (START_Client[d][d_ap] == 1) {
				cout << "\t" << "START_Client[" << d << "][" << d_ap << "] = 1 " << endl;
			}
		}
	}
}

int32_t d2dOptimizer::optimize() {
	int32_t status;		
	try {	
		// Create variables
		//y[d_prv][d_cns][n][c][t]
		y = new GRBVar****[deviceVector.size()];
		
		for (uint32_t d_prv = 0; d_prv<deviceVector.size(); d_prv++) {
			y[d_prv] = new GRBVar***[deviceVector.size()];
			
			for (uint32_t d_cns = 0; d_cns<deviceVector.size(); d_cns++) {	
				y[d_prv][d_cns] = new GRBVar**[contentVector.size()];
			
				for (uint32_t n = 0; n<contentVector.size(); n++) {	
					y[d_prv][d_cns][n] =  new GRBVar*[channelVector.size()];
				
					for (uint32_t c = 0; c<channelVector.size(); c++) {	
						y[d_prv][d_cns][n][c] =  new GRBVar[settings.numTimeSlots];
					
						for (int64_t t = 0; t<settings.numTimeSlots; t++) {	
							
							std::stringstream sstm;
							if (setVariableNames) {
								sstm << "y[d_prv-" << d_prv << "][d_cns-" << d_cns << "][n-" << n << "][c-" << c << "][t-" << t << "]";
							}
							y[d_prv][d_cns][n][c][t] =  model->addVar(0.0, 1.0, 0.0, GRB_BINARY, sstm.str());
						}
					}
				}
			}
		}
		
		//s[d][c][t]
		s = new GRBVar**[deviceVector.size()];
		for (uint32_t d = 0; d<deviceVector.size(); d++) {
			s[d] = new GRBVar*[channelVector.size()];

			for (uint32_t c = 0; c<channelVector.size(); c++) {	
				s[d][c] =  new GRBVar[settings.numTimeSlots];
				
				for (int64_t t = 0; t<settings.numTimeSlots; t++) {	
				
					std::stringstream sstm;
					if (setVariableNames) {
						sstm << "s[d-" << d << "][c-" << c << "][t-" << t << "]";
					}
					s[d][c][t] = model->addVar(0.0, 1.0, 0.0, GRB_BINARY, sstm.str());
				}
			}
		}
		
		//r[d][c][t]
		r = new GRBVar**[deviceVector.size()];
		for (uint32_t d = 0; d<deviceVector.size(); d++) {
			r[d] = new GRBVar*[channelVector.size()];

			for (uint32_t c = 0; c<channelVector.size(); c++) {	
				r[d][c] =  new GRBVar[settings.numTimeSlots];
				
				for (int64_t t = 0; t<settings.numTimeSlots; t++) {	
				
					std::stringstream sstm;
					if (setVariableNames) {
						sstm << "r[d-" << d << "][c-" << c << "][t-" << t << "]";
					}
					r[d][c][t] = model->addVar(0.0, 1.0, 0.0, GRB_BINARY, sstm.str());
				}
			}
		}
	
		//h[d_prv][d_cns][n][c][t]
		h = new GRBVar****[deviceVector.size()];
		for (uint32_t d_prv = 0; d_prv<deviceVector.size(); d_prv++) {
			h[d_prv] = new GRBVar***[deviceVector.size()];
			
			for (uint32_t d_cns = 0; d_cns<deviceVector.size(); d_cns++) {	
				h[d_prv][d_cns] = new GRBVar**[contentVector.size()];
				
				for (uint32_t n = 0; n<contentVector.size(); n++) {	
					h[d_prv][d_cns][n] = new GRBVar*[channelVector.size()];
					
					for (uint32_t c = 0; c<channelVector.size(); c++) {
						
						h[d_prv][d_cns][n][c] = new GRBVar[settings.numTimeSlots];
						for (int64_t t = 0; t<settings.numTimeSlots; t++) {	
						
							std::stringstream sstm;
							if (setVariableNames) {
								sstm << "h[d_prv-" << d_prv << "][d_cns-" << d_cns << "][n-" << n << "][c-" << c << "][t-" << t << "]";
							}
							h[d_prv][d_cns][n][c][t] = model->addVar(0.0, 1.0, 0.0, GRB_BINARY, sstm.str());
						}
					}
				}
			}
		}
		
		//p[d][n][t] 
		p = new GRBVar**[deviceVector.size()];
		
		for (uint32_t d = 0; d<deviceVector.size(); d++) {
			
			p[d] = new GRBVar*[contentVector.size()];
			for (uint32_t n = 0; n<contentVector.size(); n++) {	
			
				p[d][n] = new GRBVar[settings.numTimeSlots];
				for (int64_t t = 0; t<settings.numTimeSlots; t++) {	
				
					std::stringstream sstm;
					if (setVariableNames) {
						sstm << "p[d-" << d << "][n-" << n << "][t-" << t  << "]";
					}
					p[d][n][t] = model->addVar(0.0, 1.0, 0.0, GRB_BINARY, sstm.str());
				}
			}
		}
		
		// State Variables		
		S_Idle 		= new GRBVar*[deviceVector.size()]; 
		S_StartAP	= new GRBVar**[deviceVector.size()]; 
		S_SwitchAP	= new GRBVar**[deviceVector.size()]; 
		S_AP		= new GRBVar**[deviceVector.size()]; 
		
		S_StartClient	= new GRBVar**[deviceVector.size()]; 
		S_JoinAP		= new GRBVar**[deviceVector.size()]; 
		S_Client		= new GRBVar**[deviceVector.size()]; 
		
		
		for (uint32_t d = 0; d<deviceVector.size(); d++) {
			
			S_Idle[d] =  new GRBVar[settings.numTimeSlots]; 
			for (int64_t t = 0; t<settings.numTimeSlots; t++) {	
				std::stringstream sstm;
				if (setVariableNames) {
					sstm << "S_Idle[d-" << d << "][t-" << t  << "]";
				}
				S_Idle[d][t] = model->addVar(0.0, 1.0, 0.0, GRB_BINARY, sstm.str());
			}
			
			S_StartAP[d]	= new GRBVar*[channelVector.size()]; 
			S_SwitchAP[d]	= new GRBVar*[channelVector.size()]; 
			S_AP[d]			= new GRBVar*[channelVector.size()]; 
			for (uint32_t c = 0; c<channelVector.size(); c++) {	
				
				S_StartAP[d][c]		= new GRBVar[settings.numTimeSlots]; 
				S_SwitchAP[d][c]	= new GRBVar[settings.numTimeSlots]; 
				S_AP[d][c]			= new GRBVar[settings.numTimeSlots]; 
				
				for (int64_t t = 0; t<settings.numTimeSlots; t++) {	
					std::stringstream sstm;
					if (setVariableNames) {
						sstm << "S_StartAP[d-" << d << "][c-" << c <<"][t-" << t  << "]";
					}
					S_StartAP[d][c][t] 	= model->addVar(0.0, 1.0, 0.0, GRB_BINARY, sstm.str());
					
					if (setVariableNames) {					
						sstm.str("");
						sstm << "S_SwitchAP[d-" << d << "][c-" << c << "][t-" << t  << "]";
					}
					S_SwitchAP[d][c][t] = model->addVar(0.0, 1.0, 0.0, GRB_BINARY, sstm.str());
					
					if (setVariableNames) {
						sstm.str("");
						sstm << "S_AP[d-" << d << "][c-" << c << "][t-" << t  << "]";
					}
					S_AP[d][c][t] 		= model->addVar(0.0, 1.0, 0.0, GRB_BINARY, sstm.str());
				}
			}
			
			S_StartClient[d]	= new GRBVar*[deviceVector.size()]; 
			S_JoinAP[d]			= new GRBVar*[deviceVector.size()]; 
			S_Client[d]			= new GRBVar*[deviceVector.size()]; 
			for (uint32_t d_ap = 0; d_ap<deviceVector.size(); d_ap++) {
				
				S_StartClient[d][d_ap]	= new GRBVar[settings.numTimeSlots]; 
				S_JoinAP[d][d_ap]		= new GRBVar[settings.numTimeSlots]; 
				S_Client[d][d_ap]		= new GRBVar[settings.numTimeSlots];

				for (int64_t t = 0; t<settings.numTimeSlots; t++) {	
					std::stringstream sstm;
					if (setVariableNames) {
						sstm << "S_StartClient[d-" << d << "][d_ap-" << d_ap << "][t-" << t  << "]";
					}
					S_StartClient[d][d_ap][t]	= model->addVar(0.0, 1.0, 0.0, GRB_BINARY, sstm.str());
					
					if (setVariableNames) {
						sstm.str("");
						sstm << "S_JoinAP[d-" << d << "][d_ap-" << d_ap << "][t-" << t  << "]";
					}
					S_JoinAP[d][d_ap][t]		= model->addVar(0.0, 1.0, 0.0, GRB_BINARY, sstm.str());
					
					if (setVariableNames) {
						sstm.str("");
						sstm << "S_Client[d-" << d << "][d_ap-" << d_ap << "][t-" << t  << "]";
					}
					S_Client[d][d_ap][t]		= model->addVar(0.0, 1.0, 0.0, GRB_BINARY, sstm.str());
				}
			}			
		}
	
		model->update();
		
		cout << "Num Vars " <<  model->get(GRB_IntAttr_NumVars) << endl;
	
		GRBLinExpr objExpr = 0;
		for (uint32_t d_prv = 0; d_prv<deviceVector.size(); d_prv++) {
			for (uint32_t d_cns = 0; d_cns<deviceVector.size(); d_cns++) {	
				for (uint32_t n = 0; n<contentVector.size(); n++) {	
					for (uint32_t c = 0; c<channelVector.size(); c++) {	
						for (int64_t t = 0; t<settings.numTimeSlots; t++) {	
							
							objExpr += y[d_prv][d_cns][n][c][t] * U[d_prv][d_cns][n];
						}
					}
				}
			}
		}
		
		model->setObjective(objExpr, GRB_MAXIMIZE );
		model->update();
		
		
		cout << "Adding constraints...." << endl;
		
		//Content availability, if a content is available at a device in the beginning of a super slot
		// it should be available the whole duration of the super slot
		//C1
		if (debugConstraints)
			cout << "p-start constraint" << endl;
		for (uint32_t d = 0; d<deviceVector.size(); d++) {
			for (uint32_t n = 0; n<contentVector.size(); n++) {	
				std::stringstream sstm;
				
				if (P[d][n] == 1) {
					
					for (int64_t t = 0; t<settings.numTimeSlots; t++) {	
						//p_start1 C1 
						if (setConstraintNames) {
							sstm << "p_start1_d-" << d << "_n-" << n << "_t-"<< t << endl;
						}
						model->addConstr(p[d][n][t] == 1, sstm.str());
					}
				} else {
					if (setConstraintNames) {
						sstm << "p_start0_d-" << d << "_n-" << n << endl;
					}
					//p_start0 C2 
					model->addConstr(p[d][n][0] == 0, sstm.str());
				}
			}
		}
		
		if (debugConstraints)
			cout << "p-next constraint" << endl;
		for (uint32_t d = 0; d<deviceVector.size(); d++) {
			for (uint32_t n = 0; n<contentVector.size(); n++) {	
				for (int64_t t = 1; t<settings.numTimeSlots; t++) {	
					
					GRBLinExpr conExpr = 0;	
					for (uint32_t d_prv = 0; d_prv<deviceVector.size(); d_prv++) {	
						for (uint32_t c = 0; c<channelVector.size(); c++) {
							
							if (L[d_prv][d][c] > 0) {
								int64_t t_duration = ceil( (double)S[n] / (double)L[d_prv][d][c]);
								
								int64_t t_start = t - t_duration;
								
								if (t_start >= 0) {
									conExpr += y[d_prv][d][n][c][t_start];
									if (debugConstraints)
										cout << "y[d_prv-" << d_prv << "][d-" << d << "][n-" << n <<  "][c-" << c << "][t_start-" << t_start << "]" << endl;
								}
							}
						}
					}
					//p_next C3
					model->addConstr(p[d][n][t] <= p[d][n][t-1] + conExpr, "p_next");				
				}
			}
		}
		
		if (debugConstraints)
			cout << "c4 - c11 constraint" << endl;		
		for (uint32_t d_prv = 0; d_prv<deviceVector.size(); d_prv++) {
			for (uint32_t d_cns = 0; d_cns<deviceVector.size(); d_cns++) {	
				for (uint32_t n = 0; n<contentVector.size(); n++) {	
					for (uint32_t c = 0; c<channelVector.size(); c++) {	
						for (int64_t t = 0; t<settings.numTimeSlots; t++) {	
							
							if (debugConstraints)
								cout << "d_prv " << d_prv << " d_cns " << d_cns << " n " << n << " c " << c << " t " << t << endl;
							
							std::stringstream sstm;
							//
							//	Content provider must have the content C4
							//
							if (setConstraintNames) {
								sstm << "ProvideContent[d_p-" << d_prv << "][d_c-" << d_cns  << "][n-" << n << "][c-" << c << "][t-" << t << "]";
							}
							model->addConstr(y[d_prv][d_cns][n][c][t] <= p[d_prv][n][t] , "ProvideContent");
							
							//
							//	Content cosnumer must be interested C5
							//
							if (setConstraintNames) {
								sstm.str("");
								sstm << "InterestContent[d_p-" << d_prv << "][d_c-" << d_cns  << "][n-" << n << "][c-" << c << "][t-" << t << "]";
							}
							model->addConstr(y[d_prv][d_cns][n][c][t] <= I[d_cns][n], "InterestContent");
							
							//
							//	Content provider and consumer must be able to use the channel C6
							//
							if (setConstraintNames) {
								sstm.str("");
								sstm << "ChannelSupport[d_p-" << d_prv << "][d_c-" << d_cns  << "][n-" << n << "][c-" << c << "][t-" << t << "]";	
							}
							model->addConstr(2*y[d_prv][d_cns][n][c][t] <= W[d_cns][c] +  W[d_prv][c] , "ChannelSupport");
							
							//
							// Connectivity requirement C7
							///
							
							if (setConstraintNames) {
								sstm.str("");
								sstm << "Connectivity[d_p-" << d_prv << "][d_c-" << d_cns  << "][n-" << n << "][c-" << c << "][t-" << t << "]";
							}
							model->addConstr( (1 - y[d_prv][d_cns][n][c][t]) + ceil(L[d_prv][d_cns][c]) >= 1, sstm.str());
															
							//helper variables
							//set for all time slots in which content exchange occur
							if (L[d_prv][d_cns][c] > 0) {
								int64_t t_max_new = t + ceil( (double)S[n] / (double)L[d_prv][d_cns][c]);
								
								//only schedule if t_max_new is not larger than superslot length
								if (t_max_new <= settings.numTimeSlots) {
								
									for (int64_t t_new = t; t_new < t_max_new; t_new++) {	
										// C9
										//helper variable sending mode
										if (setConstraintNames) {
											sstm.str("");
											sstm << "s_helper[d_p-" << d_prv << "][d_c-" << d_cns  << "][n-" << n << "][c-" << c << "][t-" << t << "]";
											//sstm << "_t-n-" << t_new << "_S-n-" << S[n] << "_L-" << L[d_prv][d_cns][c];
										}
										model->addConstr( s[d_prv][c][t_new] >=  y[d_prv][d_cns][n][c][t], sstm.str());
										
										//C 10
										//helper variable receiving mode
										if (setConstraintNames) {
											sstm.str("");
											sstm << "r_helper[d_p-" << d_prv << "][d_c-" << d_cns  << "][n-" << n << "][c-" << c << "][t-" << t << "]";
										}
										model->addConstr( r[d_cns][c][t_new] >=  y[d_prv][d_cns][n][c][t], sstm.str());
										
										// C11
										//helper variable exchange mode
										if (setConstraintNames) {
											sstm.str("");
											sstm << "h_helper[d_p-" << d_prv << "][d_c-" << d_cns  << "][n-" << n << "][c-" << c << "][t-" << t << "]";
										}
										model->addConstr( h[d_prv][d_cns][n][c][t_new] >=  y[d_prv][d_cns][n][c][t], sstm.str());
											
									}									
	
								} 						
								//
								// Content exchange must fit into superslot C9
								//

								// A content exchange must fit into super slot. An exchange cannot be scheduled at the end of a super slot, if the transfer cannot be finished within the super slot.
								// C8
								if (setConstraintNames) {
									sstm.str("");
									sstm << "ExchangeMustFit[d_p-" << d_prv << "][d_c-" << d_cns  << "][n-" << n << "][c-" << c << "][t-" << t << "]";
								}
								model->addConstr(y[d_prv][d_cns][n][c][t] * ( (((double) t) / ((double) settings.numTimeSlots)) * settings.tau + ((double)S[n]) / ((double)L[d_prv][d_cns][c])) <= settings.tau , sstm.str());				
							}	
						}
					}
				}
			}
		}
		
		
		// Half-duplex constraint
		// C12
		if (debugConstraints)
			cout << "Half-duplex constraint" << endl;
		for (uint32_t d = 0; d<deviceVector.size(); d++) {	
			for (int64_t t = 0; t<settings.numTimeSlots; t++) {
				GRBLinExpr conExpr = 0;	
				for (uint32_t c = 0; c<channelVector.size(); c++) {
						conExpr += s[d][c][t] + r[d][c][t];
				}
				model->addConstr(conExpr <= 1 ,"half-duplex");
			}
		}
		
		//each consumer should only receive the content once
		// C13
		if (debugConstraints)
			cout << "Consumer content only once constraint" << endl;
		for (uint32_t d_cns = 0; d_cns<deviceVector.size(); d_cns++) {	
			for (uint32_t n = 0; n<contentVector.size(); n++) {	
				
				GRBLinExpr conExpr = 0;	
				for (uint32_t d_prv = 0; d_prv<deviceVector.size(); d_prv++) {
					for (uint32_t c = 0; c<channelVector.size(); c++) {
						for (int64_t t = 0; t<settings.numTimeSlots; t++) {	
							conExpr += y[d_prv][d_cns][n][c][t];
						}
					}
				}
				model->addConstr(P[d_cns][n] + conExpr <= 1 ,"CnsContentOnce");
			}
		}

		//a provider can only  serve one consumer in each time slot and channel
		// C14
		if (debugConstraints)
			cout << "Provider only one consumer constraint" << endl;
		for (uint32_t d_prv = 0; d_prv<deviceVector.size(); d_prv++) {
			for (int64_t t = 0; t<settings.numTimeSlots; t++) {	
				GRBLinExpr conExpr = 0;	
				for (uint32_t d_cns = 0; d_cns<deviceVector.size(); d_cns++) {
					for (uint32_t n = 0; n<contentVector.size(); n++) {	
						for (uint32_t c = 0; c<channelVector.size(); c++) {
							conExpr += h[d_prv][d_cns][n][c][t];
						}
					}
				}
				model->addConstr(conExpr <= 1, "PrvOnlyOneConsumer");
			}
		}
		
		//a consumer can only be served by one provider in each time slot and channel
		// C15
		if (debugConstraints)
			cout << "Consumer only one provider constraint" << endl;
		for (uint32_t d_cns = 0; d_cns<deviceVector.size(); d_cns++) {
			for (int64_t t = 0; t<settings.numTimeSlots; t++) {	
				GRBLinExpr conExpr = 0;	
				for (uint32_t d_prv = 0; d_prv<deviceVector.size(); d_prv++) {
					for (uint32_t n = 0; n<contentVector.size(); n++) {	
						for (uint32_t c = 0; c<channelVector.size(); c++) {
							conExpr += h[d_prv][d_cns][n][c][t];
						}
					}
				}
				model->addConstr(conExpr <= 1, "CnsServedByOnePrv");
			}
		}
		
		//interference avoidance constraint
		// C16
		if (debugConstraints)
			cout << "interference constraint" << endl;
		for (uint32_t d_tx = 0; d_tx<deviceVector.size(); d_tx++) {
			for (uint32_t c_tx = 0; c_tx < channelVector.size(); c_tx++) {	
				for (uint32_t d_rx = 0; d_rx<deviceVector.size(); d_rx++) {	
					if (d_tx == d_rx) {
						continue;
					}
					for (uint32_t c_rx = 0; c_rx < channelVector.size(); c_rx++) {	
						for (int64_t t = 0; t<settings.numTimeSlots; t++) {	
							GRBLinExpr sumHExpr = 0;	
							for (uint32_t n = 0; n<contentVector.size(); n++) {
										
								//exchange channel should be the transmitting channel
								sumHExpr +=  h[d_tx][d_rx][n][c_tx][t];
							}
							model->addConstr((1 -  s[d_tx][c_tx][t]) + (1 - X[d_tx][c_tx][d_rx][c_rx]) + (1 - r[d_rx][c_rx][t]) + sumHExpr >= 1, "Interference");
						}
					}
				}
			}
		}
		
		// state constraints
		
		//each device in one state max..
		// C 17
		if (debugConstraints)
			cout << "one state max constraint" << endl;
		for (uint32_t d = 0; d<deviceVector.size(); d++) {
			for (int64_t t = 0; t<settings.numTimeSlots; t++) {	
				
				GRBLinExpr sumAPStates = 0;	 
				for (uint32_t c = 0; c < channelVector.size(); c++) {
					sumAPStates += S_StartAP[d][c][t] +   S_SwitchAP[d][c][t] + S_AP[d][c][t];	
					//sumAPStates +=   S_SwitchAP[d][c][t] + S_AP[d][c][t];	
				}
				
				GRBLinExpr sumClientStates = 0;	 
				for (uint32_t d_ap = 0; d_ap<deviceVector.size(); d_ap++) {
					sumClientStates += S_StartClient[d][d_ap][t] +  S_JoinAP[d][d_ap][t] + S_Client[d][d_ap][t];	
					//sumClientStates +=   S_JoinAP[d][d_ap][t] + S_Client[d][d_ap][t];
				}
				
				model->addConstr(S_Idle[d][t] + sumAPStates + sumClientStates == 1);
			}
		}
		
		// allowed state changes to S_Idle
		if (debugConstraints)
			cout << "state transition to S_Idle constraint" << endl;
		for (uint32_t d = 0; d<deviceVector.size(); d++) {
			
			//C18
			model->addConstr(S_Idle[d][0]<= START_Idle[d]);
			
			for (int64_t t = 1; t<settings.numTimeSlots; t++) {	
				//C19
				model->addConstr(S_Idle[d][t] <= S_Idle[d][t-1]);
			}	
		}
		
		// allowed state changes to S_StartAP
		if (debugConstraints)
			cout << "state transition to S_StartAP constraint" << endl;
		for (uint32_t d = 0; d<deviceVector.size(); d++) {
			for (uint32_t c = 0; c < channelVector.size(); c++) {
				
				//Constraint transitions to state S_StartAP for t = 0
				GRBLinExpr sumAPStates = 0;	 
				for (uint32_t c_ap = 0; c_ap < channelVector.size(); c_ap++) {
					sumAPStates +=  START_AP[d][c_ap];	
				}
				GRBLinExpr sumClientStates = 0;	 
				for (uint32_t d_ap = 0; d_ap<deviceVector.size(); d_ap++) {
					sumClientStates += START_Client[d][d_ap];	
				}
				// C20
				model->addConstr(S_StartAP[d][c][0] <=  START_Idle[d] + sumAPStates + sumClientStates);
							
				for (int64_t t = 1; t<settings.numTimeSlots - settings.kappa_startAP - 1; t++) {	
				
					GRBLinExpr sumAPStates = 0;	 
					for (uint32_t c_ap = 0; c_ap < channelVector.size(); c_ap++) {
						sumAPStates +=  S_AP[d][c_ap][t-1];	
					}
					GRBLinExpr sumClientStates = 0;	 
					for (uint32_t d_ap = 0; d_ap<deviceVector.size(); d_ap++) {
						sumClientStates += S_Client[d][d_ap][t-1];	
					}
					//C21	
					model->addConstr(S_StartAP[d][c][t] <=  S_Idle[d][t-1] + sumAPStates + sumClientStates);
				}
				
				//S_StartAP not allowed if start AP cannot be finished in the super slot
				// C22
				for (int64_t t = settings.numTimeSlots - settings.kappa_startAP - 1; t<settings.numTimeSlots; t++) {	
					if (t >= 0) {
						model->addConstr(S_StartAP[d][c][t] == 0);
					}
				}			
			}
		}

		
		//Constraint transitions to state S_SwitchAP
		if (debugConstraints)
			cout << "state transition to S_SwitchAP constraint" << endl;
		for (uint32_t d = 0; d<deviceVector.size(); d++) {		
			for (uint32_t c = 0; c < channelVector.size(); c++) {
				//C23
				model->addConstr(S_SwitchAP[d][c][0] <=  S_StartAP[d][c][0]);
			}
			
			for (uint32_t c = 0; c < channelVector.size(); c++) {
				
				for (int64_t t = 0; t<settings.numTimeSlots - settings.kappa_startAP - 1; t++) {	
					
					GRBLinExpr sumSwitchStates = 0;	 
					for (int64_t t_sw = t; t_sw <= t + settings.kappa_startAP; t_sw++) {
						sumSwitchStates += S_SwitchAP[d][c][t_sw];	
					}
					//C24
					model->addConstr(settings.kappa_startAP * S_StartAP[d][c][t] <=  sumSwitchStates);
				}
				
				for (int64_t t = 1; t<settings.numTimeSlots; t++) {	
					//C25
					model->addConstr(S_SwitchAP[d][c][t] <= S_SwitchAP[d][c][t-1] + S_StartAP[d][c][t-1]);
				}
			}	
		}
		
		//Constraint transitions to state S_AP
		if (debugConstraints)
			cout << "state transition to S_AP constraint" << endl;
		for (uint32_t d = 0; d<deviceVector.size(); d++) {		
			for (uint32_t c = 0; c < channelVector.size(); c++) {
				//Constraint transitions to state S_AP t = 0
				//C26
				model->addConstr(S_AP[d][c][0] <=  START_AP[d][c]);
				
				for (int64_t t = 1; t<settings.numTimeSlots; t++) {	
					//C27
					model->addConstr(S_AP[d][c][t] <=  S_AP[d][c][t-1] + S_SwitchAP[d][c][t-1]);
				}
			}
		}

		//Constraint transitions to state S_StartClient
		if (debugConstraints)
			cout << "state transition to S_StartClient constraint" << endl;
		for (uint32_t d = 0; d<deviceVector.size(); d++) {	
			for (uint32_t d_ap = 0; d_ap<deviceVector.size(); d_ap++) {
				
				//Constraint transitions to state S_Client for t = 0
				GRBLinExpr sumAPStates = 0;	 
				for (uint32_t c = 0; c < channelVector.size(); c++) {
					sumAPStates +=  START_AP[d][c];	
				}
				GRBLinExpr sumClientStates = 0;	 
				for (uint32_t d_prev_ap = 0; d_prev_ap < deviceVector.size(); d_prev_ap++) {
					sumClientStates += START_Client[d][d_prev_ap];	
				}
				//C28
				model->addConstr(S_StartClient[d][d_ap][0] <=  START_Idle[d] + sumAPStates + sumClientStates);
				
				for (int64_t t = 1; t<settings.numTimeSlots - settings.kappa_startClient - 1; t++) {	
				
					GRBLinExpr sumAPStates = 0;	 
					for (uint32_t c = 0; c < channelVector.size(); c++) {
						sumAPStates +=  S_AP[d][c][t-1];	
					}
					GRBLinExpr sumClientStates = 0;	 
					for (uint32_t d_prev_ap = 0; d_prev_ap < deviceVector.size(); d_prev_ap++) {
						sumClientStates += S_Client[d][d_prev_ap][t-1];	
					}
					//C29
					model->addConstr(S_StartClient[d][d_ap][t] <=  S_Idle[d][t-1] + sumAPStates + sumClientStates);
				}
				
				//S_StartClient not allowed if not enough time slots to complete join process
				for (int64_t t = settings.numTimeSlots - settings.kappa_startClient - 1; t<settings.numTimeSlots; t++) {
					//C30
					if (t >= 0) {
						model->addConstr(S_StartClient[d][d_ap][t] == 0);
					}
				}
			}
		}
		
		//Constraint transitions to state S_JoinAP
		if (debugConstraints)
			cout << "state transition to S_JoinAP constraint" << endl;
		for (uint32_t d = 0; d<deviceVector.size(); d++) {		
			for (uint32_t d_ap = 0; d_ap<deviceVector.size(); d_ap++) {
				
				//C31
				model->addConstr(S_JoinAP[d][d_ap][0] <=  S_StartClient[d][d_ap][0]);
				
				for (int64_t t = 0; t<settings.numTimeSlots - settings.kappa_startClient - 1; t++) {	
					
					GRBLinExpr sumSwitchStates = 0;	 
					for (int64_t t_sw = t; t_sw <= t + settings.kappa_startClient; t_sw++) {
						sumSwitchStates += S_JoinAP[d][d_ap][t_sw];	
					}
					//C32
					model->addConstr(settings.kappa_startClient * S_StartClient[d][d_ap][t] <=  sumSwitchStates);
				}
				
				for (int64_t t = 1; t<settings.numTimeSlots; t++) {	
					//C33
					model->addConstr(S_JoinAP[d][d_ap][t] <= S_JoinAP[d][d_ap][t-1] + S_StartClient[d][d_ap][t-1]);
				}				
			}
		}
		
		//Constraint transitions to state S_Client
		if (debugConstraints)
			cout << "state transition to S_Client constraint" << endl;
		for (uint32_t d = 0; d<deviceVector.size(); d++) {		
			for (uint32_t d_ap = 0; d_ap<deviceVector.size(); d_ap++) {
				//Constraint transitions to state S_Client t = 0
				//C34
				model->addConstr(S_Client[d][d_ap][0] <=  START_Client[d][d_ap]);
				
				for (int64_t t = 1; t<settings.numTimeSlots; t++) {	
					//C35
					model->addConstr(S_Client[d][d_ap][t] <=  S_Client[d][d_ap][t-1] + S_JoinAP[d][d_ap][t-1]);
				}	
			}
		}
		
		//only one ap per channel
		if (debugConstraints)
			cout << "APperChan constraint" << endl;
		for (uint32_t c = 0; c<channelVector.size(); c++) {	
		
			for (int64_t t = 0; t<settings.numTimeSlots; t++) {	
				
				GRBLinExpr linExpr = 0;	 
				for (uint32_t d = 0; d<deviceVector.size(); d++) {	
					linExpr += S_AP[d][c][t];
				}
				//C36
				model->addConstr(linExpr <= 1, "APperChan");
			}	
		}
		
		//Clients can only connect to APs
		if (debugConstraints)
			cout << "ClientsConnectAP constraint" << endl;
		for (uint32_t d = 0; d<deviceVector.size(); d++) {	
			for (uint32_t d_ap = 0; d_ap<deviceVector.size(); d_ap++) {
				for (int64_t t = 0; t<settings.numTimeSlots; t++) {	
				
					GRBLinExpr linExpr = 0;	 
					for (uint32_t c = 0; c<channelVector.size(); c++) {	
						linExpr += S_AP[d_ap][c][t];
					}
					//C36
					model->addConstr(S_StartClient[d][d_ap][t] <= linExpr, "ClientsConnectAP");
					//C37
					model->addConstr(S_JoinAP[d][d_ap][t] <= linExpr, "ClientsConnectAP");
					//C38
					model->addConstr(S_Client[d][d_ap][t] <= linExpr, "ClientsConnectAP");
				}
			}
		}
		
		//a client cant connect to itself
		if (debugConstraints)
			cout << "NoSelfService constraint" << endl;
		for (uint32_t d = 0; d<deviceVector.size(); d++) {	
			for (int64_t t = 0; t<settings.numTimeSlots; t++) {	
				//C39
				model->addConstr(S_StartClient[d][d][t] == 0, "NoSelfService");
				//C40
				model->addConstr(S_JoinAP[d][d][t] == 0, "NoSelfService");
				//C41
				model->addConstr(S_Client[d][d][t] == 0, "NoSelfService");	
			}
		}
		
		
		// if content exchange, one device must be AP and the other client of this AP
		if (debugConstraints)
			cout << "one device AP, one client, constraint" << endl;
		for (uint32_t d_prv = 0; d_prv<deviceVector.size(); d_prv++) {	
			for (uint32_t d_cns = 0; d_cns<deviceVector.size(); d_cns++) {	
				for (uint32_t n = 0; n<contentVector.size(); n++) {	
					for (int64_t t = 0; t<settings.numTimeSlots; t++) {	
						for (uint32_t c = 0; c<channelVector.size(); c++) {
									
							//C42
							model->addConstr(2*h[d_prv][d_cns][n][c][t] <= S_AP[d_prv][c][t] + S_AP[d_cns][c][t] + S_Client[d_cns][d_prv][t] + S_Client[d_prv][d_cns][t]);
						}
					}
				}
			}
		}
		
		model->update();
		//model->write("debug.lp");
			
		cout << "optimize..." << endl;
		model->optimize();
	
		status = model->get(GRB_IntAttr_Status);
		switch(status) {
			case GRB_OPTIMAL: 
				if (debugResults) {
					printResults();
				}
				break;
			case GRB_INFEASIBLE:
				{
					cout << "The model is infeasible; computing IIS" << endl;
					deque<string> removed;

					// Loop until we reduce to a model that can be solved
					model->computeIIS();
					cout << "\nThe following constraint cannot be satisfied:" << endl;
					GRBConstr* c = NULL;		 
					c = model->getConstrs();
					for (int64_t i = 0; i < model->get(GRB_IntAttr_NumConstrs); ++i) {
						if (c[i].get(GRB_IntAttr_IISConstr) == 1.0) {
							cout << c[i].get(GRB_StringAttr_ConstrName) << endl;
							
							removed.push_back(c[i].get(GRB_StringAttr_ConstrName));
							model->remove(c[i]);
							break;
						}
					}
				}
				break;
			default:
				cout << "Unknown gurobi state " << status << endl;	
		}
					
	} catch(GRBException e) {
		cout << "ERROR code = " << e.getErrorCode() << endl;
		cout << e.getMessage() << endl;
		return e.getErrorCode();
	} catch (exception& e)	{
		cout << e.what() << endl;
		return -1;
	} catch(...) {
		cout << "Exception during optimization" << endl;
		return -1;
	}
	return status;
}

void d2dOptimizer::printResults() {
	cout << "settings.numTimeSlots " << settings.numTimeSlots << endl;
	cout << "tau " << settings.tau << " seconds " << endl;
	cout << "settings.kappa_startAP " << settings.kappa_startAP << endl;
	cout << "settings.kappa_startClient " << settings.kappa_startClient << endl;
	
	cout << endl;

	cout << "STATES" << endl;
	cout << "------" << endl;
	for (uint32_t d = 0; d<deviceVector.size(); d++) {
		cout << "Device " << d << endl;
		
		for (int64_t t = 0; t<settings.numTimeSlots; t++) {	
			if (S_Idle[d][t].get(GRB_DoubleAttr_X) > 0.5) { 
				cout << "IDLE " << d << " time " <<  t << endl;					
			}				
			for (uint32_t c = 0; c<channelVector.size(); c++) {
				if (S_AP[d][c][t].get(GRB_DoubleAttr_X) > 0.5) { 
					cout << "AP " << d << " chan " << c << " time " << t << endl;
				}
				if (S_StartAP[d][c][t].get(GRB_DoubleAttr_X) > 0.5) { 
					cout << "StartAP " << d << " chan " << c << " time " << t << endl;
				}
				if (S_SwitchAP[d][c][t].get(GRB_DoubleAttr_X) > 0.5) { 
					cout << "SwitchAP " << d << " chan " << c << " time " << t << endl;
				}
			}
			for (uint32_t d_ap = 0; d_ap<deviceVector.size(); d_ap++) {
				if (S_Client[d][d_ap][t].get(GRB_DoubleAttr_X) > 0.5) { 
					cout << "Client " << d << " AP " << d_ap << " time " << t << endl;
				}	
				if (S_StartClient[d][d_ap][t].get(GRB_DoubleAttr_X) > 0.5) { 
					cout << "StartClient " << d << " AP " << d_ap << " time " << t << endl;
				}	
				if (S_JoinAP[d][d_ap][t].get(GRB_DoubleAttr_X) > 0.5) { 
					cout << "JoinAP " << d << " AP " << d_ap << " time " << t << endl;
				}	
			}
		}	
		cout << endl;
	}
	cout << endl;
	
	cout << "y var - content exchanges" << endl;
	cout << "------" << endl;
	for (uint32_t d_prv = 0; d_prv<deviceVector.size(); d_prv++) {
		for (uint32_t d_cns = 0; d_cns<deviceVector.size(); d_cns++) {	
			for (uint32_t n = 0; n<contentVector.size(); n++) {	
				for (uint32_t c = 0; c<channelVector.size(); c++) {	
					for (int64_t t = 0; t<settings.numTimeSlots; t++) {	
						if (y[d_prv][d_cns][n][c][t].get(GRB_DoubleAttr_X) > 0.5) { 
								cout << "PRV " << d_prv << " -> CNS " << d_cns;
								cout << " CNT " << n << " (size " << S[n] << ") CH " << c << "(speed " << L[d_prv][d_cns][c] << ") TIME " << t << " DUR " << ceil((double)S[n] / (double) L[d_prv][d_cns][c]) << endl;					
						}
					}
				}
			}
		}
	}
	cout << endl;
		
	cout << "h var - allocated slots for exchanges" << endl;
	cout << "------" << endl;
	for (uint32_t d_prv = 0; d_prv<deviceVector.size(); d_prv++) {
		for (uint32_t d_cns = 0; d_cns<deviceVector.size(); d_cns++) {	
			for (uint32_t n = 0; n<contentVector.size(); n++) {	
				for (uint32_t c = 0; c<channelVector.size(); c++) {	
					for (int64_t t = 0; t<settings.numTimeSlots; t++) {	
						if ( h[d_prv][d_cns][n][c][t].get(GRB_DoubleAttr_X) > 0.5) {	
							cout << "PRV " << d_prv << " -> CNS " << d_cns << " CNT " << n << " Chan " << c << " Time " << t << endl;
						
						}
					}
				}
			}
		}
	}
	cout << endl;	
		
	cout << "s&r var - rx and tx modes" << endl;
	cout << "------" << endl;
	for (uint32_t d = 0; d<deviceVector.size(); d++) {
		for (uint32_t c = 0; c<channelVector.size(); c++) {	
			for (int64_t t = 0; t<settings.numTimeSlots; t++) {	
				if (s[d][c][t].get(GRB_DoubleAttr_X) > 0.5) {	
					cout << "TX DEV " << d << " CH " << c << " Time " << t << endl;
				}	
				if (r[d][c][t].get(GRB_DoubleAttr_X) > 0.5) {		
					cout << "RX DEV " << d << " CH " << c << " Time " << t << endl;
				}
			}
		}
		cout << endl;
	}
	cout << endl;
	
	cout << "p var - content availability" << endl;
	cout << "------" << endl;
	for (uint32_t d = 0; d<deviceVector.size(); d++) {
		cout << "Dev " << d << endl;
		for (uint32_t n = 0; n<contentVector.size(); n++) {	
			cout << "\t CNT " << n << endl;				
			for (int64_t t = 0; t<settings.numTimeSlots; t++) {	
				if (p[d][n][t].get(GRB_DoubleAttr_X) > 0.5) {
					cout << "\t\tTime " << t << endl;
				}
			}
		}	
		cout << endl;
	}
}