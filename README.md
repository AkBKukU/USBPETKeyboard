# USBPETKeyboard
Teensy++2.0 Code for using a Commodore PET keyboard (USBPETKeyboard.ino) and a CBM 64 (CBM64Keyboard.ino) as a USB keyboard.

## CBM 64 Keyboard
Details and pin layouts see source code. Since PET and CBM64 share the greatest part of application logic, only a few changes of the original PET keyboard driver were necessary. These affect the matrix layout and the special keys. For further information please see these sources:
* [Teensy++ 2.0](https://www.pjrc.com/store/teensypp.html)
* [Using USB Keyboard](https://www.pjrc.com/store/teensypp.html)
* Simon Inn's [great article](https://www.waitingforfriday.com/?p=470) on retrocomputing with a corrected keyboard matrix. The matrix is [here](https://www.waitingforfriday.com/wp-content/uploads/2017/01/C64_Keyboard_Schematics_PNG.png).
* Anders Tonfeldt's [video series](https://youtu.be/jlFf_2nGHVQ) with detailed instructions on how to wire CBM and Arduino (he's using different hardware, and his source code doesn't run with a Teensy, but this doesn't matter for understanding the basics).
