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
#define pot 18
//pin for the potentiometer.
#define button 19
//pin for the button.


RH_RF95 rf95(RFM95_CS, RFM95_INT);
//create the radio object.
//"Singleton instance"(?).

long y = 0;
//create variable for counting the loops.
int steering = 123;
//variable for first set of data.
int oldSteering = 123;
//variable to check if any steering has happened.
int speeds = 45;
//variable for second set of data.
int oldSpeed = 45;
//variable to check to see if the speed has changed.
int txNum= 0;
//variable to count the number of transmissions.

void setup(){
  pinMode(LED, OUTPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(RFM95_RST, OUTPUT);
  pinMode(button, INPUT_PULLUP);
  //turn pins to outputs as necessary.

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

void loop(){
  if(y == 500000){
    readSensors();
    txing();
    rxing();
    y = 0;
  }

  else{
    //readSensors();
    y++;
    //Serial.print("y-"); Serial.println(y);
    //counter thing to make sure it's still working while working on the transmitter.
  }

  buzzing();
}

void txing(){
  digitalWrite(LED, HIGH);
  //turn on the light.  
  
  uint8_t radioPacket[9];
  //variable for the transmission.
  //steering, speed, txNum

  radioPacket [0] = steering / 100;
  steering = steering - (radioPacket [0] * 100);
  radioPacket [1] = steering / 10;
  steering = steering - (radioPacket [1] * 10);
  radioPacket [2] = steering;
  //break up the steering variable into pieces and put it into the radio transmission array.
  
  radioPacket [3] = speeds / 10;
  speeds = speeds - (radioPacket [3] * 10);
  radioPacket [4] = speeds;
  // break up the speed variable and put it into the radio transmission array.
  
  radioPacket [5] = txNum / 1000;
  txNum = txNum - (radioPacket [5] * 1000);
  radioPacket [6] = txNum / 100;
  txNum = txNum - (radioPacket [6] * 100);
  radioPacket [7] = txNum / 10;
  txNum = txNum - (radioPacket [7] * 10);
  radioPacket [8] = txNum;
  //break up the transmission number and sort it into the transmission array.
  txNum = (radioPacket [5] * 1000) + (radioPacket [6] * 100) + (radioPacket [7] * 10) + radioPacket [8];
  //put the txNum back the way it was.
  Serial.print("txNum-"); Serial.println(txNum); Serial.println(" ");

  Serial.print("sending data: ");
  for(int a = 0; a < 9; a ++){
    Serial.print(radioPacket [a]);
  }
  Serial.println(" ");

  rf95.send(radioPacket, 9);
  //send the message.
  rf95.waitPacketSent();
  //wait for the message to finish sending.
}

void rxing(){
  uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
  uint8_t len = sizeof(buf);
  //make a buffer for the reply.

  if(rf95.waitAvailableTimeout(1000)){
    if(rf95.recv(buf, &len)){
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
      Serial.println("receive failure");
    }
  }

  else{
    Serial.println("No reply");
  }

  digitalWrite(LED, LOW);
  //turn off the built in led.

  txNum ++;
}

void readSensors(){
  steering = analogRead(pot);

  if(digitalRead(button) == LOW){
    speeds = 99;
  }

  else{
    speeds = 11;
  }

  if(steering != oldSteering){
    oldSteering = steering;
    txing();
  }

  if(speeds != oldSpeed){
    oldSpeed = speeds;
    txing();
  }
}

void buzzing(){
  //no signal
  //-15 to -100
  //larger number is stronger
  //-15 probably strongest
  //-70 is a good warning level

  //low boat battery
  //low transmitter battery
  //4.2v max
  //3.7v “normal”
  //3.2v low power safety cut off
  
  //digitalWrite(buzzer, HIGH);
  digitalWrite(buzzer, LOW);

}
