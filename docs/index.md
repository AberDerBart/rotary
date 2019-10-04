# Rotary

## Preface

I got an old rotary phone from my grand aunt which I used just as a normal telephone,
as my parents telephone system supports pulse dialing.

As I moved out for studying, I did not use it anymore and it just lay dormant back in my room.
Some day, I got the idea how awesome it would be to transform it into a mobile phone.
After some poking on the internet, I found someone had build something similar, simulating button presses on a regular mobile phone by shorting out the keyboard contacts with MOSFETs and a microcontroller.

I just had to try it, so I ordered the parts neccessary and started building.
The phone worked, but there were some issues. Especially, I had no feedback from the phone and it could get stuck in some menu.

The Phone once again lay around, not being used until one day the Adafruit Feather FONA caught my eye.
This was the solution to all my problems (as it turned out, it caused a few new problems), as it integrated a microcontroller with a GSM modem and battery charging circuitry on a neat little board.

## Reading the hook and rotary dial

The telephone hook in a FeTAp 611 is pretty straightforward to read, as it is just a simple switch.
Just some simple code for debouncing is needed.

The rotary dial has two switches.
One opens when the dial is rotated away from its initial position, originally designed to disconnect the handset, so no clicking noises would be heard.
The other one closes once per digit (once for a 1, twice for a 2, ..., 10 times for a 0) at a frequency of 10Hz.
In the original hardware, this switch would short-out the telephone line to send a signal to the telephone agency.
After each digit, there is a delay of at least 0.2s before the next, indicating the digit is complete.
Within this time, the first switch closes.

Reading the digits dialed is easy as well, just wait for the first switch to open, then count the number of times the second switch closes (debouncing of course) until the first switch closes again.

## Ringing

The bell of the FeTAp 611 normaly is powered by 60V DC coming from the telephone line.
As I am not proficient with power electronics and for simplicity I did not want to integrate additional electronics for providing this voltage.
Luckily, I found a [solution](https://www.mikrocontroller.net/topic/65293) for this on mikrocontroller.net.
The topic opener did a similar project as my first approach and just glued the phones vibration motor to the clapper of the bell.
For driving the vibration motor I just used a transistor and a flyback diode.

For the future I am considering rewinding the coils of the bell, such that they can be powered with a lower voltage.

## Microphone and speaker

I simply replaced the microphone and speaker with a pair harvested from a broken PC headset.
This is a simple solution that works perfectly well, the only issue is that the handset loses its weight, making it feel less authentic, but this can be fixed by just adding some weight.

## State Machine

With the hook switch, rotary dial, bell, microphone and speaker, all basic component of the phone are there.
The missing thing is the logic controlling it.
For this purpose, I implemented a simple state machine that mimics the behaviour of the FeTAp on a phone line as closes as possible:

![state machine](state_machine.svg)

## LED backlight

One thing lacking in the original hardware ist output capabilities.
It is only possible to play some tones via the speaker in the handset or of course to ring the bell, both of which are not appealing to me.
I came up with the idea to add LEDs behind the center of the rotary dial, providing indirection lighting, as they are covered by a label in front of them.
So far, I only implemented an animation upon ringing, but further features are planned.

## Issues and throwbacks

All in all, the project did not go completely smoothly.
I had two major throwbacks, both of which resulted in a period of several month of inactivity (at least concerning rotary).

Some day, the SIM card I used broke, so I tried another one and it didn't work either, so I set the project aside.
I considered the FONA broken, too and was about to throw it away, but I wanted to try it one last time and after several hours of searching in the forum for my issue, I found the solution.
It turned out the SIM card holder has a loose fit and you have to put something behind some slimmer SIM cards for them to have proper contact.

The second issue had to do with the power supply. On the Adafruit website for the Feather FONA, they suggest using li-ion batteries with at least 500mAh capacity.
I used a 4400mAh battery and thought I was on the safe side.
However, the protection circuitry in the battery package is limited to a current of 500mA, which is not enough for the current spikes needed by the SIM800H GSM modem of the Feather FONA.
This resulted in occasional brownouts of the microcontroller.
Replacing the battery did the trick.

## 
