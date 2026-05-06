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
uint16_t pointValue_1_threshold = 820;
uint16_t pointValue_2_threshold = 660;
uint16_t pointValue_3_threshold = 580;
uint16_t pointValue_4_threshold = 380;
uint16_t pointValue_5_threshold = 300;
uint8_t impactSensor_threshold = 200; // impact sensor values must be above this value for detection to be possible

uint16_t maxImpulse_15 = 0;
uint16_t maxImpulse_610 = 0;

uint8_t exitHole_sensor_tolerance = 7;
uint8_t exitHole_left_threshold = 50;
uint8_t exitHole_middle_threshold = 50;
uint8_t exitHole_right_threshold = 50;
uint8_t lightSensor_tolerance = 150; // light sensor values must be below this value for detection to be possible
bool exitHole_detected = false;

uint16_t exitHole_left_max = 0;
uint16_t exitHole_middle_max = 0;
uint16_t exitHole_right_max = 0;

// Transition Hole Read Value initialization
uint16_t transitionHole_sensorValue;
uint16_t transitionHole_max = 0;

// Transition Hole IR detection
uint16_t transitionHole_sensor_threshold = 300;
uint16_t transitionHole_points = 100;

// Final Hole Read Value initialization
uint16_t finalHole_sensorValue;

// Final Hole IR detection
uint16_t finalHole_sensor_threshold = 300;
uint16_t finalHole_max = 0;
uint16_t finalHole_points = 300;

// Impulse Detection


//Point assignments
uint16_t pointValue_1 = 127;
uint16_t pointValue_2 = 182;
uint16_t pointValue_3 = 209;
uint16_t pointValue_4 = 48;
uint16_t pointValue_5 = 95;
uint16_t exitHole_left_pointValue = 300;
uint16_t exitHole_middle_pointValue = 500;
uint16_t exitHole_right_pointValue = 400;

// Transition Hole Globals
Servo transitionHole_servo; // servo control object
uint8_t handDetection_count = 0;
uint8_t transitionHole_closed = 15;
uint8_t transitionHole_open = 65;
bool detectedHand = false;

// Final Hole Globals
bool homed = false;
uint16_t upTime = 4600; // time motor moves up from homed position to top
uint16_t downTime = 3000; // time motor moves down from homed position to bottom
bool motorMoving = false; // flag for when the motor moves or not
uint32_t motorStartTime = 0;
uint32_t time_score_mult = 20000000;

// Reset Globals
bool waiting = false; // flag for when to wait before activating the motor during reset
uint32_t waitStartTime = 0;
uint16_t waitTime = 7000;
uint16_t reset_motor_time = 7400;


// System State switches
enum state {
  welcome, plinko, upperStep, reveal, lowerStep, reset_system
};

state currentState = welcome;

// counters and timers
uint16_t playerScore = 0;
uint16_t plinkoPoints = 0;
uint32_t startTime = 0;
uint32_t detectedHand_time = 0;
uint32_t reset_time; 
uint32_t start_time;
uint32_t end_time;

// Function Declarations
void sensorTesting(uint16_t impulse, uint16_t target, uint16_t threshold);
void impulseDetection(uint16_t threshold);
void impactSensor_calculatePoints(uint16_t impulsePeak);
// uint16_t infraredSensor_detection(uint16_t threshold);
void exitHole_pointAssignment(uint16_t detectionPeak);
void transitionHole_function(uint16_t detectionPeak);
void finalHole_function(uint16_t detectionPeak);
void moveMotor(uint8_t upDown);
uint16_t irSensorCalibration(uint16_t analog);
void homeFinalMotor();
void resetGameState();

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
  // Serial Monitor set up
  Serial.begin(9600);

  // Servo set up and initialization
  transitionHole_servo.attach(servo_Pin);
  transitionHole_servo.write(transitionHole_closed);

  // Motor Set Up
  pinMode(finalHole_motor_down_input, OUTPUT);
  pinMode(finalHole_motor_up_input, OUTPUT);
  pinMode(finalHole_motor_enable, OUTPUT);
  digitalWrite(finalHole_motor_enable, 0);
  digitalWrite(finalHole_motor_up_input,0);
  digitalWrite(finalHole_motor_down_input, 0);
  digitalWrite(finalHole_motor_enable, 1);
  digitalWrite(finalHole_motor_up_input,0);
  digitalWrite(finalHole_motor_down_input, 1);
  delay(5000);
  digitalWrite(finalHole_motor_enable, 0);
  digitalWrite(finalHole_motor_down_input, 0);

  // Threshold caliblration
  // exitHole_left_threshold = irSensorCalibration(exitHole_left_sensorPin) - 50;
  // exitHole_right_threshold = irSensorCalibration(exitHole_right_sensorPin) - 50;
  // exitHole_middle_threshold = irSensorCalibration(exitHole_middle_sensorPin) - 50;
  // Serial.print("Left threshold: ");
  // Serial.println(exitHole_left_threshold);
  // Serial.print("Middle threshold: ");
  // Serial.println(exitHole_middle_threshold);
  // Serial.print("Right threshold: ");
  // Serial.println(exitHole_right_threshold);
  // transitionHole_sensor_threshold = irSensorCalibration(transitionHole_sensorPin) - 200;
  // Serial.print("Starting value: ");
  // Serial.println(analogRead(finalHole_sensorPin));

  // if (analogRead(finalHole_sensorPin) < 300) {
  //   Serial.print(analogRead(finalHole_sensorPin));
  //   digitalWrite(finalHole_motor_enable, 1);
  //   digitalWrite(finalHole_motor_down_input, 1);
  //   delay(5000);
  //   digitalWrite(finalHole_motor_enable, 0);
  //   digitalWrite(finalHole_motor_down_input, 0);
  // }
  // finalHole_sensor_threshold = irSensorCalibration(finalHole_sensorPin) - 200;

  // while (true) {
  //   finalHole_sensorValue = analogRead(finalHole_sensorPin);
  //   Serial.println(finalHole_sensorValue);
  // }

  
  Serial.println(exitHole_left_threshold);
  Serial.println(exitHole_right_threshold);
  Serial.println(exitHole_middle_threshold);
  Serial.println(transitionHole_sensor_threshold);
  // Serial.print("Final hole threshold: ");
  // Serial.println(finalHole_sensor_threshold);

  // Lower step motor homing sequence
  // finalHole_sensorValue = analogRead(finalHole_sensorPin);

  // if the sensor is broken, ie. the platform and/or rack is blocking sensor, then move the rack down
  // homeFinalMotor();

  // once rack is in standard position, then move rack up
  if (homed == true){
    Serial.println("moving after homing");
    digitalWrite(finalHole_motor_enable, 1);
    digitalWrite(finalHole_motor_down_input, 0);
    digitalWrite(finalHole_motor_up_input, 1);
    delay(upTime);

    // turn motor off again
    digitalWrite(finalHole_motor_enable, 0);
    digitalWrite(finalHole_motor_down_input, 0);
    digitalWrite(finalHole_motor_up_input, 0);
  }

  // LED Set Up
  mx.begin();
  mx.control(MD_MAX72XX::INTENSITY, 5);
  mx.clear();
  Serial.println("Setup complete");
}


void loop() {
  updateDisplayAnimation();
  switch (currentState) {
    // Welcome State
    case welcome:{
      // goes into plinko state when transition hole ir sensor detects a hand. LED display 
      setDisplayText("Put hand in hole to start " + ScoreBoardText);
      //IR sensor code for hand in hole to start
      transitionHole_sensorValue = analogRead(transitionHole_sensorPin);
      if (transitionHole_sensorValue < transitionHole_sensor_threshold) {
        currentState = plinko;
        Serial.println("Game Started - Place ball at Plinko");
      }
      break;
    }

    
    
    // Plinko Section
    case plinko: {
      bool leavingPlinko = false;
      bool actuallyBroken = true;
      //displaySmartTextBottom("Start->");  // if not in welcome - display the score
      setDisplayText("GO ->");
      // read sensors continuously until a hit on contact sensors is detected, then figure out what voltage reading is and add to player score
      pointSensor_15_value = analogRead(pointValue_15_sensorPin);
      pointSensor_610_value = analogRead(pointValue_610_sensorPin);
      exitHole_left_sensorValue = analogRead(exitHole_left_sensorPin);
      exitHole_middle_sensorValue = analogRead(exitHole_middle_sensorPin);
      exitHole_right_sensorValue = analogRead(exitHole_right_sensorPin);

      // exitHole_left_sensorValue = analogRead(exitHole_left_sensorPin);
      // exitHole_middle_sensorValue = analogRead(exitHole_middle_sensorPin);
      // exitHole_right_sensorValue = analogRead(exitHole_right_sensorPin);
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
      if (exitHole_left_sensorValue <= exitHole_left_threshold) {
        actuallyBroken = true;
        for (int i = 0; i < 10; i++) {
          exitHole_left_sensorValue = analogRead(exitHole_left_sensorPin);
          // Serial.println("Read at least one");
          if (exitHole_left_sensorValue > exitHole_left_threshold) {
            actuallyBroken = false;
          }
        }
        if (actuallyBroken) {
          // switch case and deal with points
          playerScore += exitHole_left_pointValue;
          leavingPlinko = true;
        }
      }
      if (exitHole_middle_sensorValue <= exitHole_middle_threshold) {
        actuallyBroken = true;
        for (int i = 0; i < 10; i++) {
          exitHole_middle_sensorValue = analogRead(exitHole_middle_sensorPin);
          if (exitHole_middle_sensorValue > exitHole_middle_threshold) {
            actuallyBroken = false;
          }
        }
        if (actuallyBroken) {
          playerScore += exitHole_middle_pointValue;
          leavingPlinko = true;
        }
      }
      if (exitHole_right_sensorValue <= exitHole_right_threshold) {
        actuallyBroken = true;
        for (int i = 0; i < 10; i++) {
          exitHole_right_sensorValue = analogRead(exitHole_right_sensorPin);
          if (exitHole_right_sensorValue > exitHole_right_threshold) {
            actuallyBroken = false;
          }
        }
        if (actuallyBroken) {
          playerScore += exitHole_right_pointValue;
          leavingPlinko = true;
        }
      }
      if (leavingPlinko) {
        currentState = upperStep;
        Serial.println("Leaving Plinko");
        start_time = millis();
      }
      break;
    }


  // Upper step section
    case upperStep:{
      // watch transition hole light sensors for detections
      setDisplayText(String(playerScore));
      transitionHole_sensorValue = analogRead(transitionHole_sensorPin);
      if (transitionHole_sensorValue < transitionHole_sensor_threshold) {
        playerScore += transitionHole_points;
        delay(2000);
        currentState = reveal;
        Serial.println("Leaving Upper Step");
      }
      break;
    }


  // Reveal State
    case reveal:{
      // move servo out and begin moving motor down
      setDisplayText(String(playerScore));  // if not in welcome - display the score
      if (motorMoving == false){
        transitionHole_servo.write(transitionHole_open);
        homeFinalMotor();
        setDisplayText("!!");  // something happened display something fun

        motorMoving = true;
        motorStartTime = millis();
        digitalWrite(finalHole_motor_enable, 1);
        digitalWrite(finalHole_motor_down_input, 1);
        digitalWrite(finalHole_motor_up_input, 0);
      }

      if (motorMoving == true && (millis() - motorStartTime >= downTime)) {
        // motor should be at bottom now, so turn off motor
        digitalWrite(finalHole_motor_enable, 0);
        digitalWrite(finalHole_motor_down_input, 0);

      // exit state to lower step phase
        currentState = lowerStep;
        delay(2000);
        Serial.println("Leaving Reveal Step");
        motorMoving = false;

        // play zelda discover treasure theme

        // exit state to lower step phase
      
      }
      break;

    }


  // Lower State
    case lowerStep:{
      // Ball detection and final score assignment
      setDisplayText(String(playerScore));  // if not in welcome - display the score
      finalHole_sensorValue = analogRead(finalHole_sensorPin);
      if (finalHole_sensorValue < finalHole_sensor_threshold) {
        bool actuallyBroken = true;
        for (int i = 0; i < 5; i++) {
          finalHole_sensorValue = analogRead(finalHole_sensorPin);
          if (finalHole_sensorValue > finalHole_sensor_threshold) {
            actuallyBroken = false;
          }
        }
        if (actuallyBroken) {
          end_time = millis() - start_time;
          playerScore += (time_score_mult/end_time);
          playerScore += finalHole_points;
          Serial.print("Mult: ");
          Serial.println(time_score_mult);
          Serial.print("End time: ");
          Serial.println(end_time);
          Serial.print("End score: ");
          Serial.println(time_score_mult/end_time);

          // do nothing until timer hits specified number
          waitStartTime = millis();
          waiting = true;
          currentState = reset_system;
        }
      }
      break;
    }

    // Lower Step and reset system
    case reset_system:{
      if ((waiting && (millis() - waitStartTime >= waitTime)) || !waiting) {
        setDisplayText(String(playerScore));  // if not in welcome - display the score
      }
      // motor move up after wait timer expires
      if (waiting && (millis() - waitStartTime >= waitTime)){
        motorStartTime = millis();
        motorMoving = true;
        waiting = false;
        digitalWrite(finalHole_motor_enable, 1);
        digitalWrite(finalHole_motor_up_input, 1);
      } else if (waiting) {
        setDisplayText("You Won! Final Score: " + String(playerScore));
      }

      // motor stops moving after move timer expires
      if (motorMoving == true && (millis() - motorStartTime >= reset_motor_time)){
          digitalWrite(finalHole_motor_enable, 0);
          digitalWrite(finalHole_motor_up_input, 0);
          transitionHole_servo.write(transitionHole_closed);

          motorMoving = false;
          resetGameState();
          delay(2000);
          Serial.println("Starting new game");
          currentState = welcome;
      }
      // End
      break;
    }
  }
}

void resetGameState() {
  playerScore = 0;
  plinkoPoints = 0;
  motorMoving = false;
  waiting = false;
  motorStartTime = 0;
  waitStartTime = 0;
  start_time = 0;
  end_time = 0;
  exitHole_detected = false;
}

// Functions 
void homeFinalMotor() {
  // if the sensor is broken, ie. the platform and/or rack is blocking sensor, then move the rack down
  finalHole_sensorValue = analogRead(finalHole_sensorPin);
  if (finalHole_sensorValue < finalHole_sensor_threshold) {
    digitalWrite(finalHole_motor_enable, 1);
    digitalWrite(finalHole_motor_down_input, 1);
    while (finalHole_sensorValue < finalHole_sensor_threshold){
      finalHole_sensorValue = analogRead(finalHole_sensorPin);
    }
    // turn off motor
    digitalWrite(finalHole_motor_enable, 0);
    digitalWrite(finalHole_motor_down_input, 0);
    homed = true;
  } else {
    digitalWrite(finalHole_motor_enable, 1);
    digitalWrite(finalHole_motor_up_input, 1);
    while (finalHole_sensorValue >= finalHole_sensor_threshold){
      finalHole_sensorValue = analogRead(finalHole_sensorPin);
    }
    // turn off motor 
    digitalWrite(finalHole_motor_enable, 0);
    digitalWrite(finalHole_motor_up_input, 0);
    homed = true;
  }
}

uint16_t irSensorCalibration(uint16_t analog) {
  uint16_t sensor_value = analogRead(analog);
  uint16_t min_value = sensor_value;
  for (int i = 0; i < 10000; i++) {
    sensor_value = analogRead(analog);
    if (sensor_value < min_value) {
      min_value = sensor_value;
    }
  }
  return min_value;
}

void impulseDetection(uint16_t threshold) {
    pointSensor_15_value = analogRead(pointValue_15_sensorPin);
    pointSensor_610_value = analogRead(pointValue_610_sensorPin);
    uint16_t maxValue = 0;
    if (pointSensor_15_value > threshold) {
      for (int i = 0; i < 1000; i++) {
        if (pointSensor_15_value > maxValue) {
            maxValue = pointSensor_15_value;
        }
      }
      pointSensor_15_value = analogRead(pointValue_15_sensorPin);
    }

    if (pointSensor_610_value > threshold) {
      for (int i = 0; i < 1000; i++) {
        if (pointSensor_610_value > maxValue) {
            maxValue = pointSensor_610_value;
        }
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
    plinkoPoints += pointValue_1;
  } 
  else if (impulsePeak >= pointValue_2_threshold){     
    plinkoPoints += pointValue_2;
  } 
  else if (impulsePeak >= pointValue_3_threshold){     
    plinkoPoints += pointValue_3;
  } 
  else if (impulsePeak >= pointValue_4_threshold){     
    plinkoPoints += pointValue_4;
  } 
  else if (impulsePeak >= pointValue_5_threshold){     
    plinkoPoints += pointValue_5;
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

// void transitionHole_function(uint16_t detectionPeak){
//   handDetection_count += 1;
// }

// void finalHole_function(uint16_t detectionPeak){
//   // if ran, then already in final hole, so give player points and begin reset timer
//   playerScore += finalHole_points;
//   startTimer = true;
// }

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