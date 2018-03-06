#include "mhc.h"

#define MAX_TIME 85
#define DHT11PIN 7
int dht11_val[5]={0,0,0,0,0};

/* Read value from DHT11 sensor
 * calculate HEAT index
 * save results in global variables (if read was successful)
 * set flag hasWEIJSON
 */
void dht11Read() {
	return;
	if (DHT_read()) {
		if (dht11DebugOn) {
			dbgLen = sprintf(debugMsg,"Read from DHT11 %d, %d, %d, %d, %d", dht11_val[0], dht11_val[1], dht11_val[2], dht11_val[3], dht11_val[4]);
			tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
		}
		// Init JSON object
		json_value_free(weiJSON); // Free the allocated memory (if any)
		weiJSON = json_value_init_object(); // Init JSON object
		JSON_Object *root_object = json_value_get_object(weiJSON);
		json_object_set_string(root_object,"de","wei");
			
		// Get temperature as float
		float floatTemp = (dht11_val[2]*1.0) + (dht11_val[3]/10.0);
		floatTemp = (float)(dht11_val[2] + dht11_val[3] / 10);
			
		json_object_set_number(root_object,"te",floatTemp);
			
		// Get humidity as float
		float floatHumid = dht11_val[0] + (dht11_val[1]/10.0);
		floatHumid = (float)(dht11_val[0] + dht11_val[1] / 10);
		json_object_set_number(root_object,"hu",floatHumid);
			
		// Calculate heat index
		float floatHeat = calcHeatIndex(floatTemp, floatHumid);
		json_object_set_number(root_object,"hi",floatHeat);
		tempInside = floatTemp;
		humidInside = floatHumid;
		heatInside = floatHeat;
		hasWEIJSON = true;

		// String for debug output
		char *serialized_string = json_serialize_to_string_pretty(weiJSON);
		if (dht11DebugOn) {
			dbgLen = sprintf(debugMsg,"New internal weather JSON: %s", serialized_string);
			tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
		}
		json_free_serialized_string(serialized_string);

	} else {
		if (dht11DebugOn) {
			dbgLen = sprintf(debugMsg,"Invalid Data!!"); 
			tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
		}
	}
}

/* Read data from DHT sensor
 * <return> <code>bool</code>
 *		true if reading was successful
 *		false if reading failed
 */
uint32_t _maxcycles;
bool DHT_read() {
	_maxcycles = 1200; // ==> microsecondsToClockCycles(1000) ==> 1.2Ghz/1000000*1000;
	// Reset 40 bits of received data to zero.
	dht11_val[0] = dht11_val[1] = dht11_val[2] = dht11_val[3] = dht11_val[4] = 0;

	// Send start signal.	See DHT datasheet for full signal diagram:
	// 	http://www.adafruit.com/datasheets/Digital%20humidity%20and%20temperature%20sensor%20AM2302.pdf

	// Go into high impedence state to let pull-up raise data line level and
	// start the reading process.
	digitalWrite(DHT11PIN, HIGH);
	delay(250);

	// First set data line low for 20 milliseconds.
	pinMode(DHT11PIN, OUTPUT);
	digitalWrite(DHT11PIN, LOW);
	delay(20);

	uint32_t cycles[80];
	{
		// End the start signal by setting data line high for 40 microseconds.
		digitalWrite(DHT11PIN, HIGH);
		delayMicroseconds(40);

		// Now start reading the data line to get the value from the DHT sensor.
		pinMode(DHT11PIN, INPUT);
		delayMicroseconds(10);	// Delay a bit to let sensor pull data line low.

		// First expect a low signal for ~80 microseconds followed by a high signal
		// for ~80 microseconds again.
		if (DHT_expectPulse(LOW) == 0) {
			if (dht11DebugOn) {
				dbgLen = sprintf(debugMsg,"Timeout waiting for start signal low pulse."); 
				tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
			}
			return false;
		}
		if (DHT_expectPulse(HIGH) == 0) {
			if (dht11DebugOn) {
				dbgLen = sprintf(debugMsg,"Timeout waiting for start signal high pulse."); 
				tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
			}
			return false;
		}

		// Now read the 40 bits sent by the sensor.	Each bit is sent as a 50
		// microsecond low pulse followed by a variable length high pulse.	If the
		// high pulse is ~28 microseconds then it's a 0 and if it's ~70 microseconds
		// then it's a 1.	We measure the cycle count of the initial 50us low pulse
		// and use that to compare to the cycle count of the high pulse to determine
		// if the bit is a 0 (high state cycle count < low state cycle count), or a
		// 1 (high state cycle count > low state cycle count). Note that for speed all
		// the pulses are read into a array and then examined in a later step.
		for (int i=0; i<80; i+=2) {
			cycles[i] 	= DHT_expectPulse(LOW);
			cycles[i+1] = DHT_expectPulse(HIGH);
		}
	} // Timing critical code is now complete.

	// Inspect pulses and determine which ones are 0 (high state cycle count < low
	// state cycle count), or 1 (high state cycle count > low state cycle count).
	for (int i=0; i<40; ++i) {
		uint32_t lowCycles	= cycles[2*i];
		uint32_t highCycles = cycles[2*i+1];
		if ((lowCycles == 0) || (highCycles == 0)) {
		if (dht11DebugOn) {
			dbgLen = sprintf(debugMsg,"Timeout waiting for pulse."); 
			tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
		}
		return false;
		}
		dht11_val[i/8] <<= 1;
		// Now compare the low and high cycle times to see if the bit is a 0 or 1.
		if (highCycles > lowCycles) {
			// High cycles are greater than 50us low cycle count, must be a 1.
			dht11_val[i/8] |= 1;
		}
		// Else high cycles are less than (or equal to, a weird case) the 50us low
		// cycle count so this must be a zero.	Nothing needs to be changed in the
		// stored data.
	}

	// Check that the checksum matches.
	if (dht11_val[4] == ((dht11_val[0] + dht11_val[1] + dht11_val[2] + dht11_val[3]) & 0xFF)) {
		return true;
	} else {
		if (dht11DebugOn) {
			dbgLen = sprintf(debugMsg,"Checksum failure!"); 
			tcpSendToPort(debugMsg, ipDBG, dbgLen, DEBUG_TCP_PORT_ANDROID);
		}
		return false;
	}
}

/* Wait for change on DHT11 data line
 * Expect the signal line to be at the specified level for a period of time and
 * return a count of loop cycles spent at that level (this cycle count can be
 * used to compare the relative time of two pulses). If more than a millisecond
 * ellapses without the level changing then the call fails with a 0 response.
 * This is adapted from Arduino's pulseInLong function (which is only available
 * in the very latest IDE versions):
 * https://github.com/arduino/Arduino/blob/master/hardware/arduino/avr/cores/arduino/wiring_pulse.c
 * 
 * <param> level
 * 		last level of DHT11 data line
 * <return> <code>uint32_t</code>
 *		measured counts until change detected
 *		0 if timeout occured
 */
uint32_t DHT_expectPulse(bool level) {
	uint32_t count = 0;
	while (digitalRead(DHT11PIN) == level) {
		if (count++ >= _maxcycles) {
		return 0; // Exceeded timeout, fail.
		}
	}
	return count;
}

/* Convert Celsius to Fahrenheit
 * <param> c
 * 		Value in Celsius
 * <return> <code>float</code>
 *		Value in Fahrenheit
 */
float convertCtoF(float c) {
  return c * 1.8 + 32;
}

/* Convert Fahrenheit to Celsius
 * <param> f
 * 		Value in Fahrenheit
 * <return> <code>float</code>
 *		Value in Celsius
 */
float convertFtoC(float f) {
  return (f - 32) * 0.55555;
}

/* Calculate heat index
 * Using both Rothfusz and Steadman's equations
 * http://www.wpc.ncep.noaa.gov/html/heatindex_equation.shtml
 * 
 * <param> floatTemp
 * 		Temperature (in Fahrenheit)
 * <param> floatHumid
 * 		Humidity (in %)
 * <return> <code>float</code>
 *		Heat index (in Fahrenheit)
 */
float calcHeatIndex(float floatTemp, float floatHumid) {
	floatTemp = convertCtoF(floatTemp); // We are always on Centigrade!
	
	float calcHeat = 0.5 * (floatTemp + 61.0 + ((floatTemp - 68.0) * 1.2) + (floatHumid * 0.094));
	if (calcHeat > 79) {
		calcHeat = -42.379 +
					2.04901523 * floatTemp +
					10.14333127 * floatHumid +
					-0.22475541 * floatTemp*floatHumid +
					-0.00683783 * pow(floatTemp, 2) +
					-0.05481717 * pow(floatHumid, 2) +
					0.00122874 * pow(floatTemp, 2) * floatHumid +
					0.00085282 * floatTemp*pow(floatHumid, 2) +
					-0.00000199 * pow(floatTemp, 2) * pow(floatHumid, 2);
		if((floatHumid < 13) && (floatTemp >= 80.0) && (floatTemp <= 112.0)) {
			calcHeat -= ((13.0 - floatHumid) * 0.25) * sqrt((17.0 - abs(floatTemp - 95.0)) * 0.05882);
		} else if((floatHumid > 85.0) && (floatTemp >= 80.0) && (floatTemp <= 87.0)) {
			calcHeat += ((floatHumid - 85.0) * 0.1) * ((87.0 - floatTemp) * 0.2);
		}
	}
	return convertFtoC(calcHeat);
}
