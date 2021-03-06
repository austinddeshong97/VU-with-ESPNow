#include <esp_now.h>
#include <WiFi.h>
#include "soc/sens_reg.h" // needed for manipulating ADC2 control register
uint64_t reg_b; // Used to store ADC2 control register
#define PIN 34 // Substitute xx with your ADC2 Pin number
int value;
#define RIGHT_IN_PIN 34

// REPLACE WITH YOUR ESP RECEIVER'S MAC ADDRESS
uint8_t broadcastAddress1[] = {0x24, 0x62, 0xAB, 0xFB, 0x09, 0x84};
//uint8_t broadcastAddress2[] = {0xFF, , , , , };
//uint8_t broadcastAddress3[] = {0xFF, , , , , };

typedef struct test_struct {
  int x;
  int y;
} test_struct;

test_struct test;

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
 
void setup() {
  Serial.begin(115200);

 
  // Save ADC2 control register value : Do this before begin Wifi/Bluetooth
  reg_b = READ_PERI_REG(SENS_SAR_READ_CTRL2_REG);
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
  memcpy(peerInfo.peer_addr, broadcastAddress1, 6);
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
  /*
  // register second peer  
  memcpy(peerInfo.peer_addr, broadcastAddress2, 6);
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
  /// register third peer
  memcpy(peerInfo.peer_addr, broadcastAddress3, 6);
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }*/
}
 
void loop() {
  WRITE_PERI_REG(SENS_SAR_READ_CTRL2_REG, reg_b);
  //VERY IMPORTANT: DO THIS TO NOT HAVE INVERTED VALUES!
  SET_PERI_REG_MASK(SENS_SAR_READ_CTRL2_REG, SENS_SAR2_DATA_INV);
  value = analogRead(PIN);
  delay(1);
  //We have to do the 2 previous instructions BEFORE EVERY analogRead() calling!
  test.x = value;
 
  esp_err_t result = esp_now_send(0, (uint8_t *) &test, sizeof(test_struct));
/*   
  if (result == ESP_OK) {
    Serial.println("Sent with success");
  }
  else {
    Serial.println("Error sending the data");
  }
  //delay(30);
*/
}
