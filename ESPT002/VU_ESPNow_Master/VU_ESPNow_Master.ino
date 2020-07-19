#include <FastLED.h>
#include <WiFi.h>
#include <esp_now.h>
#include <EEPROM.h>
#include <JC_Button.h>

# define LEFT_OUT_PIN 12             // Left channel data out pin to LEDs [6]
# define RIGHT_OUT_PIN 13            // Right channel data out pin to LEDs [5]
# define LEFT_IN_PIN 34             // Left aux in signal [A5]
# define RIGHT_IN_PIN 35            // Right aux in signal [A4]
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

// --------------------
// ------ESP NOW-------
// --------------------
uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

int dataOut;

// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  char macStr[18];
  Serial.print("Packet to: ");
  // Copies the sender mac address to a string
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.print(macStr);
  Serial.print(" send status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

// --------------------
// -----LED STUFF------
// --------------------
/*uint8_t volCountLeft = 0;           // Frame counter for storing past volume data
int volLeft[SAMPLES];               // Collection of prior volume samples
int lvlLeft = 0;                    // Current "dampened" audio level
int minLvlAvgLeft = 0;              // For dynamic adjustment of graph low & high
int maxLvlAvgLeft = 512;*/

uint8_t volCountRight = 0;          // Frame counter for storing past volume data
int volRight[SAMPLES];              // Collection of prior volume samples
int lvlRight = 0;                   // Current "dampened" audio level
int minLvlAvgRight = 0;             // For dynamic adjustment of graph low & high
int maxLvlAvgRight = 512;


CRGB ledsLeft[N_PIXELS];
CRGB ledsRight[N_PIXELS];

// --------------------
// -------SETUP--------
// --------------------
void setup() {
  
  //ESP Now
  Serial.begin(115200);
 
  WiFi.mode(WIFI_STA);
 
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  
  esp_now_register_send_cb(OnDataSent);
   
  // register peer
  esp_now_peer_info_t peerInfo;
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  // register first peer  
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
}

void loop(){
  dataOut = analogRead(RIGHT_IN_PIN);
  
  
  esp_err_t result = esp_now_send(0, (uint8_t *) &dataOut, sizeof(dataOut));
   
  if (result == ESP_OK) {
    Serial.println("Sent with success");
  }
  else {
    Serial.println("Error sending the data");
  }
  delay(2000);
}
