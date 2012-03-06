#include "wheel_encoders.h"
#include "constants.h"
#include "utilities.h"

WheelEncoders::WheelEncoders(RobotInterface *robotInterface)
: PositionSensor(robotInterface) {
	_filterLeft = new FIRFilter("filters/we.ffc");
	_filterRight = new FIRFilter("filters/we.ffc");
	_filterRear = new FIRFilter("filters/we.ffc");
}

WheelEncoders::~WheelEncoders() {
	delete _filterLeft;
	delete _filterRight;
	delete _filterRear;
}

/* Requires the interface be updated prior to calling */
void WheelEncoders::updatePose(int room) {
	float x = getX() + _getDeltaX();
	float y = getY() + _getDeltaY();
	float theta = Util::normalizeTheta(getTheta() + _getDeltaTheta());

	_adjustTotalTheta(theta);

	_pose->setX(x);
	_pose->setY(y);
	_pose->setTheta(theta);
}

/* Returns delta x in terms of global coord system */
float WheelEncoders::_getDeltaX() {
	float rotatedDeltaX = _getRobotDeltaY() * cos(getTheta());
	float scaledDeltaX = rotatedDeltaX / WE_SCALE;
    return scaledDeltaX;
}

/* Returns delta y in terms of global coord system */
float WheelEncoders::_getDeltaY() {
	float rotatedDeltaY = _getRobotDeltaY() * sin(getTheta());
	float scaledDeltaY = rotatedDeltaY / WE_SCALE;
    return scaledDeltaY;
}

/* Returns delta theta in terms of global coord system */
float WheelEncoders::_getDeltaTheta() {
	float rearRobotDeltaX = _getFilteredDeltaRear();
	float scaledRobotDeltaX = rearRobotDeltaX / WE_SCALE;
	return -2 * scaledRobotDeltaX / ROBOT_DIAMETER;
}

/* Returns a delta y value in terms of robot axis */
float WheelEncoders::_getRobotDeltaY() {
    float leftRobotDeltaY = _getFilteredDeltaLeft() * sin(DEGREE_150);
    float rightRobotDeltaY = _getFilteredDeltaRight() * sin(DEGREE_30);
    float avgRobotDeltaY = (leftRobotDeltaY + rightRobotDeltaY) / 2.0;
    return avgRobotDeltaY;
}

// Returns: filtered wheel encoder (delta) ticks for the left wheel
float WheelEncoders::_getFilteredDeltaLeft() {
    int left = _robotInterface->getWheelEncoder(RI_WHEEL_LEFT);
    return _filterLeft->filter((float) left);
}

// Returns: filtered wheel encoder (delta) ticks for the right wheel
float WheelEncoders::_getFilteredDeltaRight() {
    int right = _robotInterface->getWheelEncoder(RI_WHEEL_RIGHT);
    return _filterRight->filter((float) right);
}

// Returns: filtered wheel encoder (delta) ticks for the rear wheel
float WheelEncoders::_getFilteredDeltaRear() {
    int rear = _robotInterface->getWheelEncoder(RI_WHEEL_REAR);
    return _filterRear->filter((float) rear);
}