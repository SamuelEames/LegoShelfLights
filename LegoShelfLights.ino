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

#define PIN_BTN     8
#define PIN_PIXEL   7   // Data pin for LEDs
#define NUM_LEDS    3   // Note: I'm using single colour LEDs, so '1 pixel' = 3 LEDs


Adafruit_NeoPixel leds(NUM_LEDS, PIN_PIXEL, NEO_GRB + NEO_KHZ800);

bool btnState_Last = HIGH;


// Parameters

bool lightsOn = 0;                // Current state of lights (on or off)

#define LED_FADE_TIME     1000    // (ms) Fade time for an individual module to turn on/off
#define MODULE_DELAY_TIME 500     // (ms) delay between lighting successive modules
#define DEBOUNCE          20      // (ms) debounce time
#define SHORT_PRESS_TIME  500     // (ms) Max period button can be pressed for to trigger on/off action
#define INTENSITY_MIN     10      // (0-255) Minimum intensity to drop to
#define FADE_SPEED        20      // kinda speed at which lights fade lower value = faster fade

uint32_t btnStartTime = 0;


void setup() 
{
  Serial.begin(115200);
  pinMode(PIN_BTN, INPUT_PULLUP);
  leds.begin();       // Initialize Pixels
  leds.show();          // Initialize all pixels to 'off'
}

void loop() 
{
  bool btnState = digitalRead(PIN_BTN);


  if (!btnState)                    // If button pressed ...
  {
    if (millis() < btnStartTime)    // (handle timer overflow)
    {
      btnStartTime = millis();
      Serial.println("\t\t\t OVERFLOW");
      return;
    }

    if (btnState != btnState_Last)  // ... and wasn't previously
      btnStartTime = millis();      // Record time button was initially pressed

    // If button pressed for longer than ShortPressTime - change intensity
    else if (millis() - btnStartTime >= SHORT_PRESS_TIME)
    {
      // Serial.println(F("Long Press"));
      if (lightsOn)       
        ChangeIntensity(millis() - btnStartTime);
    }

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

  btnState_Last = btnState;         // Record btnState for next time
  updateLEDs();



  // Serial.print(F("The light is "));
  // if (lightsOn)
  //  Serial.println("on");
  // else
  //  Serial.println("off");


}


void ChangeIntensity(uint16_t pressTime)
{
  // Changes intensity preset
  static bool fadeUp = true;                  // Direction to fade lights in when button is held
  static bool stopFade = true;                // Stop changing intensity once we hit max or min
  static uint16_t pressTime_last = pressTime + 1; 
  static uint8_t  intensity_preset = 255;     // Preset intensity to fade lights up to when toggled on
  static uint8_t  intensity_initial;          // Intensity_preset value at start of long button press event

  // Serial.print("pressTime = ");
  // Serial.println(pressTime);

  if (pressTime < pressTime_last)             // Reset if new button press event
  {
    // Serial.println("New Button Press Event");
    stopFade = false;                         // Enable fading again!
    intensity_initial = intensity_preset;     // Remember this for next time
    fadeUp = !fadeUp;                         // Reverse fade direction each time a long press is initiated
  }

  pressTime_last = pressTime;                 // Remember for next time



  if (fadeUp)                                 // Stop fading if we hit an end
  {
    if (intensity_preset >= 255)              
      stopFade = true;
  }
  else //fade down
  {
    if (intensity_preset <= INTENSITY_MIN)
      stopFade = true;
  }


  if (!stopFade)
  {
    if (fadeUp)
      intensity_preset = intensity_initial + ((pressTime - SHORT_PRESS_TIME) / FADE_SPEED);
    else
      intensity_preset = intensity_initial - ((pressTime - SHORT_PRESS_TIME) / FADE_SPEED);

    // Serial.print("\t\t Intensity Preset = ");
    // Serial.println(intensity_preset);
    leds.setBrightness(intensity_preset);       // Note; leds.show() must be called to show update
  }


  return;
}


void updateLEDs()
{
  // Updates LED state

  if (lightsOn)
    leds.fill(0xFFFFFF);
  else
    leds.clear();

  leds.show();

  return;
}
