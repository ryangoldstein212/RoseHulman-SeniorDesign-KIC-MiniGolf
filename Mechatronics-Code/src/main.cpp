#include <Arduino.h>
#include <Servo.h>

// Digital Pin definitions
#define servoPin 9
\
// Analog Pin definitions
#define transitionHole_sensorPin 0
#define pointValue_15_sensorPin 1 // analog port for sensors 1-5
#define pointValue_610_sensorPin 2 // analog port for sensors 6-10

// Global variable setting
int transitionHole_closed = 90;
int transitionHole_open = 150;
int lightSensor_threshold = 50;
int point_threshold = 20;
long pointValue_1_threshold = 325;
long pointValue_2_threshold = 415;
long pointValue_3_threshold = 620;
long pointValue_4_threshold = 690;
long pointValue_5_threshold = 830;
int detectionCount = 0;

bool detectedHand = false;
bool finalHole = false;

// counter and timer setting
int handDetection_count = 0;
int playerScore = 0;
int pointValue_1 = 25;
int pointValue_2 = 50;
int pointValue_3 = 75;
int pointValue_4 = 100;
int pointValue_5 = 125;

// Transition hole servo object
Servo transitionHole_servo;

// put function declarations here:
int myFunction(int, int);

void setup() {
  // put your setup code here, to run once:
  transitionHole_servo.attach(servoPin);

  Serial.begin(9600);
}

void loop() {
  
  // Plinko and upper step system are on only until detection count is at 2
  
  

  // Upper Step into Transition Hole
  if (detectionCount >= 2) {
    detectedHand = true;
  }else{
    analogRead(pointValue_15_sensorPin);
    analogRead(pointValue_610_sensorPin);

  }

  if (detectedHand = true) {
    // When a hand is detected, open transition hole and then set timer for 30 sec before closing
    transitionHole_servo.write(transitionHole_open);
    detectionCount = 0;
  }

  if (detectedHand_time = 30) {
    // After 30 seconds has passed, close servo and end detectedHand state
    transitionHole_servo.write(transitionHole_closed);
    detectedHand = false;
  }
  // Lower Step
  if (finalHole == true) {

  }
  // End

  /*

  //testing
  transitionHole_servo.write(tranistionHole_open);
  
  int transitionHole_sensorValue = analogRead(transitionHole_sensorPin);
  Serial.print(transitionHole_sensorValue);
  Serial.print("\n       ");
  
  */
  
}

// put function definitions here:
int myFunction(int x, int y) {
  return x + y;
}

long countScore(int ){

}

void detectedHandISR(){
  // when light beam is broken, incriment detection count
}

void finalHoleISR(){
  // 
}