// Reciever end of a system using the Adafruit MCP4728 for analog output

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Adafruit_MCP4728.h>
#include <Wire.h>

Adafruit_MCP4728 mcp;

RF24 radio(8, 9);//CE,CSN

const byte address[6] = "00001";

int relay = 2;


void setup() {
  pinMode(relay, OUTPUT);
  Serial.begin(9600);
  mcp.begin();
  radio.begin();
  radio.setRadiation(RF24_PA_MAX, RF24_2MBPS);
  radio.startListening();
  radio.openReadingPipe(0, address);
}
void loop() {
  if (radio.available());
    byte Array[3];
    radio.read(&Array, sizeof(Array));
    mcp.setChannelValue(MCP4728_CHANNEL_A, Array[0]);
    mcp.setChannelValue(MCP4728_CHANNEL_B, Array[1]);
    Serial.println(Array[0]);
    Serial.println(Array[1]);
    Serial.println(Array[2]);
    if(Array[2] == LOW){
      digitalWrite(relay, HIGH);
      delay(500);
      }else{
        digitalWrite(relay, LOW);
      }
    delay(500);
}
