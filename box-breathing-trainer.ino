#include <gamma.h>
#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>

// Define Pins
const int NEOMATRIX_PIN = 6;  // digital pin

// Helper const values
const int NEOMATRIX_DIMENSION = 8;

// Define I/O
Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(NEOMATRIX_DIMENSION, NEOMATRIX_DIMENSION, NEOMATRIX_PIN,
                                               NEO_MATRIX_TOP + NEO_MATRIX_LEFT + NEO_MATRIX_ROWS + NEO_MATRIX_PROGRESSIVE,
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
int counter = 0;

void setup() {
  // matrix setup
  matrix.begin();
  matrix.setBrightness(5);
}

void loop() {
  Serial.println("running");
  const uint16_t currentColor = COLORS[counter % MAX_NUMBER_OF_COLORS];
  matrix.drawPixel(2, 3, currentColor);
  matrix.show();
  delay(100);
  counter = counter + 1;
}
