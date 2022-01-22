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

// IO Pins
#define PIN_BTN     0
#define PIN_PIXEL   2   // Data pin for LEDs

// Pixel Setup
#define NUM_LEDS    3             // Note: I'm using single colour LEDs, so '1 pixel' = 3 LEDs
uint8_t modules[NUM_LEDS*3 + 1];  // Stores intensity values for each module - Ideally I'd use the leds arrays to do that, but I don't know where teh raw values of that is stored :(
                                  // Note: 1 extra module is just to make fade on/off easier to program
Adafruit_NeoPixel leds(NUM_LEDS, PIN_PIXEL, NEO_GRB + NEO_KHZ800);

// Global Variables
bool btnState_Last = HIGH;        // Button state on last loop
uint32_t btnStartTime = 0;        // (ms) time button was pushed
bool lightsOn = 0;                // Current state of lights (on or off)
bool fadingOnOff = false;         // true while lights are either turning on or turning off


// Parameters
#define LED_FADE_TIME     1000    // (ms) Fade time for an individual module to turn on/off
#define MODULE_DELAY_TIME 500     // (ms) delay between lighting successive modules
#define DEBOUNCE          20      // (ms) debounce time
#define SHORT_PRESS_TIME  500     // (ms) Max period button can be pressed for to trigger on/off action
#define DIM_MIN           10      // (0-255) Minimum intensity to drop to
#define DIM_SPEED         20      // kinda speed at which lights fade lower value = faster fade
#define HALF_POINT        127     // Value at which next module starts fading


void setup() 
{
  // Serial.begin(115200);
  pinMode(PIN_BTN, INPUT_PULLUP);
  leds.begin();                               // Initialize Pixels
  leds.show();                                // Initialize all pixels to 'off'
}


void loop() 
{
  bool btnState = digitalRead(PIN_BTN);

  if (!fadingOnOff)                           // Don't allow changes while fading off/on
  {
    if (!btnState)                            // If button pressed ...
    {
      if (millis() < btnStartTime)            // (handle timer overflow)
      {
        btnStartTime = millis();
        return;
      }

      if (btnState != btnState_Last)          // ... and wasn't previously
        btnStartTime = millis();              // Record time button was initially pressed

      // If button pressed for longer than ShortPressTime - change intensity
      else if (millis() - btnStartTime >= SHORT_PRESS_TIME)
      {
        if (lightsOn)       
          ChangeIntensity(millis() - btnStartTime);
      }
    }
    else
    {
      if (btnState != btnState_Last)          // Button just released
      {
        // Debounce & toggle light state if short press 
        if (( (millis() - btnStartTime) <= SHORT_PRESS_TIME ) && ( (millis() - btnStartTime) >= DEBOUNCE) ) 
          lightsOn = !lightsOn;
      }
    }
  }

  btnState_Last = btnState;                   // Record btnState for next time
  updateLEDs();
}


void ChangeIntensity(uint16_t pressTime)
{
  // Changes intensity preset
  static bool fadeUp = true;                  // Direction to fade lights in when button is held
  static bool stopFade = true;                // Stop changing intensity once we hit max or min
  static uint16_t pressTime_last = pressTime + 1; 
  static uint8_t  intensity_preset = 255;     // Preset intensity to fade lights up to when toggled on
  static uint8_t  intensity_initial;          // Intensity_preset value at start of long button press event


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
    if (intensity_preset <= DIM_MIN)
      stopFade = true;
  }

  if (!stopFade)
  {
    if (fadeUp)
      intensity_preset = intensity_initial + ((pressTime - SHORT_PRESS_TIME) / DIM_SPEED);
    else
      intensity_preset = intensity_initial - ((pressTime - SHORT_PRESS_TIME) / DIM_SPEED);

    leds.setBrightness(intensity_preset);     // Update LEDs
    leds.show();
  }

  return;
}


void updateLEDs()
{
  // Updates LED state - and does fade on/off animations
  static bool lightsOn_last = lightsOn;
  static uint8_t fadeStage = 0;               // Current stage of fading (kidna equal to module number)

  if (lightsOn_last != lightsOn)              // State just changed - reset timer
  {
    fadingOnOff = true;
    fadeStage = 0;
  }

  if (fadingOnOff)
  {
    delay(1);                                 // Slow if down a little

    if (lightsOn)                             // Fade on
    {
      modules[fadeStage] += 1;                // Increment value

      if (fadeStage)                          // If not first module
      {
        if (modules[fadeStage] <= HALF_POINT) // Increment last module value as well
          modules[fadeStage-1] += 1;
      }

      if (modules[fadeStage] > HALF_POINT)    // Once we've faded up half way on one module, begin the next one
        fadeStage++;

      if (fadeStage >= sizeof(modules)) 
      {
        fadingOnOff = false;                  // Stop once we've done all the modules
        modules[sizeof(modules) -1] = 255;    // Set last value to same as others (another random thing to make fades work properly)
      }
    }
    else                                      // Fade off - almost identical to fading on 
    {
      modules[fadeStage] -= 1;                // de-Increment value

      if (fadeStage)                          // If not first module
      {
        if (modules[fadeStage] > HALF_POINT)  // de-Increment last module value as well
          modules[fadeStage-1] -= 1;
      }

      if (modules[fadeStage] <= HALF_POINT)   // Once we've faded down half way on one module, begin the next one
        fadeStage++;

      if (fadeStage >= sizeof(modules)) 
      {
        fadingOnOff = false;                  // Stop once we've done all the modules
        modules[sizeof(modules) -1] = 0;      // Set last value to same as others (another random thing to make fades work properly)
      }
    }
  }

  writeLEDs();                                // Display LEDs state
  lightsOn_last = lightsOn;
  
  return;
}


void writeLEDs()
{
  // Writes modules buffer into leds
  static uint8_t modules_last[sizeof(modules)];
  bool newData = false;                   

  for (uint8_t i = 0; i < sizeof(modules); ++i)
  {
    if (modules[i] != modules_last[i])        // Check for changes in data
      newData = true;

    modules_last[i] = modules[i];             // Record for next loop
  }

  if (newData)                                // Only refresh LEDs if we actually have a new state to show
  {
    for (uint8_t i = 0; i < NUM_LEDS; ++i)
      leds.setPixelColor(i, modules[0+3*i], modules[1+3*i], modules[2+3*i]);

    leds.show();
  }

  return;
}
