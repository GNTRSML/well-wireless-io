//The transmitting side of a system for plunger control only

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <avr/wdt.h>

RF24 radio(8, 9);//CE,CSN

const byte address[6] = "00001";

const int plg = 2;

unsigned long previousmillis;
unsigned long currentmillis;
const long interval = 10000;

bool plunge;

void setup() {
  Serial.begin(9600);
  while(!Serial){}

  wdt_enable(WDTO_8S);

  pinMode(plg, INPUT_PULLUP);
  retry:
  radio.begin();
  radio.openWritingPipe(address);
  radio.setRadiation(RF24_PA_HIGH, RF24_2MBPS);
  radio.stopListening();
  radio.enableDynamicPayloads();

  /*if(radio.isChipConnected() == false){
    Serial.println("Chip Not Found");
    delay(1000);
    goto retry;
  }*/
}

void loop() {
  currentmillis = millis();

  plunge = digitalRead(plg);
  Serial.println(plunge);
  if(radio.failureDetected == true){
    reset();
  }

  if(radio.isChipConnected() == false){
    radio.failureDetected = true;
  }

  if(radio.getDataRate() != RF24_2MBPS || radio.getPALevel() != RF24_PA_HIGH){
    radio.failureDetected = true;
  }

  while(!radio.write(&plunge, sizeof(plunge))){
    currentmillis = millis();
    if(currentmillis - previousmillis > interval){
      radio.failureDetected = true;
      previousmillis = currentmillis;
      break;
    }
    delay(500);
  }
    previousmillis = currentmillis;
  delay(500);
}




void reset(){
  Serial.println("Radio timeout, resetting.");
  retry:
  radio.begin();
  radio.openWritingPipe(address);
  radio.setRadiation(RF24_PA_HIGH, RF24_2MBPS);
  radio.stopListening();
  radio.enableDynamicPayloads();
  if(!radio.isChipConnected() || radio.getDataRate() != RF24_2MBPS || radio.getPALevel() != RF24_PA_HIGH){
    Serial.println("Settings Mismatch or Chip Not Found");
    goto retry;
  }
  for(int i = 0; i < 10; i++){
    Serial.println("");
  }
  radio.failureDetected = false;
}
