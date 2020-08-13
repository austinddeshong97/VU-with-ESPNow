// ADC2 restoring procedure
// This is a Workaround to use ADC2 Pins on ESP32 when Wifi or Bluetooth is on.
// (usually only ADC1 Pins are usable for analogRead() when Wifi or Bluetooth is on.)
// -- We save the ADC2 control register before WifiBegin() or BluetoothBegin()
// -- then restore its original value, then set a specific bit of the ADC2 control register   
//to avoid inverted readings: we do the latter two before every analogRead() cycle.
// -- This achieves ADC2 usability even when Wifi/Bluetooth are turned on!

#include "soc/sens_reg.h" // needed for manipulating ADC2 control register
uint64_t reg_b; // Used to store ADC2 control register
#define PIN xx // Substitute xx with your ADC2 Pin number
int value;

void setup() {
// Save ADC2 control register value : Do this before begin Wifi/Bluetooth
reg_b = READ_PERI_REG(SENS_SAR_READ_CTRL2_REG);
Wifi.Begin(); // or similar wifi init function or Bluetooth begin()
}

void loop() {
// ADC2 control register restoring
WRITE_PERI_REG(SENS_SAR_READ_CTRL2_REG, reg_b);
//VERY IMPORTANT: DO THIS TO NOT HAVE INVERTED VALUES!
SET_PERI_REG_MASK(SENS_SAR_READ_CTRL2_REG, SENS_SAR2_DATA_INV);
//We have to do the 2 previous instructions BEFORE EVERY analogRead() calling!
value = analogRead(PIN);
delay(1);
}
