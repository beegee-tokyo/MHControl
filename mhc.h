#ifndef mhc_h
#define mhc_h

//#define DO_DEBUG 1
#define CAM_ACTIV 1

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <stdbool.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>
#include <pthread.h>
#include <dirent.h>
#include <sys/statvfs.h>

// For OneWire interface
#include <wiringPi.h>

// For MQTT client
#include <MQTTAsync.h>

// For JSON en/decoding
#include "parson.h"

// The "private" stuff
#include "wifi.h"
// All other declarations, globals and functions
#include "declarations.h"
#include "globals.h"
#include "functions.h"

#endif
