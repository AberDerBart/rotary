# Serial Communciation

rotary can interact with a computer via the serial connection over USB.
Each command is terminated with a line break (\n) except for AT commands which are terminated by a carriage return (\r).
Some commands have additional arguments that are provided line break separated.
The following commands are available:

## AT\* -- AT command passthrough

All AT commands are passed through to the SIM800H module.
For available AT commands, see the SIM800 Series AT command manual.

## s -- speeddial

Set a speed dial number.
The first argument is the digit that the speed dial is assigned to.
The second argument is the phone number that is assigned to the digit.

## h -- help

Print a help message.
