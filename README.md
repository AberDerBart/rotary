# Rotary

Rotary is a software for turning an Adafruit FONA and a rotary phone into a rotary mobile phone.

## Build environment setup

Rotary is being developed with Arduino IDE. For setting up the IDE properly, install the Adafruit Arduino boards package: https://adafruit.github.io/arduino-board-index/package_adafruit_index.json
Also, you need to install the Adafruit FONA library: https://github.com/adafruit/Adafruit_FONA_Library/

## Configuration

The software is configured with the definitions in config.h

## Usage

Rotary offers these features:

 * Call a number: Pick up the hook, dial the number and wait for a timeout, hang up when you have finished talking
 * Answer a call: When Rotary rings, pick up the hook, talk, hang up

## Planned Features

 * Speed dial
 * Indication of status information (battery, network status)
