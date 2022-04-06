readme

:Author: groshank
:Email: {BulletProofHooker@gmail.com}
:Date: 02/09/2020
:Revision: 1.1

= Project: RC Boat Replacement Guts

Removed RC boat's control system from the transmiter and receiver and replaced them with a pair of Adafruit Feathers with built-in 900 MHz LoRa radios.
Also replaced the old 9v battery and AA batteries with rechargable LiPo batteries and dedicated chargers for the transmiter and receiver.

== Steps
Cut out old boat's receiver's guts.
Install Feather in boat and connect H-bridge chip to propeller and rudder motors.
Stuff LiPo battery, charger, and extra USB-micro female port to the boat.
Remove old transmiter's guts.
Install feather into transmitter's body.
Install new potentiometers and power switch.
Install LiPo battery, charging circuit, and USB-micro female port.
Using Arduino's IDE push firmware to both Feathers.
Find water to play in.
Play in the water.

=== Contributing
To contribute to this project please contact groshank https://id.arduino.cc/groshank

=== BOM
Add the bill of the materials you need for this project.

|===
| ID   | Part name                 | Part number | Quantity
| F1-2 | Feather with LoRa Radio   | 3178        | 2
| N1   | NeoPixel LED              | 1260        | 5
| H1   | Dual H-Bridge Motor Driver| 807         | 1
| P1   | 100K potentiometer        | 1831        | 1
| P2   | 10K potentiometer         | 356         | 1
| S1   | SPDT switch               | 805         | 1
| all part numbers are for Adafruit store.
|===

Websites that helped...
https://learn.adafruit.com/adafruit-arduino-lesson-15-dc-motor-reversing/lm293d
site that describes the pinout and function of the L293D chip (H-Bridge motor thing.)
https://learn.adafruit.com/adafruit-feather-m0-radio-with-lora-radio-module/pinouts
feather M0 radio with LoRa pinouts and description.
https://cdn-learn.adafruit.com/assets/assets/000/046/254/original/feather_Feather_M0_LoRa_v1.2-1.png?1504886587
feather pinout
https://learn.adafruit.com/adafruit-arduino-lesson-13-dc-motors/breadboard-layout
motor on arduino pins
https://www.build-electronic-circuits.com/h-bridge/
h bridge with transistors
https://learn.adafruit.com/adafruit-arduino-lesson-15-dc-motor-reversing
potentiometer and button help
https://www.onsemi.com/pub/Collateral/PN2222A-D.pdf
pn2222a datasheet (transistor used, NPN)
https://www.build-electronic-circuits.com/h-bridge/
h bridge with transistors

=== Help
This document is written in the _AsciiDoc_ format, a markup language to describe documents.
If you need help you can search the http://www.methods.co.nz/asciidoc[AsciiDoc homepage]
or consult the http://powerman.name/doc/asciidoc[AsciiDoc cheatsheet]
