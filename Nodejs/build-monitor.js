const noble = require("@abandonware/noble");

const serviceUuid = "4242";
const buildStatusCharacteristicUuid = ["2727"];
const fixingStatusCharacteristicUuid = ["2728"];

let buildStatusCharacteristic = undefined;
let fixingStatusCharacteristic = undefined;

noble.on("discover", (peripheral) => {
    noble.stopScanning();
    console.log(
        peripheral.address + ", " +
        peripheral.advertisement.localName + ", " +
        peripheral.advertisement.uuid);
    peripheral.connect((err) => {
        console.log("connected");
        peripheral.discoverServices([serviceUuid], (err, services) => {
            console.log("discovered services " + services);
            services.forEach((service) => {
                console.log("found service:", service.uuid);
                service.discoverCharacteristics(buildStatusCharacteristicUuid, (err, characteristics) => {
                    characteristics.forEach((characteristic) => {
                        console.log("found build status characteristic:", characteristic.uuid);
                        buildStatusCharacteristic = characteristic;
                        console.log("writing 0x01...");
                        let data = Buffer.from([0x01]);
                        buildStatusCharacteristic.write(data, true);
                        setTimeout(() => {
                            console.log("writing 0x02...");
                            data = Buffer.from([0x02]);
                            buildStatusCharacteristic.write(data, true);
                            setTimeout(() => {
                                console.log("writing 0x00...");
                                data = Buffer.from([0x00]);
                                buildStatusCharacteristic.write(data, true);
                            }, 2000);
                        }, 2000);
                    });
                });
                /*service.discoverCharacteristics(fixingStatusCharacteristicUuid, (err, characteristics) => {
                    characteristics.forEach((characteristic) => {
                        console.log("found fixing build characteristic:", characteristic.uuid);
                        fixingStatusCharacteristic = characteristic;
                        try {
                            fixingStatusCharacteristic.read((error, data) => {
                                console.log(data.readUInt8());
                            });
                        } catch(error) {
                            console.log(error);
                        }
                    });
                });*/

                /*characteristics.forEach((characteristic) => {
                    console.log("found characteristic:", characteristic.uuid);
                    characteristic.write((error, data) => {
                        const value = data.readUInt8(0);
                        console.log("read characteristic value:", value);
                        peripheral.disconnect((err) => {
                            console.log("disconnected");
                            process.exit();
                        });
                    });
                });*/
            });
        });
    });
});

console.log("scanning...");
noble.startScanning(serviceUuid);
