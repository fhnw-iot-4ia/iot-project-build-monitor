# Development Guide
## Prerequisites
In order to get started you will need the following:
1) Have Arduino installed (preferably version 1.8.8)
2) Have the board Adafruit nRF52840 installed
3) Have the [Chainable LED library](https://github.com/pjpmarques/ChainableLED) installed. It is also present in the lib directory. Follow [this guide](http://wiki.seeedstudio.com/How_to_install_Arduino_Library/)

## Hardware
The hardware is wired according to the following table:

Hardware component | Purpose | Wiring
------------------ | ------- | ------
Adafruit nRF52840  | Main component where the state machine runs on. Must be BLE capable | Needs to be connected to a power source and put on top of the Grove Shield
Grove Shield for Feather | Adapter for sensors and actuators | -
Chainable LED | Gives visual feedback on the build status | `D4/D5` on Grove Shield
Buzzer | Gives acoustic feedback if the build failed | `D2` on Grove Shield
Button | Used to commit fixing of failed build | `A0`

## State Machine
The code is structured as a time machine. The main state is an int which can take on of the following states:

State Name | Triggered by | characteristic uuid `0x2728` (readable) | Buzzer | Light Bulb | Description 
---------- | ------------ | ------------------------------------------- | ------ | ---------- | -----------
`BUILD_SUCCEEDED`  | `0x00` on characteristic uuid `0x2727` | `0x00` | - | Green | Indicates that the las build was successful
`BUILD_FAILED` | `0x01` on characteristic uuid `0x2727` | `0x00` | On | Red blue blinking | Indicates that the last build failed
`BUILD_FIXING` | Physical button pressed | `0x01` | - | Different yellow blinking | Indicates that the user is working on the build (all other build monitors are in the state `BUILD_FIXING_WAITING`)
`BUILD_FIXING_WAITING` | `0x02` characteristic uuid `0x2727` | `0x00` | - | Different purple blinking | Indicates that someone else is working on the build (one other build monitor is in the state `BUILD_FIXING`)
`ADVERTISING`  | Initialization procedure on startup | `0x00` | - | Blue blinking | Indicates that build monitor waits for a central to connect
