const noble = require("@abandonware/noble");
const mqtt = require("mqtt");

const broker = "mqtt://test.mosquitto.org";
const client = mqtt.connect(broker);

const serviceUuid = "4242";
const characteristicUuids = ["2727", "2728"];

let buildStatusCharacteristic = undefined;
let fixingStatusCharacteristic = undefined;

noble.on("discover", (peripheral) => {
    noble.stopScanning();
    console.log(
        peripheral.address + ", " +
        peripheral.advertisement.localName + ", " +
        peripheral.advertisement.uuid);
    peripheral.connect((error) => {
        console.log("connected");
        peripheral.discoverServices([serviceUuid], (error, services) => {
            console.log("discovered services " + services);
            services.forEach((service) => {
                console.log("found service:", service.uuid);
                service.discoverCharacteristics(characteristicUuids, (error, characteristics) => {
                    characteristics.forEach((characteristic) => {
                        console.log("found characteristic:", characteristic.uuid);
                        if (characteristic.uuid === "2727") {
                            buildStatusCharacteristic = characteristic;
                            console.log("found 2727");
                        } else if (characteristic.uuid === "2728") {
                            fixingStatusCharacteristic = characteristic;
                            console.log("found 2728");
                        }
                    });
                });
            });
        });
        /*
        console.log("reading fixing status...");
        try {
            fixingStatusCharacteristic.read((error, data) => {
                console.log(data.readUInt16());
            });
        } catch(error) {
            console.log(error);
        }*/
    });
});

client.on("connect", () => {
    console.log("connected over mqtt");
    client.subscribe("build-monitor/build-status", error => {
        if (error) {
            console.log(error);
        }
    });
    client.on("message", (topic, message) => {
        console.log(topic + " : " + message.toString());
        if (buildStatusCharacteristic !== undefined) {
            let receivedState = message.toString();
            if (receivedState === '00') {
                let data = Buffer.from([0x00]);
                buildStatusCharacteristic.write(data, true);
            } else if (receivedState === '01') {
                let data = Buffer.from([0x01]);
                buildStatusCharacteristic.write(data, true);
            } else if (receivedState === '02') {
                let data = Buffer.from([0x02]);
                buildStatusCharacteristic.write(data, true);
            }
        }
    });
    console.log("scanning...");
    noble.startScanning(serviceUuid);
});
