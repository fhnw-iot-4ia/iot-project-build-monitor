#include <ChainableLED.h>
#include <bluefruit.h>

#define NUM_LEDS 1




/******************** hardware & wiring *******************/
int clk_pin = 9; // D4
int data_pin = 10; // D5
ChainableLED leds(clk_pin, data_pin, NUM_LEDS);

int button_pin = A0;
int button_value = LOW;

int buzzer_pin = 5; // D2
/**********************************************************/




/************************** states ************************/
int state = 0;

// INTERNAL STATES
int BUILD_SUCCEEDED = 0;
int BUILD_FAILED = 1;
int SETUP = 50;
int ADVERTISING = 51;

// BLE STATES FROM PI
uint16_t BUILD_SUCCEEDED_NOTIFICATION = 0x00;
uint16_t BUILD_FAILED_NOTIFICATION = 0x01;

// 0 = green, everything okay
// 1 = red, alarm
// 2 = blue, alarm
/**********************************************************/




/********************* color management ********************/
int GREEN = 0;
int RED = 1;
int BLUE = 2;
int NO_COLOR = 99;

/* returns the first value if tick is true, the second if tick is false */
int determineTickingColor(int firstColor, int secondColor, boolean tick);

boolean tick;
int color_state = GREEN;
/**********************************************************/




/**************** ble fields & methods ********************/
BLEService bleService = BLEService(0x4242);
BLECharacteristic buildStatusCharacteristic = BLECharacteristic(0x2727);
void connectCallback(uint16_t conn_handle);
void disconnectCallback(uint16_t conn_handle, uint8_t reason);
void setupBLE();
void startAdvertising();
/**********************************************************/


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

  //uint8_t data[2] = { 0b00000110, button_value };
  //buildStatusCharacteristic.notify(data, sizeof(data));

  if (state == BUILD_SUCCEEDED && button_value == HIGH) {
    Serial.println("state 0, button triggers state 1 for testing");
    state = BUILD_FAILED;
  } else if(state == BUILD_FAILED && button_value == HIGH) {
    Serial.println("state 1, button triggers state 0");
    state = BUILD_SUCCEEDED;
    color_state = GREEN;
  } else if (state == BUILD_FAILED && button_value == LOW) {
    color_state = determineTickingColor(RED, BLUE, tick);
  } else if (state == ADVERTISING) {
    color_state = determineTickingColor(BLUE, NO_COLOR, tick);
  }

  if (color_state == GREEN) {
    leds.setColorRGB(0, 0, 255, 0);
    digitalWrite(buzzer_pin, LOW);
  } else if (color_state == RED) {
    leds.setColorRGB(0, 255, 0, 0);
    digitalWrite(buzzer_pin, HIGH);
  } else if (color_state == BLUE) {
    leds.setColorRGB(0, 0, 255, 255);
    digitalWrite(buzzer_pin, LOW);
  } else if (color_state == NO_COLOR) {
    leds.setColorRGB(0, 0, 0, 0);
  }
  tick = !tick;
  delay(150);
}

int determineTickingColor(int firstColor, int secondColor, boolean tick) {
  if (tick) {
    return firstColor;
  } else {
    return secondColor;
  }
}

void writeCallback(BLECharacteristic& givenCharacteristic, unsigned char* data, short unsigned int len, short unsigned int a) {
  if (givenCharacteristic.uuid == buildStatusCharacteristic.uuid) {
    Serial.print("write callback called");
    Serial.print(*data);
    if (*data == BUILD_FAILED_NOTIFICATION) {
      state = BUILD_FAILED;
    } else if (*data == BUILD_SUCCEEDED_NOTIFICATION) {
      state = BUILD_SUCCEEDED;
    }
  }
}

void setupBLE() {
  Bluefruit.begin();
  Bluefruit.setName("build monitor alarm");
  Bluefruit.setConnectCallback(connectCallback);
  Bluefruit.setDisconnectCallback(disconnectCallback);
  
  bleService.begin(); // Must be called before calling .begin() on its characteristics

  buildStatusCharacteristic.setProperties(CHR_PROPS_WRITE);
  buildStatusCharacteristic.setPermission(SECMODE_NO_ACCESS, SECMODE_OPEN);
  buildStatusCharacteristic.setFixedLen(1);
  buildStatusCharacteristic.setWriteCallback(writeCallback);
  buildStatusCharacteristic.begin();

  startAdvertising();
}

void connectCallback(uint16_t conn_handle) {
  char central_name[32] = { 0 };
  Bluefruit.Gap.getPeerName(conn_handle, central_name, sizeof(central_name));

  Serial.print("Connected to ");
  Serial.println(central_name);

  state = BUILD_SUCCEEDED;
  color_state = GREEN;
}

void disconnectCallback(uint16_t connectionHandle, uint8_t reason) {
  Serial.print(connectionHandle);
  Serial.print(" disconnected, reason = ");
  Serial.println(reason); // see https://github.com/adafruit/Adafruit_nRF52_Arduino
  // /blob/master/cores/nRF5/nordic/softdevice/s140_nrf52_6.1.1_API/include/ble_hci.h
  Serial.println("Advertising ...");
  startAdvertising();
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

  state = ADVERTISING;

  Serial.println("Advertising");
}
