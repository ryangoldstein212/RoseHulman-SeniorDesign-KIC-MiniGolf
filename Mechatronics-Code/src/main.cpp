#include <Arduino.h>
#include <Servo.h>

// Digital Pin definitions
#define servoPin 9

// Analog Pin definitions
#define pointValue_15_sensorPin 0 // analog port for plinko touch sensors 1-5
#define pointValue_610_sensorPin 1 // analog port for plinko touch sensors 6-10
#define exitHole_left_sensorPin 2 // analog port for plinko left ball exit 
#define exitHole_middle_sensorPin 3 //analog port for plinko middle ball exit
#define exitHole_right_sensorPin 4 // analog port for plinko right ball exit
#define transitionHole_sensorPin 5 // analog port for transition hole light sensor
#define finalHold_sensorPin 6 // analog port for final hole light sensor

// Servo positions DOUBLE CHECK!!!!!!!!!!!
int transitionHole_closed = 90;
int transitionHole_open = 150;

//Plinko detection thresholds
int lightSensor_threshold = 50; // for exit hole detection
int point_threshold = 20;
long pointValue_1_threshold = 325;
long pointValue_2_threshold = 415;
long pointValue_3_threshold = 620;
long pointValue_4_threshold = 690;
long pointValue_5_threshold = 830;

//Point assignments
int pointValue_1 = 20;
int pointValue_2 = 20;
int pointValue_3 = 20;
int pointValue_4 = 20;
int pointValue_5 = 20;
int exitHole_left_pointValue = 20;
int exitHole_middle_pointValue = 20;
int exitHole_right_pointValue = 20;

// Impulse Detection Globals
bool detectImpulse_15 = false
bool detectImpuse_610 = false;
int maxImpulse_15 = 0;
int maxImpulse_610 = 0;

// System State switches
bool state_plinko;
bool state_upperStep;
bool state_reveal;
bool state_lowerStep;
bool state_reset_system;
bool state_reset_transiton;

// counter and timer setting
int handDetection_count = 0;
int playerScore = 0;
long startTime = 0;
long timer = 0;

// ISR flags
volatile int mainEventflag = 0;
#define detectHand_flag 0x01 // hand detection event flag
#define finalHole_flag 0x02 // final hole event flag

// Transition hole globals
Servo transitionHole_servo; // servo control object
int detectionCount = 0;

void setup() {
  // Servo set up and initialization
  transitionHole_servo.attach(servoPin);
  transitionHole_servo.write(transitionHole_closed);

  // Arduino PIN setup
  pinMode(8, OUTPUT);

  // Serial Monitor set up
  Serial.begin(9600);
}

void loop() {

  // Plinko Section
  if (state_plinko == true){
    // read sensors continuously until a hit on contact sensors is detected, then figure out what voltage reading is and add to player score
    pointSensor_15_value = analogRead(pointValue_15_sensorPin);
    pointSensor_610_value = analogRead(pointValue_610_sensorPin);
    exitHole_left_sensorValue = analogRead(exitHole_left_sensorPin);
    exitHole_middle_sensorValue = analogRead(exitHole_middle_sensorPin);
    exitHole_right_sensorValue = analogRead(exitHole_right_sensorPin);
    
    // Impact sensor impulse detection
    impulseDetecton(pointSensor_15_value, lightSensor_threshold, maxImpulse);
    impulseDetecton(pointSensor_610_value, lightSensor_threshold, maxImpulse);

    // Exit hole light sensor impulse detection
        // After exiting exit holes, enter Upper step state
        state_plinko = false;
        state_upperStep = true;
      //
  }

  // Upper step section
  if (state_upperStep == true){
    // watch transition hole light sensors for detections
    transitionHole_sensorValue = analogRead(transitionHole_sensorPin);
        // add function for light sensor detection here
    //thresholding for light sensors and point assignment(?)

    // when the light sensors detect something twice, then go to reveal state
    if (detectionCount >= 2){
      state_reveal = true;
      state_upperStep = false;
    }
  }

  // Reveal State
  if (state_reveal == true){

  }

  // Lower State
  if (state_lowerStep == true){

  }

  // Servo Reset
  if (state_reset_transiton == true){
    
  }
  // Move servo to drop ball to lower level
  if (detectedHand = true) {
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
  if (finalHole == true) {
    // restart system

  }
  // End
  
}

// Functions

void exitHole_detection(int exitHole_sensorValue){
  //theshold detection for falling analog signal
  if (exitHole_sensorValue < detectionThreshold){
    exitHole_pointAssignment(exitHole_sensorValue);
  }
}

void impulseDetecton(int readValue, int threshold, int max){
  //checks if readValue reaches the impulse detection threshold
  if readValue > threshold{
    // if the impulse detection threshold is met, then continuously update max
    if readValue > max{
      max = readValue;
      detectImpulse = true;
    }else{
      // if the detection threshold is met again, then stop running and calculate points
      if readValue < (threshold + 5){
        calculatePoints(max);
        detectImpulse = false;
      }
    }
  }
}

int calculatePoints(int impulsePeak){
  if (impulsePeak > (pointValue_1_threshold - point_threshold)) && (impulsePeak < (pointValue_1_threshold + point_threshold)){
    playerScore &plus;= pointValue_1;
  }elseif (impulsePeak > (pointValue_2_threshold - point_threshold)) && (impulsePeak < (pointValue_2_threshold + point_threshold)){
    playerScore &plus;= pointValue_2;
  }elseif (impulsePeak > (pointValue_3_threshold - point_threshold)) && (impulsePeak < (pointValue_3_threshold + point_threshold)){
    playerScore &plus;= pointValue_3;
  }elseif (impulsePeak > (pointValue_4_threshold - point_threshold)) && (impulsePeak < (pointValue_4_threshold + point_threshold)){
    playerScore &plus;= pointValue_4;
  }elseif (impulsePeak > (pointValue_5_threshold - point_threshold)) && (impulsePeak < (pointValue_5_threshold + point_threshold)){
    playerScore &plus;= pointValue_5;
  }
  return(playerScore);
}

void detectedHandISR(){
  // when light beam is broken, incriment detection count
  mainEventflag |= detectedHandISR;
  detectedHand = true;
}

void finalHoleISR(){
  // wheen light beam is broken, reset game
}

