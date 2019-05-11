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
int BUILD_FIXING = 2;
int BUILD_FIXING_WAITING = 3;
int ADVERTISING = 50;

// BLE STATES FROM PI
uint16_t BUILD_SUCCEEDED_NOTIFICATION = 0x00;
uint16_t BUILD_FAILED_NOTIFICATION = 0x01;
uint16_t BUILD_FIXING_BY_SOMEONE_ELSE = 0x02;

// BLE STATES FOR PI
uint16_t BUILD_NOT_FIXING_NOTIFICATION = 0x00;
uint16_t BUILD_FIXING_NOTIFICATION = 0x01;

// 0 = green, everything okay
// 1 = red, alarm
// 2 = blue, alarm
/**********************************************************/




/********************* color management ********************/
int GREEN = 0;
int RED = 1;
int BLUE = 2;
int YELLOW = 3;
int PURPLE1 = 4;
int PURPLE2 = 5;
int ORANGE = 6;
int NO_COLOR = 99;

/* returns the first value if tick is true, the second if tick is false */
int determineTickingColor(int firstColor, int secondColor, boolean tick);

boolean tick;
int delays = 300;
int color_state = GREEN;
/**********************************************************/




/**************** ble fields & methods ********************/
// TODO: Check codes with coach
BLEService bleService = BLEService(0x4242);
BLECharacteristic buildStatusCharacteristic = BLECharacteristic(0x2727);
BLECharacteristic buildFixingCharacteristic = BLECharacteristic(0x2728);
void connectCallback(uint16_t conn_handle);
void disconnectCallback(uint16_t conn_handle, uint8_t reason);
void setupBLE();
void startAdvertising();
/**********************************************************/


void setup() {
  Serial.begin(115200);
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
  delays = 300;
  digitalWrite(buzzer_pin, LOW);

  // TODO: remove the first if block for production. This serves only to trigger a failed build
  if (state == BUILD_SUCCEEDED && button_value == HIGH) {
    Serial.println("trigger failed build for testing");
    state = BUILD_FAILED;
  } else if (state == BUILD_SUCCEEDED && button_value == LOW) {
    color_state = GREEN;
    buildFixingCharacteristic.write16(BUILD_NOT_FIXING_NOTIFICATION);
  } else if (state == BUILD_FAILED && button_value == HIGH) {
    digitalWrite(buzzer_pin, LOW);
    Serial.println("state failed, button triggers state fixing");
    state = BUILD_FIXING;
  } else if (state == BUILD_FAILED && button_value == LOW) {
    digitalWrite(buzzer_pin, HIGH);
    delays = 150;
    color_state = determineTickingColor(RED, BLUE, tick);
    buildFixingCharacteristic.write16(BUILD_NOT_FIXING_NOTIFICATION);
  } else if (state == ADVERTISING) {
    color_state = determineTickingColor(BLUE, NO_COLOR, tick);
    buildFixingCharacteristic.write16(BUILD_NOT_FIXING_NOTIFICATION);
  } else if (state == BUILD_FIXING) {
    delays = 600;
    buildFixingCharacteristic.write16(BUILD_FIXING_NOTIFICATION);
    delays = 600;
    color_state = determineTickingColor(YELLOW, ORANGE, tick);
  } else if (state == BUILD_FIXING_WAITING) {
    color_state = determineTickingColor(PURPLE1, PURPLE2, tick);
    buildFixingCharacteristic.write16(BUILD_NOT_FIXING_NOTIFICATION);
  }

  if (color_state == GREEN) {
    leds.setColorRGB(0, 0, 255, 0);
  } else if (color_state == RED) {
    leds.setColorRGB(0, 255, 0, 0);
  } else if (color_state == BLUE) {
    leds.setColorRGB(0, 0, 255, 255);
  } else if (color_state == NO_COLOR) {
    leds.setColorRGB(0, 0, 0, 0);
  } else if (color_state == YELLOW) {
    leds.setColorRGB(0, 255, 213, 87);
  } else if (color_state == ORANGE) {
    leds.setColorRGB(0, 255, 191, 0);
  } else if (color_state == PURPLE1) {
    leds.setColorRGB(0, 169, 77, 209);
  } else if (color_state == PURPLE2) {
    leds.setColorRGB(0, 114, 4, 183);
  }
  tick = !tick;
  delay(delays);
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
    Serial.print("write callback called ");
    Serial.println(*data);
    if (*data == BUILD_FAILED_NOTIFICATION) {
      Serial.println("build failed");
      state = BUILD_FAILED;
    } else if (*data == BUILD_SUCCEEDED_NOTIFICATION) {
      Serial.println("build succeeded");
      state = BUILD_SUCCEEDED;
    } else if (*data == BUILD_FIXING_BY_SOMEONE_ELSE) {
      state = BUILD_FIXING_WAITING;
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

  buildFixingCharacteristic.setProperties(CHR_PROPS_READ);
  buildFixingCharacteristic.setPermission(SECMODE_OPEN, SECMODE_NO_ACCESS);
  buildFixingCharacteristic.setFixedLen(1);
  buildFixingCharacteristic.write16(BUILD_NOT_FIXING_NOTIFICATION);
  buildFixingCharacteristic.begin();
  
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
