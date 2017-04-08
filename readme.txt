RFRTS
This is a arduino build of an Somfy RTS remote with a serial interface.
The serial interface is made to work as the RFLINK to interact with Domoticz home automation server.

HARDWARE:
Arduino Nano
Cheap 433.93 MHz Transmitter
433.43MHz sawtooth generator as replacement for the cheap transmitter.
Antenna

Connect Power to 5Vpin, GND and the DATA pin to pin 5 on Arduino.
The power can be up to 12V.



COMMAND SYNTAX:
10;RTS;<BLIND ADDRESS>;<DATA>;<COMMAND>;

OPEN BLINDS (UP)
10;RTS;121300;0;UP;
or
10;RTS;121300;0;ON;

CLOSE BLINDS (DOWN)
10;RTS;121300;0;DOWN; 
or 
10;RTS;121300;0;OFF;

STOP BLINDS (the MY-Button on the remote)
10;RTS;121300;0;STOP; 
or 
10;RTS;121300;0;STOP;

ENTER PROGRAMMING MODE:
10;RTS;121300;0;MODEPROG;
or
10;RTS;121300;0;MODEPAIR;
(Blinds will go up/down)

EXIT PROGRAMMING MODE:
10;RTS;121300;0;PROG; 
or
10;RTS;121300;0;PAIR;
(Blinds will go up/down)


TO PROGRAMM NEW BLINDS
enter the program mode on the blinds, e.g press the program button on the remote for a longer time until the blinds will wipe up/down
then enter on the serial console:
10;RTS;<BLIND ADDRESS>;<ROLLING CODE>,PAIR
or
10;RTS;<BLIND ADDRESS>;<ROLLING CODE>,PROG

BIND ADDRESS......0x000000 to 0xffffff, however i've tried aa0000-aa0010, aa0100-aa010b and 121300-121318 so far.
ROLLING CODE......0x0000 to 0xffff, i used many different codes below 0x0fff.

e.g. 10;RTS;121300;0432;PROG;

After entering the above command, the blinds should wipe up/down again, otherwise they didn't learn the code sent by RFRTS.

The blind address can be a random number, but it has to be unique in your system. 
Equal addresses for different blinds cause all blinds with the same address to move at the same time.


Additional commands (for debugging):
On serial console you can enter:

DUMP
this is for showing EEPROM content. You'll get an output similar to this:

EEProm Size = 400h Bytes = 1024 Bytes

       0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
000   12 00 FF FF FF FF FF FF FF FF FF FF FF FF FF FF
010   00 13 12 00 FF FF C0 00 FF FF FF FF FF FF FF FF
020   10 13 12 00 FF FF B4 00 FF FF FF FF FF FF FF FF
030   01 13 12 00 FF FF 15 01 FF FF FF FF FF FF FF FF
040   02 13 12 00 FF FF 25 02 FF FF FF FF FF FF FF FF
050   03 13 12 00 FF FF 37 03 FF FF FF FF FF FF FF FF
060   04 13 12 00 FF FF 48 04 FF FF FF FF FF FF FF FF
070   05 13 12 00 FF FF 58 05 FF FF FF FF FF FF FF FF
080   06 13 12 00 FF FF 6A 06 FF FF FF FF FF FF FF FF
090   07 13 12 00 FF FF 7B 07 FF FF FF FF FF FF FF FF
0A0   12 13 12 00 FF FF 59 06 FF FF FF FF FF FF FF FF
0B0   13 13 12 00 FF FF 85 09 FF FF FF FF FF FF FF FF
0C0   14 13 12 00 FF FF 45 02 FF FF FF FF FF FF FF FF
0D0   15 13 12 00 FF FF 59 06 FF FF FF FF FF FF FF FF
0E0   11 13 12 00 FF FF 25 04 FF FF FF FF FF FF FF FF
0F0   09 13 12 00 FF FF 45 06 FF FF FF FF FF FF FF FF
100   08 13 12 00 FF FF 38 02 FF FF FF FF FF FF FF FF
110   17 13 12 00 FF FF 59 02 FF FF FF FF FF FF FF FF
120   18 13 12 00 FF FF 35 08 FF FF FF FF FF FF FF FF
130   FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF

Reading finished.

DELETE
is not working yet. In future version you can delete entries in the eeprom, for e.g. when pairing has failed.


