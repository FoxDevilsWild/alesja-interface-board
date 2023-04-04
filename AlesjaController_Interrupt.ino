volatile byte state = 0;
byte last_state = 0;

void setup() {
  pinMode(3, INPUT);
  pinMode(4, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(3), rising, CHANGE);
  Serial.begin(9600);
}

void loop() {
  if (last_state != state){
    Serial.println(state);
    last_state = state;
  }
}

void rising(){
  state = !state;
}

void falling(){
  state = LOW;
}