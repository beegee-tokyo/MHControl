#ffmpeg -i rtsp://192.168.0.55:554/onvif1 -vcodec copy -f segment -strftime 1 -segment_time 900 -segment_format avi "/mhcv/%Y_%m_%d/frontyard_%H-%M.avi"
ffmpeg -i rtsp://192.168.0.55:554/onvif1 -vcodec copy -f segment -strftime 1 -segment_time 1800 -segment_format avi "/mhcv/frontyard_%Y-%m-%d_%H-%M.avi"
