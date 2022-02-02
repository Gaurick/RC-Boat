/*
 * links...
 * https://cdn-learn.adafruit.com/assets/assets/000/046/254/original/feather_Feather_M0_LoRa_v1.2-1.png?1504886587
 * feather pinout
 * https://learn.adafruit.com/adafruit-arduino-lesson-15-dc-motor-reversing
 * potentiometer and button help
 * https://learn.adafruit.com/adafruit-feather-m0-radio-with-lora-radio-module/overview
 * feather and radio tutorial
 * https://learn.adafruit.com/adafruit-arduino-lesson-13-dc-motors/breadboard-layout
 * motor on arduino pins
 * https://www.onsemi.com/pub/Collateral/PN2222A-D.pdf
 * pn2222a datasheet (transistor used, NPN)
 * https://www.build-electronic-circuits.com/h-bridge/
 * h bridge with transistors
 * 
 */

//transmittion testing.

#include <SPI.h>
#include <RH_RF95.h>
//libraries to make everything work.

#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 3
//set the pins for feather to work with the built in radio.
#define RF95_FREQ 915.0
//set the frequency to transmit and receive on.
#define LED 13
//set the pin for the built in led.
#define buzzer 16
//pin for the buzzer.
//need to either setup a buzzer and the stuff to signal driver when either the receiver or transmitter
//battery is low and/or if the boat gets out of transmission range.
#define speeding A1
//pin for the speed potentiometer.
#define steering A0
//pin for the steering potentiometer.
#define button 19
//pin for the button.

RH_RF95 rf95(RFM95_CS, RFM95_INT);
//create the radio object.
//"Singleton instance"(?).

long y = 0;
//create variable for counting the loops.
int steeringReading = 123;
//variable for first set of data.
int speedingReading = 45;
//variable for second set of data.
int txNum= 0;
//variable to count the number of transmissions.

void setup() {
  pinMode(LED, OUTPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(RFM95_RST, OUTPUT);
  pinMode(button, INPUT);
  //turn pins to outputs or inputs as necessary.

  digitalWrite(RFM95_RST, HIGH);
  //turn on the radio.
  rf95.init();
  //initialize the radio.
  rf95.setFrequency(RF95_FREQ);
  //set the radio frequency.
  rf95.setTxPower(23, false);
  //set the power to transmit at.
  //scale is 5 to 23, with 5 being the weakest, 23 being the strongest.
  Serial.begin(9600);
  //start the serial thing for messages.

}

void loop() {
  readSensors();
  if(y == 10000){
    //temporary way to not spam transmitions slowed down so i can read stuff.
    //need to replace the "y" countdown with something fancy to check if the reading 
    //is different from the previous reading +/- a threshold so it doesn't spam or miss
    //a turn or change of speed.
    y = 0;
    TXing();
  }

  else{
    y++;
  }

}

void readSensors(){
  steeringReading = (analogRead(steering)) / 4;
  //pot reading is 0-1024, H-bridge chip can only handle 0-256, hence the division by 4.
  speedingReading = (analogRead(speeding)) / 4;

  Serial.print("steering-"); Serial.print(steeringReading);
  Serial.print(" speed-"); Serial.println(speedingReading);
}

void TXing(){
  digitalWrite(LED, HIGH);
  //turn on the light.  
  
  uint8_t radioPacket[10];
  //variable for the transmission.
  //steering, speed, txNum
  int oldSteering = steeringReading;
  int oldSpeeding = speedingReading;
  //variables to 

  radioPacket [0] = oldSteering / 100;
  oldSteering = oldSteering - (radioPacket [0] * 100);
  radioPacket [1] = oldSteering / 10;
  oldSteering = oldSteering - (radioPacket [1] * 10);
  radioPacket [2] = oldSteering;
  //break up the steering variable into pieces and put it into the radio transmission array.

  radioPacket [3] = oldSpeeding / 100;
  oldSpeeding = oldSpeeding - (radioPacket [3] * 100);
  radioPacket [4] = oldSpeeding / 10;
  oldSpeeding = oldSpeeding - (radioPacket [3] * 10);
  radioPacket [5] = oldSpeeding;
  // break up the speed variable and put it into the radio transmission array.
  
  radioPacket [6] = txNum / 1000;
  txNum = txNum - (radioPacket [5] * 1000);
  radioPacket [7] = txNum / 100;
  txNum = txNum - (radioPacket [6] * 100);
  radioPacket [8] = txNum / 10;
  txNum = txNum - (radioPacket [7] * 10);
  radioPacket [9] = txNum;
  //break up the transmission number and sort it into the transmission array.
  txNum = (radioPacket [6] * 1000) + (radioPacket [7] * 100) + (radioPacket [8] * 10) + radioPacket [9];
  //put the txNum back the way it was.
  Serial.print("txNum-"); Serial.println(txNum); Serial.println(" ");

  Serial.print("sending data: ");
  for(int a = 0; a < 9; a ++){
    Serial.print(radioPacket [a]);
  }
  Serial.println(" ");

  rf95.send(radioPacket, 10);
  //send the message.
  rf95.waitPacketSent();
  //wait for the message to finish sending.

  RXing();
}

void RXing(){
  uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
  uint8_t len = sizeof(buf);
  //make a buffer for the reply.

  if(rf95.waitAvailableTimeout(1000)){
    if(rf95.recv(buf, &len)){
      //wait for a bit then hopefully receive a reply from the receiver.
      Serial.print("received data:");
      for(int b = 0; b < 6; b ++){
        Serial.print(buf [b]);
      }
      Serial.println(" ");
      //print what’s been received.

      Serial.print("signal:");
      Serial.println(rf95.lastRssi(), DEC);
      //print the signal of the last received.
      Serial.println(" ");
    }

    else{
      //if the reply is received, but doesn't work, say something.
      Serial.println("receive failure");
    }
  }

  else{
    //if there's no reply from the receiver before the timeout, say something.
    Serial.println("No reply");
  }

  digitalWrite(LED, LOW);
  //turn off the built in led.

  txNum ++;
  //having received a reply, increase the transmission number.
}
