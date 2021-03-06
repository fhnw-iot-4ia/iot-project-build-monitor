# IoT Engineering
## Project Build Monitor

## Introduction
This project is part of the [IoT Engineering](../../../fhnw-iot) course.

* 2-person teams, building an IoT system.
* 32 hours of work per person, 1 prototype.
* 10' presentation of the project at Demo Day.
* Slides, source code and setup steps on GitHub.
* Both team members are able to explain the project.

### Team members
* @Digitalninja01, Alessandro Calcagno
* @keniseli, Ken Iseli

## Deliverables
The following deliverables are mandatory.

### Source code
Source code, Arduino C, JS, committed to (this) project repo:

1) [Embedded code](Arduino/build-monitor/build-monitor.ino)
2) [Glue Code used on the gateway](Nodejs/build-monitor.js) 
3) [Glue Code used in the cloud](GCP/Index.js) 

### Presentation
4-slide presentation, PDF format, committed to (this) project repo.

[build-monitor-presentation.pdf](Documentation/build-monitor-presentation.pdf)

1) Use-case of your project.
2) Reference model of your project.
3) Single slide interface documentation.
4) Issues you faced, how you solved them.

### Live demo
Working end-to-end prototype, "device to cloud", part of your 10' presentation.

* [Jenkins](https://34.65.62.232) - no certificates in place
* mqtt broker: [HiveMQ](broker.hivemq.com) topic: build-monitor/build-status

1) Sensor input on a IoT device triggers an event.
2) The event or measurement shows up online, in an app or Web client.
3) The event triggers actuator output on the same or on a separate IoT device.

## Submission deadline
Commit and push to (this) project repo before Demo Day, _03.06.2019, 00:00_.
