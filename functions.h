// Main functions
int main(int argc, char** argv);
void requestAllStatus();
void initApp(bool fullInit);

// UDP receiver functions
int udpInit();
void udpClose();
int udpProcess(int udpRecvBytes, char *senderIP);
void *udpServer(void *argument);
void checkUDP();

// UDP broadcast functions
int udpSend(char *weiStatus, int weiStatusLen);

// TCP server functions
int tcpInit();
void tcpClose();
int tcpProcess();
void *tcpServer(void *argument);
int debugTcpInit();
void debugTcpClose();
int debugTcpProcess();
void *debugTcpServer(void *argument);
void checkTCP();

// TCP client functions
int tcpSend(char *devCmd, char *devIP, int cmdLen);
int tcpSendToPort(char *devCmd, char *devIP, int cmdLen, int tcpPort);

// MQTT functions
int mqttInit();
int mqttClose();
int subscribeTopic(char * topic, int qos);
int unsubscribeTopic(char * topic);
int publishTopic(char * topic, char * payload, int qos);
void mqttPublisher();
void mqttProcess();
void checkMQTT();

// Camera functions
void *camStreamThread(void *argument);
 
// Temperature sensor functions
void dht11Read();
bool DHT_read();
uint32_t DHT_expectPulse(bool level);
float calcHeatIndex(float floatTemp, float floatHumid);
float convertCtoF(float c);
float convertFtoC(float f);

// Signal handler functions
void signalHandler( int signum );

// Time functions
void updateDeviceTime();
void getSysTime(int isCmd);
void lunchReboot();
void rebootRouter();

// File & directory functions
int stringcmp(const void *a,const void *b);
void cleanDir();
void moveVideos();
void *moveVideosThread(void *argument);
void checkSamba();