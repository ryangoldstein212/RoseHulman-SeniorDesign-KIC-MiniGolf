#include <Arduino.h>
#include <Servo.h>

// Digital Pin definitions
#define servo_Pin 2
#define finalHole_motor_up_input 3
#define finalHole_motor_down_input 4
#define finalHole_motor_enable 5

// Analog Pin definitions
#define transitionHole_sensorPin 5 // analog port for transition hole light sensor
#define finalHole_sensorPin 6 // analog port for final hole light sensor
#define exitHole_left_sensorPin 4 // analog port for plinko left ball exit 
#define exitHole_middle_sensorPin 3 //analog port for plinko middle ball exit
#define exitHole_right_sensorPin 2 // analog port for plinko right ball exit
#define pointValue_15_sensorPin 1 // analog port for plinko touch sensors 1-5
#define pointValue_610_sensorPin 0 // analog port for plinko touch sensors 6-10

// Plinko Sensor Read Values Initialization
uint16_t pointSensor_15_value;
uint16_t pointSensor_610_value;
uint16_t exitHole_left_sensorValue;
uint16_t exitHole_middle_sensorValue;
uint16_t exitHole_right_sensorValue;

//Plinko IR detection 
uint8_t point_tolerance = 25;
uint16_t pointValue_1_threshold = 830;
uint16_t pointValue_2_threshold = 680;
uint16_t pointValue_3_threshold = 600;
uint16_t pointValue_4_threshold = 400;
uint16_t pointValue_5_threshold = 310;
uint8_t impactSensor_threshold = 20; // impact sensor values must be above this value for detection to be possible

uint8_t exitHole_sensor_tolerance = 20;
uint8_t exitHole_left_threshold = 5;
uint8_t exitHole_middle_threshold = 70;
uint8_t exitHole_right_threshold = 45;
uint8_t lightSensor_tolerance = 150; // light sensor values must be below this value for detection to be possible

// Transition Hole Read Value initialization
uint16_t transitionHole_sensorValue;

// Transition Hole IR detection
uint16_t transitionHole_sensor_threshold = 960;

// Final Hole Read Value initialization
uint16_t finalHole_sensorValue;

// Final Hole IR detection
uint16_t finalHole_sensor_threshold = 960;

// Impulse Detection Globals
uint16_t maxImpulse_15 = 0;
uint16_t maxImpulse_610 = 0;

//Point assignments
uint16_t pointValue_1 = 1;
uint16_t pointValue_2 = 2;
uint16_t pointValue_3 = 3;
uint16_t pointValue_4 = 4;
uint16_t pointValue_5 = 5;
uint16_t exitHole_left_pointValue = 20;
uint16_t exitHole_middle_pointValue = 20;
uint16_t exitHole_right_pointValue = 20;


// Transition Hole Globals
Servo transitionHole_servo; // servo control object
uint8_t detectionCount = 0;
uint8_t transitionHole_closed = 10;
uint8_t transitionHole_open = 65;
bool detectedHand = false;

// System State switches
bool state_plinko = false;
bool state_upperStep = false;
bool state_reveal = false;
bool state_lowerStep = false;
bool state_reset_system = false;
bool state_reset_transiton = false;

// counters and timers
uint16_t playerScore = 0;
uint16_t startTime = 0;
uint16_t detectedHand_time = 0;

// Transition hole globals


// Function Declarations
void sensorTesting(uint16_t impulse, uint16_t target, uint16_t threshold);
uint16_t impulseDetection(uint16_t readValue, uint16_t threshold, uint16_t max);
void impactSensor_calculatePoints(uint16_t impulsePeak);
void exitHole_pointAssignment();
void exitHole_detection(uint16_t exitHole_sensorValue);


void setup() {
  // Servo set up and initialization
  transitionHole_servo.attach(servo_Pin);
  transitionHole_servo.write(transitionHole_closed);

  // Motor Set Up
  pinMode(finalHole_motor_down_input, 1);
  pinMode(finalHole_motor_up_input, 1);
  pinMode(finalHole_motor_enable, 1);

  state_plinko = true;
  // Serial Monitor set up
  Serial.begin(9600);
}


void loop() {
  // Plinko Section
  if (state_plinko == true){
    // read sensors continuously until a hit on contact sensors is detected, then figure out what voltage reading is and add to player score
    uint16_t pointSensor_15_value = analogRead(pointValue_15_sensorPin);
    uint16_t pointSensor_610_value = analogRead(pointValue_610_sensorPin);
    uint16_t exitHole_left_sensorValue = analogRead(exitHole_left_sensorPin);
    uint16_t exitHole_middle_sensorValue = analogRead(exitHole_middle_sensorPin);
    uint16_t exitHole_right_sensorValue = analogRead(exitHole_right_sensorPin);
    
    // Impact sensor impulse detection
    sensorTesting(pointSensor_15_value, pointValue_3_threshold, impactSensor_threshold);
    maxImpulse_15 = impulseDetection(pointSensor_15_value, impactSensor_threshold, maxImpulse_15);
    maxImpulse_610 = impulseDetection(pointSensor_610_value, impactSensor_threshold, maxImpulse_610);

    // Exit hole light sensor impulse detection
    /*
    Serial.print(pointSensor_15_value);
    Serial.print("        ");
    Serial.print(pointSensor_610_value);
    Serial.print("        ");
    Serial.println(playerScore);
    */
    
        // After exiting exit holes, enter Upper step state

      //
  }

  // Upper step section
  if (state_upperStep == true){
    // watch transition hole light sensors for detections
    uint16_t transitionHole_sensorValue = analogRead(transitionHole_sensorPin);
        // add function for light sensor detection here
    //thresholding for light sensors and point assignment(?)

    // Sensor testing w/out serial monitor
    /*
    long impulsePeak = transitionHole_sensorValue;
    long tolerance = lightSensor_tolerance;
    long threshold = transitionHole_threshold;
    */
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
      uint16_t currentTime = millis();
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

void exitHole_detection(uint16_t exitHole_sensorValue){
  //theshold detection for falling analog signal
  if (exitHole_sensorValue < lightSensor_tolerance){
    exitHole_pointAssignment();
  }
}

uint16_t impulseDetection(uint16_t readValue, uint16_t threshold, uint16_t max){
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


void impactSensor_calculatePoints(uint16_t impulsePeak){
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

void sensorTesting(uint16_t impulse, uint16_t target, uint16_t tolerance){
  if ((impulse > (target - tolerance)) && (impulse < (target + tolerance))){
        transitionHole_servo.write(transitionHole_open);
        delay(1000);
        transitionHole_servo.write(transitionHole_closed);
  }
}