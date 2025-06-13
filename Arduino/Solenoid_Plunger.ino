//This side goes on a gas unit for control over a solenoid from meter and plunger input to meter

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <avr/wdt.h>

RF24 radio (8,9);//CE,CSN

const byte address[6] = "00001";

const int solenoid = 4;
const int plg = 5;
const int irq = 2;

bool plunge;
int sole;

unsigned long previousmillis;
const long interval = 10000;

void setup() {
  Serial.begin(9600);
  while(!Serial){}
  Serial.println("setup running");
  wdt_enable(WDTO_8S);

  pinMode(irq, INPUT);
  pinMode(solenoid, OUTPUT);
  pinMode(plg, INPUT_PULLUP);
  retry:    
  radio.begin();
  radio.openWritingPipe(address);
  radio.setRadiation(RF24_PA_MIN, RF24_2MBPS);
  radio.stopListening();
  radio.enableDynamicPayloads();
  radio.enableAckPayload();
  
  if(radio.isChipConnected() == false){
    Serial.println("Chip Not Found");
    delay(1000);
    goto retry;

  }
}

void loop() {
  wdt_reset();
  plunge = digitalRead(plg);
  sole = "";
  unsigned long currentmillis = millis();
  previousmillis = currentmillis;
  Serial.print("Plunger is: ");
  Serial.println(plunge);
  if(radio.failureDetected == true){
    digitalWrite(solenoid, LOW);
    reset();
  }

  if(radio.isChipConnected() == false){
    radio.failureDetected = true;
  }
   if(radio.getDataRate() != RF24_2MBPS || radio.getPALevel() != RF24_PA_MIN){
    radio.failureDetected = true;
  }


  while(!radio.write(&plunge, sizeof(plunge))){
    //Serial.println("Writing");
    wdt_reset();
    currentmillis = millis();
    if(currentmillis - previousmillis > interval){
      radio.failureDetected = true;
      previousmillis = currentmillis;
      break;
    }
    if(radio.isChipConnected() == false){
      radio.failureDetected = true;
      break;
    }
    if(radio.getDataRate() != RF24_2MBPS || radio.getPALevel() != RF24_PA_MIN){
      radio.failureDetected = true;
      break;
    }
  }
    while(radio.available()){
      //Serial.println("ACK Available");
      wdt_reset();
      currentmillis = millis();
      if(currentmillis - previousmillis > interval){
        radio.failureDetected = true;
        previousmillis = currentmillis;
      }
      radio.read(&sole, sizeof(sole));
      Serial.print("Meter Input is: ");
      Serial.println(sole);
      if(sole > 100){
        digitalWrite(solenoid, HIGH);
      }else{
        digitalWrite(solenoid, LOW);
        }
    }
  delay(500);
}





void reset(){
  Serial.println("Resetting");
  retry:
  radio.begin();
  radio.openWritingPipe(address);
  radio.setRadiation(RF24_PA_MIN, RF24_2MBPS);
  radio.stopListening();
  radio.enableDynamicPayloads();
  radio.enableAckPayload();
  if(!radio.isChipConnected() || radio.getDataRate() != RF24_2MBPS || radio.getPALevel() != RF24_PA_MIN){
    Serial.println("Settings Mismatch or Chip Not Found");
    goto retry;
  }
  radio.failureDetected = false;
}
