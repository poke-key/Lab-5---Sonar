#include "timerISR.h"
#include "helper.h"
#include "periph.h"

#define START -1
#define PWM_COUNT  10
#define GREEN_LED 4
#define RED_LED 5
#define BLUE_LED 3

int red_duty = 0 ;
int green_duty = 0 ;
int red_on_count = (PWM_COUNT * red_duty) /100 ;
int red_off_count = PWM_COUNT - red_on_count ;

int red_threshold = 10;
int green_threshold = 20;

int green_on_count = (PWM_COUNT * green_duty) /100 ;
int green_off_count = PWM_COUNT - green_on_count ;
enum led_states { OFF, ON} led_state ;
enum thresholds {CLOSE, MODERATE, FAR} threshold ;


//TODO: declare variables for cross-task communication

/* You have 5 tasks to implement for this lab */
#define NUM_TASKS 5
#define TASK1_PERIOD 1000
#define TASK2_PERIOD 1
#define TASK3_PERIOD 1
#define TASK4_PERIOD 1
#define TASK5_PERIOD 200

//Task struct for concurrent synchSMs implmentations
typedef struct _task{
	signed 	 char state; 		//Task's current state
	unsigned long period; 		//Task period
	unsigned long elapsedTime; 	//Time elapsed since last task tick
	int (*TickFct)(int); 		//Task tick function
} task;


//TODO: Define Periods for each task
// e.g. const unsined long TASK1_PERIOD = <PERIOD>
const unsigned long GCD_PERIOD = /* TODO: Calulate GCD of tasks */ 1;
int distance = 0 ;
int inches = 0;

task tasks[NUM_TASKS]; // declared task array with 5 tasks

//TODO: Define, for each task:
// (1) enums and
// (2) tick functions

void TimerISR() {
    
    //TODO: sample inputs here

	for ( unsigned int i = 0; i < NUM_TASKS; i++ ) {                   // Iterate through each task in the task array
		if ( tasks[i].elapsedTime == tasks[i].period ) {           // Check if the task is ready to tick
			tasks[i].state = tasks[i].TickFct(tasks[i].state); // Tick and set the next state for this task
			tasks[i].elapsedTime = 0;                          // Reset the elapsed time for the next tick
		}
		tasks[i].elapsedTime += GCD_PERIOD;                        // Increment the elapsed time by GCD_PERIOD
	}
}


int Tick_display1(int state)
{
  distance = read_sonar() ;
  switch(threshold) {
    case CLOSE:
      red_threshold = 5;
      green_threshold = 10;
      break;
    case MODERATE:
      red_threshold = 10;
      green_threshold = 20;
      break;
    case FAR:
    default: 
      red_threshold = 15;
      green_threshold = 30;
      break;
  }
  if (distance < red_threshold) {
    red_duty = 100 ;
    green_duty = 0 ;
  }
  if ( distance >= red_threshold && distance <= green_threshold) {
    red_duty = 90 ;
    green_duty = 30 ;   
  }
  if ( distance > green_threshold ) {
    red_duty = 0 ;
    green_duty = 100 ;
  }

  switch(state){
    default:
      break;
  }
  switch(state){
    default:
      break;
  }
  return state;
}

enum states {D1, D2, D3, D4} state;
int Tick_display2(int state){
  int digits[4] = {0,0,0,0} ;
  int i =3 ;

  int copydistance = distance ;
  if(inches) 
    copydistance /= 2.54;
  while ( copydistance ) {
    digits[i--] = copydistance % 10 ;
    copydistance /= 10 ;
  }

  switch(tasks[1].state){

    case START:
      tasks[1].state = D1;
      break;
    case D1:
      tasks[1].state = D2;
      break;
    case D2:
      tasks[1].state = D3;
      break;
    case D3:
      tasks[1].state = D4;
      break;  
    case D4:
      tasks[1].state = START;
      break;
    default:
      break;
  }
  switch(tasks[1].state){
    
    case D1:
      PORTB = SetBit(PORTB, 2,0x1) ;
      PORTB = SetBit(PORTB, 3 ,0x1);
      PORTB = SetBit(PORTB, 4 ,0x1);
      PORTB = SetBit(PORTB, 5 ,0x0);
      outNum(digits[0]); break;
      
    case D2:
      PORTB = SetBit(PORTB, 2,0x1) ;
      PORTB = SetBit(PORTB, 3 ,0x1);
      PORTB = SetBit(PORTB, 4 ,0x0);
      PORTB = SetBit(PORTB, 5 ,0x1);

      outNum(digits[1]) ; break;
    case D3:
      PORTB = SetBit(PORTB, 2,0x1) ;
      PORTB = SetBit(PORTB, 3 ,0x0);
      PORTB = SetBit(PORTB, 4 ,0x1);
      PORTB = SetBit(PORTB, 5 ,0x1);
      outNum(digits[2]) ; break;
    case D4:
      PORTB = SetBit(PORTB, 2,0x0) ;
      PORTB = SetBit(PORTB, 3 ,0x1);
      PORTB = SetBit(PORTB, 4 ,0x1);
      PORTB = SetBit(PORTB, 5 ,0x1);
      outNum(digits[3]) ; break;
    default:  
      break;

  }
  return tasks[1].state;
}

// RED LED
int Tick_display3(int state)
{  
  switch(tasks[2].state)
  {
    case OFF: 
      if ( !red_off_count  ) {
        tasks[2].state = ON ; // switch to OFF state
        // recalculate counters
        red_on_count = (PWM_COUNT * red_duty) /100 ;
        red_off_count = PWM_COUNT - red_on_count ;
      }
      else 
        red_off_count-- ;
      break ;
    case ON:
      if ( !red_on_count ) {
        tasks[2].state = OFF ; // switch to OFF state
        // recalculate counters
        red_on_count = (PWM_COUNT * red_duty) /100 ;
        red_off_count = PWM_COUNT - red_on_count ;
      }
      else 
        red_on_count-- ;
      break;
    default:
      break ;
  }

switch(tasks[2].state)
  {
    case OFF:
        PORTC = SetBit(PORTC, RED_LED, 0);  // LED OFF
        break ;
    case ON:
    if (red_on_count)
        PORTC = SetBit(PORTC, RED_LED, 1);  // LED ON
      break;
    default:
      break ;
  }
  return tasks[2].state;
}
//GREE LED PWM
int Tick_display4(int state)
{ 
  switch(tasks[3].state)
  {
    case OFF:
      if ( !green_off_count  ) {
        tasks[3].state = ON ; // switch to OFF state
        // recalculate counters
        green_on_count = (PWM_COUNT * green_duty) /100 ;
        green_off_count = PWM_COUNT - green_on_count ;
      }
      else
        green_off_count-- ;
      break ;
    case ON:
      if ( !green_on_count ) {
        tasks[3].state = OFF ; // switch to OFF state
        // recalculate counters
        green_on_count = (PWM_COUNT * green_duty) /100 ;
        green_off_count = PWM_COUNT - green_on_count ;
      }
      else
        green_on_count-- ;
      break;
    default:
      break ;
  }

switch(tasks[3].state)
  {
    case OFF:
        PORTC = SetBit(PORTC, GREEN_LED, 0);  // LED OFF
        break ;
    case ON:
      if (green_on_count)
        PORTC = SetBit(PORTC, GREEN_LED, 1);  // LED ON
      break;
    default:
      break ;
  }
  return tasks[3].state;
}

enum joystick_states{INIT, IDLE, UP, DOWN, REMAIN_UP, REMAIN_DOWN, RESET, REMAIN_RESET} joystick_state;
int Tick_display5(int state) {

  /*state actions*/
  switch(tasks[4].state) {
    case INIT:
      tasks[4].state = IDLE;
      break;
    case IDLE:
    // A5 A4 A3 A2 A1 A0
      if((PINC >> 1 & 0x01) == 0) tasks[4].state = RESET; 
      else if(ADC_read(0) > 950) tasks[4].state = UP;
      else if(ADC_read(0) < 100) tasks[4].state = DOWN;
      break;
    case UP:
      if(ADC_read(0) <= 950) tasks[4].state = IDLE; 
      else tasks[4].state = REMAIN_UP;
      break;
    case DOWN:
      if(ADC_read(0) >= 100) tasks[4].state = IDLE;
      else tasks[4].state = REMAIN_DOWN; 
      break;
    case REMAIN_UP:
      if(ADC_read(0) <= 950) tasks[4].state = IDLE;
      break;
    case REMAIN_DOWN:
      if(ADC_read(0) >= 100) tasks[4].state = IDLE;
      break;
    case REMAIN_RESET:
      if((PINC >> 1 & 0x01) == 0) tasks[4].state = IDLE; 
      break ;
    case RESET:
      tasks[4].state = IDLE; break;
    default: break;    
  }
  /*state actions*/
  switch(tasks[4].state) {
    case INIT:
      break;
    case IDLE:
      break;
    case UP:
    switch(threshold) {
      case CLOSE:
        threshold = MODERATE;
        break ;
      case MODERATE:
        threshold = FAR;
        break;
      case FAR:
      default: 
          break;
    }
      break;
    case DOWN:
      switch(threshold) {
        case FAR:
          threshold = MODERATE;
          break;
        case MODERATE:
          threshold = CLOSE;
          break;
        case CLOSE:
        default: 
            break;
      }
      break;
    case REMAIN_UP:
      break;
    case REMAIN_DOWN:
      break; //no action needed
    case REMAIN_RESET:
      break ;
    case RESET:
      if(inches)
        inches = 0;
      else 
        inches = 1;
      break;  
    default: break;    
  }

  return tasks[4].state;
}

int main(void) {
    //TODO: initialize all your inputs and ouputs
    DDRD = 0xFF ; PORTD = 0x00;
    DDRC = 0xFC ; PORTC = 0x03 ;
    DDRB = 0xFE ; PORTB = 0x01 ;

    ADC_init();   // initializes ADC
    init_sonar(); // initializes sonar
    threshold = MODERATE;
    //TODO: Initialize tasks here
    tasks[0].period = TASK1_PERIOD ;
    tasks[0].state = START ; //initial state
    tasks[0].elapsedTime = 0;
    tasks[0].TickFct = &Tick_display1;

    tasks[1].period = TASK2_PERIOD ;
    tasks[1].state = START ;// initial state 
    tasks[1].elapsedTime = 0;
    tasks[1].TickFct = &Tick_display2;

    tasks[2].period = TASK3_PERIOD ;
    tasks[2].state = OFF ;// initial state 
    tasks[2].elapsedTime = 0;
    tasks[2].TickFct = &Tick_display3;

    tasks[3].period = TASK4_PERIOD ;
    tasks[3].state = OFF ;// initial state 
    tasks[3].elapsedTime = 0;
    tasks[3].TickFct = &Tick_display4;

    tasks[4].period = TASK5_PERIOD ;
    tasks[4].state = INIT ;// initial state 
    tasks[4].elapsedTime = 0;
    tasks[4].TickFct = &Tick_display5;

    TimerSet(GCD_PERIOD);
    TimerOn();
    while (1) {}
    return 0;
}