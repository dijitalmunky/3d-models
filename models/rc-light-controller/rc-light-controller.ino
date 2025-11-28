#include <Arduino.h>
#include <pt.h>  // Install "Protothreads" library

// Pin definitions
const int STEERING_PIN = 2;
const int THROTTLE_PIN = 3;
const int LIGHTING_PIN = 4;

const int RED_PIN = 9;
const int GREEN_PIN = 10;
const int BLUE_PIN = 11;

const int HEADLIGHT_PIN = 7;
const int LEFT_TAILLIGHT_PIN = 5;
const int RIGHT_TAILLIGHT_PIN = 6;

// Timing parameters
const unsigned long LIGHT_UPDATE_INTERVAL = 15;  // ms between updates
const unsigned long SIGNAL_BLINK_INTERVAL = 500;  // ms between signal blinks

const int COLOR_STEP_SIZE = 1;                 // fade increment

// Protothread structure
static struct pt receiverReadThread
               , rocklightThread
               , headlightThread
               , taillightThread
               , outputThread;

// Color state
int r = 0, g = 0, b = 0;
int phase = 0;  // 0–5
unsigned long lastStepTime = 0;
unsigned long lastHeadlightTime = 0;
unsigned long lastTaillightTime = 0;

//Rx input
int leftSignal = false;
int rightSignal = false;
int brake = false;

const int ROCK_LIGHTS = 0b100;
const int TAIL_LIGHTS = 0b010;
const int HEAD_LIGHTS = 0b001;

int lightMode = 0b000;  // bits, in orde of least signifact first are headlights, taillights, rock lights.
byte curLeftSignalValue = 0;
byte curRightSignalValue = 0;

unsigned long lastPrintTime = 0;
unsigned long lastSignalTime = 0;

// Helper: write color (handles common anode)
void setColor(int rVal, int gVal, int bVal) {
  analogWrite(RED_PIN, rVal);
  analogWrite(GREEN_PIN, gVal);
  analogWrite(BLUE_PIN, bVal);
}

// The non-blocking color cycle protothread
static int rockLights(struct pt *pt) {
  PT_BEGIN(pt);

  while (true) {
    // Wait until next step time
    PT_WAIT_UNTIL(pt, millis() - lastStepTime >= LIGHT_UPDATE_INTERVAL);
    lastStepTime = millis();

    if ((lightMode & ROCK_LIGHTS) == ROCK_LIGHTS) {
      // Perform color transition only when enabled
      switch (phase) {
        case 0:
          g += COLOR_STEP_SIZE;
          if (g >= 255) {
            g = 255;
            phase++;
          }
          break;
        case 1:
          r -= COLOR_STEP_SIZE;
          if (r <= 0) {
            r = 0;
            phase++;
          }
          break;
        case 2:
          b += COLOR_STEP_SIZE;
          if (b >= 255) {
            b = 255;
            phase++;
          }
          break;
        case 3:
          g -= COLOR_STEP_SIZE;
          if (g <= 0) {
            g = 0;
            phase++;
          }
          break;
        case 4:
          r += COLOR_STEP_SIZE;
          if (r >= 255) {
            r = 255;
            phase++;
          }
          break;
        case 5:
          b -= COLOR_STEP_SIZE;
          if (b <= 0) {
            b = 0;
            phase = 0;
          }
          break;
      }

      setColor(r, g, b);
    } else {
      // LED disabled → turn off all colors
      setColor(0, 0, 0);
    }
  }

  PT_END(pt);
}

// The non-blocking headlights protothread
static int headLights(struct pt *pt) {
  PT_BEGIN(pt);

  while (true) {
    PT_WAIT_UNTIL(pt, millis() - lastHeadlightTime >= LIGHT_UPDATE_INTERVAL);
    lastHeadlightTime = millis();

    if ((lightMode & HEAD_LIGHTS) == HEAD_LIGHTS) {
      digitalWrite(HEADLIGHT_PIN, HIGH);
    } else {
      digitalWrite(HEADLIGHT_PIN, LOW);
    }
  }

  PT_END(pt);
};

static int blinkSignal(byte curVal, bool brakeOn) {
  if (millis() - lastSignalTime >= SIGNAL_BLINK_INTERVAL) {
    lastSignalTime = millis();
    if (brakeOn) {
        return curVal + 128;
    } else {
      return curVal + 256;
    }
  } else {
    return curVal;
  }
} 
 

// The non-blocking headlights protothread
static int tailLights(struct pt *pt) {
  PT_BEGIN(pt);

  while (true) {
    int curTime = millis();
    PT_WAIT_UNTIL(pt, curTime - lastTaillightTime >= LIGHT_UPDATE_INTERVAL);
    lastTaillightTime = curTime;

    byte newLeftVal = curLeftSignalValue;
    byte newRightVal = curRightSignalValue;

    if ((lightMode & TAIL_LIGHTS) == TAIL_LIGHTS) {
      if (brake) {
        if (!rightSignal) {
          newRightVal = 255;
        }
        if (!leftSignal) {
          newLeftVal = 255;
        }
      }
      else {
        if (!rightSignal) {
          newRightVal = 127;
        }
        if (!leftSignal) {
          newLeftVal = 127;
        }
      }

      if (rightSignal || leftSignal) {
        if ((millis() - lastSignalTime) >= SIGNAL_BLINK_INTERVAL) {
          lastSignalTime = millis();

          if (rightSignal) {
            newRightVal = newRightVal + 128;
          }

          if (leftSignal) {
            newLeftVal = newLeftVal + 128;
          }
        }
      }
    } else {
      newLeftVal = 0;
      newRightVal = 0;
    }

    analogWrite(LEFT_TAILLIGHT_PIN, newLeftVal);
    curLeftSignalValue = newLeftVal;
    analogWrite(RIGHT_TAILLIGHT_PIN, newRightVal);
    curRightSignalValue = newRightVal;
    
  }

  PT_END(pt);
}

int readChannel(int channelInput, int defaultValue = 0){
  int ch = pulseIn(channelInput, HIGH, 30000);
  if (ch < 100) return defaultValue;
  return map(ch, 1000, 2000, -100, 100);
}

static int readPins(struct pt *pt) {
  PT_BEGIN(pt);

  int steering = readChannel(STEERING_PIN);
  if (steering < -50) {
    leftSignal = true;
    rightSignal = false;
  } else if (steering > 50) {
    leftSignal = false;
    rightSignal = true;
  } else {
    leftSignal = false;
    rightSignal = false;
  }

  int throttle = readChannel(THROTTLE_PIN);
  if (throttle < -12) {
    brake = true;
  } else {
    brake = false;
  }

  int lights = readChannel(LIGHTING_PIN, -100);
  if (lights < -50) {
    lightMode = 0b000;
  }
  else if ( lights >=-50 and lights <=50) {
    lightMode = HEAD_LIGHTS | TAIL_LIGHTS;
  } else {
    lightMode = HEAD_LIGHTS | TAIL_LIGHTS | ROCK_LIGHTS;
  }

  outputState(pt, steering, throttle, lights);
  
  PT_END(pt);
}

static int outputState (struct pt *pt, int steeringVal, int throttleVal, int lightVal) {
  if (!Serial.availableForWrite()) return;

  PT_WAIT_UNTIL(pt, millis() - lastPrintTime >= 500);
  lastPrintTime = millis();

  Serial.print("S: ");
  Serial.print(steeringVal);
  Serial.print(", T: ");
  Serial.print(throttleVal);
  Serial.print(", L: ");
  Serial.print(lightVal);
  Serial.print(", LM: ");
  Serial.print(lightMode);
  Serial.print(", LS: ");
  Serial.print(curLeftSignalValue);
  Serial.print(", RS: ");
  Serial.print(curRightSignalValue);
  Serial.print(", BR: ");
  Serial.print(brake);
  Serial.print(", RL: ");
  Serial.print(ROCK_LIGHTS);
  Serial.print(", rs: ");
  Serial.println();
}

void setup() {
  Serial.begin(115200);

  pinMode(STEERING_PIN, INPUT);
  pinMode(THROTTLE_PIN, INPUT);
  pinMode(LIGHTING_PIN, INPUT);

  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);

  pinMode(HEADLIGHT_PIN, OUTPUT);
  pinMode(LEFT_TAILLIGHT_PIN, OUTPUT);
  pinMode(RIGHT_TAILLIGHT_PIN, OUTPUT);

  PT_INIT(&receiverReadThread);
  PT_INIT(&headlightThread);
  PT_INIT(&taillightThread);
  PT_INIT(&rocklightThread);
  PT_INIT(&outputThread);

}

void loop() {
  readPins(&receiverReadThread);
  headLights(&headlightThread);
  tailLights(&taillightThread);
  rockLights(&rocklightThread);

}
