// Include the library
#include <TM1637Display.h>

// Define the connections pins
#define CLK 2
#define DIO 3

#define buzz 8

#define btn1 4
#define btn2 5  
#define btn3 6
#define btn4 7

// Create a display object of type TM1637Display
TM1637Display display = TM1637Display(CLK, DIO);

// Create an array that turns all segments ON
const uint8_t allON[] = {0xff, 0xff, 0xff, 0xff};

// Create an array that turns all segments OFF
const uint8_t allOFF[] = {0x00, 0x00, 0x00, 0x00};

// Create an array that sets individual segments per digit to display the word "dOnE"
const uint8_t done[] = {
    SEG_B | SEG_C | SEG_D | SEG_E | SEG_G,           // d
    SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,   // O
    SEG_C | SEG_E | SEG_G,                           // n
    SEG_A | SEG_D | SEG_E | SEG_F | SEG_G            // E
};

// Create degree celsius symbol
const uint8_t celsius[] = {
  	SEG_A | SEG_B | SEG_F | SEG_G,  // Degree symbol
  	SEG_A | SEG_D | SEG_E | SEG_F   // C
};

bool btn1_read = false;
bool btn2_read = false;
bool btn3_read = false;
bool btn4_read = false;

unsigned long previousMillis = 0;
const long debounceInterval = 50;
unsigned long currentMillis;

volatile int secondCount = 0;
volatile int minuteCount = 0;
volatile int hourCount = 0;

int HourMinute;
int MinuteSecond;

bool toggle1 = 0;

bool displayMode = 0;

int selector = 0;

bool updateState = 0;

void setup() {
    pinMode(buzz, OUTPUT);

    pinMode(btn1, INPUT);
    pinMode(btn2, INPUT);
    pinMode(btn3, INPUT);
    pinMode(btn4, INPUT);

    display.setBrightness(5);
    display.setSegments(allON);
    delay(1000);
	display.clear();

    Serial.begin(9600);

    cli();//stop interrupts

	//set timer1 interrupt at 1Hz
	TCCR1A = 0;// set entire TCCR1A register to 0
	TCCR1B = 0;// same for TCCR1B
	TCNT1  = 0;//initialize counter value to 0
	// set compare match register for 1hz increments
	OCR1A = 15624;// = (16*10^6) / (1*1024) - 1 (must be <65536)
	// turn on CTC mode
	TCCR1B |= (1 << WGM12);
	// Set CS12 and CS10 bits for 1024 prescaler
	TCCR1B |= (1 << CS12) | (1 << CS10);  
	// enable timer compare interrupt
	TIMSK1 |= (1 << OCIE1A);

	sei();//allow interrupts
}

void loop() {
    currentMillis = millis();

    btn1_read = digitalRead(btn1);
    btn2_read = digitalRead(btn2);
    btn3_read = digitalRead(btn3);
    btn4_read = digitalRead(btn4);

    if (btn2_read == HIGH) {
		if (currentMillis - previousMillis >= debounceInterval) {
			previousMillis = currentMillis;
			tone(buzz, 1000, 20);
			displayMode = !displayMode;
		}
  	}
	
	if (btn1_read == HIGH) {
		if (currentMillis - previousMillis >= debounceInterval) {
			previousMillis = currentMillis;
			tone(buzz, 1000, 20);
			updateState = !updateState;
			updateClock();
		}
  	}

	HourMinute = hourCount * 100 + minuteCount;
	MinuteSecond = minuteCount * 100 + secondCount;

  	Serial.print(hourCount);
    Serial.print(":");
    Serial.print(minuteCount);
    Serial.print(":");
    Serial.println(secondCount);
}

void incrementSecond(){
	secondCount++;
	if(secondCount == 60){
		secondCount = 0;
		minuteCount++;
	}
	if(minuteCount == 60){
		minuteCount = 0;
		hourCount++;
	}
	if(hourCount == 24){
		hourCount = 0;
	}
}

ISR(TIMER1_COMPA_vect){//timer1 interrupt 1Hz toggles pin 13 (LED)
//generates pulse wave of frequency 1Hz/2 = 0.5kHz (takes two cycles for full wave- toggle high then toggle low)
	incrementSecond();
	displayTime();
	if (toggle1){
		toggle1 = 0;
	}
	else{
		toggle1 = 1;
	}
}

void displayTime(){
	if (displayMode){
		display.showNumberDecEx(HourMinute, 1, false, 4, 0);
	}
	else{
		display.showNumberDecEx(MinuteSecond, toggle1*0b11100000, false, 4, 0);
	}
}

void updateClock(){
	while(updateState){
		currentMillis = millis();

		//selector = 0 -> hour
		//selector = 1 -> minute
		//selector = 2 -> second

		if (btn1_read == HIGH) {
			if (currentMillis - previousMillis >= debounceInterval) {
				previousMillis = currentMillis;
				tone(buzz, 1000, 20);
				updateState = !updateState;
				updateClock();
			}
		}

		if (digitalRead(btn2) == HIGH) {
			if (currentMillis - previousMillis >= debounceInterval) {
				previousMillis = currentMillis;
				tone(buzz, 1000, 20);
				selector++;
				if(selector == 3){
					selector = 0;
				}
			}
		}

		//increment button
		if (digitalRead(btn4) == HIGH) {
			if (currentMillis - previousMillis >= debounceInterval) {
				previousMillis = currentMillis;
				tone(buzz, 1000, 20);
				if(selector == 0){
					hourCount++;
					if(hourCount == 24){
						hourCount = 0;
					}
				}
				else if(selector == 1){
					minuteCount++;
					if(minuteCount == 60){
						minuteCount = 0;
					}
				}
				else if(selector == 2){
					secondCount++;
					if(secondCount == 60){
						secondCount = 0;
					}
				}
			}
		}

		//decrement button
		if (digitalRead(btn3) == HIGH) {
			if (currentMillis - previousMillis >= debounceInterval) {
				previousMillis = currentMillis;
				tone(buzz, 1000, 20);
				if(selector == 0){
					hourCount--;
					if(hourCount == -1){
						hourCount = 23;
					}
				}
				else if(selector == 1){
					minuteCount--;
					if(minuteCount == -1){
						minuteCount = 59;
					}
				}
				else if(selector == 2){
					secondCount--;
					if(secondCount == -1){
						secondCount = 59;
					}
				}
			}
		}
		
		if(selector < 2){
			displayMode = 1;
		}
		else{
			displayMode = 0;
		}
		displayTime();
	}
}