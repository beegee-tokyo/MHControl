#include "mhc.h"

int main(int argc, char** argv)
{
	if (argc == 1) {
		printf("Sleeping for 15 seconds\n");
		// Wait for 15 seconds to make sure the lan/wifi connections are up
		sleep(15);
	} else {
		printf("Start without 15 second sleep\n");
	}
	// Save start date (needed for organisation into folders by day)
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);
	startYear = tm.tm_year + 1900;
	startMonth = tm.tm_mon + 1;
	startDay = tm.tm_mday;

	// Initialize application
	initApp(true);

//######################################################################
// Endless loop starts here. Main process goes into sleep!
// Events are triggered by signals and timer
// Events are handled by signal_handler
//######################################################################
	while (1) {
		// pause();
		sleep(60);
	}
}

void initApp(bool isNewStart) {
	FILE *fPtr;
	// Disable debugging until enabled through TCP command
	genDebugOn =
	camDebugOn =
	dht11DebugOn =
	mqttDebugOn =
	udpDebugOn = false;
	
	// Pointer to log file
	fPtr = fopen("/home/pi/mhcv.txt", "a"); // /home/pi on Raspian

	// Try to read existing configuration
	FILE *confFilePtr = fopen("/home/pi/config.txt", "r");
	if (confFilePtr != NULL) {
		// Get size of config file
		fseek(confFilePtr, 0, SEEK_END); // seek to end of file
		int configSize = ftell(confFilePtr); // get current file pointer
		fseek(confFilePtr, 0, SEEK_SET); // seek back to beginning of file
		
		char *configTxt = malloc(sizeof(char) * (configSize + 1));
		
        size_t newLen = fread(configTxt, sizeof(char), configSize, confFilePtr);
        if ( ferror( confFilePtr ) != 0 ) {
			dbgLen = sprintf(debugMsg,"Error reading config file\n");
			fwrite(debugMsg,dbgLen,1,fPtr);
		} else {
            configTxt[newLen++] = '\0'; /* Just to be safe. */
        }
		fclose(confFilePtr);

		if ((configJSON = json_parse_string(configTxt)) == NULL) {
			dbgLen = sprintf(debugMsg,"No valid configuration JSON found\n");
			fwrite(debugMsg,dbgLen,1,fPtr);
			configJSON = json_value_init_object();
		} else {
			// String for debug output
			char *serialized_string = json_serialize_to_string_pretty(configJSON);
			if (serialized_string != NULL) {
				dbgLen = sprintf(debugMsg,"Found config data: %s\n", serialized_string);
				fwrite(debugMsg,dbgLen,1,fPtr);
				json_free_serialized_string(serialized_string);
			}
			
			// Root of JSON data
			JSON_Object *config_object;
			config_object = json_value_get_object(configJSON);
			// Look for IP for solar panel
			if (json_object_has_value(config_object,"spm")) {
				strcpy(ipSPM,json_object_get_string(config_object,"spm"));
			}
			// Look for IP for front security
			if (json_object_has_value(config_object,"sf1")) {
				strcpy(ipSecFront,json_object_get_string(config_object,"sf1"));
			}
			// Look for IP for aircon 1
			if (json_object_has_value(config_object,"ac1")) {
				strcpy(ipAC1,json_object_get_string(config_object,"ac1"));
			}
			// Look for IP for aircon 2
			if (json_object_has_value(config_object,"ac2")) {
				strcpy(ipAC2,json_object_get_string(config_object,"ac2"));
			}
			// Look for IP for rear security
			if (json_object_has_value(config_object,"sb1")) {
				strcpy(ipSecBack,json_object_get_string(config_object,"sb1"));
			}
			// Look for IP for camera 1
			if (json_object_has_value(config_object,"cm1")) {
				strcpy(ipCam1,json_object_get_string(config_object,"cm1"));
			}
			// Look for IP for camera 2
			if (json_object_has_value(config_object,"cm2")) {
				strcpy(ipCam2,json_object_get_string(config_object,"cm2"));
			}
			// Look for IP for bedroom lights
			if (json_object_has_value(config_object,"lb1")) {
				strcpy(ipLB1,json_object_get_string(config_object,"lb1"));
			}
			// Look for IP for backyard light
			if (json_object_has_value(config_object,"ly1")) {
				strcpy(ipLY1,json_object_get_string(config_object,"ly1"));
			}
			// Look for IP for solar panel
			if (json_object_has_value(config_object,"spm")) {
				strcpy(ipSPM,json_object_get_string(config_object,"spm"));
			}
			// Look for IP for solar panel
			if (json_object_has_value(config_object,"db1")) {
				strcpy(ipDB1,json_object_get_string(config_object,"db1"));
			}
		}
		
		free(configTxt);
	} else {
		dbgLen = sprintf(debugMsg,"No configuration file found\n");
		fwrite(debugMsg,dbgLen,1,fPtr);
		configJSON = json_value_init_object();
	}

	// Set Broadcast IP address
	strcpy(ipUDPBroadcast,"192.168.0.255");
	// Set IP address debugging tablet
	strcpy(ipDBG,"192.168.0.10");
	
	if (!isNewStart) { // Restart of application
		printf("initApp - !isNewStart\n");
		// Write Log data
		getSysTime(1);
		// Pointer to log file
		fPtr = fopen("/home/pi/mhcv.txt", "a"); // /home/pi on Raspian
		fwrite(sysTime,20,1,fPtr);
		dbgLen = sprintf(debugMsg,"initApp - !isNewStart\n");
		fwrite(debugMsg,dbgLen,1,fPtr);
		fclose(fPtr);
		// Stop the TCP server thread
#ifdef DO_DEBUG
		// Write Log data
		// Pointer to log file
		fPtr = fopen("/home/pi/mhcv.txt", "a"); // /home/pi on Raspian
		dbgLen = sprintf(debugMsg,"initApp - Stop TCP thread and close TCP socket\n");
		fwrite(debugMsg,dbgLen,1,fPtr);
		fclose(fPtr);
#endif
		// Close TCP socket
		if (tcpUp) {
			tcpClose();
#ifdef DO_DEBUG
			// Write Log data
			// Pointer to log file
			fPtr = fopen("/home/pi/mhcv.txt", "a"); // /home/pi on Raspian
			dbgLen = sprintf(debugMsg,"initApp - TCP thread closed\n");
			fwrite(debugMsg,dbgLen,1,fPtr);
			fclose(fPtr);
#endif
		}
		pthread_cancel(threadTCPserv);
#ifdef DO_DEBUG
		// Write Log data
		// Pointer to log file
		fPtr = fopen("/home/pi/mhcv.txt", "a"); // /home/pi on Raspian
		dbgLen = sprintf(debugMsg,"initApp - TCP thread stopped\n");
		fwrite(debugMsg,dbgLen,1,fPtr);
		fclose(fPtr);
#endif
		// Stop the TCP debug server thread
#ifdef DO_DEBUG
		// Write Log data
		// Pointer to log file
		fPtr = fopen("/home/pi/mhcv.txt", "a"); // /home/pi on Raspian
		dbgLen = sprintf(debugMsg,"initApp - Stop TCP debug thread and close TCP debug socket\n");
		fwrite(debugMsg,dbgLen,1,fPtr);
		fclose(fPtr);
#endif
		pthread_cancel(threadDebugserv);
		// Close TCP socket
		if (tcpDebugUp) {
			debugTcpClose();
		}
		// Stop the UDP server thread
#ifdef DO_DEBUG
		// Write Log data
		// Pointer to log file
		fPtr = fopen("/home/pi/mhcv.txt", "a"); // /home/pi on Raspian
		dbgLen = sprintf(debugMsg,"initApp - Stop UDP thread and close UDP socket\n");
		fwrite(debugMsg,dbgLen,1,fPtr);
		fclose(fPtr);
#endif
		pthread_cancel(threadUDPserv);
		// Close UDP socket
		if (udpUp) {
			udpClose();
		}

#ifdef DO_DEBUG
		// Write Log data
		// Pointer to log file
		fPtr = fopen("/home/pi/mhcv.txt", "a"); // /home/pi on Raspian
		dbgLen = sprintf(debugMsg,"initApp - Stop timer\n");
		fwrite(debugMsg,dbgLen,1,fPtr);
		fclose(fPtr);
#endif
		// Stop the interval timer
		myIntTimer.it_value.tv_sec = 0;
		myIntTimer.it_value.tv_usec = 0;
		myIntTimer.it_interval.tv_sec = 0;
		myIntTimer.it_interval.tv_usec = 0;
		setitimer (ITIMER_REAL, &myIntTimer, NULL);

		// Unsubscribe from MQTT topic(s)
#ifdef DO_DEBUG
		// Write Log data
		// Pointer to log file
		fPtr = fopen("/home/pi/mhcv.txt", "a"); // /home/pi on Raspian
		dbgLen = sprintf(debugMsg,"initApp - Unsubscribe from MQTT and close connection\n");
		fwrite(debugMsg,dbgLen,1,fPtr);
		fclose(fPtr);
#endif
		unsubscribeTopic("/CMD");
		// Close MQTT connection
		if (mqttUp) {
			mqttClose();
		}

#ifdef CAM_ACTIV
		// Stop the IP CAM streaming thread
#ifdef DO_DEBUG
		// Write Log data
		// Pointer to log file
		fPtr = fopen("/home/pi/mhcv.txt", "a"); // /home/pi on Raspian
		dbgLen = sprintf(debugMsg,"initApp - Stop IP Camera streaming thread\n");
		fwrite(debugMsg,dbgLen,1,fPtr);
		fclose(fPtr);
#endif
		pthread_cancel(threadCamStream);
#endif
	}
	
	// Return codes from function calls
	int rc;

	// Initialize globals
	// Write Log data
	getSysTime(1);
	// Pointer to log file
	fPtr = fopen("/home/pi/mhcv.txt", "a"); // /home/pi on Raspian
	fwrite(sysTime,20,1,fPtr);
	dbgLen = sprintf(debugMsg,"initApp - Initialize globals\n");
	fwrite(debugMsg,dbgLen,1,fPtr);
	fclose(fPtr);
	udpUp = tcpUp = tcpDebugUp = mqttUp = false;
	hasFD1JSON = hasCA1JSON = hasLB1JSON = hasLY1JSON = hasSPMJSON = hasSF1JSON 
		= hasSB1JSON = hasWEIJSON = hasWEOJSON = hasMqttCmd = hasMHCJSON = hasVC2JSON
		= hasTCPdata
		= streamCam = false;
		
	ntpCnt = camStatusCnt = 0;

	//----- REGISTER EXTERNAL SIGNAL FUNCTION HANDLERS -----
	//Functions to be called on these special signals being triggered
#ifdef DO_DEBUG
	// Write Log data
	// Pointer to log file
	fPtr = fopen("/home/pi/mhcv.txt", "a"); // /home/pi on Raspian
	dbgLen = sprintf(debugMsg,"initApp - Register signal handler\n");
	fwrite(debugMsg,dbgLen,1,fPtr);
	fclose(fPtr);
#endif
	signal(SIGUSR1, signalHandler);
	signal(SIGUSR2, signalHandler);
	signal(SIGINT, signalHandler);
	atexit((void *) signalHandler);

	// Start UDP server thread
	int parameter;
	if ((rc = pthread_create(&threadUDPserv, NULL, udpServer, &parameter)) != 0) {
		// Write Log data
		// Pointer to log file
		fPtr = fopen("/home/pi/mhcv.txt", "a"); // /home/pi on Raspian
		dbgLen = sprintf(debugMsg,"initApp - UDP socket server thread creation failed with result %d\n", rc);
		fwrite(debugMsg,dbgLen,1,fPtr);
		fclose(fPtr);
		exit(2);
	}
#ifdef DO_DEBUG
	// Write Log data
	// Pointer to log file
	fPtr = fopen("/home/pi/mhcv.txt", "a"); // /home/pi on Raspian
	dbgLen = sprintf(debugMsg,"initApp - UDP server thread ok\n");
	fwrite(debugMsg,dbgLen,1,fPtr);
	fclose(fPtr);
#endif

	// Start TCP server thread
	if ((rc = pthread_create(&threadTCPserv, NULL, tcpServer, &parameter)) != 0) {
		// Write Log data
		// Pointer to log file
		fPtr = fopen("/home/pi/mhcv.txt", "a"); // /home/pi on Raspian
		dbgLen = sprintf(debugMsg,"initApp - TCP socket server thread creation failed with result %d\n", rc);
		fwrite(debugMsg,dbgLen,1,fPtr);
		fclose(fPtr);
		udpClose();
		exit(3);
	}
#ifdef DO_DEBUG
	// Write Log data
	// Pointer to log file
	fPtr = fopen("/home/pi/mhcv.txt", "a"); // /home/pi on Raspian
	dbgLen = sprintf(debugMsg,"initApp - TDP server thread ok\n");
	fwrite(debugMsg,dbgLen,1,fPtr);
	fclose(fPtr);
#endif
	
	// Start TCP debug server thread
	if ((rc = pthread_create(&threadDebugserv, NULL, debugTcpServer, &parameter)) != 0) {
		// Write Log data
		// Pointer to log file
		fPtr = fopen("/home/pi/mhcv.txt", "a"); // /home/pi on Raspian
		dbgLen = sprintf(debugMsg,"initApp - TCP debug socket server thread creation failed with result %d\n", rc);
		fwrite(debugMsg,dbgLen,1,fPtr);
		fclose(fPtr);
		udpClose();
		tcpClose();
		exit(3);
	}
#ifdef DO_DEBUG
	// Write Log data
	// Pointer to log file
	fPtr = fopen("/home/pi/mhcv.txt", "a"); // /home/pi on Raspian
	dbgLen = sprintf(debugMsg,"initApp - TDP debug server thread ok\n");
	fwrite(debugMsg,dbgLen,1,fPtr);
	fclose(fPtr);
#endif

	// Setup MQTT connection
	if ((rc = mqttInit()) != 0) {
		// Write Log data
		// Pointer to log file
		fPtr = fopen("/home/pi/mhcv.txt", "a"); // /home/pi on Raspian
		dbgLen = sprintf(debugMsg,"initApp - MQTT connection initialization failed with result %d\n", rc); 
		fwrite(debugMsg,dbgLen,1,fPtr);
		fclose(fPtr);
	} else {
		mqttUp = true;
#ifdef DO_DEBUG
		// Write Log data
		// Pointer to log file
		fPtr = fopen("/home/pi/mhcv.txt", "a"); // /home/pi on Raspian
		dbgLen = sprintf(debugMsg,"initApp - MQTT connection initialized\n");
		fwrite(debugMsg,dbgLen,1,fPtr);
		fclose(fPtr);
#endif
	}
	
#ifdef CAM_ACTIV
	// Start IP Camera streaming
	parameter = 0; // Start thread for camera 0
	if ((rc = pthread_create(&threadCamStream, NULL, camStreamThread, &parameter)) != 0) {
		// Write Log data
		// Pointer to log file
		fPtr = fopen("/home/pi/mhcv.txt", "a"); // /home/pi on Raspian
		dbgLen = sprintf(debugMsg,"initApp - IP Cam streaming thread creation failed with result %d\n", rc);
		fwrite(debugMsg,dbgLen,1,fPtr);
		fclose(fPtr);
	} else {
		// Init JSON object
		json_value_free(mhcJSON); // Free the allocated memory (if any)
		mhcJSON = json_value_init_object(); // Init JSON object
		JSON_Object *root_object = json_value_get_object(mhcJSON);
		json_object_set_string(root_object,"de","mhc");

		// Add video invalid flag
		json_object_set_number(root_object,"pi",0);

		hasMHCJSON = true;

		// String for debug output
		char *serialized_string = json_serialize_to_string(mhcJSON);
		int jsonLenRoot = 0;
		jsonLenRoot = mbstowcs(NULL,serialized_string,0);
		if (jsonLenRoot != 0) {
			udpSend(serialized_string, jsonLenRoot);
		}
		json_free_serialized_string(serialized_string);
#ifdef DO_DEBUG
		// Write Log data
		// Pointer to log file
		fPtr = fopen("/home/pi/mhcv.txt", "a"); // /home/pi on Raspian
		dbgLen = sprintf(debugMsg,"initApp - IP Cam streaming thread ok\n");
		fwrite(debugMsg,dbgLen,1,fPtr);
		fclose(fPtr);
#endif
	}
#endif

	if (isNewStart) {
		// Setup OneWire interface to DHT11 sensor
		if(wiringPiSetup()==-1) {
		// Write Log data
		// Pointer to log file
		fPtr = fopen("/home/pi/mhcv.txt", "a"); // /home/pi on Raspian
		dbgLen = sprintf(debugMsg,"initApp - Initializing OneWire interface failed\n"); 
		fwrite(debugMsg,dbgLen,1,fPtr);
		fclose(fPtr);
			unsubscribeTopic("/CMD");
			udpClose();
			tcpClose();
			debugTcpClose();
			mqttClose();
			exit(6);
		}
//#ifdef DO_DEBUG
		// Write Log data
		// Pointer to log file
		fPtr = fopen("/home/pi/mhcv.txt", "a"); // /home/pi on Raspian
		dbgLen = sprintf(debugMsg,"initApp - Initializing OneWire interface ok\n");
		fwrite(debugMsg,dbgLen,1,fPtr);
		fclose(fPtr);
//#endif
	}
	// Get initial DHT data
//	dht11Read();

	// Signal for time triggered tasks
	memset (&saMyTimer, 0, sizeof (saMyTimer));
	saMyTimer.sa_handler = &signalHandler;
	sigaction (SIGALRM, &saMyTimer, NULL);
	// Timer to be triggered every 10 seconds
	myIntTimer.it_value.tv_sec = 10;
	myIntTimer.it_value.tv_usec = 0;
	/* ... and every 10 seconds after that. */
	myIntTimer.it_interval.tv_sec = 10;
	myIntTimer.it_interval.tv_usec = 0;
	
	// Start the timer!
	setitimer (ITIMER_REAL, &myIntTimer, NULL);
	
	// Request status from all devices 
	requestAllStatus();

	// Send time to all devices
	updateDeviceTime();
	
	// Subscribe to MQTT '/CMD' topic
	if (mqttUp) {
		// Subscribe to topic(s)
		if (mqttUp && ((rc = subscribeTopic("/CMD", 1)) != 0)) {
			// Write Log data
			// Pointer to log file
			fPtr = fopen("/home/pi/mhcv.txt", "a"); // /home/pi on Raspian
			dbgLen = sprintf(debugMsg,"initApp - MQTT topic subscription failed with result %d\n", rc); 
			fwrite(debugMsg,dbgLen,1,fPtr);
			fclose(fPtr);
		} 
#ifdef DO_DEBUG
		// Write Log data
		// Pointer to log file
		fPtr = fopen("/home/pi/mhcv.txt", "a"); // /home/pi on Raspian
		dbgLen = sprintf(debugMsg,"initApp - MQTT topic subscription ok\n");
		fwrite(debugMsg,dbgLen,1,fPtr);
		fclose(fPtr);
#endif
	}
	dbgLen = sprintf(debugMsg,"initApp successfull finished");
	tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
#ifdef DO_DEBUG
		// Write Log data
		// Pointer to log file
		fPtr = fopen("/home/pi/mhcv.txt", "a"); // /home/pi on Raspian
		fwrite(debugMsg,dbgLen,1,fPtr);
		fclose(fPtr);
#endif
}
