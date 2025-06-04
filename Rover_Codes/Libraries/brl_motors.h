#ifndef BRL_MOTORS_H
#define BRL_MOTORS_H

#include <SPI.h>
#include <mcp2515.h>

class BRLMotors {
public:
    BRLMotors(uint8_t csPin);
    void begin();

    int detectMotorID(int motorNumber);
    void enableMotor(int id);
    void disableMotor(int id);
    void moveMotor(int id, int32_t speed);

    // Steering modes
    void differential(int32_t linearSpeed, int32_t angularSpeed);
    void pureRotation(int32_t speed); 
    void translation(int32_t speed);
    void ackermann(int32_t baseSpeed, float steeringAngle);

private:
    MCP2515 mcp2515;
    struct can_frame canMsg;

    const float WHEEL_RADIUS = 110.0;  // mm (wheelbase)
    const float LENGTH_RADIUS = 843.87;   // mm (wheelbase)
    const float ROVER_WIDTH = 846.196;  // mm (track width)
};

#endif
