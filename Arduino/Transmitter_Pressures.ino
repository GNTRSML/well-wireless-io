//This is technically a test but a couple lines can be added to make functional for real world setup.
//Transmitter side of a system for wireless transmission of analog inputs
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>


byte Array[3];

RF24 radio(8, 9);//CE,CSN

const byte address[6] = "00001";

void setup() {
  pinMode(2, INPUT_PULLUP);
  pinMode(A5, INPUT);
  pinMode(A4, INPUT);
  analogReference(EXTERNAL);
  radio.begin();
  radio.setRadiation(RF24_PA_MAX, RF24_2MBPS);
  radio.stopListening();
  radio.openWritingPipe(address);
}
void loop() {
  Array[0] = analogRead(A5);
  Array[1] = analogRead(A4);
  Array[2] = digitalRead(2);
  radio.write(&Array, sizeof(Array));
  delay(500);
}
