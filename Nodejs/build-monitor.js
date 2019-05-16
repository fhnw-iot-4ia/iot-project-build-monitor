
// This goes to or comes from mqtt
const MQTT_BUILD_PASSED = '00';
const MQTT_BUILD_FAILED = '01';
const MQTT_BUILD_FIXING_BY_SOMEONE_ELSE = '02';

// BLE status and characteristic codes
const BLE_SERVICE_UUID = '4242';
const BUILD_STATUS_CHARACTERISTIC = '2727';
const BUILD_FIXING_CHARACTERISTIC = '2728';
const BLE_BUILD_PASSED = [0x00];
const BLE_BUILD_FAILED = [0x01];
const BLE_BUILD_FIXING_BY_SOMEONE_ELSE = [0x02];

// This comes from the Adafruit as int8
const BUILD_FIXING_NOTIFICATION = 1;

const noble = require('@abandonware/noble');
const mqtt = require('mqtt');

//const broker = 'mqtt://test.mosquitto.org';
const broker = 'mqtt://broker.hivemq.com';

console.log('connecting to mqtt broker...');
const mqttClient = mqtt.connect(broker);

const characteristicUuids = [BUILD_STATUS_CHARACTERISTIC, BUILD_FIXING_CHARACTERISTIC];

let buildStatusCharacteristic = undefined;
let fixingStatusCharacteristic = undefined;

noble.on('discover', (peripheral) => {
    noble.stopScanning();
    console.log('discovered peripheral: ' + peripheral.address + ', ' + peripheral.advertisement.localName);
    peripheral.connect((error) => {
        console.log('connected to peripheral');
        peripheral.discoverServices([BLE_SERVICE_UUID], (error, services) => {
            console.log('discovered services ' + services);
            services.forEach((service) => {
                service.discoverCharacteristics(characteristicUuids, (error, characteristics) => {
                    characteristics.forEach((characteristic) => {
                        console.log('found characteristic:', characteristic.uuid);
                        if (characteristic.uuid === BUILD_STATUS_CHARACTERISTIC) {
                            buildStatusCharacteristic = characteristic;
                            console.log('recognizing ' + BUILD_STATUS_CHARACTERISTIC + ' for writing build status');
                        } else if (characteristic.uuid === BUILD_FIXING_CHARACTERISTIC) {
                            fixingStatusCharacteristic = characteristic;
                            console.log('recognizing ' + BUILD_FIXING_CHARACTERISTIC + ' for fixing build status');
                            fixingStatusCharacteristic.on('data', (data, isNotification) =>{
                                if (data.readInt8() === BUILD_FIXING_NOTIFICATION) {
                                    if (mqttClient !== undefined) {
                                        mqttClient.publish('build-monitor/build-status', MQTT_BUILD_FIXING_BY_SOMEONE_ELSE);
                                    } else {
                                        console.error('Could not publish ' + MQTT_BUILD_FIXING_BY_SOMEONE_ELSE + ': mqtt client is undefined.');
                                    }
                                }
                            });
                            fixingStatusCharacteristic.subscribe((error) => {
                                if (error) {
                                    console.log('An error occurred when subscribing to build fixing notifications: ', error);
                                } else {
                                    console.log('Subscribed to build fixing notifications.');
                                }
                            });
                        }
                    });
                });
            });
        });
    });
    peripheral.on('disconnect',() => {
        console.log('disconnected from peripheral');
        console.log('scanning for bluetooth service...');
        noble.startScanning(BLE_SERVICE_UUID);
    });
});

while(mqttClient === undefined) {
    // wait until mqttClient connected to mqtt
}

mqttClient.on('connect', () => {
    console.log('connected over mqtt');
    mqttClient.subscribe('build-monitor/build-status', error => {
        if (error) {
            console.log(error);
        }
    });
    mqttClient.on('message', (topic, message) => {
        console.log(topic + ' : ' + message.toString());
        if (buildStatusCharacteristic !== undefined) {
            let receivedState = message.toString();
            if (receivedState === MQTT_BUILD_PASSED) {
                let data = Buffer.from(BLE_BUILD_PASSED);
                buildStatusCharacteristic.write(data, true);
            } else if (receivedState === MQTT_BUILD_FAILED) {
                let data = Buffer.from(BLE_BUILD_FAILED);
                buildStatusCharacteristic.write(data, true);
            } else if (receivedState === MQTT_BUILD_FIXING_BY_SOMEONE_ELSE) {
                let data = Buffer.from(BLE_BUILD_FIXING_BY_SOMEONE_ELSE);
                buildStatusCharacteristic.write(data, true);
            }
        }
    });
    console.log('scanning for bluetooth service...');
    noble.startScanning(BLE_SERVICE_UUID);
});
