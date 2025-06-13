//Reciever end of radio system with only basic plunger control to the meter

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <avr/wdt.h>

RF24 radio(8, 9);//CE,CSN

const byte address[6] = "00001";

const int relay = 2;

unsigned long previousmillis;
unsigned long currentmillis;
const long interval = 10000;

bool meter;
bool plunge;

void setup() {
  Serial.begin(9600);
  while(!Serial){}

  wdt_enable(WDTO_8S);

  pinMode(relay, OUTPUT);

  radio.begin();
  radio.openReadingPipe(0, address);
  radio.setRadiation(RF24_PA_HIGH, RF24_2MBPS);
  radio.startListening();
  radio.enableDynamicPayloads();
}




void loop() {
  currentmillis = millis();
  plunge = "";
  if(radio.failureDetected == true){
    reset();
  }
  if(radio.isChipConnected() == false){
    radio.failureDetected = true;
  }
  if(radio.getDataRate() != RF24_2MBPS || radio.getPALevel() != RF24_PA_HIGH){
    radio.failureDetected = true;
  }
  while(!radio.available()){
    currentmillis = millis();
    if(currentmillis - previousmillis > interval){
      radio.failureDetected = true;
      previousmillis = currentmillis;
      break;
    }
  }
  while(radio.available()){
    currentmillis = millis();
    if(currentmillis - previousmillis > interval){
      previousmillis = currentmillis;
      radio.failureDetected = true;
      break;
    }
    radio.read(&plunge, sizeof(plunge));
    Serial.println(plunge);
    if(plunge == 0){
      digitalWrite(meter, HIGH);
    }else{
      digitalWrite(meter, LOW);
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
  radio.openReadingPipe(0, address);
  radio.setRadiation(RF24_PA_HIGH, RF24_2MBPS);
  radio.startListening();
  radio.enableDynamicPayloads();
  if(!radio.isChipConnected() || radio.getDataRate() != RF24_2MBPS || radio.getPALevel() != RF24_PA_HIGH){
    Serial.println("Settings Mismatch or Chip Not Found");
    goto retry;
  }
  radio.failureDetected = false;
}
