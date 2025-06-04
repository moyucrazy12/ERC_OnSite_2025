#include "brl_motors.h"

BRLMotors::BRLMotors(uint8_t csPin) : mcp2515(csPin) {}

void BRLMotors::begin() {
    mcp2515.reset();
    mcp2515.setBitrate(CAN_1000KBPS, MCP_8MHZ);
    delay(1000);
    mcp2515.setNormalMode();
}

int BRLMotors::detectMotorID(int motorNumber) {
    struct can_frame msg;
    msg.can_dlc = 8;
    msg.data[0] = 0x9A;

    for (int id = motorNumber; id <= 32; id++) {
        msg.can_id = 0x140 + id;
        mcp2515.sendMessage(&msg);
        delay(10);

        if (mcp2515.readMessage(&msg) == MCP2515::ERROR_OK) {
            if (msg.data[0] == 0x9A) {
                return id;
            }
        }
    }
    return -1;
}

void BRLMotors::enableMotor(int id) {
    struct can_frame msg;
    msg.can_id = 0x140 + id;
    msg.can_dlc = 8;
    msg.data[0] = 0x88;
    for (int i = 1; i < 8; i++) msg.data[i] = 0x00;
    mcp2515.sendMessage(&msg);
}

void BRLMotors::disableMotor(int id) {
    struct can_frame msg;
    msg.can_id = 0x140 + id;
    msg.can_dlc = 8;
    msg.data[0] = 0x80;
    for (int i = 1; i < 8; i++) msg.data[i] = 0x00;
    mcp2515.sendMessage(&msg);
}

void BRLMotors::moveMotor(int id, int32_t speed) {
    struct can_frame msg;
    msg.can_id = 0x140 + id;
    msg.can_dlc = 8;
    msg.data[0] = 0xA2;
    msg.data[1] = 0x00;
    msg.data[2] = 0x00;
    msg.data[3] = 0x00;
    msg.data[4] = (speed & 0xFF);
    msg.data[5] = (speed >> 8);
    msg.data[6] = (speed >> 16);
    msg.data[7] = (speed >> 24);
    mcp2515.sendMessage(&msg);
}

void BRLMotors::differential(int32_t linearSpeed, int32_t angularSpeed) {
    
    float v_rad, w_rad, vR, vL, wR_rad, wL_rad;
    int32_t wR_dps, wL_dps;
    
    v_rad = linearSpeed * PI / 180.0;
    w_rad = angularSpeed * PI / 180.0;

    // Compute linear velocities for left and right sides
    vR = v_rad + w_rad * ROVER_WIDTH / 2.0;
    vL = v_rad - w_rad * ROVER_WIDTH / 2.0;

    // Convert linear to angular velocities (rad/s)
    wR_rad = vR / WHEEL_RADIUS;
    wL_rad = vL / WHEEL_RADIUS;

    // Convert back to dps
    wR_dps = int32_t(wR_rad * 20*(180.0 / PI));
    wL_dps = int32_t(wL_rad * 20*(180.0 / PI));
    
    //Serial.println(wR_dps);
    //Serial.println(wL_dps);
    
    moveMotor(1, wR_dps);
    moveMotor(2, -wL_dps);
    moveMotor(3, wR_dps);
    moveMotor(4, -wL_dps);
}

void BRLMotors::pureRotation(int32_t speed) {
    moveMotor(1, speed);
    moveMotor(2, speed);
    moveMotor(3, speed);
    moveMotor(4, speed);
}

void BRLMotors::translation(int32_t speed) {
    moveMotor(1, speed);
    moveMotor(2, -speed);
    moveMotor(3, speed);
    moveMotor(4, -speed);
}

void BRLMotors::ackermann(int32_t baseSpeed, float steeringAngle) {
    // Path radii for each wheel
    float steering_angle_rad = radians(steeringAngle);
    if (steering_angle_rad == 0){
        moveMotor(3, baseSpeed);
        moveMotor(4, -baseSpeed);
        moveMotor(1, baseSpeed);
        moveMotor(2, -baseSpeed);
    }
    else{
        float R =  LENGTH_RADIUS / tan(steering_angle_rad);
        float R_left  = sqrt(pow(R - ROVER_WIDTH/ 2.0, 2) + pow(LENGTH_RADIUS, 2));
        float R_right = sqrt(pow(R + ROVER_WIDTH/ 2.0, 2) + pow(LENGTH_RADIUS, 2));

        // Ratios for scaling speed
        float scale_left = R_left / abs(R);
        float scale_right = R_right / abs(R);
        moveMotor(3, baseSpeed*scale_right);
        moveMotor(4, -baseSpeed*scale_left);
        moveMotor(1, baseSpeed);
        moveMotor(2, -baseSpeed);
    }
    
}
