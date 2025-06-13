//This is for testing 4-20mA sensors
//Ensure there is 250 Ohm resistor between input and sensor return
const int press = A5;
int pressValue = 0;

void setup() {
  Serial.begin(9600);
  pinMode(press, INPUT);
}

void loop() {
  pressValue = analogRead(press);
  Serial.println(pressValue);
  delay(500);
}
