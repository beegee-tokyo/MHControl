#include "mhc.h"

/*
 * Publish device status to MQTT broker
 */
void mqttPublisher() {
	if (mqttDebugOn) {
		dbgLen = sprintf(debugMsg,"MQTT publish triggered\n");
		tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
	}
	// String for publishing output
	char *serialized_string = NULL;

	// Publish available data
	if(hasFD1JSON) {
		if ((serialized_string = json_serialize_to_string(fd1JSON)) != NULL) {
			publishTopic("/AC1", serialized_string, 0);
			if (mqttDebugOn) {
				dbgLen = sprintf(debugMsg,"Publish FD1: %s\n",serialized_string);
				tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
			}
			json_free_serialized_string(serialized_string);
		} else {
			if (mqttDebugOn) {
				dbgLen = sprintf(debugMsg,"Problem with fd1JSON!\n");
				tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
			}
		}
		hasFD1JSON = false;
	}
	if(hasCA1JSON) {
		if ((serialized_string = json_serialize_to_string(ca1JSON)) != NULL) {
			publishTopic("/AC2", serialized_string, 0);
			if (mqttDebugOn) {
				dbgLen = sprintf(debugMsg,"Publish CA1: %s\n",serialized_string);
				tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
			}
			json_free_serialized_string(serialized_string);
		} else {
			if (mqttDebugOn) {
				dbgLen = sprintf(debugMsg,"Problem with ca1JSON!\n");
				tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
			}
		}
		hasCA1JSON = false;
	}
	if(hasLB1JSON) {
		if ((serialized_string = json_serialize_to_string(lb1JSON)) != NULL) {
			publishTopic("/LB1", serialized_string, 0);
			if (mqttDebugOn) {
				dbgLen = sprintf(debugMsg,"Publish LB1: %s\n",serialized_string);
				tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
			}
			json_free_serialized_string(serialized_string);
		} else {
			if (mqttDebugOn) {
				dbgLen = sprintf(debugMsg,"Problem with lb1JSON!\n");
				tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
			}
		}
		hasLB1JSON = false;
	}
	if(hasLY1JSON) {
		if ((serialized_string = json_serialize_to_string(ly1JSON)) != NULL) {
			publishTopic("/LY1", serialized_string, 0);
			if (mqttDebugOn) {
				dbgLen = sprintf(debugMsg,"Publish LY1: %s\n",serialized_string);
				tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
			}
			json_free_serialized_string(serialized_string);
		} else {
			if (mqttDebugOn) {
				dbgLen = sprintf(debugMsg,"Problem with ly1JSON!\n");
				tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
			}
		}
		hasLY1JSON = false;
	}
	if(hasSPMJSON) {
		if ((serialized_string = json_serialize_to_string(spmJSON)) != NULL) {
			publishTopic("/SPM", serialized_string, 0);
			if (mqttDebugOn) {
				dbgLen = sprintf(debugMsg,"Publish SPM: %s\n",serialized_string);
				tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
			}
			json_free_serialized_string(serialized_string);
		} else {
			if (mqttDebugOn) {
				dbgLen = sprintf(debugMsg,"Problem with spmJSON!\n");
				tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
			}
		}
		hasSPMJSON = false;
	}
	if(hasSF1JSON) {
		if ((serialized_string = json_serialize_to_string(sf1JSON)) != NULL) {
			publishTopic("/SEF", serialized_string, 0);
			if (mqttDebugOn) {
				dbgLen = sprintf(debugMsg,"Publish SEF: %s\n",serialized_string);
				tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
			}
			json_free_serialized_string(serialized_string);
		} else {
			if (mqttDebugOn) {
				dbgLen = sprintf(debugMsg,"Problem with sf1JSON!\n");
				tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
			}
		}
		hasSF1JSON = false;
	}
	if(hasSB1JSON) {
		if ((serialized_string = json_serialize_to_string(sb1JSON)) != NULL) {
			publishTopic("/SEB", serialized_string, 0);
			if (mqttDebugOn) {
				dbgLen = sprintf(debugMsg,"Publish SEB: %s\n",serialized_string);
				tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
			}
			json_free_serialized_string(serialized_string);
		} else {
			if (mqttDebugOn) {
				dbgLen = sprintf(debugMsg,"Problem with sb1JSON!\n");
				tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
			}
		}
		hasSB1JSON = false;
	}
	if(hasWEIJSON) {
		if ((serialized_string = json_serialize_to_string(weiJSON)) != NULL) {
			publishTopic("/WEI", serialized_string, 0);
			if (mqttDebugOn) {
				dbgLen = sprintf(debugMsg,"Publish WEI: %s\n",serialized_string);
				tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
			}
			json_free_serialized_string(serialized_string);
		} else {
			if (mqttDebugOn) {
				dbgLen = sprintf(debugMsg,"Problem with weiJSON!\n");
				tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
			}
		}
		hasWEIJSON = false;
	}
	if(hasWEOJSON) {
		if ((serialized_string = json_serialize_to_string(weoJSON)) != NULL) {
			publishTopic("/WEO", serialized_string, 0);
			if (mqttDebugOn) {
				dbgLen = sprintf(debugMsg,"Publish WEO: %s\n",serialized_string);
				tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
			}
			json_free_serialized_string(serialized_string);
		} else {
			if (mqttDebugOn) {
				dbgLen = sprintf(debugMsg,"Problem with weoJSON!\n");
				tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
			}
		}
		hasWEOJSON = false;
	}
	if(hasCM1JSON) {
		if ((serialized_string = json_serialize_to_string(cm1JSON)) != NULL) {
			publishTopic("/CM1", serialized_string, 2);
			if (mqttDebugOn) {
				dbgLen = sprintf(debugMsg,"Publish CM1: %s\n",serialized_string);
				tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
			}
			json_free_serialized_string(serialized_string);
		} else {
			if (mqttDebugOn) {
				dbgLen = sprintf(debugMsg,"Problem with cm1JSON!\n");
				tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
			}
		}
		hasCM1JSON = false;
	}
	if(hasMHCJSON) {
		if ((serialized_string = json_serialize_to_string(mhcJSON)) != NULL) {
			publishTopic("/MHC", serialized_string, 2);
			if (mqttDebugOn) {
				dbgLen = sprintf(debugMsg,"Publish MHC: %s\n",serialized_string);
				tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
			}
			json_free_serialized_string(serialized_string);
		} else {
			if (mqttDebugOn) {
				dbgLen = sprintf(debugMsg,"Problem with mhcJSON!\n");
				tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
			}
		}
		hasMHCJSON = false;
	}
	if(hasVC2JSON) {
		if ((serialized_string = json_serialize_to_string(vc2JSON)) != NULL) {
			publishTopic("/VC2", serialized_string, 2);
			if (mqttDebugOn) {
				dbgLen = sprintf(debugMsg,"Publish VC2: %s\n",serialized_string);
				tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
			}
			json_free_serialized_string(serialized_string);
		} else {
			if (mqttDebugOn) {
				dbgLen = sprintf(debugMsg,"Problem with vc2JSON!\n");
				tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
			}
		}
		hasVC2JSON = false;
	}
	if(hasDB1JSON) {
		if ((serialized_string = json_serialize_to_string(db1JSON)) != NULL) {
			publishTopic("/DB1", serialized_string, 0);
			if (mqttDebugOn) {
				dbgLen = sprintf(debugMsg,"Publish DB1: %s\n",serialized_string);
				tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
			}
			json_free_serialized_string(serialized_string);
		} else {
			if (mqttDebugOn) {
				dbgLen = sprintf(debugMsg,"Problem with db1JSON!\n");
				tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
			}
		}
		hasDB1JSON = false;
	}
}

/*
 * Process received command from MQTT broker
 */
void mqttProcess() {
	// Received data as JSON
	JSON_Value *jsonPacket;
	// Root of JSON data
	JSON_Object *root_object;
	// Device ID as string
	const char * device;
	// Device ID as number
	int deviceID = 0;
	// Device IP address
	char devIP[20];
	// Command
	const char * devCmd;

	if ((jsonPacket = json_parse_string(mqttBuf)) != NULL) {
		root_object = json_value_get_object(jsonPacket);
		if ((device = json_object_get_string(root_object,"ip")) != 0) {
			if ((devCmd = json_object_get_string(root_object,"cm")) != 0) {
				deviceID = 0;
				for (int j=0; j<strlen(device); j++) {
					deviceID += device[j]+(j*16);
				}
				if (mqttDebugOn) {
					dbgLen = sprintf(debugMsg,"Found device %s - %d\n", device, deviceID);
					tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
				}

				switch (deviceID) {
					case devFD1:
						strcpy(devIP,ipAC1);
						break;
					case devCA1:
						strcpy(devIP,ipAC2);
						break;
					case devSF1:
						strcpy(devIP,ipSecFront);
						break;
					case devSB1:
						strcpy(devIP,ipSecBack);
						break;
					case devSPM:
						strcpy(devIP,ipSPM);
						break;
					case devLB1:
						strcpy(devIP,ipLB1);
						break;
					case devLY1:
						strcpy(devIP,ipLY1);
						break;
					case devCM1:
						strcpy(devIP,ipCam1);
						break;
					default:
						strcpy(devIP,"-");
						break;
				}

				if (strncmp(devIP,"-",1) != 0) {
					// Forward message to device if valid
					if (tcpSend((char *)devCmd, devIP, strlen(devCmd)) == 0) {
						hasMqttCmd = false;
					}
				} else {
					if (mqttDebugOn) {
						dbgLen = sprintf(debugMsg,"No matching device found\n");
						tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
					}
					hasMqttCmd = false;
				}
			} else {
				if (mqttDebugOn) {
					dbgLen = sprintf(debugMsg,"No command found\n");
					tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
				}
				hasMqttCmd = false;
				// Cleanup buffer and temp JSON object
				json_value_free(jsonPacket);
			}
		} else {
			if (mqttDebugOn) {
				dbgLen = sprintf(debugMsg,"No device ID found\n");
				tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
			}
			hasMqttCmd = false;
			// Cleanup buffer and temp JSON object
			json_value_free(jsonPacket);
		}
	} else {
		if (mqttDebugOn) {
			dbgLen = sprintf(debugMsg,"No valid JSON found\n");
			tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
		}
		// Cleanup buffer and temp JSON object
		json_value_free(jsonPacket);
	}
}

/*
 * Check MQTT connection and try to reconnect if necessary
 */
void checkMQTT() {
	// Return codes from function calls
	int rc;
//	if (!mqttUp) { // not connected or connection lost
	if (!MQTTAsync_isConnected(mqttClient)) { // not connected or connection lost
		// Reconnect to MQTT broker
		if ((rc = mqttInit()) != 0) {
			if (mqttDebugOn) {
				dbgLen = sprintf(debugMsg,"MQTT connection reconnection failed with result %d\n", rc);
				tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
			}
			
		} else {
			mqttUp = true;

			// Subscribe to topic(s)
			if (mqttUp && ((rc = subscribeTopic("/CMD", 1)) != 0)) {
				if (mqttDebugOn) {
					dbgLen = sprintf(debugMsg,"MQTT topic subscription failed with result %d\n", rc);
					tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
				}
				mqttClose();
				mqttUp = false;
			}
		}
	}
 }
