#include "mhc.h"

/* Initialize TCP socket for listening
 * to debug messages
 * <return> <code>int</code>
 *		0 socket is created for listening
 *		-1 socket creation failed
 * 		-2 reuse of IP address failed
 *		-3 socket bind failed
 *		-4 socket listen failed
 */
int debugTcpInit() {
	// Get TCP socket
	if ((debugTcpSocket=socket(PF_INET, SOCK_STREAM, IPPROTO_TCP))==-1)
		return -1;

	// Set socket to reuse IP address
	int param = 1;
	if(setsockopt(debugTcpSocket, SOL_SOCKET, SO_REUSEADDR, &param, sizeof(int)) == -1)
		return -2;

	// Cleanup TCP socket structure
	memset((char *)&debugTcpSocketServer, 0, sizeof(debugTcpSocketServer));
	// Setup TCP socket
	debugTcpSocketServer.sin_family = AF_INET;
	debugTcpSocketServer.sin_port = htons(DEBUG_TCP_PORT);
	debugTcpSocketServer.sin_addr.s_addr = htonl(INADDR_ANY);
	
	// Bind TCP socket
	if (bind(debugTcpSocket, (struct sockaddr *)&debugTcpSocketServer, sizeof(debugTcpSocketServer)) == -1)
			return -3;
	// Listen to TCP socket
	if (listen(debugTcpSocket, TCP_QUEUE) == -1)
			return -4;
	return 0;
}

/* Close TCP socket receiver
 */
void debugTcpClose() {
	close(debugTcpSocket);
	tcpDebugUp = false;
}

/* Process received TCP packet
 * <return> <code>int</code>
 *		0 if packet could be processed
 *		-1 Empty packet received
 */
int debugTcpProcess() {
	/* File pointer for debug */
	FILE *fPtr;
#ifdef DO_DEBUG
	// Write Log data
	getSysTime(1);
	// Pointer to log file
	fPtr = fopen("/home/pi/mhcv.txt", "a"); // /home/pi on Raspian
	fwrite(sysTime,20,1,fPtr);
	dbgLen = sprintf(debugMsg,"debugTcpProcess: debugTcpProcess was called %s:%d\n\n", inet_ntoa(debugTcpSocketClient.sin_addr), ntohs(debugTcpSocketClient.sin_port)); 
	fwrite(debugMsg,dbgLen,1,fPtr);
	fclose(fPtr);
#endif

	int debugTcpRecvBytes = recv(debugTcpClientSocket, debugTcpBuf, BUFLEN, 0);
	if (debugTcpRecvBytes < 0) { // Receive failed
		// Write Log data
		getSysTime(1);
		// Pointer to log file
		fPtr = fopen("/home/pi/mhcv.txt", "a"); // /home/pi on Raspian
		fwrite(sysTime,20,1,fPtr);
		dbgLen = sprintf(debugMsg,"debugTcpProcess: Receive failed with error:%d\n\n", debugTcpRecvBytes); 
		fwrite(debugMsg,dbgLen,1,fPtr);
		fclose(fPtr);
	}

#ifdef DO_DEBUG
	// Write Log data
	getSysTime(1);
	// Pointer to log file
	fPtr = fopen("/home/pi/mhcv.txt", "a"); // /home/pi on Raspian
	fwrite(sysTime,20,1,fPtr);
	dbgLen = sprintf(debugMsg,"Received packet from %s:%d - Data: %s\n\n", inet_ntoa(debugTcpSocketClient.sin_addr), ntohs(debugTcpSocketClient.sin_port), debugTcpBuf); 
	fwrite(debugMsg,dbgLen,1,fPtr);
	fclose(fPtr);
#endif

	// TODO decide if we handle received data here or in the main thread
	// Signal to main thread that we have received data
	hasDebugTCPdata = true;
	memcpy((char *) &debugTcpBufTemp, (char *) & debugTcpBuf, debugTcpRecvBytes);
	debugTcpRecvBytesTemp = debugTcpRecvBytes;
	// Close the communication socket
	close(debugTcpClientSocket);

	// Cleanup buffer
	memset((char *) &debugTcpBuf, 0, BUFLEN);
	raise(SIGUSR2);
	return 0;
}

void *debugTcpServer(void *argument) {
	/* File pointer for debug */
	FILE *fPtr;
	// Setup TCP listener
	if ((resultValue = debugTcpInit()) != 0) {
		// Write Log data
		getSysTime(1);
		// Pointer to log file
		fPtr = fopen("/home/pi/mhcv.txt", "a"); // /home/pi on Raspian
		fwrite(sysTime,20,1,fPtr);
		dbgLen = sprintf(debugMsg,"debugTcpServer: TCP socket listener initialization failed with result %d\n", resultValue);
		fwrite(debugMsg,dbgLen,1,fPtr);
		fclose(fPtr);
		if (resultValue == -2) {
			debugTcpClose();
		}
		return (void *)&resultValue;
	} else {
		tcpDebugUp = true;
#ifdef DO_DEBUG
		// Write Log data
		getSysTime(1);
		// Pointer to log file
		fPtr = fopen("/home/pi/mhcv.txt", "a"); // /home/pi on Raspian
		fwrite(sysTime,20,1,fPtr);
		dbgLen = sprintf(debugMsg,"debugTcpServer: TCP socket listener initialized\n"); 
		fwrite(debugMsg,dbgLen,1,fPtr);
		fclose(fPtr);
#endif
	}
	
	int error = 0;
	while (error == 0) {
		// Check if there is an incoming TCP request
		debugTcpBuffClient = BUFLEN;
		debugTcpClientSocket = accept(debugTcpSocket, (struct sockaddr *)&debugTcpSocketClient, &debugTcpBuffClient);
		if (debugTcpClientSocket > 0) {
			resultValue = debugTcpProcess();
			if (resultValue != 0) {
				// Write Log data
				getSysTime(1);
				// Pointer to log file
				fPtr = fopen("/home/pi/mhcv.txt", "a"); // /home/pi on Raspian
				fwrite(sysTime,20,1,fPtr);
				dbgLen = sprintf(debugMsg,"debugTcpServer: TCP packet process failed with result %d\n", resultValue);
				fwrite(debugMsg,dbgLen,1,fPtr);
				fclose(fPtr);
			}
		}
	}
	return (void *)&resultValue;
}
