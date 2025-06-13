//Sketch to check for possbile wiring or module issues
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <printf.h>

RF24 radio(8, 9);//CE,CSN

const byte address[6] = "BNG03";

void setup() {
  Serial.begin(9600);
  radio.begin();
  radio.setChannel(120);
  radio.setDataRate(RF24_250KBPS);
  radio.setPALevel(RF24_PA_MIN);
  radio.stopListening();
  radio.openWritingPipe(address);
  printf_begin();
  delay(2000);
  radio.isChipConnected();{
    if (radio.isChipConnected() == 1){
      Serial.println("Chip Found");
      radio.printPrettyDetails();  
    }else{                           
      Serial.println("Chip Not Found");
    }
  }
}

void loop() {
  //empty loop, reset board for new printout
}
