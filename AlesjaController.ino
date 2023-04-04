///////////
//  CONSTANTS
////////////
// Pinning
const byte PIN_INPUT = 3;
const byte PIN_OUT_A1 = 4;
const byte PIN_OUT_A2 = 5;
const byte PIN_OUT_A3 = 6;
const byte PIN_OUT_A4 = 7;
const byte PIN_OUT_A5 = 8;
const byte PIN_OUT_A6 = 9;
const byte PIN_OUT_A7 = 10;
const byte PIN_OUT_A8 = 11;

// pin values
const byte INPUT_ACTIVE_STATE = HIGH;
const byte OUTPUT_ACTIVE_STATE = HIGH;

// TIMING
const int DEBOUNCE_DELAY = 25;
const int MIN_SHORT_TS = 50;
const int MAX_SHORT_TS = 250;
const int MIN_LONG_TS = 400;
const int MAX_LONG_TS = 1500;
const int MIN_ONOFF_TS = 2000;
const int MAX_ONOFF_TS = 4000;
const int RESET_DELAY = 1500;
const int OUTPUT_DELAY = 3000;

////////////
//  Dynamic variables
////////////
// programm states
byte input_state = LOW;
byte last_input_state = LOW;
unsigned long last_time_edge = 0;
byte n_short = 0;
byte n_long = 0;

// MISC
const bool debug = true;

void setup() {
  pinMode(PIN_OUT_A1, OUTPUT);
  pinMode(PIN_OUT_A2, OUTPUT);
  pinMode(PIN_OUT_A3, OUTPUT);
  pinMode(PIN_OUT_A4, OUTPUT);
  pinMode(PIN_OUT_A5, OUTPUT);
  pinMode(PIN_OUT_A6, OUTPUT);
  pinMode(PIN_OUT_A7, OUTPUT);
  pinMode(PIN_OUT_A8, OUTPUT);
  pinMode(PIN_INPUT, INPUT);
  if (debug) {
    Serial.begin(9600);
  }
}

void loop() {
  input_state = digitalRead(PIN_INPUT);

  bool input_changed = (input_state != last_input_state);
  // edge detection with software input debounce (just in case)
  if (input_changed) {    
    unsigned long current_time = millis();
    unsigned long time_since_last_edge = current_time - last_time_edge;

    // input parsing
    if (INPUT_ACTIVE_STATE == input_state){  // rising edge
      last_time_edge = current_time;
      if (time_since_last_edge > RESET_DELAY){
        reset();
      }
    } else { // falling edge
      bool is_short_input = (time_since_last_edge > MIN_SHORT_TS) && (time_since_last_edge < MAX_SHORT_TS);
      bool is_long_input = (time_since_last_edge > MIN_LONG_TS) && (time_since_last_edge < MAX_LONG_TS);
      if (is_short_input){
        n_short++;
      }
      if (is_long_input){
        n_long++;
      }
    }

    // pattern parsing
    if (2 == n_short && 1 == n_long){
      writeOutput(PIN_OUT_A1);      
    }

    // output
    digitalWrite(PIN_OUT_A8, input_state); // allways patch thorugh for pc
    

    // update    
    last_input_state = input_state;

    // debug
    if (debug) {      
      Serial.println(time_since_last_edge);
      Serial.println(input_state);
    }
  }
}

void writeOutput(const byte pin){
  digitalWrite(pin, OUTPUT_ACTIVE_STATE);
  delay(OUTPUT_DELAY);
  digitalWrite(pin, !OUTPUT_ACTIVE_STATE);
  reset();
}

void reset(){
  n_short = 0;
  n_long = 0;
}