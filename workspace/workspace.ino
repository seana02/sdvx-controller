
#include <Keyboard.h>

const int btnCount = 7;
const int knobCount = 2;
const bool KEYBOARD_ENABLED = true;

/**
 * 
 * Pin assignments
 * 
*/

// buttons
const int START     = 0;
const int BT_A      = 2;
const int BT_B      = 4;
const int BT_C      = 6;
const int BT_D      = 8;
const int FX_L      = 14;
const int FX_R      = 10;

// light
const int LED_ST    = 1;
const int LED_A     = 3;
const int LED_B     = 5;
const int LED_C     = 7;
const int LED_D     = 9;
const int LED_L     = 15;
const int LED_R     = 16;

const int VOL_L1    = 19;
const int VOL_L2    = 18;
const int VOL_R1    = 21;
const int VOL_R2    = 20;

// params
//const int ACTIVE_CYCLES = 20;
const int KNOB_TIME_INCREASE = 8;
const int KNOB_TIME_MAX = 40;
const int KNOB_THRESHOLD = 20;

// consts
const int CW        = 1;
const int CCW       = -1;
const int EXT       = 99;
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

/*
const int knobDirection[16] = {
      0, CCW,  CW, BAD,
     CW,   0, BAD, CCW,
    CCW, BAD,  0,   CW,
    BAD,  CW, CCW,   0
};
*/

const int knobDirection[16] = {
      0, CCW,  CW, EXT,
     CW,   0, EXT, CCW,
    CCW, EXT,   0,  CW,
    EXT,  CW, CCW,   0
};

const int btnArr[btnCount] = {START, BT_A, BT_B, BT_C, BT_D, FX_L, FX_R};
const String btnName[btnCount] = {"START", "A_BT", "B_BT", "C_BT", "D_BT", "L_FX", "R_FX"};
const int ledArr[btnCount] = {LED_ST, LED_A, LED_B, LED_C, LED_D, LED_L, LED_R};

int states[btnCount + knobCount];
int prevBtnStates[btnCount];
int knobStates[knobCount];


const char btnKeys[btnCount] = {'\\', 'w', 'a', 's', 'd', 'q', 'e'}; //fix start btn to Enter
const int specialKeys[btnCount] = {KEY_RETURN, 0, 0, 0, 0, 0, 0};

const char L_CCW    = '1';
const char L_CW     = '2';
const char R_CCW    = '9';
const char R_CW     = '0';


int lTime[2], rTime[2];


//int prevL, lState;

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

    for (int i = 0; i < btnCount; i++) {
      prevBtnStates[i] = HIGH;
    }

    // set rotary encoders as inputs
    pinMode(VOL_L1, INPUT_PULLUP);
    pinMode(VOL_L2, INPUT_PULLUP);
    pinMode(VOL_R1, INPUT_PULLUP);
    pinMode(VOL_R2, INPUT_PULLUP);

    if (KEYBOARD_ENABLED) Keyboard.begin();

    lTime[0] = lTime[1] = rTime[0] = rTime[1] = 0;
    //prevL = lState = 0;
}
int count = 0;
int prev = 0;
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
            if (KEYBOARD_ENABLED && prevBtnStates[i] == HIGH) pressIndex(i); //Keyboard.press(btnKeys[i]);
        } else {
            output += "x";
            if (KEYBOARD_ENABLED && prevBtnStates[i] == LOW) releaseIndex(i); //Keyboard.release(btnKeys[i]);
        }

        // configure LEDs
        digitalWrite(ledArr[i], !states[i]);
    }

    // output text spacer
    output += "  ";

    // read encoders: convert to format prev1prev2curr1curr
    knobStates[0] = ((knobStates[0] << 2) | (PINF >> 6)      ) % (1 << 4);
    knobStates[1] = ((knobStates[1] << 2) | ((PINF >> 4) % 4)) % (1 << 4);

    // VOL_L and VOL_R
    output += processKnob(lTime, knobStates[0], L_CW, L_CCW);
    output += processKnob(rTime, knobStates[1], R_CW, R_CCW);

    
    /*if (true || prev != (PINF >> 4) % 4) {
      Serial.print((PINF >> 4) % 4);
      if (count++ % 50 == 0) {
        Serial.println();
      }
      prev = (PINF >> 4) % 4;
    }*/

    //Serial.println(output);
    
    for (int i = 0; i < btnCount; i++) {
      prevBtnStates[i] = states[i];
    }
    
    //delay(1);
}

int clamp(int val, int min, int max) {
    if (val < min) return min;
    if (val > max) return max;
    return val;
}

char processKnob(int time[], int knobState, char CWkey, char CCWkey) {
    // update knob direction if turning
    if (knobDirection[knobState] == CCW) {
        //time[0] = ACTIVE_CYCLES;
        //time[1] = 0;
        time[0] = clamp(time[0] + KNOB_TIME_INCREASE, 0, KNOB_TIME_MAX);
        time[1] = clamp(time[1] - KNOB_TIME_INCREASE, 0, KNOB_TIME_MAX);
    } else if (knobDirection[knobState] == CW) {
        //time[1] = ACTIVE_CYCLES;
        //time[0] = 0;
        time[0] = clamp(time[0] - KNOB_TIME_INCREASE, 0, KNOB_TIME_MAX);
        time[1] = clamp(time[1] + KNOB_TIME_INCREASE, 0, KNOB_TIME_MAX);
    }

    // decide output
    char output;
    if (time[0] > KNOB_THRESHOLD) {
        output = 'L';
        if (KEYBOARD_ENABLED) {
            Keyboard.release(CWkey);
            Keyboard.press(CCWkey);
        }
    } else if (time[1] > KNOB_THRESHOLD) {
        output = 'R';
        if (KEYBOARD_ENABLED) {
            Keyboard.release(CCWkey);
            Keyboard.press(CWkey);
        }
    } else {
        output = 'x';
        if (KEYBOARD_ENABLED) {
            Keyboard.release(CCWkey);
            Keyboard.release(CWkey);
        }
    }

    // decrement active time
    time[0] -= (time[0] > 0 ? 1 : 0);
    time[1] -= (time[1] > 0 ? 1 : 0);

    return output;
    return 'x';
}

void pressIndex(int i) {
  if (btnKeys[i] == '\\') {
    Keyboard.press(specialKeys[i]);
    return;
  }

  Keyboard.press(btnKeys[i]);
}

void releaseIndex(int i) {
  if (btnKeys[i] == '\\') {
    Keyboard.release(specialKeys[i]);
    Serial.println("special key released");
    return;
  }
  
  Keyboard.release(btnKeys[i]);
}
