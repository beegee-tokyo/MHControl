#include "mhc.h"

/* Initialize UDP socket for listening
 * <return> <code>int</code>
 *		0 socket is created for listening
 *		-1 socket creation failed
 *		-2 socket bind failed
 */
int udpInit(void)
{
	// Result from function calls
	int rc;

	// Get UDP socket
	if ((udpSocket=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1) {
		if (udpDebugOn) {
			dbgLen = sprintf(debugMsg,"Could not get UDP socket!\n");
			tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
		}
		return -1;
	}

	// Setup timeouts for sending
	struct timeval timeout;      
    timeout.tv_sec = 60;
    timeout.tv_usec = 0;

    if ((rc = setsockopt (udpSocket, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout,
                sizeof(timeout))) < 0) {
		if (udpDebugOn) {
			dbgLen = sprintf(debugMsg,"Could not setup UDP send timouts %d!\n", rc);
			tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
		}
	}

	int broadcastPermission = 1;
    if ((rc = setsockopt(udpSocket, SOL_SOCKET, SO_BROADCAST, (void*)&broadcastPermission, 
		sizeof(broadcastPermission))) < 0) {
		if (udpDebugOn) {
			dbgLen = sprintf(debugMsg,"Could not get broadcast permission! %d\n", rc);
			tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
		}
	}

	// Cleanup UDP socket structure
	memset((char *) &udpSocketMe, 0, sizeof(udpSocketMe));
	// Setup UDP socket
	udpSocketMe.sin_family = AF_INET;
	udpSocketMe.sin_port = htons(UDP_PORT);
	udpSocketMe.sin_addr.s_addr = htonl(INADDR_ANY);
	// Bind UDP socket
	if (bind(udpSocket, (struct sockaddr *)&udpSocketMe, sizeof(udpSocketMe))==-1)
			return -2;
	return 0;
}

/* Close UDP socket receiver
 */
void udpClose() {
	close(udpSocket);
	udpUp = false;
}

/* Process received UDP packet
 * <return> <code>int</code>
 *		0 packet could be processed
 *		-1 UDP packet was no JSON packet
 *		-2 no device ID in JSON packet
 *		!0 if error occured TODO define failure return values
 */
int udpProcess(int udpRecvBytes, char *senderIP) {
	// Received data as JSON
	JSON_Value *jsonPacket;
	// Root of JSON data
	JSON_Object *root_object;
	// Device ID as string
	const char * device;
	// Device ID as number
	int deviceID = 0;
	// Loop counter
	int j;
	// String for debug output
	char *serialized_string = NULL;
	// Flag if device address has changed and needs to be saved
	bool needToSave = false;

	if ((jsonPacket = json_parse_string(udpBuf)) == NULL) {
		if (udpDebugOn) {
			dbgLen = sprintf(debugMsg,"No valid JSON found\n");
			tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
		}
		// Cleanup buffer and temp JSON object
		json_value_free(jsonPacket);
		memset((char *) &udpBuf, 0, BUFLEN);
		return -1;
	}

	// Clear receiving buffer
	memset((char *) &udpBuf, 0, BUFLEN);

	root_object = json_value_get_object(jsonPacket);
	if ((device = json_object_get_string(root_object,"de")) == 0) {
		// Cleanup buffer and temp JSON object
		json_value_free(jsonPacket);
		return -2; // No device ID found in JSON
	}

	deviceID = 0;
	for (j=0; j<strlen(device); j++) {
		deviceID += device[j]+(j*16);
	}

	// Root of JSON data
	JSON_Object *config_object = json_value_get_object(configJSON);

	switch (deviceID) {
		case devFD1:
//{"de":"fd1","po":0,"mo":0,"sp":0,"te":16,"co":559.89,"st":0,"au":1,"ti":0,"ot":2,"ts":"03:33","bo":0,"dt":0,"tm":"0:23"}
//{"de":"fd1","po":0,"mo":0,"sp":0,"te":16,"co":559.89,"st":0,"au":1,"ti":0,"ot":2,"ts":"03:33"}
			if (udpDebugOn) {
				serialized_string = json_serialize_to_string_pretty(fd1JSON);
				dbgLen = sprintf(debugMsg,"Start aircon 1 process id:%d\n",deviceID);
				tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
				if (serialized_string != NULL) {
					dbgLen = sprintf(debugMsg,"Original FD1 JSON:\n%s\n",serialized_string);
					tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
					json_free_serialized_string(serialized_string);
				}
			}
			if (strcmp(ipAC1,senderIP) != 0) {
				needToSave = true;
				json_object_set_string(config_object,"ac1",senderIP);
//				if (udpDebugOn) {
					dbgLen = sprintf(debugMsg,"New ipAC1 address = %s old = %s\n",senderIP, ipAC1);
					tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
//				}
				strcpy(ipAC1,senderIP);
			}

			json_value_free(fd1JSON); // Free the allocated memory (if any)
			fd1JSON = json_value_deep_copy(jsonPacket);
			// Root of JSON data
			JSON_Object *fd1_object = json_value_get_object(fd1JSON);
			// Clean JSON
			json_object_remove(fd1_object, "bo");
			json_object_remove(fd1_object, "dt");
			json_object_remove(fd1_object, "tm");
			hasFD1JSON = true;
			if (udpDebugOn) {
				serialized_string = json_serialize_to_string_pretty(fd1JSON);
				dbgLen = sprintf(debugMsg,"End aircon 1 process\n");
				tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
				if (serialized_string != NULL) {
					dbgLen = sprintf(debugMsg,"Copied and cleaned FD1 JSON:\n%s\n",serialized_string);
					tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
					json_free_serialized_string(serialized_string);
				}
			}
			break;
		case devCA1:
//{"de":"ca1","po":0,"mo":0,"sp":0,"te":16,"co":559.89,"st":0,"au":1,"ti":0,"ot":2,"ts":"03:33","bo":0,"dt":0,"tm":"0:23","sw":0,"tu":0,"io":0}
//{"de":"ca1","po":0,"mo":0,"sp":0,"te":16,"co":559.89,"st":0,"au":1,"ti":0,"ot":2,"ts":"03:33","sw":0,"tu":0,"io":0}
			if (udpDebugOn) {
				serialized_string = json_serialize_to_string_pretty(ca1JSON);
				dbgLen = sprintf(debugMsg,"Start aircon 2 process id:%d\n",deviceID);
				tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
				if (serialized_string != NULL) {
					dbgLen = sprintf(debugMsg,"Original CA1 JSON:\n%s\n",serialized_string);
					tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
					json_free_serialized_string(serialized_string);
				}
			}
			if (strcmp(ipAC2,senderIP) != 0) {
				needToSave = true;
				json_object_set_string(config_object,"ac2",senderIP);
//				if (udpDebugOn) {
					dbgLen = sprintf(debugMsg,"New ipAC2 address = %s old = %s\n",senderIP, ipAC2);
					tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
//				}
				strcpy(ipAC2,senderIP);
			}

			json_value_free(ca1JSON); // Free the allocated memory (if any)
			ca1JSON = json_value_deep_copy(jsonPacket);
			// Root of JSON data
			JSON_Object *ca1_object = json_value_get_object(ca1JSON);
			// Clean JSON
			json_object_remove(ca1_object, "bo");
			json_object_remove(ca1_object, "dt");
			json_object_remove(ca1_object, "tm");
			hasCA1JSON = true;
			if (udpDebugOn) {
				serialized_string = json_serialize_to_string_pretty(ca1JSON);
				dbgLen = sprintf(debugMsg,"End aircon 2 process\n");
				tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
				if (serialized_string != NULL) {
					dbgLen = sprintf(debugMsg,"Copied and cleaned CA1 JSON:\n%s\n",serialized_string);
					tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
					json_free_serialized_string(serialized_string);
				}
			}
			break;
		case devSF1:
//{"de":"sf1","al":0,"ao":1,"au":1,"an":22,"af":7,"lo":1}
			if (udpDebugOn) {
				dbgLen = sprintf(debugMsg,"Start front security process id:%d\n",deviceID);
				tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
			}
			if (strcmp(ipSecFront,senderIP) != 0) {
				needToSave = true;
				json_object_set_string(config_object,"sf1",senderIP);
//				if (udpDebugOn) {
					dbgLen = sprintf(debugMsg,"New ipSecFront address = %s old = %s\n",senderIP, ipSecFront);
					tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
//				}
				strcpy(ipSecFront,senderIP);
			}

			json_value_free(sf1JSON); // Free the allocated memory (if any)
			sf1JSON = json_value_deep_copy(jsonPacket);
			hasSF1JSON = true;
			if (udpDebugOn) {
				serialized_string = json_serialize_to_string_pretty(sf1JSON);
				dbgLen = sprintf(debugMsg,"End front security process\n");
				tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
				if (serialized_string != NULL) {
					dbgLen = sprintf(debugMsg,"Copied SF1 JSON:\n%s\n",serialized_string);
					tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
					json_free_serialized_string(serialized_string);
				}
			}
			break;
		case devSB1:
//{"de":"sb1","al":0,"ao":1,"au":1,"an":22,"af":7,"lo":1,"te":26.00,"hu":61.00,"he":26.96}
//{"de":"sb1","al":0,"ao":1,"au":1,"an":22,"af":7,"lo":1}
//{"de":"weo","te":29.00,"hu":58.00,"hi":30.73}
			if (udpDebugOn) {
				serialized_string = json_serialize_to_string_pretty(sb1JSON);
				dbgLen = sprintf(debugMsg,"Start back security process id:%d\n",deviceID);
				tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
				if (serialized_string != NULL) {
					dbgLen = sprintf(debugMsg,"Original SB1 JSON:\n%s\n",serialized_string);
					tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
					json_free_serialized_string(serialized_string);
				}
			}
			if (strcmp(ipSecBack,senderIP) != 0) {
				needToSave = true;
				json_object_set_string(config_object,"sb1",senderIP);
//				if (udpDebugOn) {
					dbgLen = sprintf(debugMsg,"New ipSecBack address = %s old = %s\n",senderIP, ipSecBack);
					tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
//				}
				strcpy(ipSecBack,senderIP);
			}

			json_value_free(sb1JSON); // Free the allocated memory (if any)
			sb1JSON = json_value_deep_copy(jsonPacket);
			// Root of JSON data
			JSON_Object *sb1_object = json_value_get_object(sb1JSON);
			// Clean JSON
			json_object_remove(sb1_object, "te");
			json_object_remove(sb1_object, "hu");
			json_object_remove(sb1_object, "he");
			// Create external weather JSON
			json_value_free(weoJSON); // Free the allocated memory (if any)
			weoJSON = json_value_init_object(); // Init JSON object
			JSON_Object *root_object_weo = json_value_get_object(weoJSON);
			json_object_set_string(root_object_weo,"de","weo");
			if (json_object_has_value(root_object,"te")) {
				float tempOutside = json_object_get_number(root_object,"te");
				json_object_set_number(root_object_weo,"te",tempOutside);
			}
			if (json_object_has_value(root_object,"hu")) {
				float humidOutside = json_object_get_number(root_object,"hu");
				json_object_set_number(root_object_weo,"hu",humidOutside);
			}
			if (json_object_has_value(root_object,"he")) {
				float heatOutside = json_object_get_number(root_object,"he");
				json_object_set_number(root_object_weo,"hi",heatOutside);
			}
			hasSB1JSON = true;
			hasWEOJSON = true;
			if (udpDebugOn) {
				serialized_string = json_serialize_to_string_pretty(sb1JSON);
				dbgLen = sprintf(debugMsg,"End back security process\n");
				tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
				if (serialized_string != NULL) {
					dbgLen = sprintf(debugMsg,"Copied and cleaned SB1 JSON:\n%s\n",serialized_string);
					tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
					json_free_serialized_string(serialized_string);
				}
				serialized_string = json_serialize_to_string_pretty(weoJSON);
				if (serialized_string != NULL) {
					dbgLen = sprintf(debugMsg,"Created WEO JSON:\n%s\n",serialized_string);
					tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
					json_free_serialized_string(serialized_string);
				}
			}
			break;
		case devSPM:
			//{"de":"spm","s":0.00,"c":591.97}
			if (udpDebugOn) {
				dbgLen = sprintf(debugMsg,"Start spm process id:%d\n",deviceID);
				tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
			}
			if (strcmp(ipSPM,senderIP) != 0) {
				needToSave = true;
				json_object_set_string(config_object,"spm",senderIP);
//				if (udpDebugOn) {
					dbgLen = sprintf(debugMsg,"New ipSPM address = %s old = %s\n",senderIP, ipSPM);
					tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
//				}
				strcpy(ipSPM,senderIP);
			}

			json_value_free(spmJSON); // Free the allocated memory (if any)
			spmJSON = json_value_deep_copy(jsonPacket);
			hasSPMJSON = true;
			if (udpDebugOn) {
				serialized_string = json_serialize_to_string_pretty(spmJSON);
				if (serialized_string != NULL) {
					dbgLen = sprintf(debugMsg,"New solar panel JSON:\n%s\n",serialized_string);
					tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
					json_free_serialized_string(serialized_string);
				}
			}
			break;
		case devLB1:
			//{"de":"lb1","bo":0,"br":255,"di":202}
			if (udpDebugOn) {
				dbgLen = sprintf(debugMsg,"Start bedroom light process id:%d\n",deviceID);
				tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
			}
			if (strcmp(ipLB1,senderIP) != 0) {
				needToSave = true;
				json_object_set_string(config_object,"lb1",senderIP);
//				if (udpDebugOn) {
					dbgLen = sprintf(debugMsg,"New ipLB1 address = %s old = %s\n",senderIP, ipLB1);
					tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
//				}
				strcpy(ipLB1,senderIP);
			}

			json_value_free(lb1JSON); // Free the allocated memory (if any)
			lb1JSON = json_value_deep_copy(jsonPacket);
			// Root of JSON data
			JSON_Object *lb1_object = json_value_get_object(lb1JSON);
			// Clean JSON
			json_object_remove(lb1_object, "bo");
			hasLB1JSON = true;
			if (udpDebugOn) {
				serialized_string = json_serialize_to_string_pretty(lb1JSON);
				if (serialized_string != NULL) {
					dbgLen = sprintf(debugMsg,"New light JSON:\n%s\n",serialized_string);
					tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
					json_free_serialized_string(serialized_string);
				}
			}
			break;
		case devLY1:
			//{"de":"ly1","lo":0}
			if (udpDebugOn) {
				dbgLen = sprintf(debugMsg,"Start backyard light process id:%d\n",deviceID);
				tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
			}
			if (strcmp(ipLY1,senderIP) != 0) {
				needToSave = true;
				json_object_set_string(config_object,"ly1",senderIP);
//				if (udpDebugOn) {
					dbgLen = sprintf(debugMsg,"New ipLY1 address = %s old = %s\n",senderIP, ipLY1);
					tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
//				}
				strcpy(ipLY1,senderIP);
			}

			json_value_free(ly1JSON); // Free the allocated memory (if any)
			ly1JSON = json_value_deep_copy(jsonPacket);
			// Root of JSON data
			JSON_Object *ly1_object = json_value_get_object(ly1JSON);
			hasLY1JSON = true;
			if (udpDebugOn) {
				serialized_string = json_serialize_to_string_pretty(ly1JSON);
				if (serialized_string != NULL) {
					dbgLen = sprintf(debugMsg,"New backyard light JSON:\n%s\n",serialized_string);
					tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
					json_free_serialized_string(serialized_string);
				}
			}
			break;
		case devCM1:
			//{"de":"cm1","bo":0,"tm":"16:42","pi":0}
			if (udpDebugOn) {
				dbgLen = sprintf(debugMsg,"Start camera process id:%d\n",deviceID);
				tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
			}
			if (strcmp(ipCam1,senderIP) != 0) {
				needToSave = true;
				json_object_set_string(config_object,"cm1",senderIP);
//				if (udpDebugOn) {
					dbgLen = sprintf(debugMsg,"New ipCam1 address = %s old = %s\n",senderIP, ipCam1);
					tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
//				}
				strcpy(ipCam1,senderIP);
			}

			json_value_free(cm1JSON); // Free the allocated memory (if any)
			cm1JSON = json_value_deep_copy(jsonPacket);
			// Root of JSON data
			JSON_Object *cm1_object = json_value_get_object(cm1JSON);
			// Clean JSON
			json_object_remove(cm1_object, "bo");
			hasCM1JSON = true;
			if (udpDebugOn) {
				serialized_string = json_serialize_to_string_pretty(cm1JSON);
				if (serialized_string != NULL) {
					dbgLen = sprintf(debugMsg,"New camera JSON:\n%s\n",serialized_string);
					tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
					json_free_serialized_string(serialized_string);
				}
			}
		   break;
		case devWEI: // Internal wether data, send by this device, therefor ignore it
		case devMHC: // Send by this device, therefor ignore it
			break;
		default: // Unknown device ID
			json_value_free(newJSON); // Free the allocated memory (if any)
			newJSON = json_value_deep_copy(jsonPacket);
			serialized_string = json_serialize_to_string_pretty(newJSON);
			if (serialized_string != NULL) {
				dbgLen = sprintf(debugMsg,"New device ID: %d\nMessage: %s\n",deviceID,serialized_string);
				tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
				json_free_serialized_string(serialized_string);
			}
			break;
		case devDB1:
			//{"de":"db1","rd":0,"u":0,"v":1}
			if (udpDebugOn) {
				dbgLen = sprintf(debugMsg,"Start doorbell process id:%d\n",deviceID);
				tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
			}
			if (strcmp(ipDB1,senderIP) != 0) {
				needToSave = true;
				json_object_set_string(config_object,"db1",senderIP);
//				if (udpDebugOn) {
					dbgLen = sprintf(debugMsg,"New ipDB1 address = %s old = %s\n",senderIP, ipDB1);
					tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
//				}
				strcpy(ipDB1,senderIP);
			}

			json_value_free(db1JSON); // Free the allocated memory (if any)
			db1JSON = json_value_deep_copy(jsonPacket);
			// Root of JSON data
			JSON_Object *db1_object = json_value_get_object(db1JSON);
			// Clean JSON
			json_object_remove(db1_object, "bo");
			hasDB1JSON = true;
			if (udpDebugOn) {
				serialized_string = json_serialize_to_string_pretty(db1JSON);
				if (serialized_string != NULL) {
					dbgLen = sprintf(debugMsg,"New doorbell JSON:\n%s\n",serialized_string);
					tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
					json_free_serialized_string(serialized_string);
				}
			}
			break;
   }
   
   // Check if we need to save a new configuration
   if (needToSave) {
 		FILE *configFilePtr = fopen("/home/pi/config.txt", "w+"); // /home/pi on Raspian
		if (configFilePtr != NULL) {
			serialized_string = json_serialize_to_string(configJSON);
			if (serialized_string != NULL) {
				dbgLen = sprintf(debugMsg,"%s", serialized_string);
				fwrite(debugMsg,dbgLen,1,configFilePtr);
				dbgLen = sprintf(debugMsg,"Wrote new config: %s\n", serialized_string);
				json_free_serialized_string(serialized_string);
				tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
			} else {
				dbgLen = sprintf(debugMsg,"Failed to save config file");
				json_free_serialized_string(serialized_string);
				tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
			}
			fclose(configFilePtr);
		}
	}

	// Clear JSON packet
	json_value_free(jsonPacket);

	if (udpDebugOn) {
		dbgLen = sprintf(debugMsg,"udpProcess finished\n");
		tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
	}
	raise(SIGUSR1);
	return 0;
}

void *udpServer(void *argument) {
	// Setup UDP listener
	if ((resultValue = udpInit()) != 0) {
		if (udpDebugOn) {
			dbgLen = sprintf(debugMsg,"UDP socket listener initialization failed with result %d\n", resultValue);
			tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
		}
		if (resultValue == -2) {
			udpClose();
		}
		return (void *)&resultValue;
	} else {
		udpUp = true;
		if (udpDebugOn) {
			dbgLen = sprintf(debugMsg,"UDP socket listener initialized\n"); 
			tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
		}
	}
	
	int error = 0;
	// Size of UDP socket structure
	int udpSockLen=sizeof(udpSocketOther);

	while (error == 0) {
		// Check if we got any packet over UDP
		resultValue = recvfrom(udpSocket, udpBuf, BUFLEN, 0, (struct sockaddr *)&udpSocketOther, &udpSockLen);
		if (resultValue > 0) {
			if (udpDebugOn) {
				dbgLen = sprintf(debugMsg,"Received packet from %s:%d\non %s:%d\nData: %s\n"
					, inet_ntoa(udpSocketOther.sin_addr), ntohs(udpSocketOther.sin_port)
					, inet_ntoa(udpSocketMe.sin_addr), ntohs(udpSocketMe.sin_port)
					, udpBuf); 
				tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
			}
			// Get sender IP address
			struct sockaddr_in* senderSockAddr = (struct sockaddr_in*)&udpSocketOther;
			struct in_addr senderAddr = senderSockAddr->sin_addr;
			char senderIP[INET_ADDRSTRLEN];
			inet_ntop( AF_INET, &senderAddr, senderIP, INET_ADDRSTRLEN );
			if (udpDebugOn) {
				dbgLen = sprintf(debugMsg,"UDP Sender address = %s \n",senderIP);
				tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
			}
			resultValue = udpProcess(resultValue, senderIP);
			if (udpDebugOn && resultValue != 0) {
				dbgLen = sprintf(debugMsg,"UDP packet process failed with result %d\n", resultValue);
				tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
			}
		}
	}
	udpUp = false;
	return (void *)&resultValue;
}

/* Send broadcast status over UDP socket
 * <param> udpBCpacket
 * 		Pointer to message to be sent
 * <param> packetLen
 * 		Length of message
 * <return> <code>int</code>
 *		0 message forwarded to device
 *		-1 connection failed
 *		-2 broadcast failed
 */
int udpSend(char *udpBCpacket, int packetLen) {
	// Result from function calls
	int rc;

	if (udpDebugOn) {
		dbgLen = sprintf(debugMsg,"udpSend %s\n", udpBCpacket);
		tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
	}

	// Cleanup UDP socket structure
	memset((char *) &udpSocketOut, 0, sizeof(udpSocketOut));
	// Setup UDP socket
	udpSocketOut.sin_family = AF_INET;
	udpSocketOut.sin_port = htons(UDP_PORT);
	udpSocketOut.sin_addr.s_addr = inet_addr(ipUDPBroadcast);

	// Send data to server
	if (udpDebugOn) {
		dbgLen = sprintf(debugMsg,"Sending %s with length %d\n", udpBCpacket, packetLen);
		tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
	}
	rc = sendto(udpSocket, udpBCpacket, packetLen, 0, (struct sockaddr *) &udpSocketOut, sizeof(udpSocketOut));
	if (rc != packetLen) {
		if (udpDebugOn) {
			dbgLen = sprintf(debugMsg,"Failed to broadcast UDP %d\n", rc);
			tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
			
		}
			FILE *fPtr = fopen("/home/pi/mhcv.txt", "a"); // /home/pi on Raspian
			dbgLen = sprintf(debugMsg,"Failed to broadcast UDP %d\n", rc);
			fwrite(debugMsg,dbgLen,1,fPtr);
			fclose(fPtr);
		return -2;
	}
	return 0;
}

/* Check if UDP listener is still active
 * restart UDP listener if stopped
*/
void checkUDP() {
	if (!udpUp) {
		int rc;
		int parameter;
		if ((rc = pthread_create(&threadUDPserv, NULL, udpServer, &parameter)) != 0) {
			// Write Log data
			// Pointer to log file
			FILE *fPtr = fopen("/home/pi/mhcv.txt", "a"); // /home/pi on Raspian
			dbgLen = sprintf(debugMsg,"initApp - UDP socket server thread creation failed with result %d\n", rc);
			fwrite(debugMsg,dbgLen,1,fPtr);
			fclose(fPtr);
		}
	}
}