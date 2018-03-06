#include "mhc.h"

void *camStreamThread(void *argument) {
	int cameraNum = *(int *)argument; // get requested camera number
	if (camDebugOn) {
		dbgLen = sprintf(debugMsg,"Streaming camera %d", cameraNum);
		tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
	}
	char mjpgCmd[1024];

//	sprintf(mjpgCmd,"mjpg_streamer -i 'input_uvc.so -yuv -d /dev/video0 -n -r 640x480' -o 'output_http.so -p %i -w /home/pi/mjpg-streamer/www'", CAM_PORT);
//	sprintf(mjpgCmd,"ffmpeg -r 5 -i rtsp://192.168.0.55:554/onvif1 -vcodec copy -map 0 -f segment -strftime 1 -segment_time 300 -segment_format avi \"/mhcv/frontyard_%%Y-%%m-%%d_%%H-%%M-%%S.avi\"");
	if (cameraNum == 0) {
		sprintf(mjpgCmd,"ffmpeg -i rtsp://192.168.0.55:554/onvif1 -vcodec copy -f segment -strftime 1 -segment_time 1800 -segment_format avi \"/mhcv/frontyard_%%Y-%%m-%%d_%%H-%%M.avi\"");
	} else if (cameraNum == 1) {
		sprintf(mjpgCmd,"/usr/local/bin/localCam.sh");
	} else {
		if (camDebugOn) {
			dbgLen = sprintf(debugMsg,"Invalid camera number: %d", cameraNum);
			tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
			pthread_exit(NULL);
		}
	}
	streamCam = true;
	if (camDebugOn) {
		dbgLen = sprintf(debugMsg,"Streaming command: %s", mjpgCmd);
		tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
	}
	while (1) {
		system(mjpgCmd);
	}
	streamCam = false;
	raise(SIGUSR1);
	pthread_exit(NULL);
}
