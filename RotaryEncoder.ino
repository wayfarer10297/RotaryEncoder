/*==================================================================================
  Filename: RotaryEncoder.ino
  Author  : Roger Thompson
  Date  :   18-October-2021
==================================================================================*/


/*****************************************************************************************
DESCRIPTION:
  This sketch defines a class and interrupt routines for a Rotary Encoder


 Rotary Encoder connections to the ESP3a2 are as follows:
    Ground      Ground
    Vcc         5v
    CLK     	GPIO36 on ESP32 (labelled SP on the ciruit board)
    DT      	GPIO34 on ESP32
    SW      	GPIO35 on ESP332 (labelled "SP" on the board)
*******************************************************************************************/
#include "Arduino.h"

#define CLK_PIN   36
#define DT_PIN    34
#define SW_PIN    35


class RotaryEncoder{
public:
	bool buttonPressed;
	bool clockPulseReceived;
	long timeOfLastButtonPress;
	int clockCount;
	int sumIncrements, sumDecrements;  // for diagnostics only

	//default constructorRotaryEncoder(){
	RotaryEncoder(){
		buttonPressed = false;
		timeOfLastButtonPress = 0;
		clockPulseReceived = false;
		clockCount = 0;
		sumIncrements = sumDecrements = 0;
	}

	void updateClockCount(){
		int dtValue = digitalRead(DT_PIN);
		int clkValue = digitalRead(CLK_PIN);
		Serial.print("dtValue = "); Serial.print(dtValue); Serial.print("\t clkValue = "); Serial.println(clkValue);
		if (dtValue == clkValue) {
			Serial.print ("Anti-clockwise:  decrement");
			sumDecrements++;
			clockCount--;
		} else {
			Serial.print ("Clockwise: INCREMENT       ");
			clockCount++;
			sumIncrements++;
		}
		clockPulseReceived = false;
	}

private:
};


RotaryEncoder rotaryEncoder;  // create an instance of the RotaryEncoder class


// Interrupt Service Routine activated when the button on the rotary encoder is pressed
void IRAM_ATTR isrSwitch(){
	rotaryEncoder.buttonPressed = true;
}

// Interrupt Service Routine activated when the knob on the rotary encoder is turned
void IRAM_ATTR isrClock(){
	rotaryEncoder.clockPulseReceived = true;
}



void setup(){
	// set the Rotary Encoder pins to INPUT mode (all three have 10k pull-up resistor on the board)
	pinMode(CLK_PIN,INPUT);
	pinMode(DT_PIN,INPUT);
	pinMode(SW_PIN,INPUT);
	Serial.begin(115200);
	delay(1000);

	attachInterrupt(SW_PIN, isrSwitch, HIGH);
	attachInterrupt(CLK_PIN, isrClock, FALLING);
	//attachInterrupt(DT_PIN, isrDt, CHANGE);

	Serial.println("Set-up completed");
}



void loop(){

	if(rotaryEncoder.buttonPressed){
		Serial.println("ISR called:  The button has been pressed");
		// execute appropriate menu actions here (or perhaps wait to see if a doubleClick is coming?  More work needed!)
		rotaryEncoder.buttonPressed = false;
		rotaryEncoder.sumIncrements = rotaryEncoder.sumDecrements = rotaryEncoder.clockCount = 0;  // zero diagnostic counters when button pressed

		if ((millis() - rotaryEncoder.timeOfLastButtonPress) < 100) {
			Serial.println("The button has been DOUBLE CLICKED");
			// execute appropriate menu actions here
		}
		rotaryEncoder.timeOfLastButtonPress = millis();2
	}

	if(rotaryEncoder.clockPulseReceived){
		noInterrupts();
		rotaryEncoder.updateClockCount();
		interrupts();

		Serial.print("\t\t\t\t\t Updated count:  ");
		Serial.print(rotaryEncoder.clockCount);
		Serial.print("\t sumIcrements:  ");
		Serial.print(rotaryEncoder.sumIncrements);
		Serial.print("\t sumDecrements:  ");
		Serial.println(rotaryEncoder.sumDecrements);

	}

}
