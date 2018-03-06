#include "mhc.h"

/* Handle termination of the program
 */
void signalHandler( int signum ) {

	// Return codes from function calls
	int rc;
	// Pointer to log file
	FILE *fPtr;
	// Exit message 
	char * exitMsg = "Unknown termination signal";
	
	// Try to catch non system signals
	if (signum > 32) {
		if (genDebugOn) {
			dbgLen = sprintf(debugMsg,"signalHandler called: %u\n",signum); 
			tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
		}
		return;
	}
	
	// Final exit signal
	if (signum == 0) {
		if (genDebugOn) {
			dbgLen = sprintf(debugMsg,"signalHandler called: %u\n",signum); 
			tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
		}
		exit(signum);
	}

	switch (signum) {
		case SIGINT:
			exitMsg = "\nSIGINT - Interactive attention signal received";
			
			getSysTime(1);
			fPtr = fopen("/home/pi/mhcv.txt", "a"); // /home/pi on Raspian
			dbgLen = sprintf(debugMsg,"%s: Received SIGINT:\nExitMsg: %s\nSignum: %d\n", sysTime, exitMsg, signum);
			fwrite(debugMsg,dbgLen,1,fPtr);
			fclose(fPtr);
			break;
		case SIGUSR1: // called when UDP data has been received and is ready to be published on MQTT
			if (genDebugOn) {
				dbgLen = sprintf(debugMsg,"signalHandler called: SIGUSR1\n"); 
				tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
			}
			checkMQTT();
			if (mqttUp) { // only try to publish if we are connected to the broker
//				if (hasWEIJSON) {
//					//{"de":"wei","te":29,"hu":46,"he":29.2}
//					char inWeather[50];
//					sprintf(inWeather,"{\"de\":\"wei\",\"te\":%.1f,\"hu\":%.1f,\"he\":%.1f}"
//						, tempInside, humidInside, heatInside);
//					udpSend(inWeather, strlen(inWeather));
//				}
//				// Publish device status to MQTT broker
//				mqttPublisher();
			}
			return;
			break;
		case SIGUSR2: // called if command arrived over MQTT or TCP server
			if (genDebugOn) {
				dbgLen = sprintf(debugMsg,"signalHandler called: SIGUSR2\n"); 
				tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
			}
			// Check if we received a command over MQTT
			if (hasMqttCmd) {
				if (mqttDebugOn) {
					dbgLen = sprintf(debugMsg,"SIGUSR2: received a command over MQTT\n"); 
					tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
				}
				mqttProcess();
			}

			// Check if there is an incoming TCP request
			if (hasTCPdata) {
				hasTCPdata = false;
				if (genDebugOn) {
					dbgLen = sprintf(debugMsg,"SIGUSR2: received TCP packet: >%s<\n", tcpBufTemp);
					tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
				}
				if (strncmp(tcpBufTemp,"r",1) == 0) {
					dbgLen = sprintf(debugMsg,"SIGUSR2: TCP packet: Reboot!");
					tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
					// Reset the machine!
					system ("sudo reboot now");
				} else if (strncmp(tcpBufTemp,"dg",2) == 0) {
					// Toggle Debug!
					genDebugOn = !genDebugOn;
					dbgLen = sprintf(debugMsg,"SIGUSR2: TCP packet: Switch General Debug to %s\n", 
						genDebugOn ? "true" : "false");
					tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
				} else if (strncmp(tcpBufTemp,"dc",2) == 0) {
					// Toggle Debug!
					camDebugOn = !camDebugOn;
					dbgLen = sprintf(debugMsg,"SIGUSR2: TCP packet: Switch Camera Debug to %s\n", 
						camDebugOn ? "true" : "false");
					tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
				} else if (strncmp(tcpBufTemp,"dd",2) == 0) {
					// Toggle Debug!
					dht11DebugOn = !dht11DebugOn;
					dbgLen = sprintf(debugMsg,"SIGUSR2: TCP packet: Switch DHT11 Debug to %s\n", 
						dht11DebugOn ? "true" : "false");
					tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
				} else if (strncmp(tcpBufTemp,"dm",2) == 0) {
					// Toggle Debug!
					mqttDebugOn = !mqttDebugOn;
					dbgLen = sprintf(debugMsg,"SIGUSR2: TCP packet: Switch MQTT Debug to %s\n", 
						mqttDebugOn ? "true" : "false");
					tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
				} else if (strncmp(tcpBufTemp,"du",2) == 0) {
					// Toggle Debug!
					udpDebugOn = !udpDebugOn;
					dbgLen = sprintf(debugMsg,"SIGUSR2: TCP packet: Switch UDP Debug to %s\n", 
						udpDebugOn ? "true" : "false");
					tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
				}
			}
			// Check if there is an incoming TCP debug request
			if (hasDebugTCPdata) {
				hasDebugTCPdata = false;
				if (genDebugOn) {
					dbgLen = sprintf(debugMsg,"SIGUSR2: received TCP debug packet: %s\n", tcpBufTemp);
					tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
				}
				// Write the received data into a file
				getSysTime(1);
				fPtr = fopen("/home/pi/log.txt", "a"); // /home/pi on Raspian
				fwrite(sysTime,20,1,fPtr);
				fwrite(debugTcpBufTemp,debugTcpRecvBytesTemp,1,fPtr);
				fwrite("\n",1,1,fPtr);
				fclose(fPtr);
			}
			return;
			break;
		case SIGALRM: // called every 10 seconds
			// Count up events for device's clocks update time
			ntpCnt++;
			// Count up events for camera status update time
			camStatusCnt++;

			// Check if we are still connected to the MQTT server
			checkMQTT();
			// Check if the UDP listener is still active
			checkUDP();
			// Check if TCP listener is still active
			checkTCP();
			// Check if we are still connected to DiskStation
			checkSamba();
			
			// Get temperature from DHT11 sensor
//			if (genDebugOn) {
//				dbgLen = sprintf(debugMsg,"SIGALRM triggered -> get temperature\n"); 
//				tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
//			}
//			dht11Read();
			
			// Check if it is time to reboot
			lunchReboot();
			// Check if it is time to reboot the WiFi router
//			rebootRouter();
			
			if (ntpCnt >= 1080) { // 10s * 1080 = 10800s = 180min = 3 hours
				// Update device's clocks every 3 hours
				if (genDebugOn) {
					dbgLen = sprintf(debugMsg,"SIGALRM 1080 times triggered -> update device's clocks\n"); 
					tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
				}
				if (tcpUp) { // only try to update if TCP socket is open
					updateDeviceTime();
				}
				ntpCnt = 0;
				
				// Check if storage is getting full and clean it up if necessary
				cleanDir();
			}

			if (camStatusCnt >= 5) { // 10s * 6 = 1min
				camStatusCnt = 0;
				// Send camera status every 1 minute
				if (camDebugOn) {
					dbgLen = sprintf(debugMsg,"SIGALRM 60 times triggered -> send camera status\n"); 
					tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
				}
				// Init JSON object
				json_value_free(mhcJSON); // Free the allocated memory (if any)
				mhcJSON = json_value_init_object(); // Init JSON object
				JSON_Object *root_object = json_value_get_object(mhcJSON);
				json_object_set_string(root_object,"de","mhc");
				hasMHCJSON = true;

				// Broadcast status
				char *statusMsg = "{\"de\":\"mhc\"}\00";
				int statMsgLen = 0;
				statMsgLen = mbstowcs(NULL,statusMsg,0);
				udpSend(statusMsg, statMsgLen);
				
//				// String for status output
//				char *serialized_string = json_serialize_to_string(mhcJSON);
//				int jsonLenRoot = 0;
//				jsonLenRoot = mbstowcs(NULL,serialized_string,0);
//				if (jsonLenRoot != 0) {
//					udpSend(serialized_string, jsonLenRoot);
//				}
//				json_free_serialized_string(serialized_string);
				// Publish device status to MQTT broker
				mqttPublisher();
			}
			// Check if we have a day change and move videos into subfolder
			moveVideos();

			if (!udpUp) { // UDP receiver stopped
				int parameter;
				pthread_create(&threadUDPserv, NULL, udpServer, &parameter);
			}
			return;
			break;
		default:
			if (genDebugOn) {
				dbgLen = sprintf(debugMsg,"Received unknown signal:\nExitMsg: %s\nSignum: %d\n", exitMsg, signum);
				tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
			}
			getSysTime(1);
			fPtr = fopen("/home/pi/mhcv.txt", "a"); // /home/pi on Raspian
			dbgLen = sprintf(debugMsg,"%s: Received signal:\nExitMsg: %s\nSignum: %d\n", sysTime, exitMsg, signum);
			fwrite(debugMsg,dbgLen,1,fPtr);
			fclose(fPtr);

			exit(signum);
			break;
	}
	printf("%s - %d\n", exitMsg, signum); 

	fPtr = fopen("/home/pi/mhcv.txt", "a"); // /home/pi on Raspian
	dbgLen = sprintf(debugMsg,"%s: Received SIGINT:\nExitMsg: %s\nSignum: %d\n", sysTime, exitMsg, signum);
	fwrite(debugMsg,dbgLen,1,fPtr);
	fclose(fPtr);

	// Stop the CAM streaming thread
#ifdef CAM_ACTIV
	pthread_cancel(threadCamStream);
#endif

	// Stop the TCP server thread
	pthread_cancel(threadTCPserv);
	// Close TCP socket
	if (tcpUp) {
		tcpClose();
	}

	// Stop the TCP debug server thread
	pthread_cancel(threadDebugserv);
	// Close TCP socket
	if (tcpDebugUp) {
		debugTcpClose();
	}

	// Stop the interval timer
	myIntTimer.it_value.tv_sec = 0;
	myIntTimer.it_value.tv_usec = 0;
	myIntTimer.it_interval.tv_sec = 0;
	myIntTimer.it_interval.tv_usec = 0;
	setitimer (ITIMER_REAL, &myIntTimer, NULL);

	// Unsubscribe from MQTT topic(s)
	unsubscribeTopic("/CMD");
	// Close MQTT connection
	if (mqttUp) {
		mqttClose();
	}

	// Close UDP socket
	if (udpUp) {
		udpClose();
	}

	system("sudo /usr/local/bin/mhc &");
	// terminate program  
	exit(signum);  
}
