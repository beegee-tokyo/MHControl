#include "mhc.h"

static unsigned int total = 0;
// Counter for reconnect tries to Diskstation
unsigned int sambaError = 0;

/* Compare two strings
 * Used by qsort()
 */
int stringcmp(const void *a,const void *b) {
	return (strcmp((char *)a,(char *)b));
}

/* Check used disk space and delete oldest directory if necessary
 */
void cleanDir() {
	/* The directory (drive) to be checked */
	char *filename = "/mhcv";
	/* The drive information structure */
	struct statvfs buf;

	if (!statvfs(filename, &buf)) {
		/* Disk usage in percent */
		unsigned long long used_perc = (
			((unsigned long long)buf.f_blocks * (unsigned long long)buf.f_bsize) 
				- ((unsigned long long)buf.f_bfree * (unsigned long long)buf.f_bsize)) 
				/ ((unsigned long long)buf.f_blocks * (unsigned long long)buf.f_bsize / 100);
		if (used_perc >= 75) { // Used 75% or more? Time to clean up!
			// Write Log data
			getSysTime(1);
			// Pointer to log file
			FILE *fPtr = fopen("/home/pi/mhcv.txt", "a"); // /home/pi on Raspian
			fwrite(sysTime,20,1,fPtr);
			dbgLen = sprintf(debugMsg,"cleanDir: usage %d \n",used_perc);
			fwrite(debugMsg,dbgLen,1,fPtr);
			fclose(fPtr);
			/* Get all directories and sort them */
			/* Directory information */
			DIR *dp;
			/* Directory entry information */
			struct dirent *ep;
			/* Try to open the directory */
			dp = opendir("/mhcv");

			/* List of sub directories */
			char movieDirs[60][11];
			/* Pointer to fill list of sub directories */
			short arrayPtr = 0;

			if (dp != NULL) { // Directory found
				while (ep = readdir(dp)) {
					if (strncmp(ep->d_name,"2017",4) == 0) { // We are looking only for record folders!
						strcpy(movieDirs[arrayPtr],ep->d_name);
						arrayPtr++;
						if (arrayPtr > 60) {
							break;
						}
					}
				}
				closedir(dp);
			} else {
				return;
			}

			// Sort the sub directories by name (oldest day first)
			qsort(movieDirs, arrayPtr, 11, stringcmp);
			// Delete oldest directory
			char sysCmd[1024];
			sprintf(sysCmd,"rm -r -f /mhcv/%s",movieDirs[0]);
			// TODO for testing only!!!
			// sprintf(sysCmd,"mv -f /mhcv/%s /home/pi/%s",movieDirs[0],movieDirs[0]);
			// Write Log data
			// Pointer to log file
			fPtr = fopen("/home/pi/mhcv.txt", "a"); // /home/pi on Raspian
			dbgLen = sprintf(debugMsg,"cleanDir: command: %s \n",sysCmd);
			fwrite(debugMsg,dbgLen,1,fPtr);
			fclose(fPtr);
			system(sysCmd);
		}
	}
}

/* Write debug string to file 
 */
void writeBuffToDebugFile() {
	// Write the received data into a file
	getSysTime(1);
	// Pointer to log file
	FILE *fPtr = fopen("/home/pi/mhcv.txt", "a"); // /home/pi on Raspian
	fwrite(sysTime,20,1,fPtr);
	fwrite(debugTcpBufTemp,debugTcpRecvBytesTemp,1,fPtr);
	fwrite("\n",1,1,fPtr);
	fclose(fPtr);
}

/* Reboot system once a day
 * Set to be done at 12:00 pm 
 */
void lunchReboot() {
	// Get current time
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);
	if ((tm.tm_hour == 12) && (tm.tm_min == 00) && (tm.tm_sec <= 10)) {
		// Write the received data into a file
		getSysTime(1);
		// Pointer to log file
		FILE *fPtr = fopen("/home/pi/mhcv.txt", "a"); // /home/pi on Raspian
		fwrite(sysTime,20,1,fPtr);
		dbgLen = sprintf(debugMsg,"lunchReboot triggered at: %s\n", sysTime);
		fwrite(debugMsg,dbgLen,1,fPtr);
		fclose(fPtr);
		// Reset the machine!
		system ("sudo reboot now");
	}
}

/* Reboot WiFi router once a day
 * Set to be done at 6am
 */
void rebootRouter() {
	// Wait until 6am, to make sure everybody is in bed
	// Get current time
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);
	if ((tm.tm_hour == 6) && (tm.tm_min == 00) && (tm.tm_sec <= 10)) {
		// Write the received data into a file
		getSysTime(1);
		// Pointer to log file
		FILE *fPtr = fopen("/home/pi/mhcv.txt", "a"); // /home/pi on Raspian
		fwrite(sysTime,20,1,fPtr);
		dbgLen = sprintf(debugMsg,"Reboot WiFi router triggered at: %s\n", sysTime);
		fwrite(debugMsg,dbgLen,1,fPtr);
		fclose(fPtr);
		// Reset WiFi router!
		char sysCmd[1024];
		sprintf(sysCmd,"curl --referer http://192.168.0.1/ --user admin:admin http://192.168.0.1/userRpm/SysRebootRpm.htm?Reboot=Reboot > /dev/null");
		system(sysCmd);
	}
}

/* Move all files from yesterday into a folder
 */
void moveVideos() {
	// Check if the date has changed
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);

	// Check if Day or Month or Year has changed
	if ((startYear != tm.tm_year + 1900) || (startMonth != tm.tm_mon + 1) || (startDay != tm.tm_mday)) {
		if (genDebugOn) {
			// Write Log data
			// Pointer to log file
			FILE *fPtr = fopen("/home/pi/mhcv.txt", "a"); // /home/pi on Raspian
			dbgLen = sprintf(debugMsg,"moveVideos - startYear: %4d startMonth: %2d startDay: %2d\n", startYear, startMonth, startDay);
			fwrite(debugMsg,dbgLen,1,fPtr);
			dbgLen = sprintf(debugMsg,"moveVideos -  currYear: %4d  currMonth: %2d currDay: %2d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
			fwrite(debugMsg,dbgLen,1,fPtr);
			dbgLen = sprintf(debugMsg,"moveVideos -  hour: %d min: %02d sec: _%02d\n",tm.tm_hour, tm.tm_min, tm.tm_sec);
			fwrite(debugMsg,dbgLen,1,fPtr);
			fclose(fPtr);
		}
		// Wait until 1am, to make sure a current recording is not affected
		if ((tm.tm_hour >= 1) && (tm.tm_min <= 1) && (tm.tm_sec <= 30)) {
			int parameter, rc;
			if ((rc = pthread_create(&threadMoveVideoStream, NULL, moveVideosThread, &parameter)) != 0) {
				// Write Log data
				// Pointer to log file
				FILE *fPtr = fopen("/home/pi/mhcv.txt", "a"); // /home/pi on Raspian
				dbgLen = sprintf(debugMsg,"moveVideos - Move video thread creation failed with result %d\n", rc);
				fwrite(debugMsg,dbgLen,1,fPtr);
				fclose(fPtr);
				exit(2);
			}
		}
	}
}

/* Move videofiles in separate thread
 */
void *moveVideosThread(void *argument) {
	char sysCmd[1024];
	struct stat info;
	
	// Write Log data
	getSysTime(1);
	// Pointer to log file
	FILE *fPtr = fopen("/home/pi/mhcv.txt", "a"); // /home/pi on Raspian
	fwrite(sysTime,20,1,fPtr);
	dbgLen = sprintf(debugMsg,"moveVideos thread started: %s\n", sysTime);
	fwrite(debugMsg,dbgLen,1,fPtr);
	fclose(fPtr);
	// Create new directory if necessary
	sprintf(sysCmd,"/mhcv/%4d_%02d_%02d",startYear, startMonth, startDay);
	if (stat(sysCmd, &info) != 0) { // path does not exist
		sprintf(sysCmd,"mkdir /mhcv/%4d_%02d_%02d\n",startYear, startMonth, startDay);
		system(sysCmd);
		sprintf(sysCmd,"/mhcv/%4d_%02d_%02d",startYear, startMonth, startDay);
		if (stat(sysCmd, &info) != 0) { // path does not exist
			printf("Creating directory %s failed\n",sysCmd);
			// Write Log data
			FILE *fPtr = fopen("/home/pi/mhcv.txt", "a"); // /home/pi on Raspian
			dbgLen = sprintf(debugMsg,"moveVideos: Creating directory %s failed\n",sysCmd);
			fwrite(debugMsg,dbgLen,1,fPtr);
			fclose(fPtr);
		}
	} else {
		if (genDebugOn) {
			// Write Log data
			fPtr = fopen("/home/pi/mhcv.txt", "a"); // /home/pi on Raspian
			dbgLen = sprintf(debugMsg,"moveVideos: Directory %s already exists\n",sysCmd);
			fwrite(debugMsg,dbgLen,1,fPtr);
			fclose(fPtr);
		}
	}
	// Move video files of last day into a directory
	sprintf(sysCmd,"cd /mhcv; for filename in *%4d-%02d-%02d*.avi; do mv \"$filename\" \"%4d_%02d_%02d/$(echo $filename | sed -e 's/_%4d-%02d-%02d//g')\";  done"
		,startYear, startMonth, startDay
		,startYear, startMonth, startDay
		,startYear, startMonth, startDay);
	if (genDebugOn) {
		// Write Log data
		fPtr = fopen("/home/pi/mhcv.txt", "a"); // /home/pi on Raspian
		dbgLen = sprintf(debugMsg,"moveVideos: Executing %s \n",sysCmd);
		fwrite(debugMsg,dbgLen,1,fPtr);
		fclose(fPtr);
	}
	system(sysCmd);
	// Update current Day, Month and Year
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);
	startYear = tm.tm_year + 1900;
	startMonth = tm.tm_mon + 1;
	startDay = tm.tm_mday;
	pthread_exit(NULL);
}

/* Send status update request to all devices
 */
void requestAllStatus() {
//	tcpSend("s", ipSPM, 1);
	tcpSend("s", ipSecFront, 1);
//	tcpSend("s", ipAC1, 1);
//	tcpSend("s", ipAC2, 1);
	tcpSend("s", ipSecBack, 1);
	tcpSend("s", ipCam1, 1);
	tcpSend("s", ipLB1, 1);
	tcpSend("s", ipLY1, 1);
}

/* Send system time to all devices
 */
void updateDeviceTime() {
	getSysTime(0);
	tcpSend(sysTime, ipSecFront, 21);
	getSysTime(0);
	tcpSend(sysTime, ipSecBack, 21);
//	getSysTime(0);
//	tcpSend(sysTime, ipAC1, 21);
//	getSysTime(0);
//	tcpSend(sysTime, ipAC2, 21);
	getSysTime(0);
	tcpSend(sysTime, ipCam1, 21);
	getSysTime(0);
	tcpSend(sysTime, ipLY1, 21);
}

/* Get local time and store it in global variable
 * sysTime formatted as time command for the
 * devices
 * @param isCmd
 * 		if 0, sysTime is command format: y=YYYY,MM,DD,HH,mm,ss
 * 		if 1, systTime is for debug log file format YYYY.MM.DD HH:mm:ss
 * 		if 2, sysTime is for recording format YYYY-MM-DD_HH-mm-ss
 * -MM-DD_HH-mm-ss
 * y=YYYY,MM,DD,HH,mm,ss
 */
void getSysTime(int isCmd) {
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);
	if (isCmd == 0) {
		sprintf(sysTime,"y=%4d,%02d,%02d,%02d,%02d,%02d\n",tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
	} else if (isCmd == 1) {
		sprintf(sysTime,"%4d.%02d.%02d %02d:%02d:%02d ",tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
	} else {
		sprintf(sysTime,"%4d-%02d-%02d_%02d-%02d-%02d",tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
	}
	if (genDebugOn) {
		dbgLen = sprintf(debugMsg,"getSysTime: time command: %s", sysTime);
		tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
	}
}

void checkSamba() {
	FILE *fPtr;
	const char *filename = "/mhcv/connected.txt";
	if( access( filename, F_OK ) == -1 ) {
		// Cannot find file, connection interrupted?

		// Stop the video recording or the SDcard will be filled up
		pthread_cancel(threadCamStream);

		// Write Log data
		fPtr = fopen("/home/pi/mhcv.txt", "a"); // /home/pi on Raspian
		getSysTime(1);
		dbgLen = sprintf(debugMsg,"%s: checkSamba: cannot find >connected.txt<\n", sysTime);
		fwrite(debugMsg,dbgLen,1,fPtr);
		fclose(fPtr);

		// Try to remount the drive
		system ("sudo mount 192.168.0.252:/volume1/web/cctv/MHCV");
		// check if the drive is available now
		if( access( filename, F_OK ) == -1 ) {
			// File still not available
			sambaError++;
			if (sambaError == 360) { // Could not reconnect for 1 hour, try a reboot
				// Write Log data
				fPtr = fopen("/home/pi/mhcv.txt", "a"); // /home/pi on Raspian
				getSysTime(1);
				dbgLen = sprintf(debugMsg,"%s: checkSamba: failed 1h to reconnected to Diskstation\n", sysTime);
				fwrite(debugMsg,dbgLen,1,fPtr);
				fclose(fPtr);

				// // Reset the machine!
				// system ("sudo reboot now");
			}
		} else {
			sambaError = 0;
			// Restart camera thread
			int parameter = 0; // Start thread for camera 0
			int rc = 0; // return code
			if ((rc = pthread_create(&threadCamStream, NULL, camStreamThread, &parameter)) != 0) {
				// Write Log data
				// Pointer to log file
				fPtr = fopen("/home/pi/mhcv.txt", "a"); // /home/pi on Raspian
				dbgLen = sprintf(debugMsg,"%s: checkSamba - IP Cam streaming thread creation failed with result %d\n", sysTime, rc);
				fwrite(debugMsg,dbgLen,1,fPtr);
				fclose(fPtr);
			}

			// Write Log data
			fPtr = fopen("/home/pi/mhcv.txt", "a"); // /home/pi on Raspian
			getSysTime(1);
			dbgLen = sprintf(debugMsg,"%s: checkSamba: reconnected to Diskstation\n", sysTime);
			fwrite(debugMsg,dbgLen,1,fPtr);
			fclose(fPtr);
		}
	} else {
		sambaError = 0;
		if (genDebugOn) {
			// Write Log data
			fPtr = fopen("/home/pi/mhcv.txt", "a"); // /home/pi on Raspian
			getSysTime(1);
			dbgLen = sprintf(debugMsg,"%s: checkSamba: found >connected.txt<\n", sysTime);
			fwrite(debugMsg,dbgLen,1,fPtr);
			fclose(fPtr);
		}
	}
}