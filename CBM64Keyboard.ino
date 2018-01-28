//
// CBM 64 Keyboard Driver (USB)
//
// For Teensy++ 2.0
//
// Layout
// 
// Connect CBM Drive Ports to Teensy++ pins:
// 
// H/13 <--> A7/45/F7
// G/14 <--> A6/44/F6
// F/15 <--> A5/43/F5
// E/16 <--> A4/42/F4
// D/17 <--> A3/41/F3
// C/18 <--> A2/40/F2
// B/19 <--> A1/39/F1
// A/20 <--> A0/38/F0
// I/ 1 <--> 18/INT6/E6 (for Restore key)
//
// Connect CBM Sense Ports to Teensy++ pins:
//
// 0/12 <--> 10/C0
// 1/11 <--> 11/C1
// 2/10 <--> 12/C2
// 3/ 9 <--> 13/C3
// 4/ 8 <--> 14/C4
// 5/ 7 <--> 15/C5
// 6/ 6 <--> 16/C6
// 7/ 5 <--> 17/C7
// 8/ 3 <-->  9/E1
//
// Please refer to matrix and pin layouts as described in 
// https://www.waitingforfriday.com/?p=470 (for CBM 64) and
// https://www.pjrc.com/store/teensypp.html
//


#include <Keyboard.h>

// Define number of each pin type (one additional for restore key (8.I)
#define DRIVE_COUNT 9
#define SENSE_COUNT 9

// pinout from keyboard to teensy
const uint8_t DRIVE_PINS[DRIVE_COUNT+1] = {10, 11, 12, 13, 14, 15, 16, 17, 9};
const uint8_t SENSE_PINS[SENSE_COUNT+1] = {A0, A1, A2, A3, A4, A5, A6, A7, 18}; // pins 38-45, 18

// teensy key codes to position with keyId 
const uint16_t KEY_CODES[DRIVE_COUNT * SENSE_COUNT] = {
  KEY_1,            KEY_3,             KEY_5, KEY_7, KEY_9, KEYPAD_PLUS,   KEY_LEFT_BRACE,          KEY_DELETE, KEY_0, // row 0
  KEY_ESC,          KEY_W,             KEY_R, KEY_Y, KEY_I, KEY_P,         KEYPAD_ASTERIX,          KEY_ENTER,  KEY_0, // row 1
  MODIFIERKEY_CTRL, KEY_A,             KEY_D, KEY_G, KEY_J, KEY_L,         KEY_SEMICOLON,           0x50,       KEY_0, // row 2
  KEY_CAPS_LOCK,    MODIFIERKEY_SHIFT, KEY_X, KEY_V, KEY_N, KEY_COMMA,     KEY_SLASH,               0x52,       KEY_0, // row 3
  KEY_SPACE,        KEY_Z,             KEY_C, KEY_B, KEY_M, KEY_PERIOD,    MODIFIERKEY_RIGHT_SHIFT, KEY_F1,     KEY_0, // row 4
  MODIFIERKEY_GUI,  KEY_S,             KEY_F, KEY_H, KEY_K, KEY_SEMICOLON, KEY_EQUAL,               KEY_F3,     KEY_0, // row 5
  KEY_Q,            KEY_E,             KEY_T, KEY_U, KEY_O, KEY_2,         KEY_BACKSLASH,           KEY_F5,     KEY_0, // row 6
  KEY_2,            KEY_4,             KEY_6, KEY_8, KEY_0, KEY_MINUS,     KEY_HOME,                KEY_F7,     KEY_0, // row 7
  KEY_TAB,          KEY_0,             KEY_0, KEY_0, KEY_0, KEY_0,         KEY_0,                   KEY_0,      KEY_0  // row 8 (restore key)
};

// determine which keys must be shifted when pressed  
// 1: apply shift, 0: don't apply, -1: ignore shift
// this table contained bool values first; 'ignore shift' is experimental
// (and doesn't work yet)
const short KEY_CODES_SHIFT_NEEDED[DRIVE_COUNT * SENSE_COUNT] = {
  // 1,       3,      4, 5, 9, +, £,       DEL,           nil
     0,       0,      0, 0, 0, 0, 1,       0,             0,
     
  // ESC,     W,      R, Y, I, P, *,       RETURN,        nil
     0,       0,      0, 0, 0, 0, 1,       0,             0,
     
  // CTRL,    A,      D, G, J, L, :,       CRSRleftright, nil
     0,       0,      0, 0, 0, 0, 0,       0,             0,
     
  // RUNSTOP, LSHIFT, X, V, N, ,, /,       CRSRUPDOWN,    nil
     0,       0,      0, 0, 0, 0, 0,       0,             0,
     
  // SPACE,   Z,      C, B, M, ., RSHIFT,  f1,            nil
     0,       0,      0, 0, 0, 0, 0,       0,             0,
     
  // CBM,     S,      F, H, K, :, =,       f3,            nil
     0,       0,      0, 0, 0, 0, 0,       0,             0,
     
  // Q,       E,      T, U, O, @, arrowup, f5,            nil
     0,       0,      0, 0, 0, 1, 0,       0,             0,
     
  // 2,       4,      6, 8, 0, -, CLRHOME, f7,            nil
     0,       0,      0, 0, 0, 0, 0,       0,             0,
     
  // RESTORE, nil...
     0,       0,      0, 0, 0, 0, 0,       0,             0
};

// keyId and duration of debounce
int8_t debounceKeys[6][2] = {{0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}};

// debounce delay
// TODO - Change from loop cycles to ms
const uint8_t DEBOUNCE_DELAY = 2;

// keys currently being pressed
int8_t usbKeys[6] = {-1,-1,-1,-1,-1,-1};
// number of keys pressed
uint8_t keysUsed = 0;

// modifiers key states
bool lShift = 0, rShift = 0, ctrl = 0, lArrow = 0, rArrow = 0, lbrace = 0, rbrace = 0;

// active modifiers to pass to library
int modifiers = 0;

// update keys state
bool keyStateChange = false;

void setup() {
  // Initilize drive pins that will sink when active
  for (uint8_t i = 0; i < DRIVE_COUNT ; i++ ) {
    pinMode(DRIVE_PINS[i], OUTPUT);
    digitalWrite(DRIVE_PINS[i], HIGH);
  }
  
  // Initilize sense pins with pullup
  for (uint8_t j = 0; j < SENSE_COUNT ; j++) {
    pinMode(SENSE_PINS[j], INPUT_PULLUP);
  }

  // Initilize Serial to relay button presses
  Serial.begin(9600);
  Serial.println("Setup Complete");
}

void loop() {
  // check for new key presses
  detectKeys();
  
  // check if key update required
  if (keyStateChange) {
    
    // check if a opposite arrow direction is intended 
    if (!(lArrow || rArrow) && (lShift || rShift)) {
      // if not then apply shift key
      modifiers = modifiers | MODIFIERKEY_SHIFT;
    }

    // clear key update and send keys
    keyStateChange = false;
    Keyboard.set_modifier(modifiers);
    Keyboard.send_now();

    // release shift key 
    modifiers = modifiers & ~MODIFIERKEY_SHIFT;
  }

  // debug that prints key information to build matrix reference
  // note: when this is active, keypresses act irregularly; for output
  // on serial console only. Comment this out when not needed.
  // printKeyPosition();
}

// check all possible pin combinations to determine key presses
void detectKeys() {
  // Check all sense pins on each drive pin for a low signal
  for (uint8_t i = 0; i < DRIVE_COUNT ; i++) {
    // Drive pin LOW to scan for inputs
    digitalWrite(DRIVE_PINS[i], LOW);
    // check sense pins
    for (uint8_t j = 0; j < SENSE_COUNT; j++) {
      // get id for current key
      uint8_t keyId = getKeyId(i,j);
      // check if id requires special handling
      if (!parseSpecial(keyId, digitalRead(SENSE_PINS[j]))) {
        // set normal key state
        if (digitalRead(SENSE_PINS[j]) == LOW) {
          setKey(keyId, 0);
        } else {
          checkRelease(keyId);
        }
      }
    }

    // Set pin HIGH to disable
    digitalWrite(DRIVE_PINS[i], HIGH);
  }
}

// check if a held key was released
// TODO - break out a removeUSBKey(uint8_t keyCode) function
void checkRelease(uint8_t keyId) {
  if( debounce(keyId) ) return; // filter actuation
  
  bool keySet = false;
  uint8_t keyNum = 0;
  // check usb keys for keyId
  for (keyNum = 0; keyNum < 6 ; keyNum++) {
    // key was set and will be removed
    if (usbKeys[keyNum] == keyId) {
      usbKeys[keyNum] = -1;
      keySet = true;
      keysUsed--;
      break;
    }
  }
  
  if (keySet) {
    // clear shift modifier if it was needed
    if (KEY_CODES_SHIFT_NEEDED[keyId] == 1) {
      modifiers = modifiers & ~MODIFIERKEY_SHIFT;
    }

    // mark key update required
    keyStateChange = true;

    // clear correct USB key
    switch (keyNum+1) {
      case 1:
        Keyboard.set_key1(0);
        break;
      case 2:
        Keyboard.set_key2(0);
        break;
      case 3:
        Keyboard.set_key3(0);
        break;
      case 4:
        Keyboard.set_key4(0);
        break;
      case 5:
        Keyboard.set_key5(0);
        break;
      case 6:
        Keyboard.set_key6(0);
        break;
    }
  }
}

// handle keys that require special actions
// TODO - breakout each key's logic into seperate functions.
bool parseSpecial(uint8_t keyId, bool state) {
  // return value that indicates a special key was handled
  bool isSpecial = false;
  
  switch (keyId) {
    case 6: // pound symbol
      if (state == LOW) {
        Serial.println("parseSpecial> £");
        if (lShift || rShift) {
          setKey(keyId,0);
        } else {
          setKey(keyId, KEY_LEFT_BRACE);
        }
      } else {
        checkRelease(keyId); 
      }
      isSpecial=true;
      break;
      
    case 7: // INST DEL
      if (state == LOW) {
        Serial.println("parseSpecial> inst del");
        if (lShift || rShift) {
          setKey(keyId, KEY_DELETE);
        } else {
          setKey(keyId, KEY_BACKSPACE);
        }
      } else {
        checkRelease(keyId); 
      }
      isSpecial=true;
      break;

    case 28: // Left shift
      // Key pressed and not active
      if (state == LOW && lShift == false) {
        Serial.println("parseSpecial> LSHIFT pressed (inactive)");
        if (debounce(keyId)) return true;
        lShift = true;
        keyStateChange = true;
      }
      
      // Key released and active
      if (state == HIGH && lShift == true) {
        if (debounce(keyId)) return true;
        lShift = false;
        keyStateChange = true;
      }

      isSpecial=true;
      break;

    case 42: // Right shift
      // Key pressed and not active
      if (state == LOW && rShift == false) {
        Serial.println("parseSpecial> RSHIFT pressed (inactive)");
        if (debounce(keyId)) return true;
        rShift = true;
        keyStateChange = true;
      }
      
      // Key released and active
      if (state == HIGH && rShift == true) {
        if (debounce(keyId)) return true;
        rShift = false;
        keyStateChange = true;
      }

      isSpecial=true;
      break;

    case 18: // Control
      // pressed
      if (state == LOW) {
        Serial.println("parseSpecial> CTRL pressed");
        if (debounce(keyId)) return true;
        // set control modifier 
        modifiers = modifiers | MODIFIERKEY_CTRL;
        keyStateChange = true;
      // released
      } else {
        if (debounce(keyId)) return true;
        // mask remove control modifier 
        modifiers = modifiers & ~MODIFIERKEY_CTRL;
        keyStateChange = true;
      }

      isSpecial=true;
      break;

    case 25: // Horizontal CRSR
      // pressed
      if (state == LOW) {
        Serial.println("parseSpecial> Cursor right/left pressed");
        // check for active shift to reverse direction
        if (lShift || rShift) {
          setKey(keyId,KEY_LEFT);
        } else {
          setKey(keyId,KEY_RIGHT);
        }
        rArrow = true;
      } else {
        checkRelease(keyId);      
        rArrow = false;
      }
      isSpecial=true;
      break;
      
    case 27: // run stop (becomes alt)
     if (state == LOW) {
        // pressed
        Serial.println("parseSpecial> run stop pressed");
        if (debounce(keyId)) return true;
        // set control modifier 
        modifiers = modifiers | MODIFIERKEY_ALT;
        keyStateChange = true; 
      } else {
        // released
        if (debounce(keyId)) return true;
        // mask remove gui modifier 
        modifiers = modifiers & ~MODIFIERKEY_ALT;
        keyStateChange = true;
      }
      isSpecial=true;
      break;

    case 34: // Vertical CRSR
      // pressed
      if (state == LOW) {
        Serial.println("parseSpecial> Cursor up/down pressed");
        // check for active shift to reverse direction
        if (lShift || rShift) {
          setKey(keyId,KEY_UP);
        } else {
          setKey(keyId,KEY_DOWN);
        }

        lArrow = true;
      } else {
        checkRelease(keyId);      
        lArrow = false;
      }
      isSpecial=true;
      break;
      
    case 45: // commodore key
      if (state == LOW) {
        // pressed
        Serial.println("parseSpecial> CBM pressed");
        if (debounce(keyId)) return true;
        // set control modifier 
        modifiers = modifiers | MODIFIERKEY_GUI;
        keyStateChange = true; 
      } else {
        // released
        if (debounce(keyId)) return true;
        // mask remove gui modifier 
        modifiers = modifiers & ~MODIFIERKEY_GUI;
        keyStateChange = true;
      }

      isSpecial=true;
      break;
      
    case 50: // :
      if (state == LOW) {
        Serial.println("parseSpecial> :");
        if (lShift || rShift) {
          // modifiers = modifiers & ~MODIFIERKEY_SHIFT;
          setKey(keyId, KEY_LEFT_BRACE);
        } else {
          modifiers = modifiers | MODIFIERKEY_SHIFT;
          setKey(keyId, KEY_SEMICOLON);
          
        }
      } else {
        modifiers = modifiers & ~MODIFIERKEY_SHIFT;
        checkRelease(keyId); 
        
      }
      isSpecial=true;
      break;
      
    case 60: // pointer up (becomes backslash)
      if (state == LOW) {
        Serial.println("parseSpecial> pointer up");
        setKey(keyId, KEY_BACKSLASH);
      } else {
        checkRelease(keyId); 
      }
      isSpecial=true;
    case 69: // clr home (becomes home + end (when shifted))
      if (state == LOW) {
        Serial.println("parseSpecial> clr home");
        if (lShift || rShift) {
          setKey(keyId, KEY_END);
        } else {
          setKey(keyId, KEY_HOME);
        }
      } else {
        checkRelease(keyId); 
      }
      isSpecial=true;
      break;
   
    case 80: // restore
      if (state == LOW) {
        Serial.println("parseSpecial> restore");
        setKey(keyId, KEY_TAB);
      } else {
        checkRelease(keyId); 
      }
      isSpecial=true;
      break;
  }

  return isSpecial;
}

// debounce the key input.
bool debounce(uint8_t keyId) {
  // for each of the 6 possible usb keys held go through each
  for (uint8_t i = 0; i < 6; i++) {
    if (debounceKeys[i][0] == keyId) {
      // keyid exists
      if (debounceKeys[i][1] != 1) {
        // key being debounced
        debounceKeys[i][1]--;
        return true;
      } else if( debounceKeys[i][1] == 1) {
        // key has finished debounce
        debounceKeys[i][1] = 0;
        return false;
      } else if( debounceKeys[i][1] == 0) {
        // reset key debounce
        debounceKeys[i][1] = DEBOUNCE_DELAY;
        return true;
      }
    }
  }

  // set new key to debounce
  for (uint8_t i = 0; i < 6; i++) {
    if (debounceKeys[i][1] == 0) {
      debounceKeys[i][0] = keyId;
      debounceKeys[i][1] = DEBOUNCE_DELAY;
      return true;
    } 
  }
}

// set key as pressed
bool setKey(uint8_t keyId, uint16_t keyCode) {
  // check if all keys are in use
  if (keysUsed == 6) return false;

  if (debounce(keyId) ) return false;

  // keyId 45 (CBM): Alt, with Shift: AltGr

  bool keySet = false;
  uint8_t keyNum = 0;
  // check if key is already set
  for (keyNum = 0; keyNum < 6 ; keyNum++) {
    if (usbKeys[keyNum] == keyId) {
      return true;
    }
  }
  
  // find free usb key and set it
  for (keyNum = 0; keyNum < 6 ; keyNum++) {
    if (usbKeys[keyNum] == -1) {
      usbKeys[keyNum] = keyId;
      keySet = true;
      keysUsed++;
      break;
    }
  }
  
  // key was set
  if (keySet) {
    // if the key needs shift to print the character set it
    if (KEY_CODES_SHIFT_NEEDED[keyId] == 1) {
      modifiers = modifiers | MODIFIERKEY_SHIFT;
    } else if (KEY_CODES_SHIFT_NEEDED[keyId] == -1) {
      Serial.println("ignore shift key");
      modifiers = modifiers & ~MODIFIERKEY_SHIFT;
    }
    keyStateChange = true;
    // get teensy key code for key
    uint16_t keyToSet = KEY_CODES[keyId];
    
    if (keyCode) {
      // override keyToSet if keyCode is given
      keyToSet = keyCode;
    }
    // set free usb key to key pressed
    switch (keyNum+1) {
      case 1:
        Keyboard.set_key1(keyToSet);
        break;
      case 2:
        Keyboard.set_key2(keyToSet);
        break;
      case 3:
        Keyboard.set_key3(keyToSet);
        break;
      case 4:
        Keyboard.set_key4(keyToSet);
        break;
      case 5:
        Keyboard.set_key5(keyToSet);
        break;
      case 6:
        Keyboard.set_key6(keyToSet);
        break;
    }
  }

  return keySet;
}

// get the id from the row and column
// TODO - probably safe to always inline this or convert it to a macro
uint8_t getKeyId(uint8_t i, uint8_t j) {
  return (i*SENSE_COUNT) + (j);
}

// debug to plot the matrix
void printKeyPosition() {
  // Check all sense pins on each drive pin for a low signal
  for (uint8_t i = 0; i < DRIVE_COUNT ; i++) {
    // Drive pin LOW to scan for inputs
    digitalWrite(DRIVE_PINS[i], LOW);
    delay(10);
    for (uint8_t j = 0; j < SENSE_COUNT ; j++) {
      if (digitalRead(SENSE_PINS[j]) == LOW) {
        // Button Pressed
        Serial.print("i: ");
        Serial.print(i+1);
        Serial.print(" ,k: ");
        char out = 1;
        switch (j) {
         case 0: 
          out = 'A';
          break;
         case 1: 
          out = 'B';
          break;
         case 2: 
          out = 'C';
          break;
         case 3: 
          out = 'D';
          break;
         case 4: 
          out = 'E';
          break;
         case 5: 
          out = 'F';
          break;
         case 6: 
          out = 'G';
          break;
         case 7: 
          out = 'H';
          break;
         case 8:
          out = 'I';
          break;
        }
        Serial.print(out);
        Serial.print(" ,keyId: ");
        Serial.print(getKeyId(i,j));
        Serial.print(" ,keyId (hex): ");
        Serial.println(getKeyId(i,j), HEX);
      }
    }
    digitalWrite(DRIVE_PINS[i], HIGH);
  }
}

