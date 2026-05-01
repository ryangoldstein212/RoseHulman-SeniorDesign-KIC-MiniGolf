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

uint16_t maxImpulse_15 = 0;
uint16_t maxImpulse_610 = 0;

uint8_t exitHole_sensor_tolerance = 7;
uint8_t exitHole_left_threshold = 20;
uint8_t exitHole_middle_threshold = 30;
uint8_t exitHole_right_threshold = 40;
uint8_t lightSensor_tolerance = 150; // light sensor values must be below this value for detection to be possible
bool exitHole_detected = false;

uint16_t exitHole_left_max = 0;
uint16_t exitHole_middle_max = 0;
uint16_t exitHole_right_max = 0;

// Transition Hole Read Value initialization
uint16_t transitionHole_sensorValue;
uint16_t transitionHole_max = 0;

// Transition Hole IR detection
uint16_t transitionHole_sensor_threshold = 890;
uint16_t transitionHole_points = 100;

// Final Hole Read Value initialization
uint16_t finalHole_sensorValue;

// Final Hole IR detection
uint16_t finalHole_sensor_threshold = 880;
uint16_t finalHole_max = 0;
uint16_t finalHole_points = 300;

// Impulse Detection


//Point assignments
uint16_t pointValue_1 = 10;
uint16_t pointValue_2 = 20;
uint16_t pointValue_3 = 30;
uint16_t pointValue_4 = 40;
uint16_t pointValue_5 = 50;
uint16_t exitHole_left_pointValue = 50;
uint16_t exitHole_middle_pointValue = 100;
uint16_t exitHole_right_pointValue = 50;

// Transition Hole Globals
Servo transitionHole_servo; // servo control object
uint8_t handDetection_count = 0;
uint8_t transitionHole_closed = 10;
uint8_t transitionHole_open = 65;
bool detectedHand = false;

// System State switches
enum state{
  welcome, plinko, upperStep, reveal, lowerStep, reset_transition, reset_system
};

state currentState = plinko;

// counters and timers
uint16_t playerScore = 0;
uint32_t startTime = 0;
uint32_t detectedHand_time = 0;
uint32_t reset_time; 
bool startTimer = false;

// Function Declarations
void sensorTesting(uint16_t impulse, uint16_t target, uint16_t threshold);
uint16_t impulseDetection(uint16_t readValue, uint16_t threshold, uint16_t max);
void impactSensor_calculatePoints(uint16_t impulsePeak);
uint16_t infraredSensor_detection(uint16_t sensorValue, uint16_t threshold, uint16_t max, void (*pointsFunction)(uint16_t));
void exitHole_pointAssignment(uint16_t detectionPeak);
void transitionHole_function(uint16_t detectionPeak);
void finalHole_function(uint16_t detectionPeak);
void moveMotor(uint8_t upDown);



void setup() {
  // Servo set up and initialization
  transitionHole_servo.attach(servo_Pin);
  transitionHole_servo.write(transitionHole_closed);

  // Motor Set Up
  pinMode(finalHole_motor_down_input, 1);
  pinMode(finalHole_motor_up_input, 1);
  pinMode(finalHole_motor_enable, 1);
  
  Serial.println("Entering Plinko");
  // Serial Monitor set up
  Serial.begin(9600);
}


void loop() {
  switch (currentState){
    // Welcome State
    case welcome:
      // goes into plinko state when transition hole ir sensor detects a hand. LED display 
      break;
    
    // Plinko Section
    case plinko: {
      // read sensors continuously until a hit on contact sensors is detected, then figure out what voltage reading is and add to player score
      uint16_t pointSensor_15_value = analogRead(pointValue_15_sensorPin);
      uint16_t pointSensor_610_value = analogRead(pointValue_610_sensorPin);
      uint16_t exitHole_left_sensorValue = analogRead(exitHole_left_sensorPin);
      uint16_t exitHole_middle_sensorValue = analogRead(exitHole_middle_sensorPin);
      uint16_t exitHole_right_sensorValue = analogRead(exitHole_right_sensorPin);

      // Impact sensor impulse detection
      maxImpulse_15 = impulseDetection(pointSensor_15_value, impactSensor_threshold, maxImpulse_15);
      maxImpulse_610 = impulseDetection(pointSensor_610_value, impactSensor_threshold, maxImpulse_610);

      // Exit hole light sensor impulse detection
      exitHole_left_max = infraredSensor_detection(exitHole_left_sensorValue, exitHole_left_threshold, exitHole_left_max, exitHole_pointAssignment);
      exitHole_middle_max = infraredSensor_detection(exitHole_middle_sensorValue, exitHole_middle_threshold, exitHole_middle_max, exitHole_pointAssignment);
      exitHole_right_max = infraredSensor_detection(exitHole_right_sensorValue, exitHole_right_threshold, exitHole_right_max, exitHole_pointAssignment);
      Serial.print(exitHole_detected);
      Serial.print("        ");
      if (exitHole_detected == true){
        //currentState = upperStep;
        Serial.println("Leaving Plinko");
      }
      break;
      }


  // Upper step section
    case upperStep:{
      Serial.println("Entering Upper Step");
      // watch transition hole light sensors for detections
      uint16_t transitionHole_sensorValue = analogRead(transitionHole_sensorPin);
      transitionHole_max = infraredSensor_detection(transitionHole_sensorValue, transitionHole_sensor_threshold, transitionHole_max, transitionHole_function);
      
      Serial.print(handDetection_count);
      Serial.print("          ");
      Serial.println(transitionHole_sensorValue);
      
      // when the light sensors detect something twice, then go to reveal state
      if (handDetection_count >= 2){
        currentState = reveal;
        Serial.println("Leaving Upper Step");
      }
    break;
    }


  // Reveal State
    case reveal:{
      Serial.println("Entering Reveal Step");
      // swing transition hole open, start servo reset timer
      transitionHole_servo.write(transitionHole_open);
      uint16_t currentTime = millis();
      reset_time = (currentTime - startTime)/1000;

      // Lower final hole motor

      // play zelda discover treasure theme

      // exit state to lower step phase
      currentState = lowerStep;
      Serial.println("Leaving Reveal Step");
      break;
    }


  // Lower State
    case lowerStep:{
      Serial.println("Entering Lower Step");
      // Ball detection and final score assignment
      finalHole_sensorValue = analogRead(finalHole_sensorPin);
      finalHole_max = infraredSensor_detection(finalHole_sensorValue, finalHole_sensor_threshold, finalHole_max, finalHole_function);

      if (startTimer == true){
        reset_time = millis() - startTime;
        
      }
      break;
    }
    

  // Servo Reset
    case reset_transition:{
    
      break;
    }
  // return servo to initial position
  if (reset_time >= 30) {
    // After 30 seconds has passed, close servo and end detectedHand state
    transitionHole_servo.write(transitionHole_closed);
    detectedHand = false;
  }

  // Lower Step and reset system
    case reset_system:{
    // restart system, move motor up, 
    //moveMotor(true);

    // End
      break;
    }
  }
}

// Functions 
uint16_t infraredSensor_detection(uint16_t sensorValue, uint16_t threshold, uint16_t max, void (*pointsFunction)(uint16_t)){
  // Checks if sensorValue falls below ir detection threshold
  if (sensorValue < threshold){
    // if detection threshold is met, then continuously update max if max is less than current read value
    if (sensorValue > max){
      return sensorValue;
    }else{
      // if detection threshold is not met, then return what was passed in
      return max;
    }
  }
  else if (sensorValue > threshold + 10){
    // if detection threshold is reached again, then update points and reset max to zero
    pointsFunction(max);
    return 0;
  }
  // if no detection, return keep max the same
  return max;
}

uint16_t impulseDetection(uint16_t readValue, uint16_t threshold, uint16_t max){
  //checks if readValue reaches the impulse detection threshold
  if (readValue > threshold){
    // if the impulse detection threshold is met, then continuously update max if max is less than current read value
    if (readValue > max){
      Serial.println("yo");
      return readValue; // updates max to readValue 
    }
    else{
      Serial.println("gurt");
      return max;
    }
  }
  // if the detection threshold is met again, then stop running, calculate points, and reset max back to zero
  else if (readValue < threshold){
    Serial.print("whats up?");
    impactSensor_calculatePoints(max);
    return 0;
  }
  // if no detection, return keep max the same
  Serial.println("the ceiling");
  return max;
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

void exitHole_pointAssignment(uint16_t detectionPeak){
  exitHole_detected = true;
  // checking from highest to lowest point values
  if (detectionPeak >= exitHole_middle_threshold){
    playerScore += exitHole_middle_pointValue;
  }
  else if (detectionPeak >= exitHole_left_threshold){
    playerScore += exitHole_left_pointValue;
  }
  else if (detectionPeak >= exitHole_right_threshold){
    playerScore += exitHole_right_pointValue;
  }
}

void transitionHole_function(uint16_t detectionPeak){
  handDetection_count += 1;
}

void finalHole_function(uint16_t detectionPeak){
  // if ran, then already in final hole, so give player points and begin reset timer
  playerScore += finalHole_points;
  startTimer = true;
}

void moveMotor(uint8_t upDown){
  pinMode(finalHole_motor_enable, 1);
  if (upDown == 1){
    // Move motor up
    digitalWrite(finalHole_motor_up_input, 1);
    digitalWrite(finalHole_motor_down_input, 0);
  }
  if (upDown == 0){
    // move motor down

    digitalWrite(finalHole_motor_up_input, 0);
    digitalWrite(finalHole_motor_down_input, 1);
  }
}

void sensorTesting(uint16_t impulse, uint16_t target, uint16_t tolerance){
  if ((impulse > (target - tolerance)) && (impulse < (target + tolerance))){
        transitionHole_servo.write(transitionHole_open);
        delay(1000);
        transitionHole_servo.write(transitionHole_closed);
  }
}