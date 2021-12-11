/*==================================================================================
  Filename: RotaryEncoder.ino
  Author  : Roger Thompson
  Version :	2
  Date	  :	21-October-2021
==================================================================================*/

/*****************************************************************************************
DESCRIPTION:
This sketch defines a simple Class and the interrupt routines for a Rotary Encoder. The
loop() function contains diagnostic code to monitor the performance of the software
detecting the changes of state of the RE. The counting is imperfect and errors are in the
range of 5%->10% in terms of increments being detected as decrements and vice versa.  If
the knob is rotated fairly slowly and smoothly, as would be the case if it is being used
to negotiate a menu system, then the error rate is somewhat less than this and the
performance should be adequate for AudioNav.

The Rotary Encoder connections to the ESP32 are as follows:

    Encoder		ESP32
    =======     =====
    Ground      n/a
    Vcc         Uses the 3.3V output pin on the ESP32
    CLK     	GPIO34  (has 470nF connected to ground)
    DT      	GPIO39  (labelled "SN" on the ESP32 board) (has 100nF connected to ground)
    SW      	GPIO36  (labelled "SP" on the ESP32 board) (has 470nF connected to ground)
 *******************************************************************************************/

#include "Arduino.h"

#define CLK_PIN   34
#define DT_PIN    39
#define SW_PIN    36


class RotaryEncoder{
	int clockCount;
public:
	volatile bool buttonPress = false;	// the Switch pin has gone HIGH
	volatile bool clockPulse = false;	// the Clock pin has gone HIGH
	volatile bool dtLevel = 0;			// level on DT pin when the Clock pin went HIGH

	//default constructorRotaryEncoder(){
	RotaryEncoder(){
		buttonPress = false;
		clockPulse = false;
		dtLevel = 0;
		clockCount = 0;
	}

	void updateCount(){
		if(dtLevel) clockCount--; else clockCount++;
		clockPulse = false;
	}

	void resetCount(){
		clockCount = 0;
	}

	int getCount(){
		return(clockCount);
	}
};


RotaryEncoder rotaryEncoder;  // create an instance of the RotaryEncoder class



portMUX_TYPE synch = portMUX_INITIALIZER_UNLOCKED;

void IRAM_ATTR isrSwitch(){
	//portENTER_CRITICAL(&synch);
	static unsigned long prevMicros;
	unsigned long thisMicros = micros();
	if(thisMicros - prevMicros >5000) rotaryEncoder.buttonPress = true;
	prevMicros = thisMicros;
	//portEXIT_CRITICAL(&synch);
}


void IRAM_ATTR isrClock(){
	//portENTER_CRITICAL(&synch);
	static unsigned long prevMicros;
	unsigned long thisMicros = micros();
	if(thisMicros - prevMicros >5000) {
		rotaryEncoder.clockPulse = true;
		rotaryEncoder.dtLevel = digitalRead(DT_PIN);  // read the DT pin to determine direction
	}
	prevMicros = thisMicros;
	//portEXIT_CRITICAL(&synch);
}


void setup(){
	// set the Rotary Encoder pins to INPUT mode (all three have 10k pull-up resistor on the board)
	pinMode(SW_PIN,INPUT);
	pinMode(CLK_PIN,INPUT);
	pinMode(DT_PIN,INPUT);

	Serial.begin(115200);
	while(!Serial);
	attachInterrupt(SW_PIN, isrSwitch, RISING);
	attachInterrupt(CLK_PIN, isrClock, RISING);
	Serial.println("Set-up complete");
}



void loop(){
	static int t = 0;
	static long thisMicros = 0;
	static long prevMicros = 0;

	if (rotaryEncoder.buttonPress) {
		Serial.print(digitalRead(SW_PIN));
		thisMicros = micros();
		Serial.print("   Transition: "); Serial.print(t);  Serial.print ("          ");
		Serial.print(thisMicros); Serial.print ("    "); Serial.print(thisMicros - prevMicros);
		Serial.print ("    ");
		Serial.println(digitalRead(SW_PIN));

		prevMicros = thisMicros;
		rotaryEncoder.buttonPress = false;
		rotaryEncoder.resetCount();
	}

	if(rotaryEncoder.clockPulse){
		rotaryEncoder.updateCount();
		Serial.print("DT level = "); Serial.print(rotaryEncoder.dtLevel); Serial.print("  ");
		Serial.print(millis());   Serial.print("    Count: "); Serial.println(rotaryEncoder.getCount());
	}
}
