/*
  AlesjaInterface

  The outputs are switched on the basis of different input patterns.
  The interface is used with different external devices, but not in parallel.
  That is why, there are four different modes working in parallel but not interfering
  with one another, since only device is connected.
  (Device for mode 1 is connected but mode 2 and 3 still do emit on their outputs)

  MODE ON_OFF (0) - Output 2
  When the input is active for at least an amount of time the output is toggled.

  MODE 1 - Output 1
  This mode searches for a pattern in the input signal after each signal edge and 
  toggles the desired output after a falling edge, if any pattern was found.

  MODE 2 - Output 3-6
  This mode also searches for a pattern in the input signal but in contrast to MODE 1,
  the parsing is only done after for an amount of there was no new input (rising edge)
  after a falling edge. Hence, the input pattern can be similar to on another.

  MODE 3 - Output 8
  This mode directly outputs if the input is active and stops if not.

  NOTE
  As long as the an output (except mode 3) is active the programm is in a sleep mode.
  Hence no user input will be evaluated during that time.
  But, since the output time is very short, it should not make a different in practice.
  TODO: Switch to an intterupt solution.

*/
//////////////////////////////
//  CONSTANTS (won't change)
//////////////////////////////
const bool debug = true;
const bool DISABLE_MODE_ON_OFF = false;
const bool DISABLE_MODE_1 = false;
const bool DISABLE_MODE_2 = false;
const bool DISABLE_MODE_3 = false;
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

// PIN VALUES
const byte INPUT_ACTIVE_STATE = HIGH;
const byte OUTPUT_ACTIVE_STATE = HIGH;

// TIMINGs [ms]
const int DEBOUNCE_DELAY = 25;
const int MIN_SHORT_TS = 50;
const int MAX_SHORT_TS = 300;
const int MIN_LONG_TS = 400;
const int MAX_LONG_TS = 1500;
const int MIN_ONOFF_TS = 2000;
const int MAX_ONOFF_TS = 2500;
const int RESET_DELAY = 1500;
const int ACCEPT_DELAY = 750;
const int OUTPUT_DELAY = 200;

// MISC
const byte MODE_ON_OFF = 0;
const byte MODE_1 = 1;
const byte MODE_2 = 2;
const byte MODE_3 = 3;

//////////////////////////////
//  DYNAMIC VARIABLES (will change)
//////////////////////////////
// programm states
byte input_state = LOW;
byte last_input_state = LOW;
unsigned long last_time_edge = 0;
unsigned long last_time_rising_edge = 0;
unsigned long last_time_falling_edge = 0;
byte n_short_mode_1 = 0;
byte n_long_mode_1 = 0;
byte n_short_mode_2 = 0;
byte n_long_mode_2 = 0;
bool on_off_emitted = false;

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

  unsigned long current_time = millis();
  unsigned long time_since_last_edge = current_time - last_time_edge;
  unsigned long time_since_last_rising_edge = 0;
  unsigned long time_since_last_falling_edge = 0;
  bool input_is_active = INPUT_ACTIVE_STATE == input_state;

  if (input_is_active){
    time_since_last_rising_edge = time_since_last_edge;
  } else {
    time_since_last_falling_edge = time_since_last_edge;
  }

  // MODE 3
  if (!DISABLE_MODE_ON_OFF){    
    digitalWrite(PIN_OUT_A8, input_state);
  }
  
  // MODE ON/OFF  
  bool on_off_input_time_is_reached = (time_since_last_rising_edge > MIN_ONOFF_TS) && (time_since_last_rising_edge < MAX_ONOFF_TS);
  if (input_is_active && !on_off_emitted && on_off_input_time_is_reached){
    on_off_emitted = true;
    writeOutput(PIN_OUT_A2, MODE_ON_OFF);
  }

  // MODE 2
  bool mode_2_parsing_active = (time_since_last_falling_edge > ACCEPT_DELAY) && (time_since_last_falling_edge < (2*ACCEPT_DELAY));
  if (mode_2_parsing_active){
    // parse pattern
    switch (n_short_mode_2) {
      case 1:
        writeOutput(PIN_OUT_A3, MODE_2);
        break;
      case 2:
        writeOutput(PIN_OUT_A4, MODE_2);
        break;
      default:
        break;      
    }
  }

  // MODE 1 & 2 Reset if no input for too long
  if (!input_is_active && (time_since_last_falling_edge > RESET_DELAY)){
    reset();
  }

  // edge detection
  bool input_changed = (input_state != last_input_state);
  if (input_changed) {      
    last_time_edge = current_time;      
    // input parsing
    if (input_is_active){  // rising edge
      last_time_rising_edge = current_time;
      if (debug){
        Serial.print("Pause: ");
        Serial.print(time_since_last_edge);
        Serial.print("ms");
      }
    } else { // falling edge
      last_time_falling_edge = current_time;
      parse_last_input_length(time_since_last_edge);

      on_off_emitted = false;  // reset MODE on off

      if (debug){
        Serial.print(" - Active: ");
        Serial.print(time_since_last_edge);
        Serial.println("ms");
        print_state();
      }
    }

    // MODE 1
    // pattern parsing
    if (2 == n_short_mode_1 && 1 == n_long_mode_1){
      writeOutput(PIN_OUT_A1, MODE_1);      
    }

    // update    
    last_input_state = input_state;
  }
}

void parse_last_input_length(unsigned long time_since_last_edge){
  bool is_short_input = (time_since_last_edge > MIN_SHORT_TS) && (time_since_last_edge < MAX_SHORT_TS);
  bool is_long_input = (time_since_last_edge > MIN_LONG_TS) && (time_since_last_edge < MAX_LONG_TS);
  if (is_short_input){
    n_short_mode_1++;
    n_short_mode_2++;
  }
  if (is_long_input){
    n_long_mode_1++;
    n_long_mode_2++;
  }
}

void writeOutput(const byte pin, const byte mode){
  if (disable_output(pin, mode)){
    return;
  }

  if (debug){
    Serial.print("Mode ");
    Serial.print(mode);
    Serial.print(" - Write: ");
    Serial.println(pin);
  }

  digitalWrite(pin, OUTPUT_ACTIVE_STATE);
  delay(OUTPUT_DELAY);
  digitalWrite(pin, !OUTPUT_ACTIVE_STATE);
  
  reset_by_mode(mode);
}

void reset_by_mode(const byte mode){
  // reset counter
  switch (mode){
    case MODE_ON_OFF:
      break;
    case MODE_1:
      reset_mode_1();
      break;
    case MODE_2:
      reset_mode_2();
      break;
    default:
      if (debug){
        Serial.print("ERROR: [writeOutput] - Mode was ");
        Serial.println(mode);
      }
      break;
  }
}

void reset_mode_1(){
  n_short_mode_1 = 0;
  n_long_mode_1 = 0;
}

void reset_mode_2(){
  n_short_mode_2 = 0;
  n_long_mode_2 = 0;
}

void reset(){
  reset_mode_1();
  reset_mode_2();
}

bool disable_output(const byte pin, const byte mode){
switch (mode){
    case MODE_ON_OFF:
      return DISABLE_MODE_ON_OFF;
    case MODE_1:
      return DISABLE_MODE_1;
    case MODE_2:
      return DISABLE_MODE_2;
    case MODE_3:
      return DISABLE_MODE_3;
    default:
      return false;
  }
}

void print_state(){
  Serial.print("#Short: ");
  Serial.print(n_short_mode_1);
  Serial.print(", #Long: ");
  Serial.println(n_long_mode_2);
}