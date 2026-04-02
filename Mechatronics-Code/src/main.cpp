#include <Arduino.h>
#include <Servo.h>

#define servoPin 30

Servo transitionHoleservo;

// put function declarations here:
int myFunction(int, int);

void setup() {
  // put your setup code here, to run once:
  transitionHoleservo.attach(servoPin);
  pinMode(13, OUTPUT);
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(13, 1);
  transitionHoleservo.write(90);
  Serial.print("Hello World");
  delay(1000);

  digitalWrite(13, 0);
  transitionHoleservo.write(0);
  Serial.print("Goodbye World");
  delay(1000);
  
  
}

// put function definitions here:
int myFunction(int x, int y) {
  return x + y;
}

