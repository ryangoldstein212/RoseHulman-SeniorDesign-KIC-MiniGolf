#include <Arduino.h>
#include <Servo.h>

//LED matrix ------------------------------------------------
#include <MD_MAX72xx.h>
#include <SPI.h>

#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 8
#define CS_PIN 53

MD_MAX72XX mx = MD_MAX72XX(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);

// Your confirmed orientation settings
bool TOP_IS_FIRST = false;
bool REVERSE_TOP_X = true;
bool REVERSE_BOTTOM_X = false;
bool FLIP_TOP_Y = false;
bool FLIP_BOTTOM_Y = true;

// Scrolling speed
int SCROLL_DELAY = 40;

// text to display when LED is not used as a score board
String ScoreBoardText = "Rose Show";

// Non-blocking display state
String displayMessage = "";
bool scrollingActive = false;

int scrollOffset = 32;
int scrollEndOffset = 0;

unsigned long lastScrollUpdate = 0;


// LED Matrix ^ --------------------------------------------

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
uint8_t exitHole_left_threshold = 10;
uint8_t exitHole_middle_threshold = 20;
uint8_t exitHole_right_threshold = 30;
uint8_t lightSensor_tolerance = 150; // light sensor values must be below this value for detection to be possible
bool exitHole_detected = false;

uint16_t exitHole_left_max = 0;
uint16_t exitHole_middle_max = 0;
uint16_t exitHole_right_max = 0;

// Transition Hole Read Value initialization
uint16_t transitionHole_sensorValue;
uint16_t transitionHole_max = 0;

// Transition Hole IR detection
uint16_t transitionHole_sensor_threshold = 825;
uint16_t transitionHole_points = 100;

// Final Hole Read Value initialization
uint16_t finalHole_sensorValue;

// Final Hole IR detection
uint16_t finalHole_sensor_threshold = 825;
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
uint8_t transitionHole_closed = 15;
uint8_t transitionHole_open = 65;
bool detectedHand = false;

// Final Hole Globals
bool homed = false;
uint16_t upTime = 2000; // time motor moves up from homed position to top
uint16_t downTime = 1000; // time motor moves down from homed position to bottom
bool motorMoving = false; // flag for when the motor moves or not
uint32_t motorStartTime = 0;

// Reset Globals
bool waiting = false; // flag for when to wait before activating the motor during reset
uint32_t waitStartTime = 0;
uint16_t waitTime = 1000;


// System State switches
enum state {
  welcome, plinko, upperStep, reveal, lowerStep, reset_transition, reset_system
};

state currentState = plinko;

// counters and timers
uint16_t playerScore = 0;
uint32_t startTime = 0;
uint32_t detectedHand_time = 0;
uint32_t reset_time; 
bool startTimer = false;
bool ballEntered = false;

// Function Declarations
void sensorTesting(uint16_t impulse, uint16_t target, uint16_t threshold);
void impulseDetection(uint16_t threshold);
void impactSensor_calculatePoints(uint16_t impulsePeak);
// uint16_t infraredSensor_detection(uint16_t threshold);
void exitHole_pointAssignment(uint16_t detectionPeak);
void transitionHole_function(uint16_t detectionPeak);
void finalHole_function(uint16_t detectionPeak);
void moveMotor(uint8_t upDown);

//LED function Declarations 
void displaySmartTextBottom(String text); 
void displayStaticTextBottom(String text);
void setPixelBottom(int x, int y, bool state); 
//void scrollTextBottom(String message, int scrollDelay);
void clearBottomOnly();
void drawCharBottom(char c, int startX); 
byte getCharRow(char c, int row);
void startScrollingTextBottom(String message);
void updateDisplayAnimation();
void setDisplayText(String text);

//LED letters and numbers------------
// 5x7 digit font
byte digitFont[10][7] = {
  {0b01110, 0b10001, 0b10011, 0b10101, 0b11001, 0b10001, 0b01110}, // 0
  {0b00100, 0b01100, 0b00100, 0b00100, 0b00100, 0b00100, 0b01110}, // 1
  {0b01110, 0b10001, 0b00001, 0b00010, 0b00100, 0b01000, 0b11111}, // 2
  {0b11110, 0b00001, 0b00001, 0b01110, 0b00001, 0b00001, 0b11110}, // 3
  {0b00010, 0b00110, 0b01010, 0b10010, 0b11111, 0b00010, 0b00010}, // 4
  {0b11111, 0b10000, 0b10000, 0b11110, 0b00001, 0b00001, 0b11110}, // 5
  {0b01110, 0b10000, 0b10000, 0b11110, 0b10001, 0b10001, 0b01110}, // 6
  {0b11111, 0b00001, 0b00010, 0b00100, 0b01000, 0b01000, 0b01000}, // 7
  {0b01110, 0b10001, 0b10001, 0b01110, 0b10001, 0b10001, 0b01110}, // 8
  {0b01110, 0b10001, 0b10001, 0b01111, 0b00001, 0b00001, 0b01110}  // 9
};

// 5x7 uppercase letter font
byte letterFont[26][7] = {
  {0b01110, 0b10001, 0b10001, 0b11111, 0b10001, 0b10001, 0b10001}, // A
  {0b11110, 0b10001, 0b10001, 0b11110, 0b10001, 0b10001, 0b11110}, // B
  {0b01110, 0b10001, 0b10000, 0b10000, 0b10000, 0b10001, 0b01110}, // C
  {0b11110, 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b11110}, // D
  {0b11111, 0b10000, 0b10000, 0b11110, 0b10000, 0b10000, 0b11111}, // E
  {0b11111, 0b10000, 0b10000, 0b11110, 0b10000, 0b10000, 0b10000}, // F
  {0b01110, 0b10001, 0b10000, 0b10111, 0b10001, 0b10001, 0b01110}, // G
  {0b10001, 0b10001, 0b10001, 0b11111, 0b10001, 0b10001, 0b10001}, // H
  {0b01110, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100, 0b01110}, // I
  {0b00111, 0b00010, 0b00010, 0b00010, 0b10010, 0b10010, 0b01100}, // J
  {0b10001, 0b10010, 0b10100, 0b11000, 0b10100, 0b10010, 0b10001}, // K
  {0b10000, 0b10000, 0b10000, 0b10000, 0b10000, 0b10000, 0b11111}, // L
  {0b10001, 0b11011, 0b10101, 0b10101, 0b10001, 0b10001, 0b10001}, // M
  {0b10001, 0b11001, 0b10101, 0b10011, 0b10001, 0b10001, 0b10001}, // N
  {0b01110, 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b01110}, // O
  {0b11110, 0b10001, 0b10001, 0b11110, 0b10000, 0b10000, 0b10000}, // P
  {0b01110, 0b10001, 0b10001, 0b10001, 0b10101, 0b10010, 0b01101}, // Q
  {0b11110, 0b10001, 0b10001, 0b11110, 0b10100, 0b10010, 0b10001}, // R
  {0b01111, 0b10000, 0b10000, 0b01110, 0b00001, 0b00001, 0b11110}, // S
  {0b11111, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100}, // T
  {0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b01110}, // U
  {0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b01010, 0b00100}, // V
  {0b10001, 0b10001, 0b10001, 0b10101, 0b10101, 0b10101, 0b01010}, // W
  {0b10001, 0b10001, 0b01010, 0b00100, 0b01010, 0b10001, 0b10001}, // X
  {0b10001, 0b10001, 0b01010, 0b00100, 0b00100, 0b00100, 0b00100}, // Y
  {0b11111, 0b00001, 0b00010, 0b00100, 0b01000, 0b10000, 0b11111}  // Z
};

struct SymbolMap {
  char symbol;
  byte rows[7];
};

SymbolMap symbolFont[] = {
  {'.', {0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b01100, 0b01100}},
  {',', {0b00000, 0b00000, 0b00000, 0b00000, 0b01100, 0b01100, 0b01000}},
  {':', {0b00000, 0b01100, 0b01100, 0b00000, 0b01100, 0b01100, 0b00000}},
  {';', {0b00000, 0b01100, 0b01100, 0b00000, 0b01100, 0b01100, 0b01000}},
  {'!', {0b00100, 0b00100, 0b00100, 0b00100, 0b00100, 0b00000, 0b00100}},
  {'?', {0b01110, 0b10001, 0b00001, 0b00010, 0b00100, 0b00000, 0b00100}},
  {'-', {0b00000, 0b00000, 0b00000, 0b11111, 0b00000, 0b00000, 0b00000}},
  {'_', {0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b11111}},
  {'+', {0b00000, 0b00100, 0b00100, 0b11111, 0b00100, 0b00100, 0b00000}},
  {'=', {0b00000, 0b00000, 0b11111, 0b00000, 0b11111, 0b00000, 0b00000}},
  {'/', {0b00001, 0b00010, 0b00010, 0b00100, 0b01000, 0b01000, 0b10000}},
  {'\\',{0b10000, 0b01000, 0b01000, 0b00100, 0b00010, 0b00010, 0b00001}},
  {'|', {0b00100, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100}},
  {'\'',{0b00100, 0b00100, 0b01000, 0b00000, 0b00000, 0b00000, 0b00000}},
  {'"', {0b01010, 0b01010, 0b01010, 0b00000, 0b00000, 0b00000, 0b00000}},
  {'(', {0b00010, 0b00100, 0b01000, 0b01000, 0b01000, 0b00100, 0b00010}},
  {')', {0b01000, 0b00100, 0b00010, 0b00010, 0b00010, 0b00100, 0b01000}},
  {'[', {0b01110, 0b01000, 0b01000, 0b01000, 0b01000, 0b01000, 0b01110}},
  {']', {0b01110, 0b00010, 0b00010, 0b00010, 0b00010, 0b00010, 0b01110}},
  {'<', {0b00010, 0b00100, 0b01000, 0b10000, 0b01000, 0b00100, 0b00010}},
  {'>', {0b01000, 0b00100, 0b00010, 0b00001, 0b00010, 0b00100, 0b01000}},
  {'#', {0b01010, 0b01010, 0b11111, 0b01010, 0b11111, 0b01010, 0b01010}},
  {'*', {0b00000, 0b10101, 0b01110, 0b11111, 0b01110, 0b10101, 0b00000}},
  {'%', {0b11001, 0b11010, 0b00010, 0b00100, 0b01000, 0b01011, 0b10011}},
  {' ', {0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000}}
};

int symbolCount = sizeof(symbolFont) / sizeof(symbolFont[0]);
// LED ^-----

void setup() {


  // Servo set up and initialization
  transitionHole_servo.attach(servo_Pin);
  transitionHole_servo.write(transitionHole_closed);

  // Motor Set Up
  pinMode(finalHole_motor_down_input, 1);
  pinMode(finalHole_motor_up_input, 1);
  pinMode(finalHole_motor_enable, 1);
  
  // Lower step motor homing sequence
  finalHole_sensorValue = analogRead(finalHole_sensorPin);

  // if the sensor is broken, ie. the platform and/or rack is blocking sensor, then move the rack down
  if (finalHole_sensorValue < finalHole_sensor_threshold){
    digitalWrite(finalHole_motor_enable, 1);
    while (finalHole_sensorValue < finalHole_sensor_threshold){
      finalHole_sensorValue = analogRead(finalHole_sensorPin);
      digitalWrite(finalHole_motor_up_input, 0);
      digitalWrite(finalHole_motor_down_input, 1);
    }
    // turn off motor
    digitalWrite(finalHole_motor_enable, 0);
    digitalWrite(finalHole_motor_down_input, 0);
    digitalWrite(finalHole_motor_up_input, 0);
    homed = true;
  }
  // if the sensor is not broken, i.e. the platform is below the sensor, then move the rack up
  else if (finalHole_sensorValue > finalHole_sensor_threshold){
    digitalWrite(finalHole_motor_enable, 1);
    while (finalHole_sensorValue > finalHole_sensor_threshold){
      finalHole_sensorValue = analogRead(finalHole_sensorPin);
      // move motor up
      digitalWrite(finalHole_motor_down_input, 0);
      digitalWrite(finalHole_motor_up_input, 1);
    }
    // turn off motor 
    digitalWrite(finalHole_motor_enable, 0);
    digitalWrite(finalHole_motor_down_input, 0);
    digitalWrite(finalHole_motor_up_input, 0);
    homed = true;
  }
  else{
    homed = true;
  }

  // LED Set Up
  mx.begin();
  mx.control(MD_MAX72XX::INTENSITY, 5);
  mx.clear();
  
  // once rack is in standard position, then move rack up
  if (homed == true){
    digitalWrite(finalHole_motor_enable, 1);
    digitalWrite(finalHole_motor_down_input, 0);
    digitalWrite(finalHole_motor_up_input, 1);
    delay(upTime);

    // turn motor off again
    digitalWrite(finalHole_motor_enable, 0);
    digitalWrite(finalHole_motor_down_input, 0);
    digitalWrite(finalHole_motor_up_input, 0);
  }

  Serial.println("Entering Plinko");
  // Serial Monitor set up
  Serial.begin(9600);
}


void loop() {
  updateDisplayAnimation();
  switch (currentState){
    // Welcome State
    case welcome:{
      // goes into plinko state when transition hole ir sensor detects a hand. LED display 
      setDisplayText(" Put hand in hole to start " + ScoreBoardText);
      //IR sensor code for hand in hole to start
      transitionHole_sensorValue = analogRead(transitionHole_sensorPin);
      Serial.println(transitionHole_sensorValue);
      if (transitionHole_sensorValue < transitionHole_sensor_threshold) {
        Serial.println(transitionHole_sensorValue);
        if (ballEntered) {
          Serial.println(ballEntered);
            currentState = plinko;
            Serial.println("Game Started - Place ball at Plinko");
            //break;
        } else {
          Serial.println(ballEntered);
            ballEntered = true;
            delay(1000);
        }
      }

      break;
    }

    
    
    // Plinko Section
    case plinko: {
      //displaySmartTextBottom("Start->");  // if not in welcome - display the score
      setDisplayText("Start->");
      // read sensors continuously until a hit on contact sensors is detected, then figure out what voltage reading is and add to player score
      pointSensor_15_value = analogRead(pointValue_15_sensorPin);
      pointSensor_610_value = analogRead(pointValue_610_sensorPin);
      exitHole_left_sensorValue = analogRead(exitHole_left_sensorPin);
      exitHole_middle_sensorValue = analogRead(exitHole_middle_sensorPin);
      exitHole_right_sensorValue = analogRead(exitHole_right_sensorPin);

      // Serial.print(exitHole_left_sensorValue);
      // Serial.print("       ");
      // Serial.print(exitHole_middle_sensorValue);
      // Serial.print("       ");
      // Serial.println(exitHole_right_sensorValue);

      // Impact sensor impulse detection
      if (pointSensor_15_value > impactSensor_threshold) {
        impulseDetection(impactSensor_threshold);
        
      }
      if (pointSensor_610_value > impactSensor_threshold) {
        impulseDetection(impactSensor_threshold);
      }

      // Exit hole light sensor impulse detection
      if (exitHole_left_sensorValue < exitHole_left_threshold) {
        // switch case and deal with points
        playerScore += exitHole_left_pointValue;
        Serial.println(playerScore);
        currentState = upperStep;
        Serial.println("Leaving Plinko");
        setDisplayText(static_cast<String>(playerScore));  // if not in welcome - display the score

      }
      if (exitHole_middle_sensorValue < exitHole_middle_threshold) {
        // Serial.println(exitHole_middle_sensorValue);
        playerScore += exitHole_middle_pointValue;
        Serial.println(playerScore);
        currentState = upperStep;
        Serial.println("Leaving Plinko");
        setDisplayText(String(playerScore));
      }
      if (exitHole_right_sensorValue < exitHole_right_threshold) {
        playerScore += exitHole_right_pointValue;
        Serial.println(playerScore);
        currentState = upperStep;
        Serial.println("Leaving Plinko");
        setDisplayText(String(playerScore));
      }
      // Serial.print(exitHole_detected);
      // Serial.print("        ");
      // if (exitHole_detected == true){
      //   //currentState = upperStep;
      //   Serial.println("Leaving Plinko");
      // }

      //displaySmartTextBottom(static_cast<String>(playerScore));  // if not in welcome - display the score
      
      break;
      }


  // Upper step section
    case upperStep:{
      // watch transition hole light sensors for detections
      setDisplayText(String(playerScore));
      transitionHole_sensorValue = analogRead(transitionHole_sensorPin);
      if (transitionHole_sensorValue < transitionHole_sensor_threshold) {
        Serial.println(transitionHole_sensorValue);
        if (ballEntered) {
          Serial.println(ballEntered);
            currentState = reveal;
            Serial.println("Leaving Upper Step");
        } else {
          Serial.println(ballEntered);
            ballEntered = true;
            delay(1000);
        }
    }
      
    //   Serial.print(handDetection_count);
    //   Serial.print("          ");
    //   Serial.println(transitionHole_sensorValue);
      
      // when the light sensors detect something twice, then go to reveal state
    //   if (handDetection_count >= 2){
    //     currentState = reveal;
    //     Serial.println("Leaving Upper Step");
    //   }
    setDisplayText(static_cast<String>(playerScore));  // if not in welcome - display the score
    break;
    }


  // Reveal State
    case reveal:{
      // move servo out and begin moving motor down
      if (motorMoving == false){
      transitionHole_servo.write(transitionHole_open);

      digitalWrite(finalHole_motor_enable, 1);
      digitalWrite(finalHole_motor_down_input, 1);
      digitalWrite(finalHole_motor_up_input, 0);
      delay(7300);
      digitalWrite(finalHole_motor_enable, 0);
      setDisplayText("!!");  // something happened display something fun
      delay(1000);

      motorStartTime = millis();
      motorMoving = true;
      
      // delay(7300);
      // digitalWrite(finalHole_motor_enable, 0);
      // delay(1000);
      }

      if (motorMoving == true && (millis() - motorStartTime >= downTime)){
        // motor should be at bottom now, so turn off motor
        digitalWrite(finalHole_motor_enable, 0);
        digitalWrite(finalHole_motor_down_input, 0);

      // exit state to lower step phase
      currentState = lowerStep;
      Serial.println("Leaving Reveal Step");
      setDisplayText(static_cast<String>(playerScore));  // if not in welcome - display the score
        motorMoving = false;

        currentState = lowerStep;
        Serial.println("Leaving Reveal Step");

        // play zelda discover treasure theme

        // exit state to lower step phase
      
      }
      break;

    }


  // Lower State
    case lowerStep:{
      // Ball detection and final score assignment
      setDisplayText(static_cast<String>(playerScore));  // if not in welcome - display the score
      finalHole_sensorValue = analogRead(finalHole_sensorPin);
      if (finalHole_sensorValue < finalHole_sensor_threshold && motorMoving == false && waiting == false) {
        // do nothing until timer hits specified number
        waitStartTime = millis();
        waiting = true;

      // motor move up after wait timer expires
      if (waiting == true && (millis() - waitStartTime >= waitTime)){
        digitalWrite(finalHole_motor_enable, 1); // turns motor on
        digitalWrite(finalHole_motor_down_input, 0);
        digitalWrite(finalHole_motor_up_input, 1);

        motorStartTime = millis();
        motorMoving = true;
        waiting = false;

      }
      
      // motor stops moving after move timer expires
      if (motorMoving == true && (millis() - motorStartTime >= upTime)){
          digitalWrite(finalHole_motor_enable, 0);
          digitalWrite(finalHole_motor_up_input, 0);

          motorMoving = false;
          currentState = plinko;
      }
        
      }
    //   finalHole_max = infraredSensor_detection(finalHole_sensorValue, finalHole_sensor_threshold, finalHole_max, finalHole_function);

      if (startTimer == true){
        reset_time = millis() - startTime;
        
      }
      setDisplayText(static_cast<String>(playerScore));  // if not in welcome - display the score
      break;
    }
    

  // Servo Reset
    case reset_transition:{
      setDisplayText(static_cast<String>(playerScore));  // if not in welcome - display the score
    
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
      setDisplayText(static_cast<String>(playerScore));  // if not in welcome - display the score
      break;
    }
  }
}

// Functions 
// uint16_t infraredSensor_detection(uint16_t threshold){
//   // Checks if sensorValue falls below ir detection threshold
//   if (sensorValue < threshold){
//     // if detection threshold is met, then continuously update max if max is less than current read value
//     if (sensorValue > max){
//       return sensorValue;
//     }else{
//       // if detection threshold is not met, then return what was passed in
//       return max;
//     }
//   }
//   else if ((sensorValue > threshold) && (max > 0)){
//     // if detection threshold is reached again, then update points and reset max to zero
//     pointsFunction(max);
//     return 0;
//   }
//   // if no detection, return keep max the same
//   return max;
// }

void impulseDetection(uint16_t threshold) {
    pointSensor_15_value = analogRead(pointValue_15_sensorPin);
    pointSensor_610_value = analogRead(pointValue_610_sensorPin);
    uint16_t maxValue = 0;
    while (pointSensor_15_value > threshold) {
        if (pointSensor_15_value > maxValue) {
            maxValue = pointSensor_15_value;
        }
        pointSensor_15_value = analogRead(pointValue_15_sensorPin);
    }

    while (pointSensor_610_value > threshold) {
        if (pointSensor_610_value > maxValue) {
            maxValue = pointSensor_610_value;
        }
        pointSensor_610_value = analogRead(pointValue_610_sensorPin);
    }

    if (maxValue > 0) {
        impactSensor_calculatePoints(maxValue);
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
  Serial.println(playerScore);
}

// void exitHole_pointAssignment(uint16_t detectionPeak){
//   exitHole_detected = true;
//   // checking from highest to lowest point values
//   if (detectionPeak >= exitHole_middle_threshold){
//     playerScore += exitHole_middle_pointValue;
//   }
//   else if (detectionPeak >= exitHole_left_threshold){
//     playerScore += exitHole_left_pointValue;
//   }
//   else if (detectionPeak >= exitHole_right_threshold){
//     playerScore += exitHole_right_pointValue;
//   }
// }

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


// LED matrix code -------------------------------------
void displaySmartTextBottom(String text) {
  text.toUpperCase();
  displayMessage = text;

  if (displayMessage.length() <= 5) {
    scrollingActive = false;
    displayStaticTextBottom(displayMessage);
  } else {
    startScrollingTextBottom(displayMessage);
  }
}

void startScrollingTextBottom(String message) {
  message.toUpperCase();

  displayMessage = message;
  scrollingActive = true;

  scrollOffset = 32;
  scrollEndOffset = -(displayMessage.length() * 6);

  lastScrollUpdate = 0;
}

void updateDisplayAnimation() {
  if (!scrollingActive) {
    return;
  }

  if (millis() - lastScrollUpdate < SCROLL_DELAY) {
    return;
  }

  lastScrollUpdate = millis();

  clearBottomOnly();

  for (int i = 0; i < displayMessage.length(); i++) {
    drawCharBottom(displayMessage[i], scrollOffset + i * 6);
  }

  scrollOffset--;

  if (scrollOffset < scrollEndOffset) {
    scrollOffset = 32;  // restart scrolling from the right
  }
}

// Static text using only bottom 32x8 row
void displayStaticTextBottom(String text) {
  clearBottomOnly();

  if (text.length() > 5) {
    text = text.substring(0, 5);
  }

  int charWidth = 6;
  int totalWidth = text.length() * charWidth - 1;
  int startX = (32 - totalWidth) / 2;

  for (int i = 0; i < text.length(); i++) {
    drawCharBottom(text[i], startX + i * charWidth);
  }
}

// Scroll text using only bottom 32x8 row
/*
void scrollTextBottom(String message, int scrollDelay) {
  message.toUpperCase();

  int totalWidth = message.length() * 6;
  int startOffset = 32;
  int endOffset = -totalWidth;

  for (int offset = startOffset; offset >= endOffset; offset--) {
    clearBottomOnly();

    for (int i = 0; i < message.length(); i++) {
      drawCharBottom(message[i], offset + i * 6);
    }

    delay(scrollDelay);
  }

  clearBottomOnly();
}
*/

// =======================================================
// BOTTOM ROW ONLY PIXEL MAPPING
// Logical bottom display: x = 0..31, y = 0..7
// =======================================================

void setPixelBottom(int x, int y, bool state) {
  if (x < 0 || x >= 32 || y < 0 || y >= 8) return;

  int localX = x;
  int localY = y;

  // Bottom module orientation from your working setup
  if (REVERSE_BOTTOM_X) {
    localX = 31 - localX;
  }

  if (FLIP_BOTTOM_Y) {
    localY = 7 - localY;
  }

  int physicalX;

  // Since TOP_IS_FIRST = false, bottom row is the first 32 pixels in the chain
  if (TOP_IS_FIRST) {
    physicalX = localX + 32;
  } else {
    physicalX = localX;
  }

  mx.setPoint(localY, physicalX, state);
}

void clearBottomOnly() {
  for (int y = 0; y < 8; y++) {
    for (int x = 0; x < 32; x++) {
      setPixelBottom(x, y, false);
    }
  }
}

// =======================================================
// DRAWING FUNCTIONS
// =======================================================

void drawCharBottom(char c, int startX) {
  int startY = 0;

  for (int row = 0; row < 7; row++) {
    byte rowData = getCharRow(c, row);

    for (int col = 0; col < 5; col++) {
      bool pixelOn = rowData & (1 << (4 - col));

      if (pixelOn) {
        setPixelBottom(startX + col, startY + row, true);
      }
    }
  }
}

byte getCharRow(char c, int row) {
  if (c >= '0' && c <= '9') {
    return digitFont[c - '0'][row];
  }

  if (c >= 'A' && c <= 'Z') {
    return letterFont[c - 'A'][row];
  }

  if (c >= 'a' && c <= 'z') {
    return letterFont[c - 'a'][row];
  }

  for (int i = 0; i < symbolCount; i++) {
    if (symbolFont[i].symbol == c) {
      return symbolFont[i].rows[row];
    }
  }

  return 0b00000;
}

void setDisplayText(String text) {
  text.toUpperCase();

  if (text != displayMessage) {
    displaySmartTextBottom(text);
  }
}

// LED matrix code ----------------------------^