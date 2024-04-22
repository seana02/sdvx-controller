
// for NKRO Keyboard support
#include "HID-Project.h"

const int btnCount = 7;
const int knobCount = 2;
const bool KEYBOARD_ENABLED = false;

// buttons
#define START 0
#define BT_A 1
#define BT_B 2
#define BT_C 3
#define BT_D 4
#define FX_L 5
#define FX_R 6

// light
#define LED_ST 7
#define LED_A 8
#define LED_B 9
#define LED_C 10
#define LED_D 11
#define LED_L 12
#define LED_R 13

#define VOL_L1 14
#define VOL_L2 15
#define VOL_R1 16
#define VOL_R2 17

#define CW 1
#define CCW -1
#define BAD 0
// prev1 prev2 curr1 curr2
// 1110 => CCW
// 1000 => CCW
// 0001 => CCW
// 0111 => CCW

// 1101 => CW
// 0100 => CW
// 0010 => CW
// 1011 => CW

// 0000 => 0
// 0011 => x
// 0101 => 0
// 0110 => x
// 

const int knobDirection[16] = {
      0, CCW,  CW, BAD,
     CW,   0, BAD, CCW,
    CCW, BAD,  0,   CW,
    BAD,  CW, CCW,   0
};

const int btnArr[btnCount] = {START, BT_A, BT_B, BT_C, BT_D, FX_L, FX_R};
const String btnName[btnCount] = {"START", "BT_A", "BT_B", "BT_C", "BT_D", "FX_L", "FX_R"};
const int ledArr[btnCount] = {LED_ST, LED_A, LED_B, LED_C, LED_D, LED_L, LED_R};

int states[btnCount + knobCount];
int knobStates[knobCount];

const char btnKeys[btnCount] = {'T', 'D', 'F', 'J', 'K', 'C', 'M'};
#define L_CCW 'Q'
#define L_CW  'W'
#define R_CCW 'O'
#define R_CW  'P'


void setup() {
    Serial.begin(9600);
    
    // set buttons as inputs
    for (int btn : btnArr) {
        pinMode(btn, INPUT_PULLUP);
    }
    // set LEDS as outputs
    for (int led : ledArr) {
        pinMode(led, OUTPUT);
    }

    // set rotary encoders as inputs
    pinMode(VOL_L1, INPUT_PULLUP);
    pinMode(VOL_L2, INPUT_PULLUP);
    pinMode(VOL_R1, INPUT_PULLUP);
    pinMode(VOL_R2, INPUT_PULLUP);

    if (KEYBOARD_ENABLED) NKROKeyboard.begin();
}

void loop() {
    // print button pushed state
    String output = "";
    
    // read and light buttons
    for (int i = 0; i < btnCount; i++) {
        // read inputs
        states[i] = digitalRead(btnArr[i]);

        // change output string
        if (states[i] == LOW) {
            output += btnName[i].substring(0, 1);
            if (KEYBOARD_ENABLED) NKROKeyboard.add(btnKeys[i]);
        } else {
            output += "x";
            if (KEYBOARD_ENABLED) NKROKeyboard.release(btnKeys[i]);
        }

        // configure LEDs
        digitalWrite(ledArr[i], invert(states[i]));
    }

    // output text spacer
    output += "  ";

    // read encoders
    knobStates[0] = ((knobStates[0] << 2) | (digitalRead(VOL_L1) << 1) | digitalRead(VOL_L2)) % (1 << 3);
    knobStates[1] = ((knobStates[1] << 2) | (digitalRead(VOL_R1) << 1) | digitalRead(VOL_R2)) % (1 << 3);

    // VOL_L
    if (knobDirection[knobStates[0]] == CCW) {
        output += "L";
        if (KEYBOARD_ENABLED) {
            NKROKeyboard.release(L_CW);
            NKROKeyboard.add(L_CCW);
        }
    } else if (knobDirection[knobStates[0]] == CW) {
        output += "R";
        if (KEYBOARD_ENABLED) {
            NKROKeyboard.release(L_CCW);
            NKROKeyboard.add(L_CW);
        }
    } else {
        output += "x";
        if (KEYBOARD_ENABLED) {
            NKROKeyboard.release(L_CCW);
            NKROKeyboard.release(L_CW);
        }
    }

    // VOL_R
    if (knobDirection[knobStates[1]] == CCW) {
        output += "L";
        if (KEYBOARD_ENABLED) {
            NKROKeyboard.release(R_CW);
            NKROKeyboard.add(R_CCW);
        }
    } else if (knobDirection[knobStates[1]] == CW) {
        output += "R";
        if (KEYBOARD_ENABLED) {
            NKROKeyboard.release(R_CCW);
            NKROKeyboard.add(R_CW);
        }
    } else {
        output += "x";
        if (KEYBOARD_ENABLED) {
            NKROKeyboard.release(R_CCW);
            NKROKeyboard.release(R_CW);
        }
    }

    Serial.println(output);
    if (KEYBOARD_ENABLED) NKROKeyboard.send();
    
    delay(1);
}

int invert(int x) {
    // invert HIGH to LOW or LOW to HIGH
    return x == HIGH ? LOW : HIGH;
}