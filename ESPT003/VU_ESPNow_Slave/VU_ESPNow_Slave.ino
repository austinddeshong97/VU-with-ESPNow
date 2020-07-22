#include <esp_now.h>
#include <WiFi.h>
#include <EEPROM.h>
#include <JC_Button.h>
#include <FastLED.h>

# define LEFT_OUT_PIN 12             // Left channel data out pin to LEDs [6]
# define RIGHT_OUT_PIN 13            // Right channel data out pin to LEDs [5]
# define LEFT_IN_PIN 34             // Left aux in signal [A5]
//# define RIGHT_IN_PIN 35            // Right aux in signal [A4]
# define BTN_PIN 18                  // Push button on this pin [3]
# define DEBOUNCE_MS 20             // Number of ms to debounce the button [20]
# define LONG_PRESS 500             // Number of ms to hold the button to count as long press [500]
# define N_PIXELS 30                // Number of pixels in each string [24]
# define MAX_MILLIAMPS 1500          // Maximum current to draw [500]
# define COLOR_ORDER GRB            // Colour order of LED strip [GRB]
# define LED_TYPE WS2812B           // LED string type [WS2812B]
# define DC_OFFSET 0                // DC offset in aux signal [0]
# define NOISE 50                   // Noise/hum/interference in aux signal [10]
# define SAMPLES 60                 // Length of buffer for dynamic level adjustment [60]
# define TOP (N_PIXELS + 2)         // Allow dot to go slightly off scale [(N_PIXELS + 2)]
# define PEAK_FALL 20               // Rate of peak falling dot [20]
# define N_PIXELS_HALF (N_PIXELS / 2)
# define PATTERN_TIME 10            // Seconds to show eaach pattern on auto [10]
# define STEREO false //true                // If true, L&R channels are independent. If false, both L&R outputs display same data from L audio channel [false]

typedef struct test_struct {
  int x;
  int y;
} test_struct;

test_struct test;

// --------------------
// -----LED STUFF------
// --------------------
uint8_t volCountLeft = 0;           // Frame counter for storing past volume data
int volLeft[SAMPLES];               // Collection of prior volume samples
int lvlLeft = 0;                    // Current "dampened" audio level
int minLvlAvgLeft = 0;              // For dynamic adjustment of graph low & high
int maxLvlAvgLeft = 512;

uint8_t volCountRight = 0;          // Frame counter for storing past volume data
int volRight[SAMPLES];              // Collection of prior volume samples
int lvlRight = 0;                   // Current "dampened" audio level
int minLvlAvgRight = 0;             // For dynamic adjustment of graph low & high
int maxLvlAvgRight = 512;


CRGB ledsLeft[N_PIXELS];
CRGB ledsRight[N_PIXELS];

uint8_t myhue = 0;

void vu4(bool is_centered, uint8_t channel);
void vu5(bool is_centered, uint8_t channel);
void vu6(bool is_centered, uint8_t channel);
void vu7(bool show_background);
void vu8();
void vu9();
void vu10();
void balls();
void fire();
void juggle();
void ripple(boolean show_background);
void sinelon();
void twinkle();
void rainbow(uint8_t rate);

// --------------------
// ----BUTTON STUFF----
// --------------------
uint8_t state = 0;
int buttonPushCounter = 0;
bool autoChangeVisuals = false;
Button modeBtn(BTN_PIN, DEBOUNCE_MS);


void incrementButtonPushCounter() {
  buttonPushCounter = ++buttonPushCounter %17;
  EEPROM.write(1, buttonPushCounter);
}
//callback function that will be executed when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&test, incomingData, sizeof(incomingData));
  Serial.print("Bytes received");
  Serial.println();
}
 
void setup() {
  //LEDS
  FastLED.addLeds < LED_TYPE, LEFT_OUT_PIN, COLOR_ORDER > (ledsLeft, N_PIXELS).setCorrection(TypicalLEDStrip);
  FastLED.addLeds < LED_TYPE, RIGHT_OUT_PIN, COLOR_ORDER > (ledsRight, N_PIXELS).setCorrection(TypicalLEDStrip);
  //FastLED.setBrightness(BRIGHTNESS);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, MAX_MILLIAMPS);

  modeBtn.begin();

  buttonPushCounter = (int)EEPROM.read(1); // load previous setting
  buttonPushCounter = 0;
  Serial.print("Starting pattern ");
  Serial.println(buttonPushCounter);
  
  //Initialize Serial Monitor
  Serial.begin(115200);
  
  //Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  //Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  
  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info
   esp_now_register_recv_cb(OnDataRecv);
}
 
void loop() {
// Read button
  modeBtn.read(); 
  switch (state) {
    case 0:                
      if (modeBtn.wasReleased()) {
        Serial.print("Short press, pattern ");
        Serial.println(buttonPushCounter);
        incrementButtonPushCounter();
        autoChangeVisuals = false;
      }
      else if (modeBtn.pressedFor(LONG_PRESS))
        state = 1;
      break;
    
    case 1:
      if (modeBtn.wasReleased()) {
        state = 0;
        Serial.print("Long press, auto, pattern ");
        Serial.println(buttonPushCounter);
        autoChangeVisuals = true;
      }
      break;
  }
  
  // Switch pattern if on auto
  if(autoChangeVisuals){
    EVERY_N_SECONDS(PATTERN_TIME) { 
      incrementButtonPushCounter();
      Serial.print("Auto, pattern ");
      Serial.println(buttonPushCounter); 
    }
  }

  // Run selected pattern
  switch (buttonPushCounter) {
    case 0:
      vu4(false, 0);
      if (STEREO) vu4(false, 1);
      else copyLeftToRight();
      break;
  
    case 1:
      vu4(true, 0);
      if (STEREO) vu4(true, 1);
      else copyLeftToRight();
      break;
  
    case 2:
      vu5(false, 0);
      if (STEREO) vu5(false, 1);
      else copyLeftToRight();
      break;
  
    case 3:
      vu5(true, 0);
      if (STEREO) vu5(true, 1);
      else copyLeftToRight();
      break;
  
    case 4:
      vu6(false, 0);
      if (STEREO) vu6(false, 1);
      else copyLeftToRight();
      break;
      
    case 5:
      vu7(true);
      copyLeftToRight();
      break;
  
    case 6:
      vu8();
      copyLeftToRight();
      break;
  
    case 7:
      vu9();
      break;
  
    case 8:
      vu10();
      break;
    
    case 9:
      vu7(false);
      copyLeftToRight();
      break;
  
    case 10:
      twinkle();
      break;
  
    case 11:
      sinelon();
      break;
  
    case 12:
      balls();
      break;
  
    case 13:
      juggle();
      break;
  
    case 14:
      fire();
      break;
  
    case 15:
      ripple(false);
      break;
  
    case 16:
      rainbow(10);
      break;
  }
}
// ------------------
// -- VU functions --
// ------------------

uint16_t auxReading(uint8_t channel) {

  int n = 0;
  uint16_t height = 0;

  if(channel == 0) {
    int n = analogRead(LEFT_IN_PIN); // Raw reading from left line in
    n = abs(n - 512 - DC_OFFSET); // Center on zero
    n = (n <= NOISE) ? 0 : (n - NOISE); // Remove noise/hum
    lvlLeft = ((lvlLeft * 7) + n) >> 3; // "Dampened" reading else looks twitchy (>>3 is divide by 8)
    volLeft[volCountLeft] = n; // Save sample for dynamic leveling
    volCountLeft = ++volCountLeft % SAMPLES;
    // Calculate bar height based on dynamic min/max levels (fixed point):
    height = TOP * (lvlLeft - minLvlAvgLeft) / (long)(maxLvlAvgLeft - minLvlAvgLeft);
  }
  
  else {
    int n = test.x; // Raw reading from mic
    n = abs(n - 512 - DC_OFFSET); // Center on zero
    n = (n <= NOISE) ? 0 : (n - NOISE); // Remove noise/hum
    lvlRight = ((lvlRight * 7) + n) >> 3; // "Dampened" reading (else looks twitchy)
    volRight[volCountRight] = n; // Save sample for dynamic leveling
    volCountRight = ++volCountRight % SAMPLES;
    // Calculate bar height based on dynamic min/max levels (fixed point):
    height = TOP * (lvlRight - minLvlAvgRight) / (long)(maxLvlAvgRight - minLvlAvgRight);
  }

  // Calculate bar height based on dynamic min/max levels (fixed point):
  height = constrain(height, 0, TOP);
  return height;
}

void copyLeftToRight() {
  for (uint8_t i = 0; i < N_PIXELS; i++) {
    ledsRight[i] = ledsLeft[i];
  }
}

/*
 * Function for dropping the peak
 */
uint8_t peakLeft, peakRight;
void dropPeak(uint8_t channel) {
  
  static uint8_t dotCountLeft, dotCountRight;
 
  if(channel == 0) {
    if(++dotCountLeft >= PEAK_FALL) { //fall rate 
      if(peakLeft > 0) peakLeft--;
      dotCountLeft = 0;
    }
  } else {
    if(++dotCountRight >= PEAK_FALL) { //fall rate 
      if(peakRight > 0) peakRight--;
      dotCountRight = 0;
    }
  }
}

/*
 * Function for averaging the sample readings
 */
void averageReadings(uint8_t channel) {

  uint16_t minLvl, maxLvl;

  // minLvl and maxLvl indicate the volume range over prior frames, used
  // for vertically scaling the output graph (so it looks interesting
  // regardless of volume level).  If they're too close together though
  // (e.g. at very low volume levels) the graph becomes super coarse
  // and 'jumpy'...so keep some minimum distance between them (this
  // also lets the graph go to zero when no sound is playing):
  if(channel == 0) {
    minLvl = maxLvl = volLeft[0];
    for (int i = 1; i < SAMPLES; i++) {
      if (volLeft[i] < minLvl) minLvl = volLeft[i];
      else if (volLeft[i] > maxLvl) maxLvl = volLeft[i];
    }
    if ((maxLvl - minLvl) < TOP) maxLvl = minLvl + TOP;
    
    minLvlAvgLeft = (minLvlAvgLeft * 63 + minLvl) >> 6; // Dampen min/max levels
    maxLvlAvgLeft = (maxLvlAvgLeft * 63 + maxLvl) >> 6; // (fake rolling average)
  }

  else {
    minLvl = maxLvl = volRight[0];
    for (int i = 1; i < SAMPLES; i++) {
      if (volRight[i] < minLvl) minLvl = volRight[i];
      else if (volRight[i] > maxLvl) maxLvl = volRight[i];
    }
    if ((maxLvl - minLvl) < TOP) maxLvl = minLvl + TOP;
    minLvlAvgRight = (minLvlAvgRight * 63 + minLvl) >> 6; // Dampen min/max levels
    maxLvlAvgRight = (maxLvlAvgRight * 63 + maxLvl) >> 6; // (fake rolling average)
  }
}
