#include <gamma.h>
#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>
#include <Button.h>

class BreathingCycle {
public:
  // for an 8x8 matrix, we always have a max of 28 coordinates for the cycle
  int xCoordinates[28];
  int yCoordinates[28];
  int numberOfCoordinatesForCycle;

  BreathingCycle(int xCurrentCoordinates[28], int yCurrentCoordinates[28], int numberOfCoordinateForCurrentCycle) {
    numberOfCoordinatesForCycle = numberOfCoordinateForCurrentCycle;
    for (int i = 0; i < numberOfCoordinateForCurrentCycle; i++) {
      xCoordinates[i] = xCurrentCoordinates[i];
      yCoordinates[i] = yCurrentCoordinates[i];
    }
  }
};

// Define Pins
const int NEOMATRIX_PIN = 6;       // digital pin
const int POTENTIOMETER_PIN = A0;  // analog pin
const int BUTTON_PIN = 12;         // digital pin

// Helper const values
const int NEOMATRIX_DIMENSION = 8;
const int HOLD_DURATION_MS = 4000;  // 4 seconds
enum State {
  INHALE,
  EXHALE,
  HOLD
};
const int NUMBER_OF_CYCLES_TO_COMPLETE = 4;

// Define I/O
Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(NEOMATRIX_DIMENSION, NEOMATRIX_DIMENSION, NEOMATRIX_PIN,
                                               NEO_MATRIX_BOTTOM + NEO_MATRIX_LEFT + NEO_MATRIX_ROWS + NEO_MATRIX_PROGRESSIVE,
                                               NEO_GRB + NEO_KHZ800);

// Define button
Button button(BUTTON_PIN, INPUT_PULLUP);

// Define colors
const int MAX_NUMBER_OF_COLORS = 5;
const uint16_t SEA_GREEN = matrix.Color(94, 150, 151);
const uint16_t SUN_ORANGE = matrix.Color(240, 188, 105);
const uint16_t PASTEL_TEAL = matrix.Color(195, 216, 209);
const uint16_t PASTEL_PINK = matrix.Color(245, 207, 194);
const uint16_t SALMON = matrix.Color(254, 183, 161);
const uint16_t COLORS[MAX_NUMBER_OF_COLORS] = { SEA_GREEN, SUN_ORANGE, PASTEL_PINK, SALMON, PASTEL_TEAL };

// Helper variables
State currentState = INHALE;
State prevState = HOLD;
int numberOfCyclesCompleted = 0;
int prevNumberOfCyclesCompleted = 0;

// constant definitions for the coordinates
const int numberOfCoordinatesCycle1 = 4;
const int xCoordinatesCycle1[numberOfCoordinatesCycle1] = { 3, 3, 4, 4 };
const int yCoordinatesCycle1[numberOfCoordinatesCycle1] = { 3, 4, 3, 4 };
const int numberOfCoordinatesCycle2 = 12;
const int xCoordinatesCycle2[numberOfCoordinatesCycle2] = { 2, 3, 4, 5, 2, 5, 2, 5, 2, 3, 4, 5 };
const int yCoordinatesCycle2[numberOfCoordinatesCycle2] = { 2, 2, 2, 2, 3, 3, 4, 4, 5, 5, 5, 5 };
const int numberOfCoordinatesCycle3 = 20;
const int xCoordinatesCycle3[numberOfCoordinatesCycle3] = { 1, 2, 3, 4, 5, 6, 1, 6, 1, 6, 1, 6, 1, 6, 1, 2, 3, 4, 5, 6 };
const int yCoordinatesCycle3[numberOfCoordinatesCycle3] = { 1, 1, 1, 1, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 6, 6, 6, 6 };
const int numberOfCoordinatesCycle4 = 28;
const int xCoordinatesCycle4[numberOfCoordinatesCycle4] = { 0, 1, 2, 3, 4, 5, 6, 7, 0, 7, 0, 7, 0, 7, 0, 7, 0, 7, 0, 7, 0, 1, 2, 3, 4, 5, 6, 7 };
const int yCoordinatesCycle4[numberOfCoordinatesCycle4] = { 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 7, 7, 7, 7, 7, 7 };

BreathingCycle cycles[NUMBER_OF_CYCLES_TO_COMPLETE] = {
  BreathingCycle(xCoordinatesCycle1, yCoordinatesCycle1, numberOfCoordinatesCycle1),
  BreathingCycle(xCoordinatesCycle2, yCoordinatesCycle2, numberOfCoordinatesCycle2),
  BreathingCycle(xCoordinatesCycle3, yCoordinatesCycle3, numberOfCoordinatesCycle3),
  BreathingCycle(xCoordinatesCycle4, yCoordinatesCycle4, numberOfCoordinatesCycle4),
};

// Helper variables for the HOLD cycle
unsigned long millisValueOnHoldCycleStart = 0;
unsigned long currentMillisValue = 0;
unsigned long millisAfterLastHoldLedToggleOn = 0;
int countOfLightsOnForCurrentHoldCycle = 0;

void restartTheCurrentBoxBreathingRingIteration() {
  // restart all helper variables
  millisValueOnHoldCycleStart = 0;
  currentMillisValue = 0;
  countOfLightsOnForCurrentHoldCycle = 0;
}

void setState(State newState) {
  prevState = currentState;
  currentState = newState;
  if (newState == HOLD) {
    millisValueOnHoldCycleStart = millis();
    millisAfterLastHoldLedToggleOn = millisValueOnHoldCycleStart;
    countOfLightsOnForCurrentHoldCycle = 0;
  }
  if (newState == INHALE) {
    restartTheCurrentBoxBreathingRingIteration();
  }
}

void setup() {
  // Begin serial communications at 9600 beats of data per second
  Serial.begin(9600);

  // matrix setup
  matrix.begin();
  matrix.setBrightness(5);
}

int getBreathingSensorReading() {
  // value between 0-1023
  int potentiometerValue = analogRead(POTENTIOMETER_PIN);
  // make the potentiometer reading a number between 0-7 (range of 8)
  int numberOfPinsToLightUp = potentiometerValue / 128;
  return numberOfPinsToLightUp;
}

void doInhaleCycle() {
  int sensorReading = getBreathingSensorReading();
  for (int i = 0; i <= sensorReading; i++) {
    matrix.drawPixel(0, i, COLORS[numberOfCyclesCompleted]);
  }
  if (sensorReading == NEOMATRIX_DIMENSION - 1) {  // if we completed the cycle, change the state
    setState(HOLD);
  }
}

void doExhaleCycle() {
  int sensorReading = getBreathingSensorReading();
  for (int i = NEOMATRIX_DIMENSION - 1; i >= sensorReading; i--) {
    matrix.drawPixel(NEOMATRIX_DIMENSION - 1, i, COLORS[numberOfCyclesCompleted]);
  }
  if (sensorReading == 0) {  // if we completed the cycle, change the state
    setState(HOLD);
  }
}

bool isHoldWithinAcceptableRange(int sensorReading, int minAcceptableValue, int maxAcceptableValue) {
  return sensorReading >= minAcceptableValue && sensorReading <= maxAcceptableValue;
}

void doHoldCycle() {  // HOLD cycle lasts 4 seconds each
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
      bool hasHalfASecondEllapsedSinceLastLedToggleOn = currentMillisValue - millisAfterLastHoldLedToggleOn > 500;  // 500 ms
      if (hasHalfASecondEllapsedSinceLastLedToggleOn) {
        countOfLightsOnForCurrentHoldCycle = countOfLightsOnForCurrentHoldCycle + 1;
        millisAfterLastHoldLedToggleOn = currentMillisValue;
        if (countOfLightsOnForCurrentHoldCycle > NEOMATRIX_DIMENSION) {
          if (prevState == INHALE) {
            setState(EXHALE);
          } else if (prevState == EXHALE) {
            setState(INHALE);
          }
        }
      }
    } else {
      // user is not holding - restart at INHALE
      setState(INHALE);
    }
  } else {
    if (prevState == INHALE) {
      setState(EXHALE);
    } else if (prevState == EXHALE) {
      numberOfCyclesCompleted = numberOfCyclesCompleted + 1;
      setState(INHALE);
    }
  }
}

void showPreviousStatesProgressForCurrentCycle() {
  switch (currentState) {
    case EXHALE:  // previous states are 1 INHALE and 1 HOLD
      {
        for (int i = 0; i <= NEOMATRIX_DIMENSION - 1; i++) {
          matrix.drawPixel(0, i, COLORS[numberOfCyclesCompleted]);
          matrix.drawPixel(i, NEOMATRIX_DIMENSION - 1, COLORS[numberOfCyclesCompleted]);
        }
        break;
      }
    case HOLD:
      {
        int currentRow = prevState == INHALE ? NEOMATRIX_DIMENSION - 1 : 0;
        for (int i = 0; i < countOfLightsOnForCurrentHoldCycle; i++) {
          int currentColumn = prevState == INHALE ? i : NEOMATRIX_DIMENSION - 1 - i;
          // update the right row and column depending on prevState
          matrix.drawPixel(currentColumn, currentRow, COLORS[numberOfCyclesCompleted]);
        }

        for (int i = 0; i < NEOMATRIX_DIMENSION; i++) {
          matrix.drawPixel(0, i, COLORS[numberOfCyclesCompleted]);
          if (prevState == EXHALE) {
            matrix.drawPixel(i, NEOMATRIX_DIMENSION - 1, COLORS[numberOfCyclesCompleted]);
            matrix.drawPixel(NEOMATRIX_DIMENSION - 1, i, COLORS[numberOfCyclesCompleted]);
          }
        }
        break;
      }
    case INHALE:
    default:
      break;
  }
}

void showCompletedCycles() {
  for (int n = 0; n < numberOfCyclesCompleted; n++) {
    BreathingCycle cycleToShow = cycles[n];
    for (int m = 0; m < cycleToShow.numberOfCoordinatesForCycle; m++) {
      int column = cycleToShow.xCoordinates[m];
      int row = cycleToShow.yCoordinates[m];
      matrix.drawPixel(column, row, COLORS[n]);
    }
  }
}

void restartBoxBreathing() {
  // restart the current cycle
  restartTheCurrentBoxBreathingRingIteration();
  // restart the finite state machine
  currentState = INHALE;
  prevState = HOLD;
  numberOfCyclesCompleted = 0;
  prevNumberOfCyclesCompleted = 0;
}

void loop() {
  matrix.clear();
  int buttonAction = button.checkButtonAction();
  if (buttonAction == Button::CLICKED) {
    restartBoxBreathing();
  }

  showCompletedCycles();
  // we are performing box breathing (have missing cycles to complete)
  if (numberOfCyclesCompleted < NUMBER_OF_CYCLES_TO_COMPLETE) {
    showPreviousStatesProgressForCurrentCycle();
    showCompletedCycles();
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
          doHoldCycle();
          break;
        }
      default:
        Serial.println("Unrecognized state");
    }
  }
  matrix.show();
}
