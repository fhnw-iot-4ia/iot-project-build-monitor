#include <ChainableLED.h>
#include <bluefruit.h>

#define NUM_LEDS 1

int clk_pin = 9; // D4
int data_pin = 10; // D5
ChainableLED leds(clk_pin, data_pin, NUM_LEDS);

int button_pin = A0;
int button_value = LOW;

int buzzer_pin = 5; // D2

int state = 0;
// 1-digit states: business states
//  0 = not in alarm state
//  1 = in alarm state
// 2-digit states from 50 upwards: control states
//  50 = setup
//  51 = connecting to gateway

int alarm_state = 0;
// 0 = green, everything okay
// 1 = red, alarm
// 2 = blue, alarm

BLEService bleService = BLEService(0x4242);
BLECharacteristic characteristic = BLECharacteristic(0x2727);
void connectCallback(uint16_t conn_handle);
void disconnectCallback(uint16_t conn_handle, uint8_t reason);
void cccdCallback(BLECharacteristic& characteristic, uint16_t cccdValue);
void setupBLE();
void startAdvertising();


void setup() {
  Serial.begin(115200);
  while (!Serial) { delay(10); } // only if usb connected
  Serial.println("Setup");

  pinMode(clk_pin, OUTPUT);
  pinMode(data_pin, OUTPUT);
  pinMode(button_pin, INPUT);
  pinMode(buzzer_pin, OUTPUT);
  digitalWrite(buzzer_pin, LOW);

  setupBLE();
}

void loop() {
  Serial.begin(115200);
  button_value = digitalRead(button_pin);

  uint8_t data[2] = { 0b00000110, button_value };
  characteristic.notify(data, sizeof(data));

  if (state == 0 && button_value == HIGH) {
    Serial.println("state 0, button triggers state 1 for testing");
    state = 1;
    
  } else if(state == 1 && button_value == HIGH) {
    Serial.println("state 1, button triggers state 0");
    state = 0;
    alarm_state = 0;
    
  } else if (state == 1 && button_value == LOW) {
    if (alarm_state != 1) {
      alarm_state = 1;
    } else {
      alarm_state = 2;
    }
  }

  if (alarm_state == 0) {
    leds.setColorRGB(0, 0, 255, 0);
    digitalWrite(buzzer_pin, LOW);
  } else if (alarm_state == 1) {
    leds.setColorRGB(0, 255, 0, 0);
    digitalWrite(buzzer_pin, HIGH);
  } else if (alarm_state == 2) {
    leds.setColorRGB(0, 0, 255, 255);
    digitalWrite(buzzer_pin, LOW);
  }
  delay(100);
}


void setupBLE() {
  Bluefruit.begin();
  Bluefruit.setName("build monitor alarm");
  Bluefruit.setConnectCallback(connectCallback);
  Bluefruit.setDisconnectCallback(disconnectCallback);
  
  bleService.begin(); // Must be called before calling .begin() on its characteristics

  characteristic.setProperties(CHR_PROPS_NOTIFY);
  characteristic.setPermission(SECMODE_OPEN, SECMODE_NO_ACCESS);
  characteristic.setFixedLen(2);
  characteristic.setCccdWriteCallback(cccdCallback);  // Optionally capture CCCD updates
  characteristic.begin();
  uint8_t data[2] = { 0b00000110, 0x40 }; // Use 8-bit values, sensor connected and detected
  characteristic.notify(data, 2); // Use .notify instead of .write
  
  startAdvertising();
}

void connectCallback(uint16_t conn_handle) {
  char central_name[32] = { 0 };
  Bluefruit.Gap.getPeerName(conn_handle, central_name, sizeof(central_name));

  Serial.print("Connected to ");
  Serial.println(central_name);
}

void disconnectCallback(uint16_t connectionHandle, uint8_t reason) {
  Serial.print(connectionHandle);
  Serial.print(" disconnected, reason = ");
  Serial.println(reason); // see https://github.com/adafruit/Adafruit_nRF52_Arduino
  // /blob/master/cores/nRF5/nordic/softdevice/s140_nrf52_6.1.1_API/include/ble_hci.h
  Serial.println("Advertising ...");
}


void cccdCallback(BLECharacteristic& givenCharacteristic, uint16_t cccdValue) {
  if (givenCharacteristic.uuid == characteristic.uuid) {
    Serial.print("Heart Rate Measurement 'Notify', ");
    if (givenCharacteristic.notifyEnabled()) {
      Serial.println("enabled");
    } else {
      Serial.println("disabled");
    }
  }
}

void startAdvertising() {
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addTxPower();
  Bluefruit.Advertising.addService(bleService);
  Bluefruit.Advertising.addName();

  // See https://developer.apple.com/library/content/qa/qa1931/_index.html   
  const int fastModeInterval = 32; // * 0.625 ms = 20 ms
  const int slowModeInterval = 244; // * 0.625 ms = 152.5 ms
  const int fastModeTimeout = 30; // s
  Bluefruit.Advertising.restartOnDisconnect(true);
  Bluefruit.Advertising.setInterval(fastModeInterval, slowModeInterval);
  Bluefruit.Advertising.setFastTimeout(fastModeTimeout);
  // 0 = continue advertising after fast mode, until connected
  Bluefruit.Advertising.start(0);
  Serial.println("Advertising ...");
}
