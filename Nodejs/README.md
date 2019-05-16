# Prerequisites
In order to get started you will need the following:
1) Have raspbian installed (preferably codename stretch, version 9.4)
2) Have node installed(preferably version 6.2.1)
3) Have noble installed (@abandonware/noble@1.9.2-2)
4) Have mqtt installed (mqtt@2.18.8)
5) Have an intact wireless connection to the internet

# Hardware
The raspberry 0 hardware has to be connected over the USB Port

# State Machine
The code is structured as a rudimentary asynchronous program. It connects to an mqtt topic and to a bluetooth low energy service, resulting in two parts:

## BLE Part
1) connect to a service if it has uuid `4242`
2) Once connected, save the characteristic `2727` for writing incoming states from mqtt and subscribe to the characteristic `2728` for receiving build fixing committment.
3) Whenever the connected ble device notifies a build fixing commitment (characteristic `2728`, value `1`) this commitment is being published over mqtt to the topic `build-monitor/build-status`

## MQTT Part
1) connect to the mqtt broker `mqtt://broker.hivemq.com`. 
2) Once connected, subscribe to the topic `build-monitor/build-status`
3) As soon as a message is received over mqtt, propagate the message to the connected Adafruit. Only the following states are valid:
** `BUILD_SUCCEEDED`:  `0x00`
** `BUILD_FAILED`: `0x01`
** `BUILD_FIXING_WAITING`: `0x02`
