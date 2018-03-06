#include "mhc.h"

/* Initialize TCP socket for listening
 * to commands
 * <return> <code>int</code>
 *		0 socket is created for listening
 *		-1 socket creation failed
 * 		-2 reuse of IP address failed
 *		-3 socket bind failed
 *		-4 socket listen failed
 */
int tcpInit() {
	// Get TCP socket
	if ((tcpSocket=socket(PF_INET, SOCK_STREAM, IPPROTO_TCP))==-1)
		return -1;

	// Set socket to reuse IP address
	int param = 1;
	if(setsockopt(tcpSocket, SOL_SOCKET, SO_REUSEADDR, &param, sizeof(int)) == -1)
		return -2;

	// Cleanup TCP socket structure
	memset((char *)&tcpSocketServer, 0, sizeof(tcpSocketServer));
	// Setup TCP socket
	tcpSocketServer.sin_family = AF_INET;
	tcpSocketServer.sin_port = htons(TCP_PORT);
	tcpSocketServer.sin_addr.s_addr = htonl(INADDR_ANY);
	
	// Bind TCP socket
	if (bind(tcpSocket, (struct sockaddr *)&tcpSocketServer, sizeof(tcpSocketServer)) == -1)
			return -3;
	// Listen to TCP socket
	if (listen(tcpSocket, TCP_QUEUE) == -1)
			return -4;
	return 0;
}

/* Close TCP socket receiver
 */
void tcpClose() {
	close(tcpSocket);
	tcpUp = false;
}

/* Process received TCP packet
 * <return> <code>int</code>
 *		0 if packet could be processed
 *		-1 Empty packet received
 */
int tcpProcess() {
#ifdef DO_DEBUG
	printf("tcpProcess was called %s:%d\n\n", inet_ntoa(tcpSocketClient.sin_addr), ntohs(tcpSocketClient.sin_port)); 
#endif

	int tcpRecvBytes = recv(tcpClientSocket, tcpBuf, BUFLEN, 0);
	if (tcpRecvBytes < 0) { // Receive failed
#ifdef DO_DEBUG
		printf("Receive failed with error:%d\n\n", tcpRecvBytes); 
#endif
	}

#ifdef DO_DEBUG
		printf("Received packet from %s:%d - Data: %s\n\n", inet_ntoa(tcpSocketClient.sin_addr), ntohs(tcpSocketClient.sin_port), tcpBuf); 
#endif

	// TODO decide if we handle received data here or in the signal_handler thread
	// Signal to signal_handler thread that we have received data
	hasTCPdata = true;
	memcpy((char *) &tcpBufTemp, (char *) & tcpBuf, tcpRecvBytes);
	
	// Close the communication socket
	close(tcpClientSocket);

	// Cleanup buffer
	memset((char *) &tcpBuf, 0, BUFLEN);
	raise(SIGUSR2);
	return 0;
}

void *tcpServer(void *argument) {
	// Setup TCP listener
	if ((resultValue = tcpInit()) != 0) {
#ifdef DO_DEBUG
		printf("TCP socket listener initialization failed with result %d\n", resultValue);
#endif
		if (resultValue == -2) {
			tcpClose();
		}
		return (void *)&resultValue;
	} else {
		tcpUp = true;
#ifdef DO_DEBUG
		printf("TCP socket listener initialized\n"); 
#endif
	}
	
	int error = 0;
	while (error == 0) {
		// Check if there is an incoming TCP request
		tcpBuffClient = BUFLEN;
		tcpClientSocket = accept(tcpSocket, (struct sockaddr *)&tcpSocketClient, &tcpBuffClient);
		if (tcpClientSocket > 0) {
			resultValue = tcpProcess();
#ifdef DO_DEBUG
			if (resultValue != 0) {
				printf("TCP packet process failed with result %d\n", resultValue);
			}
#endif
		}
	}
	return (void *)&resultValue;
}

/* Send command to devices
 * over TCP socket
 * <return> <code>int</code>
 *		0 message sent to device
 *		-1 socket creation failed
 *		-2 connect to server failed
 *		-3 send message failed
 */
int tcpSend(char *devCmd, char *devIP, int cmdLen) {
	tcpSendToPort(devCmd, devIP, cmdLen, TCP_PORT);
}

/* Send command to devices
 * over TCP socket
 * <return> <code>int</code>
 *		0 message sent to device
 *		-1 socket creation failed
 *		-2 connect to server failed
 *		-3 send message failed
 */
int tcpSendToPort(char *devCmd, char *devIP, int cmdLen, int tcpPort) {
	// Result from function calls
	int rc;
	// TCP outgoing socket descriptor
	int tcpOutSocket;
	// File pointer for debug
	FILE *fPtr;
#ifdef DO_DEBUG
		// Write Log data
		getSysTime(1);
		// Pointer to log file
		fPtr = fopen("/home/pi/mhcv.txt", "a"); // /home/pi on Raspian
		fwrite(sysTime,20,1,fPtr);
		dbgLen = sprintf(debugMsg,"tcpSend %s\n", devCmd);
		fwrite(debugMsg,dbgLen,1,fPtr);
		fclose(fPtr);
#endif

#ifdef DO_DEBUG
		// Write Log data
		getSysTime(1);
		// Pointer to log file
		fPtr = fopen("/home/pi/mhcv.txt", "a"); // /home/pi on Raspian
		fwrite(sysTime,20,1,fPtr);
		dbgLen = sprintf(debugMsg,"Init TCP outgoing socket\n");
		fwrite(debugMsg,dbgLen,1,fPtr);
		fclose(fPtr);
#endif
	// Get TCP socket
	if ((tcpOutSocket=socket(PF_INET, SOCK_STREAM, IPPROTO_TCP))==-1) {
		// Write Log data
		getSysTime(1);
		// Pointer to log file
		fPtr = fopen("/home/pi/mhcv.txt", "a"); // /home/pi on Raspian
		fwrite(sysTime,20,1,fPtr);
		dbgLen = sprintf(debugMsg,"tcpSendToPort: Could not create outgoing socket\n");
		fwrite(debugMsg,dbgLen,1,fPtr);
		fclose(fPtr);
		return -1;
	}

	// Setup timeouts for sending
	struct timeval timeout;      
    timeout.tv_sec = 60;
    timeout.tv_usec = 0;

    if (setsockopt (tcpOutSocket, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout,
                sizeof(timeout)) < 0) {
		// Write Log data
		getSysTime(1);
		// Pointer to log file
		fPtr = fopen("/home/pi/mhcv.txt", "a"); // /home/pi on Raspian
		fwrite(sysTime,20,1,fPtr);
		dbgLen = sprintf(debugMsg,"tcpSendToPort: Could not setup timouts!\n");
		fwrite(debugMsg,dbgLen,1,fPtr);
		fclose(fPtr);
	}

#ifdef DO_DEBUG
		printf("Connecting to device on address %s\n", devIP);
#endif
	// Cleanup TCP socket structure
	memset((char *) &tcpSocketOut, 0, sizeof(tcpSocketOut));
	// Setup TCP socket
	tcpSocketOut.sin_family = AF_INET;
	tcpSocketOut.sin_port = htons(tcpPort);
	tcpSocketOut.sin_addr.s_addr = inet_addr(devIP);
	// Connect TCP socket
	if (connect(tcpOutSocket, (struct sockaddr *)&tcpSocketOut, sizeof(tcpSocketOut)) == -1) {
		// Write Log data
		getSysTime(1);
		// Pointer to log file
		fPtr = fopen("/home/pi/mhcv.txt", "a"); // /home/pi on Raspian
		fwrite(sysTime,20,1,fPtr);
		dbgLen = sprintf(debugMsg,"tcpSendToPort: Could not connect to device on address %s\n", devIP);
		fwrite(debugMsg,dbgLen,1,fPtr);
		fclose(fPtr);
		return -2;
	}
	// Send data to server
#ifdef DO_DEBUG
		// Write Log data
		getSysTime(1);
		// Pointer to log file
		fPtr = fopen("/home/pi/mhcv.txt", "a"); // /home/pi on Raspian
		fwrite(sysTime,20,1,fPtr);
		dbgLen = sprintf(debugMsg,"tcpSendToPort: Sending %s with length %d\n", devCmd, cmdLen);
		fwrite(debugMsg,dbgLen,1,fPtr);
		fclose(fPtr);
#endif
	if ((rc = send(tcpOutSocket, devCmd, cmdLen, 0)) == -1) {
		// Write Log data
		getSysTime(1);
		// Pointer to log file
		fPtr = fopen("/home/pi/mhcv.txt", "a"); // /home/pi on Raspian
		fwrite(sysTime,20,1,fPtr);
		dbgLen = sprintf(debugMsg,"tcpSendToPort: Failed to send to device. Error code %d\n", rc);
		fwrite(debugMsg,dbgLen,1,fPtr);
		fclose(fPtr);
		return -3;
	}
	close(tcpOutSocket);
	return 0;
}

/* Check if TCP server is still active
 * restart TCP server if stopped
*/
void checkTCP() {
	if (!tcpUp) {
		int rc;
		int parameter;
		if ((rc = pthread_create(&threadTCPserv, NULL, tcpServer, &parameter)) != 0) {
			// Write Log data
			// Pointer to log file
			FILE *fPtr = fopen("/home/pi/mhcv.txt", "a"); // /home/pi on Raspian
			dbgLen = sprintf(debugMsg,"initApp - TCP socket server thread creation failed with result %d\n", rc);
			fwrite(debugMsg,dbgLen,1,fPtr);
			fclose(fPtr);
		}
	}
}