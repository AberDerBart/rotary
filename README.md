# ROTARY

ROTARY is an acronym for RObust Telefone ARY. It is a software for turning an Adafruit FONA and a rotary phone into a rotary mobile phone.

## Build environment setup

ROTARY is being developed with Arduino IDE. For setting up the IDE properly, install the Adafruit Arduino boards package: https://adafruit.github.io/arduino-board-index/package_adafruit_index.json
Also, you need to install the Adafruit FONA library: https://github.com/adafruit/Adafruit_FONA_Library/archive/master.zip

## Configuration

The software is configured with the definitions on top of rotary.ino

## Usage

ROTARY offer these features:

 * Call a number: Pick up the hook, dial the number and wait for a timeout, hang up when you have finished talking
 * Answer a call: When ROTARY rings, pick up the hook, talk, hang up

## TODO

 * implement ringing
 * implement ringing while PHONING/DIALING behaviour
 * implement speed dial
 * improve DIALING code
