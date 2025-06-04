#ifndef STEPPERS_MOVEMENT_H
#define STEPPERS_MOVEMENT_H

#include <Arduino.h>

class SteppersMovement {
public:
  SteppersMovement();
  void begin();
  void update();
  void rotateStepper(uint8_t index, float angle_deg, bool clockwise);
  void rotateSteppersAckerman(float angle_deg, bool clockwise);
  void rotateSteppersPureRotation(); 
  void rotateSteppersPureTranslation(float angle); 
  void startAllHoming();

private:
  const int NUM_MOTORS = 4;
  const int stepDelay = 50;     // Microseconds between steps
  const int backTime[4] = {1700, 1850, 1850, 1850}; // ms for backing

  const uint8_t stepPins[4] = {3, 5, 9, 6}; //L1,L2,R1,R2
  const uint8_t dirPins[4]  = {4, A0, 8, 7};
  const uint8_t sensorPins[4] = {A4, A3, A5, A2};
  
  const float L = 843.87;  // mm (wheelbase)
  const float W = 846.196;  // mm (track width)
  const float stepAngle = 90.0 / 800.0;  // 0.45° per step
  const float gearRatio = 15.0 / 360.0;   // gear ratio you provided

  bool startHoming[4];
  bool homingDone[4];
  bool backing[4];
  bool homeFlags[4];
  unsigned long backStartTime[4];
  unsigned long lastStepTime[4];
  float currentAngle[4];

  void stepMotor();
  void stepOnce(uint8_t index);
};

#endif
