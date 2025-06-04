#include "SteppersMovement.h"

SteppersMovement::SteppersMovement() {
  for (int i = 0; i < NUM_MOTORS; i++) {
    startHoming[i] = false;
    homingDone[i] = false;
    backing[i] = false;
    homeFlags[i] = false;
    backStartTime[i] = 0;
    lastStepTime[i] = 0;
  }
}

void SteppersMovement::begin() {
  for (int i = 0; i < NUM_MOTORS; i++) {
    pinMode(stepPins[i], OUTPUT);
    pinMode(dirPins[i], OUTPUT);
    pinMode(sensorPins[i], INPUT);
    currentAngle[i] = 0.0;
  }
}

void SteppersMovement::startAllHoming() {
  for (int i = 0; i < NUM_MOTORS; i++) {
    startHoming[i] = true;
    homingDone[i] = false;
    backing[i] = false;
    homeFlags[i] = false;
    currentAngle[i] = 0.0;
  }
}

void SteppersMovement::update() {
  if (!(startHoming[0] || startHoming[1] || startHoming[2] || startHoming[3])) return;

  unsigned long now = micros();
  unsigned long nowMillis = millis();

  for (int i = 0; i < NUM_MOTORS; i++) {
    if (homingDone[i]) continue;

    if (now - lastStepTime[i] >= stepDelay * 2) {
      lastStepTime[i] = now;

      if (digitalRead(sensorPins[i]) == LOW) {
        // Move toward sensor
        if (currentAngle[i]<50){
        digitalWrite(dirPins[i], (i == 0 || i == 3) ? LOW : HIGH);
        }
        else{
        digitalWrite(dirPins[i], (i == 0 || i == 3) ? HIGH : LOW);
        }
        
      } else {
        homingDone[i] = true;
        backing[i] = true;
        backStartTime[i] = nowMillis;
        //Serial.print("Motor ");
        //Serial.print(i);
        //Serial.println(" homed.");
      }
    }
  }

  // Backward after homing
  for (int i = 0; i < NUM_MOTORS; i++) {
    if (backing[i]) {
      if (nowMillis - backStartTime[i] < backTime[i]) {
        if (currentAngle[i]<50){
        digitalWrite(dirPins[i], (i == 0 || i == 3) ? HIGH : LOW);
        }
        else{
        digitalWrite(dirPins[i], (i == 0 || i == 3) ? LOW : HIGH);
        }
      } else {
        backing[i] = false;
        homeFlags[i] = true;
        //Serial.print("Motor ");
        //Serial.print(i);
        //Serial.println(" backing complete.");
      }
    }
  }

  // All motors homed
  if (homeFlags[0] && homeFlags[1] && homeFlags[2] && homeFlags[3]) return;

  // Otherwise step motors
  stepMotor();
}

void SteppersMovement::stepMotor() {
  for (int i = 0; i < NUM_MOTORS; i++) {
    if (!homeFlags[i]) digitalWrite(stepPins[i], HIGH);
  }
  delayMicroseconds(stepDelay);
  for (int i = 0; i < NUM_MOTORS; i++) {
    if (!homeFlags[i]) digitalWrite(stepPins[i], LOW);
  }
  delayMicroseconds(stepDelay);
}

void SteppersMovement::stepOnce(uint8_t index) {
  digitalWrite(stepPins[index], HIGH);
  delayMicroseconds(stepDelay);
  digitalWrite(stepPins[index], LOW);
  delayMicroseconds(stepDelay);
}

void SteppersMovement::rotateStepper(uint8_t index, float angle_deg, bool clockwise) {
  if (index >= NUM_MOTORS) return;

  const float stepAngle = 360.0 / 800.0;  // 0.45° per step
  const float gearRatio = 15.0 / 360.0;   // gear ratio you provided

  //float effectiveStepAngle = stepAngle * gearRatio;
  //int steps = abs(angle_deg) / effectiveStepAngle;
  int steps = abs(angle_deg - currentAngle[index])/(stepAngle*gearRatio); 

  digitalWrite(dirPins[index], clockwise ? HIGH : LOW);

  for (int i = 0; i < steps; i++) {
    stepOnce(index);
    delayMicroseconds(100);  // Optional: add timing between steps
  }

  float directionSign = clockwise ? 1.0 : -1.0;
  currentAngle[index] = directionSign * angle_deg;

  //Serial.print("Motor ");
  //Serial.print(index);
  //Serial.print(" rotated ");
  //Serial.print(angle_deg);
  //Serial.print(" degrees. Current angle: ");
  //Serial.print(currentAngle[index]);
  //Serial.println(" degrees.");
}

void SteppersMovement::rotateSteppersAckerman(float angle_deg, bool clockwise) {
  //if (index >= NUM_MOTORS) return;

  //const float stepAngle = 360.0 / 800.0;  // 0.45° per step
  //const float gearRatio = 15.0 / 360.0;   // gear ratio you provided
  angle_deg = constrain(angle_deg, -90, 90); 
  float R = L / tan(radians(abs(angle_deg)));
  float theta_inner = degrees(atan(L / (R - W / 2)));
  float theta_outer = degrees(atan(L / (R + W / 2)));

  float angle_left, angle_right;

    if (angle_deg >= 0) { // Left turn
      angle_left = theta_inner;
      angle_right = theta_outer;
    } else { // Right turn
      angle_left = -theta_outer;
      angle_right = -theta_inner;
    }
  
  float deltaLeft = angle_left - currentAngle[0];
  float deltaRight = angle_right - currentAngle[2];
  
  int stepsLeft = abs(deltaLeft)/(stepAngle*gearRatio); 
  int stepsRight = abs(deltaRight)/(stepAngle*gearRatio);
  int maxSteps = max(stepsLeft, stepsRight); 

  digitalWrite(dirPins[0], deltaLeft >= 0 ? HIGH : LOW);
  digitalWrite(dirPins[2], deltaRight >= 0 ? HIGH : LOW);

  for (int i = 0; i < maxSteps; i++) {
    if (i < stepsLeft) digitalWrite(stepPins[0], HIGH);
    if (i < stepsRight) digitalWrite(stepPins[2], HIGH);
    delayMicroseconds(stepDelay);
    if (i < stepsLeft) digitalWrite(stepPins[0], LOW);
    if (i < stepsRight) digitalWrite(stepPins[2], LOW);
    delayMicroseconds(stepDelay);
    delayMicroseconds(100);  // Optional: add timing between steps
  }

  float directionSign = clockwise ? 1.0 : -1.0;
  currentAngle[0] = angle_left;
  currentAngle[2] = angle_right;

  //Serial.print("Motor ");
  //Serial.print("Ackerman");
  //Serial.print(" rotated ");
  //Serial.print(angle_deg);
  //Serial.print(" degrees. Current angle: ");
  //Serial.println(currentAngle[0]);
  //Serial.println(currentAngle[2]);
  //Serial.println(" degrees.");
}
void SteppersMovement::rotateSteppersPureRotation() {
  //if (index >= NUM_MOTORS) return;

  //const float stepAngle = 360.0 / 800.0;  // 0.45° per step
  //const float gearRatio = 15.0 / 360.0;   // gear ratio you provided

  float angle = 45.0;
  
  float deltaLeft = angle - currentAngle[0];
  float deltaRight = angle - currentAngle[2];
  float deltaLeft2 = angle - currentAngle[1];
  float deltaRight2 = angle - currentAngle[3];
  
  int stepsLeft = abs(deltaLeft)/(stepAngle*gearRatio); 
  int stepsRight = abs(deltaRight)/(stepAngle*gearRatio);
  int stepsLeft2 = abs(deltaLeft2)/(stepAngle*gearRatio); 
  int stepsRight2 = abs(deltaRight2)/(stepAngle*gearRatio);
  int maxSteps_1 = max(stepsLeft, stepsRight);
  int maxSteps_2 = max(stepsLeft2, stepsRight2);
  int maxSteps = max(maxSteps_1, maxSteps_2);

  digitalWrite(dirPins[0], deltaLeft >= 0 ? LOW : HIGH);
  digitalWrite(dirPins[2], deltaRight >= 0 ? HIGH : LOW);
  digitalWrite(dirPins[1], deltaLeft2 >= 0 ? HIGH : LOW);
  digitalWrite(dirPins[3], deltaRight2 >= 0 ? LOW : HIGH);

  for (int i = 0; i < maxSteps; i++) {
    if (i < stepsLeft) digitalWrite(stepPins[0], HIGH);
    if (i < stepsRight) digitalWrite(stepPins[2], HIGH);
    if (i < stepsLeft2) digitalWrite(stepPins[1], HIGH);
    if (i < stepsRight2) digitalWrite(stepPins[3], HIGH);
    delayMicroseconds(stepDelay);
    if (i < stepsLeft) digitalWrite(stepPins[0], LOW);
    if (i < stepsRight) digitalWrite(stepPins[2], LOW);
    if (i < stepsLeft2) digitalWrite(stepPins[1], LOW);
    if (i < stepsRight2) digitalWrite(stepPins[3], LOW);
    delayMicroseconds(stepDelay);
    delayMicroseconds(100);  // Optional: add timing between steps
  }

  float directionSign = dirPins[0] ? 1.0 : -1.0;
  currentAngle[0] = directionSign*angle;
  currentAngle[2] = directionSign*angle;
  currentAngle[0] = directionSign*angle;
  currentAngle[2] = directionSign*angle;

  //Serial.print("Pure Rotation ");
  //Serial.print(angle);
  //Serial.print(" degrees. Current angle: ");
  //Serial.println(currentAngle[0]);
  //Serial.println(currentAngle[2]);
  //Serial.println(currentAngle[1]);
  //Serial.println(currentAngle[3]);
  //Serial.println(" degrees.");
}
void SteppersMovement::rotateSteppersPureTranslation(float angle) {
  //if (index >= NUM_MOTORS) return;

  //const float stepAngle = 360.0 / 800.0;  // 0.45° per step
  //const float gearRatio = 15.0 / 360.0;   // gear ratio you provided
  
  float deltaLeft = angle - currentAngle[0];
  float deltaRight = angle - currentAngle[2];
  float deltaLeft2 = angle - currentAngle[1];
  float deltaRight2 = angle - currentAngle[3];
  
  int stepsLeft = abs(deltaLeft)/(stepAngle*gearRatio); 
  int stepsRight = abs(deltaRight)/(stepAngle*gearRatio);
  int stepsLeft2 = abs(deltaLeft2)/(stepAngle*gearRatio); 
  int stepsRight2 = abs(deltaRight2)/(stepAngle*gearRatio);
  int maxSteps_1 = max(stepsLeft, stepsRight);
  int maxSteps_2 = max(stepsLeft2, stepsRight2);
  int maxSteps = max(maxSteps_1, maxSteps_2);

  digitalWrite(dirPins[0], deltaLeft >= 0 ? HIGH : LOW);
  digitalWrite(dirPins[2], deltaRight >= 0 ? HIGH : LOW);
  digitalWrite(dirPins[1], deltaLeft2 >= 0 ? HIGH : LOW);
  digitalWrite(dirPins[3], deltaRight2 >= 0 ? HIGH : LOW);

  for (int i = 0; i < maxSteps; i++) {
    if (i < stepsLeft) digitalWrite(stepPins[0], HIGH);
    if (i < stepsRight) digitalWrite(stepPins[2], HIGH);
    if (i < stepsLeft2) digitalWrite(stepPins[1], HIGH);
    if (i < stepsRight2) digitalWrite(stepPins[3], HIGH);
    delayMicroseconds(stepDelay);
    if (i < stepsLeft) digitalWrite(stepPins[0], LOW);
    if (i < stepsRight) digitalWrite(stepPins[2], LOW);
    if (i < stepsLeft2) digitalWrite(stepPins[1], LOW);
    if (i < stepsRight2) digitalWrite(stepPins[3], LOW);
    delayMicroseconds(stepDelay);
    delayMicroseconds(100);  // Optional: add timing between steps
  }

  float directionSign = dirPins[0] ? 1.0 : -1.0;
  currentAngle[0] = directionSign*angle;
  currentAngle[2] = directionSign*angle;
  currentAngle[1] = directionSign*angle;
  currentAngle[3] = directionSign*angle;

  //Serial.print("Pure Translation ");
  //Serial.print(angle);
  //Serial.print(" degrees. Current angle: ");
  //Serial.println(currentAngle[0]);
  //Serial.println(currentAngle[2]);
  //Serial.println(currentAngle[1]);
  //Serial.println(currentAngle[3]);
  //Serial.println(" degrees.");
}
