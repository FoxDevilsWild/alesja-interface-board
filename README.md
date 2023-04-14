# alesja-interface-board
Simple controller to operate multiple devices by one user input.

## Hardware

* Thrid-Party Arduino Nano
* Relays developer board - 8x
* **Input**: 1 button - `Pin 3`
* **Output**: 8 jack sockets - `Pin 4-11`

## Set-up
1. Download and install [Arduino-IDE](https://www.arduino.cc/en/software)
2. Load `.ino` file
3. Connect Arduino with PC
4. Select Serial-Port
5. Select Board (Nano)
6. Select old bootloader: Tools > Processor > ATmega328p (Old Bootloader)

## Input-to-Output
![InputOutputImage](./rsrc/Input-Output-Example.png "InputOutput")