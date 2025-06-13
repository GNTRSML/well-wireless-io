//Quick test sketch for plunger sensor input
//Make aboslutely sure that the sensor you are using requires a pullup resistor on the sense wire, all sensors tested with this operated this way but it never applies to all

const int plgPin = 2;

int ledPin = 13;

void setup() {
  pinMode(plgPin, INPUT_PULLUP);
}                               

void loop() {
  digitalRead(plgPin);
    if(digitalRead(plgPin) == LOW){
    digitalWrite(ledPin, HIGH);   
    }else{                        
      digitalWrite(ledPin, LOW);  
    }
}
