//receiver testing.

#include <SPI.h>
#include <RH_RF95.h>
#include <Adafruit_NeoPixel.h>
//libraries to make everything work.

#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 3
//set the pins for feather to work with the built in radio.
#define RF95_FREQ 915.0
//set the frequency to transmit and receive on.
#define LED 13
//set the pin for the built in led.
#define VBATPIN A7
//pin to read the battery voltage.

#define en1 5
#define en2 6
//pins that set the speed based on the analog reading.  needs to be between 0 and 255.
#define in1 10
#define in2 11
#define in3 12
#define in4 13
//determines the direction of spin on the motor.

#define pot A1

#define pixelPin A2
//pin for the neopixel to "talk" to the driver.

RH_RF95 rf95(RFM95_CS, RFM95_INT);
//create the radio object.
//"Singleton instance"(?).

int x = 0;
//create variable for counting the loops.
int steering = 0;
//variable for rudder position of the boat.
int speeds = 0;
//variable for the speed of the boat.
int txNum= 0;
//variable to count the number of transmittions.

Adafruit_NeoPixel pixels(1, pixelPin, NEO_GRB + NEO_KHZ800);
//creates the pixels object.

uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
uint8_t len = sizeof(buf);
//create a buffer and a variable based on the size of the buffer.

float measuredVbat;

void setup(){
  pinMode(LED, OUTPUT);
  pinMode(RFM95_RST, OUTPUT);
  //turn the led and radio pins to outputs.

  digitalWrite(RFM95_RST, HIGH);
  //turn on the radio.
  rf95.init();
  //initialize the radio.
  rf95.setFrequency(RF95_FREQ);
  //set the radio frequency.
  rf95.setTxPower(23, false);
  //set the power to transmit at.
  //scale is 5 to 23, with 5 being the weakest, 23 being the strongest.
  pixels.begin();
  pixels.show();
  //initialize and turn the pixels off.
  Serial.begin(9600);
  //start the serial thing for messages.
}

void loop(){
  setMotor(steering, speeds);
  //make the motor do stuff.
  batteryCheck();
  //turn on the light and check the battery voltage.
  
  if (rf95.available()){

      if (rf95.recv(buf, &len)){
        digitalWrite(LED, HIGH);
        //got a message, turn on the light.
        
        rxSort();
        rxReply();

        digitalWrite(LED, LOW);
        //turn off the light.
      }

      else{
      Serial.println("Receive failed");
      //if something doesn't work, at least say something.
    }
  }
}

void rxSort(){
  //after receiving the signal from the transmitter sort it out so it can be used to do stuff.
  steering = buf [0] * 100;
  steering = steering + (buf [1] * 10);
  steering = steering + buf [2];
  //puts the pieces from the buffer into the steering variable.
  Serial.print("steering-"); Serial.print(steering);

  speeds = buf [3] * 100;
  speeds = speeds + (buf [4] * 10);
  speeds = speeds + buf [5];
  //puts other pieces from the buffer into the speed variable.

  txNum = buf [5] * 1000;
  txNum = txNum + (buf [6] *100);
  txNum = txNum + (buf [7] *10);
  txNum = txNum + buf [8];
  //puts the transmission number from the buffer into a variable.
}

void setMotor(int receivedSteering, int receivedSpeed){
  /*so, with the H-bridge chip there are two outputs, one for the propeller and one for the rudder.
   * 
   * en1 is the analog speed that the rudder should move.
   * in1 is one direction of the rudder's movement.
   * int2 is the other direction. 
   * 
   * en2 is the analog speed of the propeller.
   * in3 is one direction of the propeller.
   * in4 is the other direction of the propeller.
   * 
   * moving the rudder in with 100 for the analog speed per loop seems to be a good balance 
   * between speed and precision. 
   */
  int rudder = analogRead(pot) / 4;
  //read the pot connected to the rudder, divide it by 4 to make it compatable with the transmission.
  Serial.print(" rudder-"); Serial.println(rudder);

  if(receivedSteering < 45){
    //the rudder can only move so far in either direction.
    //if the signal says to move too far to the left, the signal is reduced to the maximum it can go.
    receivedSteering = 45;
  }

  else if(receivedSteering > 200){
    //rudder still can't move any further than it can move.
    //if it's trying to move too far to the right, reduce the signal so it doesn't stutter.
    receivedSteering = 200;
  }
  
  if(rudder < receivedSteering){
    //if the rudder's pot says it's not at the target.
    int rudderThreshold = receivedSteering - rudder;
    if(rudderThreshold > 4){
      //if it happens to be above the threshold, then move it.
      analogWrite(en1, 200);
      digitalWrite(in1, 1);
      digitalWrite(in2, 0);
      Serial.println("less than");
    }
  }

  else if(rudder > receivedSteering){
    //if the rudder's pot says it's off, but in the other direction.
    int rudderThreshold = rudder - receivedSteering;
    if(rudderThreshold > 4){
      //if the difference is above the threshold, move it.
      analogWrite(en1, 200);
      digitalWrite(in1, 0);
      digitalWrite(in2, 1);
      Serial.println("greater than");
    }
  }

  else{
    //if it's close enough, stop moving the rudder.
    analogWrite(en1, 0);
    digitalWrite(in1, 0);
    digitalWrite(in2, 0);
    Serial.println("stop");
  }

  if(receivedSpeed < 115){
    //if the speed is above 115, it should be in reverse.
    analogWrite(en2, ((115 - receivedSpeed) * 2));
    //inverse the reading, so that it slows down as you get to the middle of the pot, then hopefully stops.
    //also double it since you're only using 1/2 of the pot for reverse.
    digitalWrite(in3, 1);
    digitalWrite(in4, 0);
  }

  else if(receivedSpeed > 141){
    //if the speed is above 141, it should be forward.
    analogWrite(en2, ((receivedSpeed - 141) * 2));
    //so we take the speed, remove the reverse part of the reading, then double it since only 1/2 of the 
    //pot range is dedicated to the forward speed.
    digitalWrite(in3, 0);
    digitalWrite(in4, 1);
  }

  else{
    //if the pot is in the middle, stop the motor, hopefully.
    analogWrite(en2, 0);
    digitalWrite(in3, 0);
    digitalWrite(in4, 0);
    //stop it, right?
  }
}

void batteryCheck(){
  pixels.setPixelColor(0, 255, 255, 255);
  pixels.show();
  //function to check the battery level for the microcontroller.
  measuredVbat = analogRead(VBATPIN);
  //measure the battery voltage.

  measuredVbat *= 2;    
  //voltage divider split voltage in half, so double the read value.
  measuredVbat *= 3.3;  
  //then multiply it by the reference voltage, 3.3v.
  measuredVbat /= 1024; 
  //convert to voltage(?)
  
  Serial.print("VBat: " );     
  Serial.println(measuredVbat);
  //print out the results.
}

void rxReply(){
  //funcction to reply to the transmitter to let it know that the transmission was received.
  uint8_t radioPacket [6];
  //variable for the data to be sent out.
  
  radioPacket [0] = measuredVbat;
  measuredVbat = measuredVbat - radioPacket [0];
  radioPacket [1] = measuredVbat * 10;
  //break up the battery voltage and put it into the radio sending array.

  radioPacket [2] = txNum / 1000;
  txNum = txNum - (radioPacket [2] * 1000);
  radioPacket [3] = txNum / 100;
  txNum = txNum - (radioPacket [3] * 100);
  radioPacket [4] = txNum / 10;
  txNum = txNum - (radioPacket [4] *10);
  radioPacket [5] = txNum;

  rf95.send(radioPacket, 6);
  rf95.waitPacketSent();
   //return the received data to verify it.
   //wait for the packet to finish being sent out.
}
