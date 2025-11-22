#include "HX711.h"

#define DT 4    // HX711 DT pin
#define SCK 5   // HX711 SCK pin

HX711 scale;

float calibration_factor = -7050; // starting value

void setup() {
  Serial.begin(9600);
  scale.begin(DT, SCK);
  Serial.println("HX711 Calibration");
  Serial.println("Remove all weight from scale");
  Serial.println("After readings begin, place a known weight on the scale");
  Serial.println("Press 'a' or 'z' to adjust calibration factor");
  scale.set_scale();
  scale.tare();  // reset scale to 0
}

void loop() {
  scale.set_scale(calibration_factor);
  Serial.print("Reading: ");
  Serial.print(scale.get_units(10), 3);
  Serial.print(" kg");
  Serial.print(" | calibration_factor: ");
  Serial.println(calibration_factor);

  if (Serial.available()) {
    char temp = Serial.read();
    if (temp == 'a')
      calibration_factor += 100;
    else if (temp == 'z')
      calibration_factor -= 10000;
  }
}
