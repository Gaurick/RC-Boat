/*


Websites that helped...
https://learn.adafruit.com/adafruit-arduino-lesson-15-dc-motor-reversing/lm293d
site that describes the pinout and function of the L293D chip (H-Bridge motor thing.)
https://learn.adafruit.com/adafruit-feather-m0-radio-with-lora-radio-module/pinouts
feather M0 radio with LoRa pinouts and description.
*/
 
int rudderReading;
int receivedTarget;
bool reverse;

int en1 = 5;
int en2 = 6;
//pins that set the speed based on the analog reading.  needs to be between 0 and 255.
int in1 = 10;
int in2 = 11;
int in3 = 12;
int in4 = 13;
//determines the direction of spin on the motor.
int button = A0;
//testing button to switch the direction of spin.
int pot = A1;
//potentiometer that determines speed motor spins at.

int led = 13;
//hardware led used for debugging and moral support.

void setup() {
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);
  pinMode(en1, OUTPUT);
  pinMode(en2, OUTPUT);
  pinMode(button, INPUT_PULLUP);
  pinMode(led, OUTPUT);
  
  Serial.begin(9600);
  randomSeed(0);
  receivedTarget = random(255);

}

void loop() {
  rudderReading = analogRead(pot) / 4;
  reverse = digitalRead(button);
  //read the inputs to see what the speed and direction *should* be.

  setMotor(rudderReading, reverse);

  
  
  
  //Serial.print(" b="); Serial.println(reverse);
  //delay(250);
}

void setMotor(int rudder, bool dir){
  if((rudder/3) < (receivedTarget/3)){
    analogWrite(en1, 100);
    digitalWrite(in1, 1);
    digitalWrite(in2, 0);

    Serial.print("<<  ");
    Serial.print(" r="); Serial.print(rudder);
    Serial.print("t="); Serial.println(receivedTarget);
  }

  else if((rudder/3) > (receivedTarget/3)){
    analogWrite(en1, 100);
    digitalWrite(in1, 0);
    digitalWrite(in2, 1);

    Serial.print(">>  ");
    Serial.print(" r="); Serial.print(rudder);
    Serial.print("t="); Serial.println(receivedTarget);
  }

  else{
    analogWrite(en1, 0);
    digitalWrite(in1, 0);
    digitalWrite(in2, 0);

    analogWrite(en2, 0);
    digitalWrite(in3, 0);
    digitalWrite(in4, 0);
  
    Serial.println("made it.");
  }

  if(dir == 0){
    receivedTarget = random(255);
  }
  
}
