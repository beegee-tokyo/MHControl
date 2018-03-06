#ifndef globals_h
#define globals_h

// Socket structures for UDP receiver
extern struct sockaddr_in udpSocketMe, udpSocketOther;
// UDP socket
extern int udpSocket;
// UDP receiver buffer
extern char udpBuf[];

// Socket structure for UDP client
extern struct sockaddr_in udpSocketOut;

// Socket structures for TCP receiver (commands and debug)
extern struct sockaddr_in tcpSocketServer, tcpSocketClient, debugTcpSocketServer, debugTcpSocketClient;
// TCP incoming socket descriptor
extern int tcpSocket, debugTcpSocket;
// TCP outgoing socket descriptor
extern int tcpClientSocket, debugTcpClientSocket;
// TCP number of receiver buffer size / received bytes
extern int tcpBuffClient, debugTcpBuffClient;
// TCP receiver buffer
extern char tcpBuf[], debugTcpBuf[];
// Temporary TCP receiver buffer
extern char tcpBufTemp[], debugTcpBufTemp[];
// Number of bytes received
extern int debugTcpRecvBytesTemp;

// Socket structure for TCP client
extern struct sockaddr_in tcpSocketOut;

// Thread for TCP server
extern pthread_t threadTCPserv, threadDebugserv, threadUDPserv, threadCamStream, threadMoveVideoStream;
// Result of thread start
extern int resultValue;
// Flag if TCP data has been received
extern bool hasTCPdata, debugHasTCPdata;
// Flags for camera function
extern bool streamCam;

// Instance for MQTT async connection
extern MQTTAsync mqttClient;
// MQTT receiver buffer
extern char mqttBuf[];

// Flag for UDP socket active
extern bool udpUp;
// Flag for TCP socket active
extern bool tcpUp, tcpDebugUp;
// Flag for MQTT connection active
extern bool mqttUp;

// Flag for general debugging
bool genDebugOn;
// Flag for camera debugging
bool camDebugOn;
// Flag for DHT11 debugging
bool dht11DebugOn;
// Flag for MQTT debugging
bool mqttDebugOn;
// Flag for UDP debugging
bool udpDebugOn;
// Buffer for debug message
extern char debugMsg[];
// Length of debug message
extern int dbgLen;

// Devices predefined IP addresses
/** IP address of spMonitor module */
extern char ipSPM[];
/** IP address of security front */
extern char ipSecFront[];
/** IP address of office aircon */
extern char ipAC1[];
/** IP address of living room aircon */
extern char ipAC2[];
/** IP address of security back */
extern char ipSecBack[];
/** IP address of camera front */
extern char ipCam1[];
/** IP address of CCTV front */
extern char ipCam2[];
/** IP address of seccam front */
//extern char ipCam3[];
/** IP address of bedroom light */
extern char ipLB1[];
/** IP address of bedroom light */
extern char ipLY1[];
/** IP address of doorbell */
extern char ipDB1[];
/** Broadcast IP address */
extern char ipUDPBroadcast[];
/** IP address debugging tablet */
extern char ipDBG[];

// Received status as JSON
extern JSON_Value *fd1JSON;
extern JSON_Value *ca1JSON;
extern JSON_Value *db1JSON;
extern JSON_Value *lb1JSON;
extern JSON_Value *ly1JSON;
extern JSON_Value *spmJSON;
extern JSON_Value *sf1JSON;
extern JSON_Value *sb1JSON;
extern JSON_Value *weiJSON;
extern JSON_Value *weoJSON;
extern JSON_Value *mhcJSON;
extern JSON_Value *vc2JSON;

// Saved IP addresses as JSON
extern JSON_Value *configJSON;

// Flags if status is available
extern bool hasFD1JSON;
extern bool hasCA1JSON;
extern bool hasDB1JSON;
extern bool hasLB1JSON;
extern bool hasLY1JSON;
extern bool hasSPMJSON;
extern bool hasSF1JSON;
extern bool hasSB1JSON;
extern bool hasWEIJSON;
extern bool hasWEOJSON;
extern bool hasMHCJSON;
extern bool hasVC2JSON;

// Date values used for organisation of videos into daily folders
extern int startYear;
extern int startMonth;
extern int startDay;

// Flag if command over MQTT has arrived
extern bool hasMqttCmd;
// Flag for device's clock update request
extern bool doNTP;
// Counter for camera status update time
extern int camStatusCnt;

// Signal structure for interval timer
extern struct sigaction saMyTimer;
// Timer structure for interval timer
extern struct itimerval myIntTimer;
// Counter for device's clock update time
extern int ntpCnt;
// Current time as command for the devices to update their clock
extern char sysTime[];

#endif
