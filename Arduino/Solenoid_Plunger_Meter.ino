//This is the side that connects to the meter for plunger output to meter and an input from the meter.
//As a warning this is setup for the meter to output like it would to the solenoid normally, so ensure the voltage coming out of the meter is within 0-5v before it reaches the board
//Battery voltage can vary greatly what is output from meter and voltage divider could cause voltage to not be high enough for normal digital, therefore it is placed on an analog input.

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <avr/wdt.h>

RF24 radio (8,9);//CE,CSN

const byte address[6] = "00001";//Unique identifier, ensure same on both sides

//Input/Output Pins, change to your liking
const int meter = A0;//input and output pins
const int plg = 3;
const int irq = 2;


//Place to store values transmitted
bool plunge;
int sole;

//Timer setup, set interval as long as you like
unsigned long previousmillis;
int interval = 10000;



void setup() {
  Serial.begin(9600);
  while(!Serial){}
  
  wdt_enable(WDTO_8S);

  pinMode(plg, OUTPUT);
  pinMode(meter, INPUT);
  retry:  
  radio.begin();
  radio.openReadingPipe(1, address);
  radio.setRadiation(RF24_PA_MIN, RF24_2MBPS);
  radio.startListening();
  radio.enableDynamicPayloads();
  radio.enableAckPayload(); 
  radio.writeAckPayload(1, &sole, sizeof(sole));
  if(radio.isChipConnected() == false){
    Serial.println("Chip Not Found");
    delay(1000);
    goto retry;
  }
}



void loop() {
  wdt_reset();
  sole = analogRead(meter);
  plunge = "";
  unsigned long currentmillis = millis();
  if(radio.failureDetected == true){
    digitalWrite(plg, LOW);
    reset();
  }
  if(radio.isChipConnected() == false){
    radio.failureDetected = true;
  }
  if(radio.getDataRate() != RF24_2MBPS || radio.getPALevel() != RF24_PA_MIN){
    radio.failureDetected = true;
  }

  Serial.print("Solenoid Input is: ");
  Serial.println(sole);
  while(!radio.available()){
    //Serial.println("Data Not Available");
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
    //Serial.println("Data Available");
    wdt_reset();
    currentmillis = millis();
    if(currentmillis - previousmillis > interval){
      previousmillis = currentmillis;
      radio.failureDetected = true;
      break;
    }
    radio.read(&plunge, sizeof(plunge));
    radio.writeAckPayload(1, &sole, sizeof(sole));
    Serial.print("Plunger is: ");
    Serial.println(plunge);
    previousmillis = currentmillis;
    if(plunge == 0){
      digitalWrite(plg, HIGH);
    }else{
      digitalWrite(plg, LOW);
    }
  }
  previousmillis = currentmillis;
  delay(500);
}




void reset(){
  Serial.println("Radio timeout, resetting.");
  retry:
  radio.begin();
  radio.openReadingPipe(1, address);
  radio.setRadiation(RF24_PA_MIN, RF24_2MBPS);
  radio.startListening();
  radio.enableDynamicPayloads();
  radio.enableAckPayload(); 
  if(!radio.isChipConnected() || radio.getDataRate() != RF24_2MBPS || radio.getPALevel() != RF24_PA_MIN){
    Serial.println("Settings Mismatch or Chip Not Found");
    goto retry;
  }
  radio.writeAckPayload(1, &sole, sizeof(sole));
  radio.failureDetected = false;
}
