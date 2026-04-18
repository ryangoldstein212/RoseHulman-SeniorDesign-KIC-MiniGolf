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
int lightSensor_threshold = 50;
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

int detectionCount = 0;

// System State switches
bool impulseDetected = false;
bool detectedHand = false;
bool finalHole = false;

// counter and timer setting
int handDetection_count = 0;
int playerScore = 0;
long startTime = 0;
long timer = 0;

// ISR flags
volatile int mainEventflag = 0;
#define detectHand_flag 0x01 // hand detection event flag
#define finalHole_flag 0x02 // final hole event flag

// Transition hole servo object
Servo transitionHole_servo;

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
  
  // Plinko and upper step system are on only until detection count is at 2
  if (detectionCount >= 2) {
    // if the detection count is met, then turn off plinko and move to next section
    detectedHand = true;
  }else{
    // read sensors continuously until a hit on contact sensors is detected, then figure out what voltage reading is and add to player score
    pointSensor_15_value = analogRead(pointValue_15_sensorPin);
    pointSensor_610_value = analogRead(pointValue_610_sensorPin);

    impulseDetecton(pointSensor_15_value);
    impulseDetecton(pointSensor_610_value);
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
  // Lower Step an reset system
  if (finalHole == true) {
    // restart system

  }
  // End
  
  
  /*
  //testing
  transitionHole_servo.write(tranistionHole_open);
  
  int transitionHole_sensorValue = analogRead(transitionHole_sensorPin);
  Serial.print(digitalRead(8));
  Serial.print("\n       ");
  */
  
  
}

// Functions

void impulseDetecton(int readValue, int threshold){
  int max = 0;
  bool detectImpulse = false
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