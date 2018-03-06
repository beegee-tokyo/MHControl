#ifndef declarations_h
#define declarations_h

// Receiver/Sender max buffer length
#define BUFLEN 256
// TCP max participants
#define TCP_QUEUE 10

// Socket structures for UDP receiver
struct sockaddr_in udpSocketMe, udpSocketOther;
// UDP socket descriptor
int udpSocket;
// UDP receiver buffer
char udpBuf[BUFLEN];

// Socket structure for UDP client
struct sockaddr_in udpSocketOut;

// Socket structures for TCP receiver (commands and debug)
struct sockaddr_in tcpSocketServer, tcpSocketClient, debugTcpSocketServer, debugTcpSocketClient;
// TCP incoming socket descriptor
int tcpSocket, debugTcpSocket;
// TCP outgoing socket descriptor
int tcpClientSocket, debugTcpClientSocket;
// TCP number of receiver buffer size / received bytes
int tcpBuffClient, debugTcpBuffClient;
// TCP receiver buffer
char tcpBuf[BUFLEN], debugTcpBuf[BUFLEN];
// Temporary TCP receiver buffer
char tcpBufTemp[BUFLEN], debugTcpBufTemp[BUFLEN];
// Number of bytes received
int debugTcpRecvBytesTemp;

// Socket structure for TCP client
struct sockaddr_in tcpSocketOut;

// Thread for TCP server
pthread_t threadTCPserv, threadDebugserv, threadUDPserv, threadCamStream, threadMoveVideoStream;
// Flag if TCP data has been received
bool hasTCPdata, hasDebugTCPdata;
// Flags for camera function
bool streamCam;
// Result of thread start
int resultValue;

// MQTT receiver buffer
char mqttBuf[BUFLEN];

// Flag for UDP socket active
bool udpUp;
// Flag for TCP socket active
bool tcpUp, tcpDebugUp;
// Flag for MQTT connection active
bool mqttUp;

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
char debugMsg[2048];
// Length of debug message
int dbgLen;

// Devices predefined IP addresses
/** IP address of spMonitor module */
char ipSPM[16];// = "192.168.0.50";
/** IP address of security front */
char ipSecFront[16];// = "192.168.0.52";
/** IP address of office aircon */
char ipAC1[16];// = "192.168.0.56";
/** IP address of living room aircon */
char ipAC2[16];// = "192.168.0.57";
/** IP address of security back */
char ipSecBack[16];// = "192.168.0.53";
/** IP address of camera front */
char ipCam1[16];// = "192.168.0.54";
/** IP address of CCTV front */
char ipCam2[16];// = "192.168.0.55";
/** IP address of seccam front */
//char ipCam3[16];// = "192.168.0.147";
/** IP address of bedroom light */
char ipLB1[16];// = "192.168.0.58";
/** IP address of bedroom light */
char ipLY1[16];// = "192.168.0.59";
/** IP address of bedroom light */
char ipDB1[16];// = "192.168.0.104";
/** Broadcast IP address */
char ipUDPBroadcast[16];// = "192.168.0.255";
/** IP address debugging tablet */
char ipDBG[16];// = "192.168.0.10";

// Device ID's as numbers
#define devFD1 299 // "fd1"
#define devCA1 293 // "ca1"
#define devDB1 295 // "db1"
#define devLB1 303 // "lb1"
#define devCM1 305 // "cm1"
#define devSPM 384 // "spm"
#define devSF1 314 // "sf1"
#define devSB1 310 // "sb1"
#define devLY1 326 // "ly1"
#define devMHC 360 // "mhc"
#define devWEI 373 // "wei" (this device)

// Received status as JSON
JSON_Value *fd1JSON;
JSON_Value *ca1JSON;
JSON_Value *db1JSON;
JSON_Value *lb1JSON;
JSON_Value *ly1JSON;
JSON_Value *spmJSON;
JSON_Value *sf1JSON;
JSON_Value *sb1JSON;
JSON_Value *weiJSON;
JSON_Value *weoJSON;
JSON_Value *cm1JSON;
JSON_Value *mhcJSON;
JSON_Value *vc2JSON;
JSON_Value *newJSON;

// Saved IP addresses as JSON
JSON_Value *configJSON;

// Flags if status is available
bool hasFD1JSON;
bool hasCA1JSON;
bool hasDB1JSON;
bool hasLB1JSON;
bool hasLY1JSON;
bool hasSPMJSON;
bool hasSF1JSON;
bool hasSB1JSON;
bool hasWEIJSON;
bool hasWEOJSON;
bool hasCM1JSON;
bool hasMHCJSON;
bool hasVC2JSON;

// Flag if command over MQTT has arrived
bool hasMqttCmd;

// Signal structure for interval timer
struct sigaction saMyTimer;
// Timer structure for interval timer
struct itimerval myIntTimer;

// Date values used for organisation of videos into daily folders
int startYear;
int startMonth;
int startDay;

// Counter for device's clock update time
int ntpCnt;
// Counter for camera status update time
int camStatusCnt;
// Current time as command for the devices to update their clock ==> y=YYYY,MM,DD,HH,mm,ss
char sysTime[22];

// Values read from temperature sensor
float tempInside;
float humidInside;
float heatInside;

#endif

