/*  outline of code and how it works
 *  Before everything...
 *    Load the libraries to make everything work.
 *    Define the pins to interact with the radio and potentiometers.
 *    Create the radio object.
 *    Make a bunch of global variables because i can.
 *  Setup time...
 *    Set the pins to output.
 *    Startup the radio.
 *    Begin the serial communications for moral support.
 *  Looping around...
 *    Check the potentiometer readings.
 *    If the steering reading is different, run the transmission function.
 *    If the speed reading is different, run the transmission function.
 *    If a few seconds have passed without change, run the transission function.
 *    Otherwise continue counting.
 *  End of loop, return to the beginning of the loop.
 *
 *  Transmission function
 *    Turn on the onboard LED.
 *    Create a trannsmission packet buffer.
 *    Insert steering potentiometer reading into the transmission buffer.
 *    Insert speed potentiometer reading into the transission buffer.
 *    Take the old transmission number and ?re-add it to the transmission buffer?
 *    Send the packet of data.
 *    Run the receive function.
 *      Create a buffer to receive the data.
 *      Print the data received through the serial moral support output.
 *      Serial print if there's an error or problem.
 *    Turn off the onboard LED.
 *    Increase the transmission number.
 * 
 *
 *  TO DO
 *  actually do something with received errors.
 *  add in some fancy transmission number checking stuff.
 *  put in a buzzer or another neopixel to display errors for/from the transmitter.
 *  try it out in the real world.
 *  share it with people?
 */

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
#define speeding A1
//pin for the speed potentiometer.
#define steering A0
//pin for the steering potentiometer.

RH_RF95 rf95(RFM95_CS, RFM95_INT);
//create the radio object.
//"Singleton instance"(?).

long y = 0;
//create variable for counting the loops.
int steeringReading = 123;
//variable for first set of data.
int speedingReading = 45;
//variable for second set of data.
int oldSteering = 0;
//old steering reading to see if it's changed.
int oldSpeeding = 0;
//old speeding reading to see if it's changes.
int txNum= 0;
//variable to count the number of transmissions.

void setup() {
  pinMode(LED, OUTPUT);
  pinMode(RFM95_RST, OUTPUT);
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
  steeringReading = analogRead(steering) / 4;
  speedingReading = analogRead(speeding) / 4;
  //set the current analog reading of the two pots to place place holder variables.

  if (steeringReading != oldSteering){
    //if steering is different from the old variable.
    if (steeringReading > oldSteering){
      int steeringThreshold = steeringReading - oldSteering;
      if(steeringThreshold > 4){
        //if the difference is above the threshold, then transmit.
        //the pots i used are a bit persnickety and bounce up and down a bit.
        Serial.println("steering change");
        TXing();
      }
    }

    else {
      int steeringThreshold = oldSteering - steeringReading;
      if(steeringThreshold > 4){
        //same thing as above, but since ABS doesn't work, gotta do it somehow.
        Serial.println("steering change");
        TXing();
      }
    }
  }

  if(speedingReading != oldSpeeding){
    //if the speed is different from the old speed variable.
    if(speedingReading > oldSpeeding){
      int speedingThreshold = speedingReading - oldSpeeding;
      if(speedingThreshold > 4){
        //if it's above the threshold, transmit.
        Serial.println("speed change");
        TXing();
      }
    }

    else{
      int speedingThreshold = oldSpeeding - speedingReading;
      if(speedingThreshold > 4){
        //same as above.
        Serial.print("speed change");
        TXing();
      }
    }
  }

  if (y == 50000){
    //if it's been a while then send anyway.
    y = 0;
    Serial.println("too long");
    TXing();
  }

  else{
    y = y + 1;
  }
}

void TXing(){
  digitalWrite(LED, HIGH);
  //turn on the light.  
  Serial.println("sending");
  
  uint8_t radioPacket[10];
  //variable for the transmission.
  //steering, speed, txNum
  oldSteering = steeringReading;
  oldSpeeding = speedingReading;
  //variables to 

  radioPacket [0] = oldSteering / 100;
  oldSteering = oldSteering - (radioPacket [0] * 100);
  radioPacket [1] = oldSteering / 10;
  oldSteering = oldSteering - (radioPacket [1] * 10);
  radioPacket [2] = oldSteering;
  //break up the steering variable into pieces and put it into the radio transmission array.
  oldSteering = steeringReading;
  //put the old steering back where it was.

  radioPacket [3] = oldSpeeding / 100;
  oldSpeeding = oldSpeeding - (radioPacket [3] * 100);
  radioPacket [4] = oldSpeeding / 10;
  oldSpeeding = oldSpeeding - (radioPacket [3] * 10);
  radioPacket [5] = oldSpeeding;
  // break up the speed variable and put it into the radio transmission array.
  oldSpeeding = speedingReading;
  //put the sold speeding variable back to what it should be.
  
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
