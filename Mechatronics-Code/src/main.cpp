#include <Arduino.h>
#include <Servo.h>

#define servoPin 9
#define transitionHole_sensorPin 1

int tranistionHole_closed = 90;
int tranistionHole_open = 150;
int lightSensor_threshold = 50;

Servo transitionHole_servo;

// put function declarations here:
int myFunction(int, int);

void setup() {
  // put your setup code here, to run once:
  transitionHole_servo.attach(servoPin);
  pinMode(13, OUTPUT);

  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:

  
  
  digitalWrite(13, 1);
  transitionHole_servo.write(tranistionHole_closed);
  /*
  int transitionHole_sensorValue = analogRead(transitionHole_sensorPin);
  Serial.print(transitionHole_sensorValue);
  Serial.print("\n       ");
  */

  
}

// put function definitions here:
int myFunction(int x, int y) {
  return x + y;
}

void detectedHandISR(){

}
