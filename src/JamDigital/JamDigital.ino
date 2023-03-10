// Include the library
#include <TM1637Display.h>
#include <TimerOne.h>

// Define the connections pins
#define CLK  2
#define DIO  3
#define BTN1 4
#define BTN2 5  
#define BTN3 6
#define BTN4 7
#define BUZZ 8

// Define constant macro for state control
#define CLOCK 0x0
#define WATCH 0x1
#define SETUP 0x2

// Define constant macro for display state
#define DISP_HM  0x0
#define DISP_MS  0x1
#define DISP_SMS 0x2

// Define constant macro for watch state
#define RESET 0x0
#define START 0x1
#define PAUSE 0x2

// Define constant macro for setup state
#define CH_H  0x0
#define CH_M  0x1
#define CH_S  0x2

// Define constant macro for timer control
#define SCALING_FACTOR 0.9964

// Create a display object of type TM1637Display
TM1637Display display = TM1637Display(CLK, DIO);

// Create an array that turns segments ON/OFF
const uint8_t OFF[] = {0x00, 0x00};

// Create state control variable
int system_state = CLOCK;

// Create time counter variable
unsigned int milisecond = 0, second = 0, minute = 0, hour = 0;
unsigned int watch_ms = 0, watch_s = 0, watch_m = 0, watch_h = 0;

// Create button control variable
int btn1_state = LOW, btn2_state = LOW, btn3_state = LOW, btn4_state = LOW;

// Create control variable in every state
int display_mode = DISP_MS;
int watch_start = RESET;
int change_var = CH_S;

void setup() {
  Timer1.initialize(10000 * SCALING_FACTOR);
  Timer1.attachInterrupt(increment_time);
	// Set the brightness to 5 (0 = dimmest, 7 = brightest)
	display.setBrightness(5);
  Serial.begin(115200);
}

void loop() {
  // maintain time counter
  clock_maintain();

  // switch to suitable mode
	switch(system_state){
    case CLOCK:
      clock_mode();
      break;
    case WATCH:
      watch_mode();
      break;
    case SETUP:
      setup_mode();
      break;
    default: // case CLOCK and others
      clock_mode();
      break;
  }

  // update state based on button press
  update_state();

  // update button state at every loop end
  update_button_state();
  Serial.print(system_state);
  Serial.print("\t");
  Serial.println(change_var);
}

void increment_time(){
  milisecond += 10;
  if(watch_start == START) watch_ms += 10;
}

void clock_maintain(){
  // maintain clock
    if(milisecond > 999){
    milisecond -= 1000;
    second += 1;
  }
  if(second > 59){
    second -= 60;
    minute += 1;
  }
  if(minute > 59){
    minute -= 60;
    hour += 1;
  }
  if(hour > 23){
    hour -= 24;
  }

  // maintain watch
  if(watch_ms > 999){
    watch_ms -= 1000;
    watch_s += 1;
  }
  if(watch_s > 59){
    watch_s -= 60;
    watch_m += 1;
  }
  if(watch_m > 59){
    watch_m -= 60;
    watch_h += 1;
  }
}

void clock_mode(){
  unsigned int time_h_m = 100 * hour + minute;
  unsigned int time_m_s = 100 * minute + second;
  unsigned int time_s_ms = 100 * second + (unsigned int) (milisecond / 10);
  switch(display_mode){
    case DISP_HM:
      display.showNumberDecEx(time_h_m, 0xE0, true, 4, 0);
      break;
    case DISP_MS: // case DISP_MS and others
      display.showNumberDecEx(time_m_s, 0xE0, true, 4, 0);
      break;
    case DISP_SMS:
      display.showNumberDecEx(time_s_ms, 0xE0, true, 4, 0);
      break;
    default: // case DISP_MS and others
      display.showNumberDecEx(time_m_s, 0xE0, true, 4, 0);
      break;
  }
  update_display_mode();
}

void watch_mode(){
  if(watch_start == RESET){
    reset_watch();
  }
  unsigned int time_h_m = 100 * watch_h + watch_m;
  unsigned int time_m_s = 100 * watch_m + watch_s;
  unsigned int time_s_ms = 100 * watch_s + (unsigned int) (watch_ms / 10);
  switch(display_mode){
    case DISP_HM:
      display.showNumberDecEx(time_h_m, 0xE0, true, 4, 0);
      break;
    case DISP_MS:
      display.showNumberDecEx(time_m_s, 0xE0, true, 4, 0);
      break;
    case DISP_SMS:
      display.showNumberDecEx(time_s_ms, 0xE0, true, 4, 0);
      break;
    default: // case DISP_MS and others
      display.showNumberDecEx(time_m_s, 0xE0, true, 4, 0);
      break;
  }
  update_display_mode();
  update_watch_mode();
}

void reset_watch(){
  watch_h = 0;
  watch_m = 0;
  watch_s = 0;
  watch_ms = 0;
}

void update_watch_mode(){
  if(get_rising_state(BTN4, btn4_state) == 1){
    watch_start = (watch_start + 1) % 3;
  }
}

void update_display_mode(){
  if(get_rising_state(BTN2, btn2_state) == 1){
    display_mode -= 1;
    if(display_mode < 0){
      display_mode += 1;
    }
  } else if(get_rising_state(BTN3, btn3_state) == 1){
    display_mode += 1;
    if(display_mode > 2){
      display_mode -= 1;
    }
  }
}

void setup_mode(){
  unsigned int time_h_m = 100 * hour + minute;
  unsigned int time_m_s = 100 * minute + second;
  unsigned int time_s_ms = 100 * second + (unsigned int) (milisecond / 10);
  switch(display_mode){
    case DISP_HM:
      display.showNumberDecEx(time_h_m, 0xE0, true, 4, 0);
      break;
    case DISP_MS: // case DISP_MS and others
      display.showNumberDecEx(time_m_s, 0xE0, true, 4, 0);
      break;
    case DISP_SMS:
      display.showNumberDecEx(time_s_ms, 0xE0, true, 4, 0);
      break;
    default: // case DISP_MS and others
      display.showNumberDecEx(time_m_s, 0xE0, true, 4, 0);
      break;
  }
  blink_display();
  set_value();
}

void blink_display(){
  unsigned int time_h_m = 100 * hour + minute;
  unsigned int time_m_s = 100 * minute + second;
  unsigned int time_s_ms = 100 * second + (unsigned int) (milisecond / 10);
  switch(change_var){
    case CH_H:
      display_mode = DISP_HM;
      display.setSegments(OFF, 2, 0);
      delay(50);
      display.showNumberDecEx(time_h_m, 0xE0, true, 4, 0);
      delay(50);
      break;
    case CH_M:
      display_mode = DISP_MS;
      display.setSegments(OFF, 2, 0);
      delay(50);
      display.showNumberDecEx(time_m_s, 0xE0, true, 4, 0);
      delay(50);
      break;
    case CH_S:
      display_mode = DISP_MS;
      display.setSegments(OFF, 2, 2);
      delay(50);
      display.showNumberDecEx(time_m_s, 0xE0, true, 4, 0);
      delay(50);
      break;
    default:
      break;
  }
}

void set_value(){
  int add = 0;
  if(get_rising_state(BTN2, btn2_state) == 1){
    add -= 1;
  } else if(get_rising_state(BTN3, btn3_state) == 1){
    add += 1;
  } else if(get_rising_state(BTN4, btn4_state) == 1){
    change_var = (change_var + 1) % 3;
  }
  switch(change_var){
    case CH_H:
      hour += add;
      break;
    case CH_M:
      minute += add;
      break;
    case CH_S:
      second += add;
      break;
    default:
      break;
  }
}

void update_state(){
  if(get_rising_state(BTN1, btn1_state) == 1){
    system_state = (system_state + 1) % 3;
  }
  if(system_state != WATCH){
    watch_start = RESET;
  }
}

void update_button_state(){
  btn1_state = digitalRead(BTN1);
  btn2_state = digitalRead(BTN2);
  btn3_state = digitalRead(BTN3);
  btn4_state = digitalRead(BTN4);
}

int get_rising_state(uint8_t pin_, int& state_){
  if(state_ == LOW && digitalRead(pin_) == HIGH){
    state_ = digitalRead(pin_);
    return 1;
  } else{
    state_ = digitalRead(pin_);
    return 0;
  }
}