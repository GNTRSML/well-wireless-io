//This can be ignored for now, this is just some unfinished testing for a setup process. May or may not ever be complete by me.

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Arduino.h>
#include <EEPROM.h>

RF24 radio(8, 9);//CE,CSN
const byte address[6] = "00001";//address should be fine for anything unless multicasting

String inputyn;//Strings for serial inputs
String outputyn;
String inputty;
String txrx;
String rapower;

int settxrx;//all integers here go in the array
int inputtype;
int input;
int output;
int rpower;
int inyn;
int outyn;
int array[7];

int plunge;
int sole;
int relay;
const int irq = 2;
const int meter = A0;


void setup() {
  Serial.begin(9600);
  radio.begin();
  radio.enableDynamicPayloads();
  radio.enableAckPayload();
  pinMode(irq, INPUT);
  if(EEPROM.read(0) != 0){
    EEPROM_get();
  }else{
    config();
  }
}


void EEPROM_get(){
  EEPROM.get(0, array);
  settxrx = array[0];
  inputtype = array[1];
  input = array[2];
  output = array[3];
  rpower = array[4];
  inyn = array[5];
  outyn = array[6];
  settings();
}




void config(){
  delay(1000);


  Serial.println("Recieve or Transmit?");
  while(!Serial.available()){}
  txrx = Serial.readStringUntil('\n');
      if(txrx == "recieve"){
        radio.startListening();
        Serial.println("\nReciever");
        settxrx = 1;
        }else if(txrx == "transmit"){
          radio.stopListening();
          Serial.println("\nTransmitter");
          settxrx = 0;
        }
  delay(1000);


  Serial.println("Set Power Level: Min, Low, High, Max");
  while(!Serial.available()){}
  rapower = Serial.readStringUntil('\n');
  if(rapower == "min"){
    Serial.println("\nMin Power");
    rpower = 0;
    radio.setPALevel(RF24_PA_MIN);
  }else if(rapower == "low"){
    Serial.println("\nLow Power");
    rpower = 1;
    radio.setPALevel(RF24_PA_LOW);
  }else if(rapower == "high"){
    Serial.println("\nHigh Power");
    rpower = 2;
    radio.setPALevel(RF24_PA_HIGH);
  }else if(rapower == "max"){
    Serial.println("\nMax Power");
    rpower = 3;
    radio.setPALevel(RF24_PA_MAX);
  }


  Serial.println("Would you like to setup an input? Yes No");
  while(!Serial.available()){}
  inputyn = Serial.readStringUntil('\n');
  if(inputyn == "yes"){
    inyn = 1;
    Serial.println("\nIs this meter input or plunger input?");
    Serial.println("\nNote:If meter, input pin must be an analog input pin, these are marked as A# on the board, ");
    Serial.println("this is to prevent voltage issues, also make sure the voltage coming from meter on input is below 5v");
    Serial.println("Cannot make pin 2 input, it is reserved for IRQ");
    while(!Serial.available()){}
    inputty = Serial.readStringUntil('\n');
    if(inputty == "meter"){
      Serial.println("Input is A0");
      inputtype = 1;
      input = A0;
      pinMode(A0, INPUT);

    }else if(inputty == "plunger"){
      inputtype = 0;
      Serial.println("\nSet Input Pin #");
      while(!Serial.available()){}
      input = Serial.parseInt();
      pinMode(input, INPUT_PULLUP);
      while(Serial.available() != 0){
        Serial.read();
      }
    }
  }else if(inputyn == "no"){
    inyn = 0;
  }


  Serial.println("\nWould you like to setup an output? Yes No");
  Serial.println("Note:Cannot make pin 2 output, it is reserved for IRQ");
  while(!Serial.available()){}
  outputyn = Serial.readStringUntil('\n');
  if(outputyn == "yes"){
    outyn = 1;
    Serial.println("\nSet Output Pin #");
    while(!Serial.available()){}
    output = Serial.parseInt();
    pinMode(output, OUTPUT);
    while(Serial.available() != 0){
      Serial.read();
    }
  }else if(outputyn == "no"){
    outyn = 0;
  }
  settings();
}

void settings(){
  if(settxrx == 1){
    radio.stopListening();
    radio.openReadingPipe(1, address);
  }else{
    radio.stopListening();
    radio.openWritingPipe(address);
  }
  if(rpower == 0){
    radio.setPALevel(RF24_PA_MIN);
  }else if(rpower == 1){
    radio.setPALevel(RF24_PA_LOW);
  }else if(rpower == 2){
    radio.setPALevel(RF24_PA_HIGH);
  }else if(rpower == 3){
    radio.setPALevel(RF24_PA_MAX);
  }
  if(inyn == 1){
    if(inputtype == 1){
      input = A0;
    }else{
      
    }
  }
  confirmset();
}


void confirmset() {
  String configure;
  
  delay(5000);
  if(inputyn == "yes"){
    if(inputty =="plunger"){
    Serial.print("\nInput Pin is ");
    Serial.println(input);
    }else{
      Serial.println("\nInput Pin is A0");
    }
    Serial.print("Input Type is ");
    Serial.println(inputty);
  }
  if(outputyn == "yes"){
  Serial.print("Output pin is ");
  Serial.println(output);
  }
  Serial.print("Radio Power is ");
  Serial.println(rapower);
  Serial.print("\nMode is ");
  Serial.println(txrx);
  Serial.println("Interrupt is always pin 2");
  Serial.println("\nWould you like to reconfigure?  Yes   No");
 while(!Serial.available()){
  int count;
  int interval = 10000;
  unsigned long currentmillis = millis();
  unsigned long previousmillis = 0;
  for(count = 0; count <= 2; count++){
    previousmillis = currentmillis;
  }
  if(currentmillis - previousmillis >= interval){
    Serial.println("No User Input, Continue Program");
    previousmillis = currentmillis;
    count = 0;
    configure = "no";
    break;
  }
 }
 if(Serial.available() == 1){
  configure = Serial.readStringUntil('\n');
  if(configure == "yes"){
    config();
  }else{
    EEPROM_write();
  }
 }else{
  loop();
 }
}




void EEPROM_write(){
  array[0] = settxrx;
  array[1] = inputtype;
  array[2] = input;
  array[3] = output;
  array[4] = rpower;
  array[5] = inyn;
  array[6] = outyn;
  EEPROM.put(0, array);
  Serial.println("Saved");
}





void loop() {
  if(Serial.available()){
    String reconfigure = Serial.readStringUntil('\n');
    if(reconfigure == "reconfigure"){
      config();
    }
  }
  Serial.println("it worked.......for now");
  delay(5000);
}
