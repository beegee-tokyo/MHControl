CC = gcc
CFLAGS = -Ofast -lpaho-mqtt3a -lwiringPi -lm -lbcm2835 -lrt -lpthread -I. -I/usr/local/include -std=gnu99
OBJ = mhc.o udp-sub.o tcp-sub.o tcp_debug.o mqtt.o parson.o mqtt_sub.o dht11.o cam_thread.o signal_handler.o subs.o

%.o: %.c %.h
	$(CC) -c -o $@ $< $(CFLAGS)

mhc: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)
	ls -s mhc

clean: 
	sudo rm -f mhc *.o /usr/local/bin/mhc
	
all: clean mhc install

install: 
	sudo cp -f mhc /usr/local/bin
#	sudo cp -f ipCam.sh /usr/local/bin
#	sudo chmod +x /usr/local/bin/ipCam.sh
#	sudo cp -f localCam.sh /usr/local/bin
#	sudo chmod +x /usr/local/bin/localCam.sh

run: 
	../scripts/mhc
