#include "mhc.h"

#define TIMEOUT 3000 // 3000 times 10ms = 30 seconds
int disc_finished = 0;
int conn_finished = 0;
int subscribed = 0;
int unsubscribed = 0;

// Instance for MQTT async connection
MQTTAsync mqttClient;

/* Handle lost connection to MQTT broker
 * <param> context
 *		Pointer to application context
 * <param> cause
 *		Pointer to reason the connection was lost
 */
void connlost(void *context, char *cause) {
	if (mqttDebugOn) {
		dbgLen = sprintf(debugMsg,"MQTT connection lost, cause: %s", cause);
		tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
	}
	mqttUp = false;
}

/* Handle received topic from MQTT broker
 * <param> context
 *		Pointer to application context
 * <param> topicName
 *		Pointer to topic name
 * <param> topicLen
 *		Size of topic payload
 * <param> message
 *		Pointer to received message
 * <return> <code>int</code>
 *		Returns always 1
 */
int msgarrvd(void *context, char *topicName, int topicLen, MQTTAsync_message *message) {
	// Save message
	memcpy(mqttBuf,message->payload,message->payloadlen);
	// Set flag for received message
	hasMqttCmd = true;

    MQTTAsync_freeMessage(&message);
    MQTTAsync_free(topicName);
	raise(SIGUSR2);
    return 1;
}

/* Handle result of publishing a topic to MQTT broker
 * <param> context
 *		Pointer to application context
 * <param> response
 *		Pointer to response from MQTT broker
 */
void onSend(void* context, MQTTAsync_successData* response)
{
//	if (mqttDebugOn) {
//		dbgLen = sprintf(debugMsg,"MQTT message with token value %d delivery confirmed", response->token);
//		tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
//	}
}

/* Handle result of disconnect from MQTT broker
 * <param> context
 *		Pointer to application context
 * <param> response
 *		Pointer to response from MQTT broker
 */
void onDisconnect(void* context, MQTTAsync_successData* response) {
	if (mqttDebugOn) {
		dbgLen = sprintf(debugMsg,"MQTT successful disconnected");
		tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
	}
	disc_finished = 1;
	mqttUp = false;
}

/* Handle result of topic subscription from MQTT broker
 * <param> context
 *		Pointer to application context
 * <param> response
 *		Pointer to response from MQTT broker
 */
void onSubscribe(void* context, MQTTAsync_successData* response) {
	if (mqttDebugOn) {
		dbgLen = sprintf(debugMsg,"MQTT subscribe succeed");
		tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
	}
	subscribed = 1;
}

/* Handle result of failed topic subscription from MQTT broker
 * <param> context
 *		Pointer to application context
 * <param> response
 *		Pointer to response from MQTT broker
 */
void onSubscribeFailure(void* context, MQTTAsync_failureData* response) {
	if (mqttDebugOn) {
		dbgLen = sprintf(debugMsg,"MQTT subscribe failed, rc %d", response ? response->code : 0);
		tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
	}
	subscribed = -1;
}

/* Handle result of topic unsubscription from MQTT broker
 * <param> context
 *		Pointer to application context
 * <param> response
 *		Pointer to response from MQTT broker
 */
void onUnSubscribe(void* context, MQTTAsync_successData* response) {
	if (mqttDebugOn) {
		dbgLen = sprintf(debugMsg,"MQTT unsubscribe succeeded"); 
		tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
	}
	unsubscribed = 1;
}

/* Handle result of failed topic unsubscription from MQTT broker
 * <param> context
 *		Pointer to application context
 * <param> response
 *		Pointer to response from MQTT broker
 */
void onUnSubscribeFailure(void* context, MQTTAsync_failureData* response) {
	if (mqttDebugOn) {
		dbgLen = sprintf(debugMsg,"MQTT unsubscribe failed, rc %d", response ? response->code : 0); 
		tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
	}
	unsubscribed = -1;
}

/* Handle result of failed connection to MQTT broker
 * <param> context
 *		Pointer to application context
 * <param> response
 *		Pointer to response from MQTT broker
 */
void onConnectFailure(void* context, MQTTAsync_failureData* response) {
	if (mqttDebugOn) {
		dbgLen = sprintf(debugMsg,"MQTT connect failed, rc %d", response ? response->code : 0); 
		tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
	}
	conn_finished = -1;
	mqttUp = false;
}

/* Handle result of connection to MQTT broker
 * <param> context
 *		Pointer to application context
 * <param> response
 *		Pointer to response from MQTT broker
 */
void onConnect(void* context, MQTTAsync_successData* response) {
	if (mqttDebugOn) {
		dbgLen = sprintf(debugMsg,"MQTT successful connection"); 
		tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
	}
	conn_finished = 1;
	mqttUp = true;

	// Publish device ID
	publishTopic("/DEV/RASPBERRY PI 3", USR_HOME, 1);
}

/* Initialize connection to MQTT broker
 * <return> <code>int</code>
 *		0 if successfull connected
 *		-1 if connection request failed
 *		-2 if connection request failed after 30 seconds timeout
 *		-3 if connection failed
 */
int mqttInit() {
	// Options for MQTT async connection
	MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;
	int rc;

	MQTTAsync_create(&mqttClient, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);

	MQTTAsync_setCallbacks(mqttClient, NULL, connlost, msgarrvd, NULL);

	conn_opts.keepAliveInterval = 20;
	conn_opts.cleansession = 1;
	conn_opts.onSuccess = onConnect;
	conn_opts.onFailure = onConnectFailure;
	conn_opts.context = mqttClient;
	conn_opts.password = CLIENTPW;
	conn_opts.username = CLIENTUSER;
	
	if ((rc = MQTTAsync_connect(mqttClient, &conn_opts)) != MQTTASYNC_SUCCESS) {
		if (mqttDebugOn) {
			dbgLen = sprintf(debugMsg,"MQTT failed to start connect, return code %d", rc); 
			tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
		}
		return -1;       
	}

	int timeout = 0;
	while (conn_finished == 0) {
		usleep(10000L); // sleep for 10000us = 10sec
		timeout++;
		if (timeout == TIMEOUT) { // stop waiting after 30 seconds
			return -2;
		}
	}
	
	if (conn_finished == -1) {
		return -3;
	}
	
	return 0;
}

/* Subscribe to a topic on MQTT broker
 * <param> topic
 *		Pointer to char * topic
 * <param> qos
 *		Requested QOS level
 * <return> <code>int</code>
 *		0 if topic subscription request was send
 *		-1 if topic subscription request failed
 *		-2 if topic subscription request failed after 30 seconds timeout
 *		-1 if topic subscription failed
 */
int subscribeTopic(char * topic, int qos) {
	MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
	int rc;

	if (mqttDebugOn) {
		dbgLen = sprintf(debugMsg,"MQTT subscribing to topic %s for client %s using QoS%d", topic, CLIENTID, qos); 
		tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
	}
	opts.onSuccess = onSubscribe;
	opts.onFailure = onSubscribeFailure;
	opts.context = mqttClient;

	if ((rc = MQTTAsync_subscribe(mqttClient, topic, qos, &opts)) != MQTTASYNC_SUCCESS) {
		if (mqttDebugOn) {
			dbgLen = sprintf(debugMsg,"MQTT failed to start subscribe, return code %d", rc); 
			tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
		}
		return -1;       
	}
	
	int timeout = 0;
	while (subscribed == 0) {
		usleep(10000L); // sleep for 10000us = 10ms = 0.010s
		timeout++;
		if (timeout == TIMEOUT) { // stop waiting after 30 seconds
			return -2;
		}
	}
	
	if (subscribed == -1) {
		return -3;
	}
	
	return 0;
}

/* Unsubscribe from a topic on MQTT broker
 * <param> topic
 *		Pointer to char * topic
 * <return> <code>int</code>
 *		0 if topic unsubscription request was send
 *		-1 if topic unsubscription request failed
 *      -2 if topic unsubscription failed after 30 seconds timeout
 */
int unsubscribeTopic(char * topic) {
	if (subscribed == 1) {
		MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;

		int rc;

		opts.onSuccess = onUnSubscribe;
		opts.onFailure = onUnSubscribeFailure;
		opts.context = mqttClient;
		
		if ((rc = MQTTAsync_unsubscribe(mqttClient, topic, &opts)) != MQTTASYNC_SUCCESS) {
			if (mqttDebugOn) {
				dbgLen = sprintf(debugMsg,"MQTT failed to start unsubscribe, return code %d", rc); 
				tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
			}
			exit(-1);       
		}

		int timeout = 0;
		while (!unsubscribed) {
			usleep(10000L); // sleep for 10000us = 10sec
			timeout++;
			if (timeout == TIMEOUT) { // stop waiting after 30 seconds
				return -2;
			}
		}
	}
	return 0;
}

/* Publish a topic to MQTT broker
 * <param> topic
 *		Pointer to char * topic
 * <param> payload
 *		Pointer to char * payload
 * <param> retained
 *		1 for retained topic
 * 		0 for none retained topic
 * <return> <code>int</code>
 *		0 if send request was successful
 *		-1 if send request failed
 */
int publishTopic(char * topic, char * payload, int retained) {
	MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
	MQTTAsync_message pubmsg = MQTTAsync_message_initializer;
	int rc;

	opts.onSuccess = onSend;
	opts.context = mqttClient;

	pubmsg.payload = payload;
	pubmsg.payloadlen = strlen(payload);
	pubmsg.qos = 0;
	pubmsg.retained = retained;

	if ((rc = MQTTAsync_sendMessage(mqttClient, topic, &pubmsg, &opts)) != MQTTASYNC_SUCCESS) {
		if (mqttDebugOn) {
			dbgLen = sprintf(debugMsg,"MQTT failed to start sendMessage, return code %d", rc); 
			tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
		}
		return -1;
	}
	
	return 0;
}

/* Close connection to MQTT broker
 * <return> <code>int</code>
 *		0 if successfull disconnected
 *		-1 if disconnection request failed
 *		-2 if disconnection failed after 30 seconds timeout
 */
int mqttClose() {
	MQTTAsync_disconnectOptions disc_opts = MQTTAsync_disconnectOptions_initializer;
	int rc;

	disc_opts.onSuccess = onDisconnect;

	if ((rc = MQTTAsync_disconnect(mqttClient, &disc_opts)) != MQTTASYNC_SUCCESS) {
		if (mqttDebugOn) {
			dbgLen = sprintf(debugMsg,"MQTT failed to start disconnect, return code %d", rc); 
			tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
		}
		mqttUp = false;
		return -1;       
	}
	
	int timeout = 0;
	while (disc_finished == 0) {
		usleep(10000L); // sleep for 10000us = 10sec
		timeout++;
		if (timeout == TIMEOUT) { // stop waiting after 60 seconds
			mqttUp = false;
			return -2;
		}
	}

	mqttUp = false;
	MQTTAsync_destroy(&mqttClient);
	return 0;
}