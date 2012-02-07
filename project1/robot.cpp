#include "robot.h"
#include "utilities.h"

#define DEFAULT_NUM_FAILS 5

Robot::Robot(std::string address, int id) {
    _robotInterface = new RobotInterface(address, id);

    _nsXFilter = new FIRFilter("filters/ns_x.ffc");
    _nsYFilter = new FIRFilter("filters/ns_y.ffc");
    _nsThetaFilter = new FIRFilter("filters/ns_theta.ffc");
    _weLeftFilter = new FIRFilter("filters/we.ffc");
    _weRightFilter = new FIRFilter("filters/we.ffc");
    _weRearFilter = new FIRFilter("filters/we.ffc");

    setFailLimit(DEFAULT_NUM_FAILS);

    _wePose = new Pose(0, 0, 0);
    _nsPose = new Pose(0, 0, 0);
    _pose = new Pose(0, 0, 0);
}

Robot::~Robot() {
    delete _robotInterface;

    delete _nsXFilter;
    delete _nsYFilter;
    delete _nsThetaFilter;
    delete _weLeftFilter;
    delete _weRightFilter;
    delete _weRearFilter;

    delete _wePose;
    delete _nsPose;
    delete _pose;
}

void Robot::moveTo(int x, int y) {}

void Robot::turnTo(int theta) {}

void Robot::setFailLimit(int limit) {
    _failLimit = limit;
}

int Robot::getFailLimit() {
    return _failLimit;
}

// Updates the robot pose in terms of the global coordinate system
// with the best estimate of its position (using kalman filter)
void Robot::update() {
    // update the robot interface
    _updateInterface();
    // update each pose estimate
    _updateWEPose();
    _updateNSPose();
    // TODO: pass updated poses to kalman filter
    //       and set new pose
}

Pose* Robot::getPose() {
    return _pose;
}

// Attempts to update the robot
// Returns: true if update succeeded
//          false if update fail limit was reached
bool Robot::_updateInterface() {
    int failCount = 0;
    int failLimit = getFailLimit();

    while (_robotInterface->update() != RI_RESP_SUCCESS &&
           failCount < failLimit) {
        failCount++;
    }

    if (failCount >= failLimit) {
        return false;
    }
    return true;
}

// Returns: filtered wheel encoder (delta) ticks for the left wheel
float Robot::_getWEDeltaLeft() {
    int left = _robotInterface->getWheelEncoder(RI_WHEEL_LEFT);
    return _weLeftFilter->filter((float) left);
}

// Returns: filtered wheel encoder (delta) ticks for the right wheel
float Robot::_getWEDeltaRight() {
    int right = _robotInterface->getWheelEncoder(RI_WHEEL_RIGHT);
    return _weRightFilter->filter((float) right);
}

// Returns: filtered wheel encoder (delta) ticks for the rear wheel
float Robot::_getWEDeltaRear() {
    int rear = _robotInterface->getWheelEncoder(RI_WHEEL_REAR);
    return _weRearFilter->filter((float) rear);
}

// Returns: filtered north star x in ticks
float Robot::_getNSX() {
    int x = _robotInterface->X();
    return _nsXFilter->filter((float) x);
}

// Returns: filtered north star y in ticks
float Robot::_getNSY() {
    int y = _robotInterface->Y();
    return _nsYFilter->filter((float) y);
}

// Returns: filtered north star theta
float Robot::_getNSTheta() {
    float theta = _robotInterface->Theta();
    return _nsThetaFilter->filter(theta);
}

// Returns: filtered wheel encoder delta x for left wheel in ticks 
//          in terms of robot axis
float Robot::_getWEDeltaXLeft() {
    float deltaX = _getWEDeltaLeft();
    deltaX *= cos(DEGREE_150);
    return deltaX;
}

// Returns: filtered wheel encoder delta y for left wheel in ticks 
//          in terms of robot axis
float Robot::_getWEDeltaYLeft() {
    float deltaY = _getWEDeltaLeft();
    deltaY *= sin(DEGREE_150);
    return deltaY;
}

// Returns: filtered wheel encoder delta x for right wheel in ticks 
//          in terms of robot axis
float Robot::_getWEDeltaXRight() {
    float deltaX = _getWEDeltaRight();
    deltaX *= cos(DEGREE_30);
    return deltaX;
}

// Returns: filtered wheel encoder delta y for right wheel in ticks 
//          in terms of robot axis
float Robot::_getWEDeltaYRight() {
    float deltaY = _getWEDeltaRight();
    deltaY *= sin(DEGREE_30);
    return deltaY;
}

// Returns: filtered wheel encoder delta x for rear wheel in ticks
//          in terms of robot axis
float Robot::_getWEDeltaXRear() {
    return _getWEDeltaRear();
}

// Returns: filtered wheel encoder delta y for rear wheel in ticks 
//          in terms of robot axis
float Robot::_getWEDeltaYRear() {
    return 0;
}

// Returns: filtered wheel encoder overall delta x in ticks
//          in terms of robot axis
float Robot::_getWEDeltaX() {
    float leftDeltaX = _getWEDeltaXLeft();
    float rightDeltaX = _getWEDeltaXRight();
    float rearDeltaX = _getWEDeltaXRear();
    // return the average
    return (leftDeltaX + rightDeltaX + rearDeltaX) / 3;
}

// Returns: filtered wheel encoder overall delta y in ticks
//          in terms of robot axis
float Robot::_getWEDeltaY() {
    float leftDeltaY = _getWEDeltaYLeft();
    float rightDeltaY = _getWEDeltaYRight();
    // return the average
    return (leftDeltaY + rightDeltaY) / 2;
}

// Returns: filtered wheel encoder overall delta theta
//          in terms of robot axis
float Robot::_getWEDeltaTheta() {
    float rear = _getWEDeltaRear();
    // FIXME: does this given formula really work?
    return rear / (PI * Util::cmToWE(ROBOT_DIAMETER));
}

// Returns: transformed wheel encoder x estimate in cm of where
//          robot should now be in global coordinate system
float Robot::_getWETransDeltaX() {
    float deltaX = _getWEDeltaX();
    float scaledDeltaX = Util::weToCM(deltaX);
    // TODO: finish
}

// Returns: transformed wheel encoder y estimate in cm of where
//          robot should now be in global coordinate system
float Robot::_getWETransDeltaY() {
    float deltaY = _getWEDeltaY();
    float scaledDeltaY = Util::weToCM(deltaY);
    // TODO: finish
}

// Returns: transformed wheel encoder theta estimate of where
//          robot should now be in global coordinate system
float Robot::_getWETransDeltaTheta() {
    float deltaTheta = _getWEDeltaTheta();
    // TODO: finish
    return deltaTheta;
}

// Returns: transformed north star x estimate of where
//          robot should now be in global coordinate system
// TODO
float Robot::_getNSTransX() {
    
}

// Returns: transformed north star y estimate of where
//          robot should now be in global coordinate system
// TODO
float Robot::_getNSTransY() {
    
}

// Returns: transformed north star theta estimate of where
//          robot should now be in global coordinate system
// TODO
float Robot::_getNSTransTheta() {
    
}

// Updates transformed wheel encoder pose estimate of where
// robot should now be in global coordinate system
void Robot::_updateWEPose() {
    float deltaX = _getWETransDeltaX();
    float deltaY = _getWETransDeltaY();
    float deltaTheta = _getWETransDeltaTheta();
    _wePose->add(deltaX, deltaY, deltaTheta);
}

// Updates transformed north star pose estimate of where
// robot should now be in global coordinate system
void Robot::_updateNSPose() {
    float newX = _getNSTransX();
    float newY = _getNSTransY();
    float newTheta = _getNSTransTheta();
    _nsPose->setX(newX);
    _nsPose->setY(newY);
    _nsPose->setTheta(newTheta);
}