/* LegoShelfLights

Pixel Lights use to light up modules in a bookshelf displaying lego creations.

 * Ideally one (momentary) button control
 		* Tap --> Toggles on / off
 		* Hold --> Changes intensity
 * Animate on/off sequence
 * Maybe have another fun mode where different modules fade off and on

Electronics
 * WS2811 ICs fed into RGB amplifier, driving 12V analog LED strips (one strip per module)
 * ATTINY Microcontroller because it's small and I have a lot of them not being used
 * Button to do the things

 */

#include <Adafruit_NeoPixel.h>

#define PIN_BTN 		8
#define PIN_PIXEL 	7		// Data pin for LEDs
#define NUM_LEDS		2		// Note: I'm using single colour LEDs, so '1 pixel' = 3 LEDs


Adafruit_NeoPixel strip(NUM_LEDS, PIN_PIXEL, NEO_GRB + NEO_KHZ800);

bool btnState_Last = HIGH;


// Parameters

uint8_t intensity_preset = 255; 	// Preset intensity to fade lights up to when toggled on
bool fadeDirection = 1;						// Direction to fade lights in when button is held
bool lightsOn	= 0;								// Current state of lights (on or off)

#define LED_FADE_TIME 		1000 		// (ms) Fade time for an individual module to turn on/off
#define MODULE_DELAY_TIME	500 		// (ms) delay between lighting successive modules
#define DEBOUNCE 					20			// (ms) debounce time
#define SHORT_PRESS_TIME 	500			// (ms) Max period button can be pressed for to trigger on/off action

uint16_t btnStartTime = 0;


void setup() 
{
	Serial.begin(115200);
	pinMode(PIN_BTN, INPUT_PULLUP);
	strip.begin();				// Initialize Pixels
	strip.show();					// Initialize all pixels to 'off'
}

void loop() 
{
	bool btnState = digitalRead(PIN_BTN);


	if (!btnState) 										// If button pressed
	{
		if (btnState != btnState_Last) 	// and wasn't previously
			btnStartTime = millis();			// Record time button was initially pressed

		// If button pressed for longer than ShortPressTime - change intensity

	}
	else
	{
		if (btnState != btnState_Last) // Button just released
		{
			// Debounce & toggle light state if short press 
			if (( (millis() - btnStartTime) <= SHORT_PRESS_TIME ) && ( (millis() - btnStartTime) >= DEBOUNCE) ) 
				lightsOn = !lightsOn;
		}
	}

	btnState_Last = btnState; 				// Record btnState for next time



	Serial.print(F("The light is "));
	if (lightsOn)
		Serial.println("on");
	else
		Serial.println("off");


}
