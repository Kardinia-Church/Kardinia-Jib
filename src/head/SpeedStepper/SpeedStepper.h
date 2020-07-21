// SpeedStepp.h
#ifndef SPEED_STEPPER_H
#define SPEED_STEPPER_H

/*
 * (c)2019 Forward Computing and Control Pty. Ltd.
 * NSW Australia, www.forward.com.au
 * This code is not warranted to be fit for any purpose. You may only use it at your own risk.
 * This generated code may be freely used for both private and commercial use
 * provided this copyright is maintained.
 */

#include <Arduino.h>
class SpeedStepper {

  public:

    /**
       Stepper(int stepPin, int dirPin)
       sets the pin connected to the driver step input
       and the pin connected to the driver's dir input
       Default for dir pin is HIGH for forward
       Use InvertDirectionPin() to change this if needed
    */
    SpeedStepper(int stepPin, int dirPin);

    /**
      Set where to send debug output, if any
      Need to uncomment #define DEBUG in SpeedStepper.cpp as well as calling this method
    */
    void setDebugPrint(Print* _debugPtr);

    /**
      goHome
      set targetSpeed to maxSpeed
      in the direction to return to 0
      stop at 0
    */
    void goHome();

    /**
       isGoingHome()
       returns true if currently returning to position 0
    */
    boolean isGoingHome();

    /**
       setPlusLimit(int32_t)
       sets the max positive limit position (in steps)
       param: mPos - the upper limit, must be >= 0
       if < currentPosition, set to currentPosition
    */
    void setPlusLimit(int32_t mPos);

    /**
       getPlusLimit()
       returns the max positive limit position (in steps)
    */
    int32_t getPlusLimit();

    /**
       setMinusLimit(int32_t)
       sets the max negative limit position (in steps)
       param: mPos - the upper limit, must be <= 0
       if < currentPosition, set to currentPosition
    */
    void setMinusLimit(int32_t mPos);

    /**
       getMinusLimit()
       returns the max negative limit position (in steps)
    */
    int32_t getMinusLimit();
    
    /**
       setCurrentPosition(int32_t)
       set the current position in steps from 0, plus or minus
       It is limited to be within the set limits.
    */
    void setCurrentPosition(int32_t pos);

    /**
       getCurrentPosition()
       return the current step count
    */
    int32_t getCurrentPosition();

    /**
       isDirForward()
       returns true if direction is FORWARD
    */
    boolean isDirForward();

    /**
       setDirForward()
       set direction FORWARD
    */
    void setDirForward();

    /**
        setDirReverse()
        set direction REVERSE
    */
    void setDirReverse();

    /**
       stop()
       Sets speed to 0.0 and decelerates to a stop
    */
    void stop();

    /**
       hardStop()
       Just stops the motor without any deceleration
    */
    void hardStop();

    /**
      stopAndSetHome()
      Does a hardStop() and then setCurrentPosition(0)
      for goHome()
    */
    void stopAndSetHome();

    /**
       run()
       This needs to be called often, at least once per loop()
       Takes a step if it has been stepInterval since last step
       and then calculated new stepInterval
       returns true if still running.
    */
    boolean run();


    /**
       oneStep()
       Takes a single step in the current direction
    */
    void oneStep();

    /**
       stepForward()
       Take one step in the FORWARD direction
    */
    void stepForward();

    /**
       stepReverse()
       Take one step in the REVERSE direction
    */
    void stepReverse();

    /**
       invertDirectionLogic()
       Changes logic of DIR_PIN output.
       Default is HIGH => Forward
       Calling this method toggles the current setting.
       invertDirectionLogic(); // -> LOW for Forward
       invertDirectionLogic(); // -> back to HIGH for Forward
    */
    void invertDirectionLogic();

    /**
       isRunning()
       returns true if stepInterval not zero
       i.e. will generate step at some time in the future.
    */
    boolean isRunning();

    /**
       setSpeed(float)
       Set the speed and direction to drive the stepper
       +ve for Forward, -ve for Reverse
       The acceleration setting set the ramp rate for changes in speed.
       Calling this method terminates goHome()
       param: sp - the required speed, Speeds < minSpeed are set as 0.0
    */
    void setSpeed(float sp);

    /**
       getSetSpeed()
       Get the speed that has been set by setSpeed
       +ve for Forward, -ve for Reverse
    */
    float getSetSpeed();

    /**
       getSpeed()
       return current actual speed
       This differs from getSetSpeed due to acceleration limits and hard position limits
    */
    float getSpeed();

    /**
       setMaxSpeed(float)
       Sets the maximum abs(speed) that setSpeed can set
       Is limited to < 1000
    */
    void setMaxSpeed(float maxSp);

    /**
       setMinSpeed(float)
       Sets the minimum abs(speed) that setSpeed can set
       minSpeed is limited to > 0.0003
    */
    void setMinSpeed(float minSp);

    /**
       setAcceleration(float)
       Set the acceleration rate in steps/sec^2 used to change speeds
       Rates < 0.0001 are set to 0.0001
    */
    void setAcceleration(float newAcceleration);

    // a little less than max int32_t
    // allow for times 2 for distanceToGo to still fit in int32_t
    const static int32_t MAX_INT32_T  = 0x3ffffff0;

  private:

    /**
       runSpeed()
       return false if nothing to do
       else takes a step if it has been stepInterval since last step
    */
    boolean runSpeed();

    /**
       New speed, limited to maxSpeed and minSpeed
       if it is < minSpeed it is set to 0.0
    */
    void internalSetSpeed(float sp);

    /**
       distanceToGo()
      calculate distance to go to limit based on direction of current speed
      +ve speed increases position, -ve speed decreases position
      if stopped use targetSpeed
      Always returns a +ve number
    */
    int32_t distanceToGo();

    /**
       setDir(bool)
       Set the direction output to the stepper driver on the DIR_PIN
       Default is HIGH for forward
       Use InvertDirectionPin() to change to LOW for forward
    */
    void setDir(boolean flag);

    /**
      printComputeNewStepDebug()
      Outputs debug info on new Step calculation
      Is an empty method is DEBUG is not defined
    */
    void printComputeNewStepDebug();

    /**
       updateComputeTimes()
       Keeps track of maximum time computeNewSpeed() takes to execute
       and the total time spent in computeNewSpeed();
    */
    void updateComputeTimes();

    /**
       computeNewSpeed()
       This calculates the next stepInterval based on the requested setSpeed
       the setAcceleration and the current speed.
       Modified by the set limits
    */
    void computeNewSpeed();

    const int32_t LARGE_N = 1000000000;
    uint32_t start_uS;

    boolean dirPinInverted;
    Print* debugPtr;
    int32_t n;
    float cn;
    float c0;
    float cs; // speed c
    float final_cn; // cn at targetSpeed
    boolean goingHome; // set to true when returning to home, 0 position

    boolean dir; // dir true forward, false reverse
    // change setDir() method  to get stepper to go in required direction for forward
    const uint32_t minPulseWidth;

    uint32_t totalComputeTime;
    uint32_t maxComputeTime;


    float maxSpeed;
    float minSpeed;
    float cmin; // min step interval limited to 1000uS
    float cmax; // max step interval set min speed
    uint32_t stepInterval;
    int32_t currentPosition;
    uint32_t lastStepTime;
    float acceleration; // steps/sec per sec
    int32_t maxPositionLimit;
    int32_t minPositionLimit;

    const float maxMaxSpeed;
    // steps/sec min 1 i.e. 5e-3 rev/sec at 200 step per rev
    // i.e. 200sec/rev, 3.3min/rev
    // min speed must be > 0.0003
    const float minMaxSpeed;
    // minSpeed sets how slow you can set the target speed
    // if setSpeed called with a target speed of less than this(e.g. 0) AND current speed is < 2*minSpeed
    // then motor stops

    int STEP_PIN;
    int DIR_PIN;
    float speed;
    float a_speed;
    float targetSpeed;
    float a_targetSpeed;
    boolean targetDir;
    uint32_t start_t; // for computeNewStep debug timing
};
#endif // SPEED_STEPPER_H
