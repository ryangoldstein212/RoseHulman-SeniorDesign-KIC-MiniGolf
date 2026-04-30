#include <Arduino.h>
#include <Servo.h>

// Digital Pin definitions
#define servo_Pin 2
#define finalHole_motor_reverse_input 3
#define finalHole_motor_forward_input 4
#define finalHole_motor_enable 5

// Analog Pin definitions
#define transitionHole_sensorPin 6 // analog port for transition hole light sensor
#define finalHold_sensorPin 5 // analog port for final hole light sensor
#define exitHole_left_sensorPin 4 // analog port for plinko left ball exit 
#define exitHole_middle_sensorPin 3 //analog port for plinko middle ball exit
#define exitHole_right_sensorPin 2 // analog port for plinko right ball exit
#define pointValue_15_sensorPin 1 // analog port for plinko touch sensors 1-5
#define pointValue_610_sensorPin 0 // analog port for plinko touch sensors 6-10


//Plinko detection thresholds
int point_tolerance = 25;
long pointValue_1_threshold = 1000;
long pointValue_2_threshold = 830;
long pointValue_3_threshold = 740;
long pointValue_4_threshold = 490;
long pointValue_5_threshold = 390;
int impactSensor_threshold = 100; // impact sensor values must be above this value for detection to be possible

int exitHole_sensor_tolerance = 20;
int exitHole_left_threshold = 5;
int exitHole_middle_threshold = 70;
int exitHole_right_threshold = 45;
int lightSensor_tolerance = 150; // light sensor values must be below this value for detection to be possible

// Impulse Detection Globals
int maxImpulse_15 = 0;
int maxImpulse_610 = 0;

//Point assignments
int pointValue_1 = 20;
int pointValue_2 = 20;
int pointValue_3 = 20;
int pointValue_4 = 20;
int pointValue_5 = 20;
int exitHole_left_pointValue = 20;
int exitHole_middle_pointValue = 20;
int exitHole_right_pointValue = 20;


// Transition Hole Globals
int transitionHole_closed = 10;
int transitionHole_open = 65;
bool detectedHand;

// System State switches
bool state_plinko = false;
bool state_upperStep = false;
bool state_reveal = false;
bool state_lowerStep = false;
bool state_reset_system = false;
bool state_reset_transiton = false;

// counter and timer setting
int handDetection_count = 0;
int playerScore = 0;
long startTime = 0;
long detectedHand_time = 0;

// Transition hole globals
Servo transitionHole_servo; // servo control object
int detectionCount = 0;
long transitionHole_threshold = 5;

// Function Declarations
void sensorTesting(long impulse, long target, long threshold);
long impulseDetection(long readValue, long threshold, long max);
void impactSensor_calculatePoints(long impulsePeak);
void exitHole_pointAssignment();
void exitHole_detection(long exitHole_sensorValue);


void setup() {
  // Servo set up and initialization
  transitionHole_servo.attach(servo_Pin);
  transitionHole_servo.write(transitionHole_closed);

  state_upperStep = true;
  // Serial Monitor set up
  Serial.begin(9600);
}


void loop() {
  // Plinko Section
  if (state_plinko == true){
    // read sensors continuously until a hit on contact sensors is detected, then figure out what voltage reading is and add to player score
    long pointSensor_15_value = analogRead(pointValue_15_sensorPin);
    long pointSensor_610_value = analogRead(pointValue_610_sensorPin);
    long exitHole_left_sensorValue = analogRead(exitHole_left_sensorPin);
    long exitHole_middle_sensorValue = analogRead(exitHole_middle_sensorPin);
    long exitHole_right_sensorValue = analogRead(exitHole_right_sensorPin);
    
    // Impact sensor impulse detection
    maxImpulse_15 = impulseDetection(pointSensor_15_value, impactSensor_threshold, maxImpulse_15);
    maxImpulse_610 = impulseDetection(pointSensor_610_value, impactSensor_threshold, maxImpulse_610);

    // Exit hole light sensor impulse detection
        // After exiting exit holes, enter Upper step state
        state_plinko = false;
        state_upperStep = true;
      //
  }

  // Upper step section
  if (state_upperStep == true){
    // watch transition hole light sensors for detections
    int transitionHole_sensorValue = analogRead(exitHole_left_sensorPin);
        // add function for light sensor detection here
    //thresholding for light sensors and point assignment(?)

    // Sensor testing w/out serial monitor
    long impulsePeak = transitionHole_sensorValue;
    long tolerance = lightSensor_tolerance;
    long threshold = transitionHole_threshold;
        //sensorTesting(impulsePeak, tolerance, threshold);
    Serial.println(transitionHole_sensorValue);

    // when the light sensors detect something twice, then go to reveal state
    if (detectionCount >= 2){
      state_reveal = true;
      state_upperStep = false;
    }
  }

  // Reveal State
  if (state_reveal == true){
      // swing transition hole open, start servo reset timer
      transitionHole_servo.write(transitionHole_open);
      long currentTime = millis();
      detectedHand_time = (currentTime - startTime)/1000;

      // Lower final hole motor

      // play zelda discover treasure theme

      // exit state to lower step phase
      state_lowerStep = true;
      state_reveal = false;
  }

  // Lower State
  if (state_lowerStep == true){
    // Ball detection and final score assignment


  }

  // Servo Reset
  if (state_reset_transiton == true){
    
  }

  // Move servo to drop ball to lower level
  
  if (detectedHand == true) {
    // When a hand is detected, open transition hole and then set timer for 30 sec before closing
    transitionHole_servo.write(transitionHole_open);
    detectionCount = 0;
    detectedHand = false;
  }

  // return servo to initial position
  if (detectedHand_time >= 30) {
    // After 30 seconds has passed, close servo and end detectedHand state
    transitionHole_servo.write(transitionHole_closed);
    detectedHand = false;
  }
  // Lower Step and reset system
  if (state_reset_system == true) {
    // restart system

  }
  // End
  
}

// Functions

void exitHole_detection(long exitHole_sensorValue){
  //theshold detection for falling analog signal
  if (exitHole_sensorValue < lightSensor_tolerance){
    exitHole_pointAssignment();
  }
}

long impulseDetection(long readValue, long threshold, long max){
  //checks if readValue reaches the impulse detection threshold
  if (readValue > threshold){
    // if the impulse detection threshold is met, then continuously update max if max is less than current read value
    if (readValue > max){
      return readValue; // updates max to readValue 
    }
    else{
      return max;
    }
  }
  // if the detection threshold is met again, then stop running and calculate points
  else if (readValue < threshold +5){
    impactSensor_calculatePoints(max);
    return 0;
  }
}


void impactSensor_calculatePoints(long impulsePeak){
  // Check from highest threshold to lowest. 
  if (impulsePeak >= pointValue_1_threshold){          
    playerScore += pointValue_1;
  } 
  else if (impulsePeak >= pointValue_2_threshold){     
    playerScore += pointValue_2;
  } 
  else if (impulsePeak >= pointValue_3_threshold){     
    playerScore += pointValue_3;
  } 
  else if (impulsePeak >= pointValue_4_threshold){     
    playerScore += pointValue_4;
  } 
  else if (impulsePeak >= pointValue_5_threshold){     
    playerScore += pointValue_5;
  } 
}

void exitHole_pointAssignment(){

}

void sensorTesting(long impulse, long target, long tolerance){
  if ((impulse > (target - tolerance)) && (impulse < (target + tolerance))){
        transitionHole_servo.write(transitionHole_open);
        delay(1000);
        transitionHole_servo.write(transitionHole_closed);
  }
}