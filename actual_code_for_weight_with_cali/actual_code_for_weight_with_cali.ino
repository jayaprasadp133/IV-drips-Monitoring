#include "HX711.h"

// HX711 pins
#define DOUT 4
#define SCK 5

HX711 scale;

// Use your calibrated factor here (flip sign from negative)
float calibration_factor = 693950.0; // replace with your value

void setup() {
  Serial.begin(9600);
  scale.begin(DOUT, SCK);
  
  scale.set_scale(calibration_factor); // set calibration factor
  scale.tare(); // reset scale to 0

  Serial.println("Weight measurement started...");
}

void loop() {
  // Get average reading of 10 samples
  float weight_kg = scale.get_units(10); 
  
  // Convert to grams
  float weight_g = weight_kg * 1000.0;

  // Print to Serial Monitor
  Serial.print("Weight: ");
  Serial.print(weight_g, 1); // 1 decimal place
  Serial.println(" g");

  delay(1000); // update every 1 second
}
