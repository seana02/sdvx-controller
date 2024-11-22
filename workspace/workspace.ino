
#include <Keyboard.h>

const int btnCount = 7;
const int knobCount = 2;
const bool KEYBOARD_ENABLED = true;
const bool debugEnabled = true;


//            Pin assignments
//            Btn          LED
//const int START  =  0, LED_ST =  1;
//const int BT_A   =  2, LED_A  =  3;
const int START  = 21, LED_ST = 20;
const int BT_A   = 19, LED_A  = 18;
const int BT_B   =  4, LED_B  =  5;
const int BT_C   =  6, LED_C  =  7;
const int BT_D   =  8, LED_D  =  9;
const int FX_L   = 14, LED_L  = 15;
const int FX_R   = 10, LED_R  = 16;

//const int VOL_L1 = 19, VOL_L2 = 18;
//const int VOL_R1 = 21, VOL_R2 = 20;
const int VOL_L1 =  1, VOL_L2 =  0;
const int VOL_R1 =  2, VOL_R2 =  3;

// params
// divisor; lower is more sensitive
const int KNOB_SENSITIVITY = 1;

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

    attachInterrupt(digitalPinToInterrupt(VOL_L1), intL1, RISING);
    attachInterrupt(digitalPinToInterrupt(VOL_L2), intL2, RISING);
    attachInterrupt(digitalPinToInterrupt(VOL_R1), intR1, RISING);
    attachInterrupt(digitalPinToInterrupt(VOL_R2), intR2, RISING);

    if (KEYBOARD_ENABLED) Keyboard.begin();

    lTime[0] = lTime[1] = rTime[0] = rTime[1] = 0;
}

void loop() {
    // read and light buttons
    for (int i = 0; i < btnCount; i++) {
        // read inputs
        states[i] = digitalRead(btnArr[i]);

        if (states[i] == LOW) {
            if (KEYBOARD_ENABLED && prevBtnStates[i] == HIGH) pressIndex(i); //Keyboard.press(btnKeys[i]);
        } else {
            if (KEYBOARD_ENABLED && prevBtnStates[i] == LOW) releaseIndex(i); //Keyboard.release(btnKeys[i]);
        }

        // configure LEDs
        digitalWrite(ledArr[i], !states[i]);
    }

    // check VOL_L and VOL_R
    updateKnobs(lTime, L_CCW, L_CW);
    updateKnobs(rTime, R_CCW, R_CW);  

    lTime[0] = clamp(lTime[0] - 1, 0, 9);
    lTime[1] = clamp(lTime[1] - 1, 0, 9);
    rTime[0] = clamp(rTime[0] - 1, 0, 9);
    rTime[1] = clamp(rTime[1] - 1, 0, 9);

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

void updateKnobs(int time[], char CCW, char CW) {
  if (time[0] / KNOB_SENSITIVITY > 0) {
      Keyboard.press(CCW);
      Keyboard.release(CW);
    } else if (time[1] / KNOB_SENSITIVITY > 0) {
      Keyboard.release(CCW);
      Keyboard.press(CW);
    } else {
      Keyboard.release(CCW);
      Keyboard.release(CW);
    }
}

void intL1() { interruptHandler(lTime, VOL_L2, 1, "   L->", " <-L  "); }
void intL2() { interruptHandler(lTime, VOL_L1, 0, " <-L  ", "   L->"); }
void intR1() { interruptHandler(rTime, VOL_R2, 1, "   R->", " <-R  "); }
void intR2() { interruptHandler(rTime, VOL_R1, 0, " <-R  ", "   R->"); }

int count = 0;
void interruptHandler(int time[], int toTest, int dir, String debugString, String debugAlt) {
  if (digitalRead(toTest) == LOW) {
    time[dir] = 1;
    time[1-dir] = 0;
    if (debugEnabled) {
      count += 2*dir-1;
      Serial.print(count);
      Serial.println(debugString);
    }
  } else {
    time[1-dir] = 1;
    time[dir] = 0;
    if (debugEnabled) {
      count -= 2*dir-1;
      Serial.print(count);
      Serial.println(debugAlt);
    }
  }
}
/*
void intL1() {
  // L1 just rose
  if (digitalRead(VOL_L2) == LOW) {
    lTime[1] = 1*KNOB_SENSITIVITY;
    lTime[0] = 0;
    if (debugEnabled) {
      count++;
      Serial.print(count);
      Serial.println("   L->");
    }
  } else {
    lTime[0] = 1*KNOB_SENSITIVITY;
    lTime[1] = 0;
    if (debugEnabled) {
      count--;
      Serial.print(count);
      Serial.println(" <-L  ");
    }
  }
}

void intL2() {
  // L2 just rose
  if (digitalRead(VOL_L1) == LOW) {
    lTime[0] = 1*KNOB_SENSITIVITY;
    lTime[1] = 0;
    if (debugEnabled) {
      count--;
      Serial.print(count);
      Serial.println(" <-L  ");
    }
  } else {
    lTime[1] = 1*KNOB_SENSITIVITY;
    lTime[0] = 0;
    if (debugEnabled) {
      count++;
      Serial.print(count);
      Serial.println("   L->");
    }
  }
}

void intR1() {
  // R1 just rose
  if (digitalRead(VOL_R2) == LOW) {
    rTime[1] = 1*KNOB_SENSITIVITY;
    rTime[0] = 0;
    Serial.println("  R->");
  } else {
    rTime[0] = 1*KNOB_SENSITIVITY;
    rTime[1] = 0;
    Serial.println("<-R  ");
  }
}

void intR2() {
  // R2 just rose
  if (digitalRead(VOL_R1) == LOW) {
    rTime[0] = 1*KNOB_SENSITIVITY;
    rTime[1] = 0;
    Serial.println("<-R  ");
  } else {
    rTime[1] = 1*KNOB_SENSITIVITY;
    rTime[0] = 0;
    Serial.println("  R->");
  }
}
*/
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
