#define DRIVE_COUNT 10
#define SENSE_COUNT 8

const uint8_t DRIVE_PINS[DRIVE_COUNT] = {9,8,7,6,5,4,3,2,1,0};
const uint8_t SENSE_PINS[SENSE_COUNT] = {17,16,15,14,13,12,11,10};

void printKeyPosition();

/*
const char KEY_CODES[DRIVE_COUNT][SENSE_COUNT] = {

{ '!' , '#' , '%' , '&' , '(' , '<' , 'CLR HOME	SRSR <>
“ $ ' \ )	 CRSR/\ \/ INST DEL
q e t u o /\	7 9
w r y I p	 8 /
a d g j l	 4 6
s f h k :	 5 *
z c b m ; RETURN	1 3
x v n , ?	 2 +
LSHIFT	@ ]	 > RSHIFT	0 -
OFF RVS [ SPACE < RUN STOP		. =

}
*/

const uint16_t KEY_CODES[DRIVE_COUNT * SENSE_COUNT] = 
{
	KEY_1,KEY_3,KEY_5,KEY_7,KEY_9,KEY_TAB,KEY_0,KEY_0,
	KEY_QUOTE,KEY_4,KEY_QUOTE,KEY_BACKSLASH,KEY_0,KEY_0,KEY_0,KEY_BACKSPACE,
	KEY_Q,KEY_E,KEY_T,KEY_U,KEY_O,KEY_0,KEYPAD_7,KEYPAD_9,
	KEY_W,KEY_R,KEY_Y,KEY_I,KEY_P,KEY_0,KEYPAD_8,KEYPAD_SLASH,
	KEY_A,KEY_D,KEY_G,KEY_J,KEY_L,KEY_0,KEYPAD_4,KEYPAD_6,
	KEY_S,KEY_F,KEY_H,KEY_K,KEY_SEMICOLON,KEY_0,KEYPAD_5,KEYPAD_ASTERIX,
	KEY_Z,KEY_C,KEY_B,KEY_M,KEY_SEMICOLON,KEY_ENTER,KEYPAD_1,KEYPAD_3,
	KEY_X,KEY_V,KEY_N,KEY_COMMA,KEY_SLASH,KEY_0,KEYPAD_2,KEYPAD_PLUS,
	KEY_0,KEY_2,KEY_RIGHT_BRACE,KEY_0,KEY_PERIOD,KEY_0,KEYPAD_0,KEYPAD_MINUS,
	KEY_0,KEY_LEFT_BRACE,KEY_SPACE,KEY_COMMA,KEY_ESC,KEY_0,KEYPAD_PERIOD,KEY_EQUAL
};

const bool KEY_CODES_SHIFT_NEEDED[DRIVE_COUNT * SENSE_COUNT] = 
{
	1,1,1,1,1,0,0,0,
	1,1,0,0,1,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,1,0,0,0,
	0,0,0,0,0,0,0,0,
	0,0,0,0,1,0,0,0,
	0,1,0,0,1,0,0,0,
	0,0,0,1,0,0,0,0
	
};

int8_t debounceKeys[6][2] = 
{
	{0,0},
	{0,0},
	{0,0},
	{0,0},
	{0,0},
	{0,0}
};
const uint8_t DEBOUNCE_DELAY = 2;

int8_t usbKeys[6] = {-1,-1,-1,-1,-1,-1};
uint8_t keysUsed = 0;

bool lShift = 0, rShift = 0, ctrl = 0, lArrow = 0, rArrow = 0;

int modifiers = 0;
bool keyStateChange = false;

void setup() 
{
	// Initilize drive pins that will sink when active
	for ( uint8_t i = 0; i < DRIVE_COUNT ; i++ )
	{
		pinMode(DRIVE_PINS[i], OUTPUT);
		digitalWrite(DRIVE_PINS[i], HIGH);
	}

	// Initilize sense pins with pullup
	for ( uint8_t j = 0; j < SENSE_COUNT ; j++ )
	{
		pinMode(SENSE_PINS[j], INPUT_PULLUP);
	}

	// Initilize Serial to relay button presses
	/*
	Serial.begin(9600);
	Serial.println("Setup Complete");
	*/
}

void loop() 
{
	detectKeys();
	
	if(keyStateChange)
	{
		if (!(lArrow || rArrow) && (lShift || rShift))
		{
			modifiers = modifiers | MODIFIERKEY_SHIFT;
		}
		keyStateChange = false;
		Keyboard.set_modifier(modifiers);
		Keyboard.send_now();
		modifiers = modifiers & ~MODIFIERKEY_SHIFT;
	}
	//printKeyPosition();
}

void detectKeys()
{
	// Check all sense pins on each drive pin for a low signal
	for ( uint8_t i = 0; i < DRIVE_COUNT ; i++ )
	{
		// Drive pin LOW to scan for inputs
		digitalWrite(DRIVE_PINS[i], LOW);
		//delay(10);
		for ( uint8_t j = 0; j < SENSE_COUNT ; j++ )
		{
			uint8_t keyId = getKeyId(i,j);
			if(!parseSpecial(keyId, digitalRead(SENSE_PINS[j])))
			{
				if( digitalRead(SENSE_PINS[j]) == LOW)
				{
					setKey(keyId,0);
				}else{
					checkRelease(keyId);
				}
			}
		}

		digitalWrite(DRIVE_PINS[i], HIGH);
	}
}

void checkRelease(uint8_t keyId)
{
	if( debounce(keyId) )	return;

	bool keySet = false;
	uint8_t keyNum = 0;
	for( keyNum = 0; keyNum < 6 ; keyNum++)
	{
		if(usbKeys[keyNum] == keyId)
		{
			usbKeys[keyNum] = -1;
			keySet = true;
			keysUsed--;
			break;
		}
	}
	
	if(keySet)
	{
		if(KEY_CODES_SHIFT_NEEDED[keyId])
		{
			modifiers = modifiers & ~MODIFIERKEY_SHIFT;
		}
		keyStateChange = true;
		switch(keyNum+1)
		{
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

bool parseSpecial(uint8_t keyId, bool state)
{
	bool isSpecial = false;
	
	switch(keyId)
	{
		case 64:
			if (state == LOW && lShift == false) 
			{
				if( debounce(keyId) )	return true;
				lShift = true;
				keyStateChange = true;
			}
			
			if (state == HIGH && lShift == true) 
			{
				if( debounce(keyId) )	return true;
				lShift = false;
				keyStateChange = true;
			}

			isSpecial=true;
			break;
		case 69:
			if (state == LOW && rShift == false) 
			{
				if( debounce(keyId) )	return true;
				rShift = true;
				keyStateChange = true;
			}
			
			if (state == HIGH && rShift == true) 
			{
				if( debounce(keyId) )	return true;
				rShift = false;
				keyStateChange = true;
			}

			isSpecial=true;
			break;
		case 72:
			if ( state == LOW )
			{
				if( debounce(keyId) )	return true;
			
				modifiers = modifiers | MODIFIERKEY_CTRL;
				keyStateChange = true;
			}else{
				if( debounce(keyId) )	return true;

				modifiers = modifiers & ~MODIFIERKEY_CTRL;
				keyStateChange = true;
			}
			isSpecial=true;
			break;
		case 7: // Horizontal CRSR
			
			if(state == LOW)
			{
				if (lShift || rShift)
				{
					setKey(keyId,KEY_LEFT);
				}else{
					setKey(keyId,KEY_RIGHT);
				}
				rArrow = true;
			}else{
				checkRelease(keyId);			
				rArrow = false;
			}
			isSpecial=true;
			break;
		case 14: // Vertical CRSR
			if(state == LOW)
			{
				if (lShift || rShift)
				{
					setKey(keyId,KEY_UP);
				}else{
					setKey(keyId,KEY_DOWN);
				}

				lArrow = true;
			}else{
				checkRelease(keyId);			
				lArrow = false;
			}
			

			isSpecial=true;
			break;
	}

	return isSpecial;
}

bool debounce(uint8_t keyId)
{
	for ( uint8_t i = 0; i < 6; i++ )
	{
		if( debounceKeys[i][0] == keyId )
		{
			if ( debounceKeys[i][1] != 1 )
			{
				debounceKeys[i][1]--;
				return true;
			}
			else if( debounceKeys[i][1] == 1 )
			{
				debounceKeys[i][1] = 0;
				return false;
			}

			else if( debounceKeys[i][1] == 0 )
			{
				debounceKeys[i][1] = DEBOUNCE_DELAY;
				return true;
			}
		}
	}

	for ( uint8_t i = 0; i < 6; i++ )
	{
		if( debounceKeys[i][1] == 0 )
		{
			debounceKeys[i][0] = keyId;
			debounceKeys[i][1] = DEBOUNCE_DELAY;
			return true;
		}	
	}
}
bool setKey(uint8_t keyId, uint16_t keyCode )
{
	// Make sure all keys are in use
	if(keysUsed == 6) return false;

	if( debounce(keyId) )	return false;
	bool keySet = false;
	
	uint8_t keyNum = 0;
	
	for( keyNum = 0; keyNum < 6 ; keyNum++)
	{
		if(usbKeys[keyNum] == keyId)
		{
			return true;
		}
	}
	
	for( keyNum = 0; keyNum < 6 ; keyNum++)
	{
		if(usbKeys[keyNum] == -1)
		{
			usbKeys[keyNum] = keyId;
			keySet =true;
			keysUsed++;
			break;
		}
	}

	if(keySet)
	{
		if(KEY_CODES_SHIFT_NEEDED[keyId])
		{
			modifiers = modifiers | MODIFIERKEY_SHIFT;
		}
		keyStateChange = true;
		uint16_t keyToSet = KEY_CODES[keyId];
		if ( keyCode )
		{
			keyToSet = keyCode;
		}
		switch(keyNum+1)
		{
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

uint8_t getKeyId(uint8_t i, uint8_t j)
{
	return (i*8) + (j);
}

void printKeyPosition()
{
	// Check all sense pins on each drive pin for a low signal
	for ( uint8_t i = 0; i < DRIVE_COUNT ; i++ )
	{
		// Drive pin LOW to scan for inputs
		digitalWrite(DRIVE_PINS[i], LOW);
		delay(10);
		for ( uint8_t j = 0; j < SENSE_COUNT ; j++ )
		{
			if( digitalRead(SENSE_PINS[j]) == LOW)
			{
				// Button Pressed
				Serial.print("i: ");
				Serial.print(i+1);
				Serial.print(" ,k: ");
				char out = 1;
				switch(j)
				{
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
				}
				Serial.print(out);
				Serial.print(" ,keyId: ");
				Serial.println(getKeyId(i,j));
			}
		}
		digitalWrite(DRIVE_PINS[i], HIGH);
	}
}

