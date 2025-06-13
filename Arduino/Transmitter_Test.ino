/*This script can be used for wireless connection testing
it will print Hello World in the serial monitor every second
on the reciever end with the corresponding script to this one*/
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <printf.h>
RF24 radio(8, 9); // CE, CSN

const byte address[6] = "00001";

void setup() {
  Serial.begin(9600);
  printf_begin();
  while(!Serial){}
  radio.begin();
  radio.openWritingPipe(address);
  radio.setRadiation(RF24_PA_MIN, RF24_1MBPS);
  radio.setChannel(114);
  radio.stopListening();
  radio.printPrettyDetails();
}

void loop() {
  const char text[32] = "Hello World";
  radio.write(&text, sizeof(text));
  delay(500);
}