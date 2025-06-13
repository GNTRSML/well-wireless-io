//This is a quick and simple test for analog input from 1-5v sensors

const int press = A5;
int pressValue = 0;

void setup() {
  Serial.begin(9600);
  pinMode(press, INPUT);
  analogReference(EXTERNAL);//If using external ensure AREF is jumped to a 5v supply, also have a shared ground for the sensor and board to avoid floating input
}

void loop() {
  pressValue = analogRead(press);
  Serial.println(pressValue);
  analogWrite(pressOut, pressValue);
  delay(100);
}
