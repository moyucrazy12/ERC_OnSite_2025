#include "SteppersMovement.h"

SteppersMovement motors;

String input = "";
int32_t values[5];
int index = 0;

int32_t v_dps, w_dps, linear_speed;
float steering_angle;
int flag_motor = 0;
int mode = 0;
int start = 0;

void setup() {
  Serial.begin(115200);
  motors.begin();
  Serial.println("Ready.");
  //Serial.println("Send 'H' to home all motors.");
  //Serial.println("Send 'R' to rotate motor 0 by 90° clockwise.");

  delay(1000);
}

void loop() {

  while (Serial.available() == 0);

  char c = Serial.read();
    input += c;

    // Check if we reached the end of the message
    if (c == ']') {
      input.trim(); // Remove spaces/newlines
      input = input.substring(1, input.length() - 1); // Remove [ and ]
      //Serial.println(input);
      // Now split by comma
      index = 0;
      
      while (input.length() > 0) {
        int commaIndex = input.indexOf(',');
        String numberStr;
        if (commaIndex == -1) {
          numberStr = input;
          input = "";
        } else {
          numberStr = input.substring(0, commaIndex);
          input = input.substring(commaIndex + 1);
        }

        values[index] = numberStr.toInt(); // Parse number
        //Serial.print("Value ");
        //Serial.print(index);
        //Serial.print(": ");
        //Serial.println(values[index]);
        index++;
      }
      if (values[3]==1){
        //Serial.print("EN");
        flag_motor = 1;
        
      }
      if (values[4]==1){
        flag_motor = 0;
      }

      input = ""; // Reset for next message
    }
    mode = values[0];
    if (flag_motor == 1){
        switch (mode) {

            case 0:
              // Differential mode
              //linear_speed = int32_t(values[1]);
              while (start == 0){
                motors.startAllHoming();
                start = motors.update();
               
              }
         
              break;
      
            case 1:
              // Ackerman mode
              linear_speed = (values[1]);
              steering_angle = constrain(values[2]*40/1000, -40, 40);
              motors.rotateSteppersAckerman(steering_angle, true);
              //brl_motors.differential(values[1],steering_angle);
              
              break;

            case 2:
              // Rotation mode
              motors.rotateSteppersPureRotation();
              //brl_motors.pureRotation(linear_speed);
      
              break;

            case 3:
              // Translation mode
              linear_speed = (values[1]);
              steering_angle = constrain(values[2]*40.0/1000, -40, 40);
              motors.rotateSteppersPureTranslation(steering_angle);
              //brl_motors.translation(values[1]);
      
              break;
      
            default:
              // Homing
              motors.startAllHoming();
      
              break;
      
          }
    }
    else{
      if (values[3]==1){
        //Serial.print("EN");
        flag_motor = 1;
        
      }
      if (values[4]==1){
        flag_motor = 0;

      }
    }

}
