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
  Serial.begin(9600);
  //start the serial thing for messages.
}

void loop(){
  if (rf95.available()){

      if (rf95.recv(buf, &len)){
        digitalWrite(LED, HIGH);
        //got a message, turn on the light.
        
        rxSort();
        batteryCheck();
        motoring();
        rxReply();

        Serial.print("RSSI: ");
        Serial.println(rf95.lastRssi(), DEC);
        //print off the strength of the last signal received.
        
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
  Serial.print("steering-"); Serial.print(steering);

  speeds = buf [3] * 10;
  speeds = speeds + buf [4];
  Serial.print(" speeds-"); Serial.print(speeds);

  txNum = buf [5] * 1000;
  txNum = txNum + (buf [6] *100);
  txNum = txNum + (buf [7] *10);
  txNum = txNum + buf [8];
  Serial.print(" txNum-"); Serial.println(txNum);
}

void batteryCheck(){
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

void motoring(){
  //function to make the motor move.
  analogWrite(en1, 0);
  digitalWrite(in1, 0);
  digitalWrite(in2, 0);

  analogWrite(en2, 0);
  digitalWrite(in3, 0);
  digitalWrite(in4, 0);

  pixels.setPixelColor(0, 255, 255, 255);
  //pick the pixel then set the color.
  pixels.show();
  //actually make the pixel show the color specified.
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
