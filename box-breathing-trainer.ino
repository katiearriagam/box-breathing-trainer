#include <gamma.h>
#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>

// Define Pins
const int NEOMATRIX_PIN = 6;       // digital pin
const int POTENTIOMETER_PIN = A0;  // analog pin

// Helper const values
const int NEOMATRIX_DIMENSION = 8;
const int HOLD_DURATION_MS = 4000;  // 4 seconds
enum State {
  INHALE,
  EXHALE,
  HOLD
};

// Define I/O
Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(NEOMATRIX_DIMENSION, NEOMATRIX_DIMENSION, NEOMATRIX_PIN,
                                               NEO_MATRIX_BOTTOM + NEO_MATRIX_LEFT + NEO_MATRIX_ROWS + NEO_MATRIX_PROGRESSIVE,
                                               NEO_GRB + NEO_KHZ800);

// Define colors
const int MAX_NUMBER_OF_COLORS = 5;
const uint16_t SEA_GREEN = matrix.Color(94, 150, 151);
const uint16_t SUN_ORANGE = matrix.Color(240, 188, 105);
const uint16_t PASTEL_TEAL = matrix.Color(195, 216, 209);
const uint16_t PASTEL_PINK = matrix.Color(245, 207, 194);
const uint16_t SALMON = matrix.Color(254, 183, 161);
const uint16_t COLORS[MAX_NUMBER_OF_COLORS] = { SEA_GREEN, SUN_ORANGE, PASTEL_TEAL, PASTEL_PINK, SALMON };

// Helper variables
State currentState = INHALE;
State prevState = HOLD;
uint16_t currentColor;

// Helper variables for the HOLD cycle
unsigned long millisValueOnHoldCycleStart = 0;
unsigned long currentMillisValue = 0;
int countOfLightsOnForCurrentHoldCycle = 0;

void setState(State newState) {
  prevState = currentState;
  currentState = newState;
  if (newState == HOLD) {
    millisValueOnHoldCycleStart = millis();
    countOfLightsOnForCurrentHoldCycle = 0;
  }
}

void setup() {
  // Begin serial communications at 9600 beats of data per second
  Serial.begin(9600);

  // matrix setup
  matrix.begin();
  matrix.setBrightness(5);
}

void restartTheCurrentBoxBreathingRingIteration() {
  // restart all helper variables
  millisValueOnHoldCycleStart = 0;
  currentMillisValue = 0;
  countOfLightsOnForCurrentHoldCycle = 0; 

  // set state back to INHALE
  setState(INHALE);
}

int getBreathingSensorReading() {
  // value between 0-1023
  int potentiometerValue = analogRead(POTENTIOMETER_PIN);
  // make the potentiometer reading a number between 0-7 (range of 8)
  int numberOfPinsToLightUp = potentiometerValue / 128;
  Serial.println(potentiometerValue);
  return numberOfPinsToLightUp;
}

void doInhaleCycle() {
  int sensorReading = getBreathingSensorReading();
  for (int i = 0; i <= sensorReading; i++) {
    matrix.drawPixel(0, i, currentColor);
  }
  if (sensorReading == NEOMATRIX_DIMENSION - 1) {  // if we completed the cycle, change the state
    setState(EXHALE);
  }
}

void doExhaleCycle() {
  int sensorReading = getBreathingSensorReading();
  for (int i = NEOMATRIX_DIMENSION - 1; i >= sensorReading; i--) {
    matrix.drawPixel(NEOMATRIX_DIMENSION - 1, i, currentColor);
  }
  if (sensorReading == 0) {  // if we completed the cycle, change the state
    setState(HOLD);
  }
}

bool isHoldWithinAcceptableRange(int sensorReading, int minAcceptableValue, int maxAcceptableValue) {
  return sensorReading >= minAcceptableValue && sensorReading <= maxAcceptableValue;
}

void doHoldCycle() { // HOLD cycle lasts 4 seconds each
  currentMillisValue = millis();
  unsigned long ellapsedMsSinceHoldCycleStarted = currentMillisValue - millisValueOnHoldCycleStart;
  // we are not done with the HOLD cycle
  if (ellapsedMsSinceHoldCycleStarted < HOLD_DURATION_MS) {
    int sensorReading = getBreathingSensorReading();
    int minAcceptableValue = 0;
    int maxAcceptableValue = 0;
    if (prevState == INHALE) {
      minAcceptableValue = 6;
      maxAcceptableValue = 7;
    } else if (prevState == EXHALE) {
      minAcceptableValue = 0;
      maxAcceptableValue = 1;
    }
    bool isBreathingHoldRangeAcceptable = isHoldWithinAcceptableRange(sensorReading, minAcceptableValue, maxAcceptableValue);
    if (isBreathingHoldRangeAcceptable) {
      // check if 0.5 seconds have ellapsed since the last time we lit up another LED
      // we give 20ms buffer for Arduino processing
      bool hasHalfASecondEllapsed = ellapsedMsSinceHoldCycleStarted % 500 >= 0 && ellapsedMsSinceHoldCycleStarted % 500 <= 20;
      if(hasHalfASecondEllapsed)
      {
        countOfLightsOnForCurrentHoldCycle = countOfLightsOnForCurrentHoldCycle + 1;
        if(countOfLightsOnForCurrentHoldCycle > NEOMATRIX_DIMENSION)
        {
          if(prevState == INHALE)
          {
            setState(EXHALE);
          }
          else if(prevState == EXHALE)
          {
            setState(INHALE);
          }
        }
      }     
    }
    else{
      // user is not holding 
      restartTheCurrentBoxBreathingRingIteration();
    }
  }
}

void showPreviousStatesProgress() {
  currentColor = COLORS[2];
  switch (currentState) {
    case EXHALE:  // previous states are 1 INHALE and 1 HOLD
      {
        for (int i = 0; i <= NEOMATRIX_DIMENSION - 1; i++) {
          matrix.drawPixel(0, i, currentColor);
          matrix.drawPixel(i, 0, currentColor);
        }
        break;
      }
    case HOLD:
      {
        for (int i = 0; i <= NEOMATRIX_DIMENSION - 1; i++) {
          matrix.drawPixel(0, i, currentColor);
          matrix.drawPixel(i, 0, currentColor);
          if (prevState == EXHALE) {
            matrix.drawPixel(NEOMATRIX_DIMENSION - 1, i, currentColor);
          }
        }
        break;
      }
    case INHALE:
    default:
      break;
  }
}

  void loop() {
    showPreviousStatesProgress();
    switch (currentState) {
      case INHALE:
        {
          doInhaleCycle();
          break;
        }
      case EXHALE:
        {
          doExhaleCycle();
          break;
        }
      case HOLD:
        {
          Serial.println("HOLD");
          break;
        }
      default:
        Serial.println("Unrecognized state");
    }

    matrix.show();
    delay(100);  // delay between reads
  }
