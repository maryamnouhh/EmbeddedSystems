#include "stdint.h"
#include "stdbool.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/interrupt.h"
#include "inc/hw_memmap.h"
#include "inc/hw_gpio.h"
#include "inc/hw_ints.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "stdio.h"
#include "stdlib.h"
#include "tm4c123gh6pm.h"
#include "driverlib/timer.h"
#include "string.h"
#include "time.h"
#include "driverlib/sysctl.h"
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "tm4c123gh6pm.h"

//Interrupt Handler Declarations
void GPIOFHandler(void); //Handler for Port F Interrupts
void Timer0AHandler(void); // Handler for Timer 0A Interrupts
void Timer1AHandler(void); // Handler for Timer 1A Interrupts
void Timer2AHandler(void); // Handler for Timer 2A Interrupts

//Global Variables for FSM
static int t=0; // Indicates which timer is counting
static int red = 0; // For State Traversal
static int green = 1; // For State Traversal
static int yellow = 0; // For State Traversal
static int paused = 0; // Indicates if a timer is counting or paused

int main(){
  // Port F Initialization
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF); // To enable clock to Port F
  while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF));// Wait until clock is enabled for Port F
  GPIO_PORTF_LOCK_R = 0x4C4F434B;// To unclock Port F
  GPIO_PORTF_CR_R = 0x11;// To enable changes to pins 0 & 4
  GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_0);// Set Pin 0 as input
  GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_4);// Set Pin 4 as input
  GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_0, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU); // Set Pin 0 to pull up
  GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_4, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);// Set Pin 4 to pull up

  
  // Port A Initialization
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);// To enable clock to Port A
  while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOA));// Wait until clock is enabled for Port A
  GPIO_PORTA_LOCK_R = 0x4C4F434B;// To unclock Port A
  GPIOPinTypeGPIOOutput(GPIO_PORTA_BASE, GPIO_PIN_2);// Set Pin 2 as output Red EW
  GPIOPinTypeGPIOOutput(GPIO_PORTA_BASE, GPIO_PIN_3);// Set Pin 3 as output Yellow EW
  GPIOPinTypeGPIOOutput(GPIO_PORTA_BASE, GPIO_PIN_4);// Set Pin 4 as output Green EW
  GPIOPinTypeGPIOOutput(GPIO_PORTA_BASE, GPIO_PIN_5);// Set Pin 5 as output Red pd EW
  GPIOPinTypeGPIOOutput(GPIO_PORTA_BASE, GPIO_PIN_6);// Set Pin 6 as output Green pd EW

  
  //Port B Initialization
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);// To enable clock to Port B
  while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOB));// Wait until clock is enabled for Port B
  GPIO_PORTB_LOCK_R = 0x4C4F434B; // To unclock Port B
  GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, GPIO_PIN_0);// Set Pin 0 as output Red NS
  GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, GPIO_PIN_1);// Set Pin 1 as output Yellow NS
  GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, GPIO_PIN_2);// Set Pin 2 as output Green NS
  GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, GPIO_PIN_3);// Set Pin 3 as output Red pd NS
  GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, GPIO_PIN_4);// Set Pin 4 as output Green pd NS
  
  
  // Timer 0 A Initialization
  SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);// To enable clock to Timer 0
  while (!SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER0));// Wait until clock is enabled for Timer 0
  TimerDisable(TIMER0_BASE,TIMER_A); // Disable Timer 0 A
  TimerConfigure(TIMER0_BASE, TIMER_CFG_A_ONE_SHOT); // Make Timer 0 A oneshot
  TimerClockSourceSet(TIMER0_BASE, TIMER_CLOCK_SYSTEM); // Make Timer 0 use the system clock to count
  TimerLoadSet(TIMER0_BASE, TIMER_A,15999999); // Add a load value of 1 second to Timer 0 A
  TIMER0_CTL_R |=0x2; // Make Timer 0 A Stall for debugging
  TimerIntRegister(TIMER0_BASE, TIMER_A, Timer0AHandler); // Set the Interrupt function for Timer 0 A
  TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT); // Enable the Interrupt on Timer 0 A Timeout
  
  // Timer 1 A Initialization
  SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1); // To enable clock to Timer 1
  while (!SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER1));// Wait until clock is enabled for Timer 1
  TimerDisable(TIMER1_BASE,TIMER_A); // Disable Timer 1 A
  TimerConfigure(TIMER1_BASE, TIMER_CFG_A_ONE_SHOT ); // Make Timer 1 A oneshot
  TimerClockSourceSet(TIMER1_BASE, TIMER_CLOCK_SYSTEM); // Make Timer 1 use the system clock to count
  TimerLoadSet(TIMER1_BASE, TIMER_A,15999999); // Add a load value of 1 second to Timer 1 A
  TIMER1_CTL_R |=0x2; // Make Timer 1 A Stall for debugging
  TimerIntRegister(TIMER1_BASE, TIMER_A, Timer1AHandler); // Set the Interrupt function for Timer 1 A
  TimerIntEnable(TIMER1_BASE, TIMER_TIMA_TIMEOUT);// Enable the Interrupt on Timer 1 A Timeout
  
  
  // Timer 2 A Initialization
  SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER2);// To enable clock to Timer 2
  while (!SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER2));// Wait until clock is enabled for Timer 2
  TimerDisable(TIMER2_BASE,TIMER_A);// Disable Timer 2 A
  TimerConfigure(TIMER2_BASE, TIMER_CFG_A_ONE_SHOT );// Make Timer 2 A oneshot
  TimerClockSourceSet(TIMER2_BASE, TIMER_CLOCK_SYSTEM);// Make Timer 2 use the system clock to count
  TimerLoadSet(TIMER2_BASE, TIMER_A,15999999);// Add a load value of 1 second to Timer 2 A
  TIMER2_CTL_R |=0x2;// Make Timer 2 A Stall for debugging
  TimerIntRegister(TIMER2_BASE, TIMER_A, Timer2AHandler);// Set the Interrupt function for Timer 2 A
  TimerIntEnable(TIMER2_BASE, TIMER_TIMA_TIMEOUT);// Enable the Interrupt on Timer 2 A Timeout
  
  
  // Timer 3 A Initialization
  SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER3);// To enable clock to Timer 3
  while (!SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER3));// Wait until clock is enabled for Timer 3
  TimerDisable(TIMER3_BASE,TIMER_A);// Disable Timer 3 A
  TimerConfigure(TIMER3_BASE, TIMER_CFG_A_ONE_SHOT );// Make Timer 3 A oneshot
  TimerClockSourceSet(TIMER3_BASE, TIMER_CLOCK_SYSTEM);// Make Timer 3 use the system clock to count
  TimerLoadSet(TIMER3_BASE, TIMER_A,15999999);// Add a load value of 1 second to Timer 3 A
  TIMER3_CTL_R |=0x2;// Make Timer 3 A Stall for debugging
  
  //Set Interrupt Priority
  IntPrioritySet(INT_GPIOF, 0x10); //Highest Priority
  IntPrioritySet(INT_TIMER0A, 0x20);
  IntPrioritySet(INT_TIMER1A, 0x20);
  IntPrioritySet(INT_TIMER2A, 0x20);

  
  // Set Initial State
  GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_0,0); // Red NS OFF
  GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_1,0); // Yellow NS OFF
  GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_2,GPIO_PIN_2); // Green NS ON
  GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_3,GPIO_PIN_3); // Red pd NS ON 
  GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_4,0); // Green pd NS OFF
  
  GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_2,GPIO_PIN_2); // Red EW ON
  GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_3,0); // Yellow EW OFF
  GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_4,0); // Green EW OFF
  GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_5,0); // Red EW pd OFF
  GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_6,GPIO_PIN_6); // Green EW pd ON
  
  
  GPIOIntRegister(GPIO_PORTF_BASE, GPIOFHandler); // Set the interrupt handler for Port F
  GPIOIntEnable(GPIO_PORTF_BASE, GPIO_INT_PIN_4); // Enable Interrupt for Port F Pin 4
  GPIOIntEnable(GPIO_PORTF_BASE, GPIO_INT_PIN_0);// Enable Interrupt for Port F Pin 0
  TimerEnable(TIMER0_BASE, TIMER_A); // Start Timer 0
  
  while(1){
  
    __asm("    wfi\n"); // Allow for the processor to sleep until interrupt
  }
  
  return 0;
  }


//Timer 0A Handler
void Timer0AHandler(void){
  paused=1; // Set paused to 1 indicating that no timer is counting
  static int count=1; //counts the number of times the interrupt is called
  if(count%5==0){ // Makes sure the function works once every 5 times it is called (5 seconds)
    if (yellow==0){ //Checks state variable yellow
      
      GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_2,0); //Close Green NS
      GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_1, GPIO_PIN_1); // Open yellow NS
      yellow=1; // Sets yellow to 1 so the next time the function is called it goes to the other street
      
    }
    else{ //if yellow = 1
      
      GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_4,0); //Close Green EW
      GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_3, GPIO_PIN_3); // Open yellow EW
      yellow=0; // Sets yellow to 0 so the next time the function is called it goes to the other street
      
    }
    
    count++; //Increments count
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT); //Clears interrupt for Timer 0A
    t=1; //Indicates Timer 1A is counting
    TimerEnable(TIMER1_BASE, TIMER_A); //Enables Timer 1A
    paused=0; // Indicates that there is a timer counting
    
  }
  else{ // if count is not divisible by 5
    
    count++; //increments count
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT); // Clears interrupt for Timer 0A
    TimerEnable(TIMER0_BASE, TIMER_A); //Enables Timer 0A again
    paused=0;// Indicates that there is a timer counting
  }
  
}

//Timer 1A Handler
void Timer1AHandler(void){
  paused=1;// Set paused to 1 indicating that no timer is counting
  static int count=1;//counts the number of times the interrupt is called
  if(count%2==0){// Makes sure the function works once every 2 times it is called (2 seconds)
    
    if(red==0){//Checks state variable red
      GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_1,0); //Close Yellow NS
      GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_0, GPIO_PIN_0); // Open Red NS
      GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_3, 0); //Close Red Pd NS
      GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_4, GPIO_PIN_4); //Open Green Pd NS
      red=1;// Sets red to 1 so the next time the function is called it goes to the other street
      
    } 
    else{
      GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_3,0); //Close Yellow EW
      GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_2, GPIO_PIN_2); // Open Red EW
      GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_5, 0); //Close Red Pd EW
      GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_6, GPIO_PIN_6); //Open Green Pd EW
      red=0; // Sets red to 0 so the next time the function is called it goes to the other street
      
    }
    count++; //Increments count
    TimerIntClear(TIMER1_BASE, TIMER_TIMA_TIMEOUT); //Clear Interrupt for TImer 1A
    t=2; //Indicates Timer 2A is counting
    TimerEnable(TIMER2_BASE, TIMER_A); //Enables Timer 2A
    paused=0; // Indicates that there is a timer counting
  }
  else{
    count++; //Increments count
    TimerIntClear(TIMER1_BASE, TIMER_TIMA_TIMEOUT); //Clear Interrupt for TImer 1A
    TimerEnable(TIMER1_BASE, TIMER_A); //Enables Timer 1A again
    paused=0; // Indicates that there is a timer counting
  }
  
}
// Timer 2A Handler
void Timer2AHandler(void){
  paused=1; // Set paused to 1 indicating that no timer is counting
  if(green==0){ //Checks state variable green
    GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_0,0); //Close Red NS
    GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_2,GPIO_PIN_2);// Open Green NS
    GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_4, 0); //Close Green Pd NS
    GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_3, GPIO_PIN_3); //Open Red Pd NS
    
    green=1; // Sets green to 1 so the next time the function is called it goes to the other street
  }
  else{
    GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_2, 0); //Close Red EW
    GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_4, GPIO_PIN_4); // Open Green EW
    GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_6, 0); //Close Green Pd EW
    GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_5, GPIO_PIN_5); //Open Red Pd EW
    green=0; // Sets green to 0 so the next time the function is called it goes to the other street
  }
  TimerIntClear(TIMER2_BASE, TIMER_TIMA_TIMEOUT); // Clears Timer 2A interrupt
  t=0; // Indicates Timer 0A is counting
  TimerEnable(TIMER0_BASE, TIMER_A); // Enables Timer 0A
  paused=0; // Indicates that there is a timer counting
}

// Port F Handler
void GPIOFHandler(void){
  if(!paused){ // Checks to see if there is a timer counting
    if(t==0){ //Identifies which timer is counting
      TimerDisable(TIMER0_BASE,TIMER_A); //Pause Timer 0A
    } else if (t==1){
      TimerDisable(TIMER1_BASE,TIMER_A);//Pause Timer 1A
    } else if (t==2){
      TimerDisable(TIMER2_BASE,TIMER_A);//Pause Timer 2A
    }
    paused=1; // Set paused to 1 indicating that no timer is counting
  }
  
  //Bonus 1
  if((GPIOIntStatus(GPIO_PORTF_BASE,true)&0x11)!=0){ // Interrupt flags for both switches
    int32_t rns=GPIOPinRead(GPIO_PORTB_BASE,GPIO_PIN_0); //Get the Red NS value
    int32_t rew=GPIOPinRead(GPIO_PORTA_BASE,GPIO_PIN_2); // Get the Red EW value
    if(rew>rns){ //Check to see which one is On
      GPIOIntClear(GPIO_PORTF_BASE,GPIO_PIN_4); //Red EW is on so clear the interrupt for EW
    }
    else{
      GPIOIntClear(GPIO_PORTF_BASE,GPIO_PIN_0);//Red NS is on so clear the interrupt for NS
    }
  }
  
  
  // Pin 0 (Right Switch) is the switch for NS
  if((GPIOIntStatus(GPIO_PORTF_BASE,true)&0x01)!=0){
    // Save current State
    int32_t rns=GPIOPinRead(GPIO_PORTB_BASE,GPIO_PIN_0);  //Red NS
    int32_t yns=GPIOPinRead(GPIO_PORTB_BASE,GPIO_PIN_1); //Yellow NS
    int32_t gns=GPIOPinRead(GPIO_PORTB_BASE,GPIO_PIN_2); //Green NS
    int32_t rpns=GPIOPinRead(GPIO_PORTB_BASE,GPIO_PIN_3); //Red pd NS
    int32_t gpns=GPIOPinRead(GPIO_PORTB_BASE,GPIO_PIN_4); //Green pd NS
    
    // Turn pedestrian lights green and street lights red
    GPIOPinWrite(GPIO_PORTB_BASE,GPIO_PIN_1,0); //Yellow NS OFF
    GPIOPinWrite(GPIO_PORTB_BASE,GPIO_PIN_2,0); //Green NS OFF
    GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_3,0); //Red pd NS OFF
    GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_0,GPIO_PIN_0); //Red NS ON
    GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_4,GPIO_PIN_4); //Green pd NS ON
    
    //Delay for 2 seconds
    TimerEnable(TIMER3_BASE,TIMER_A); //Enable Timer 3A
    while((TIMER3_RIS_R & TIMER_TIMA_TIMEOUT) == 0){}; //Wait until Timer 3A Timeout
    TIMER3_ICR_R=0x1; //Clear timeout
    TimerEnable(TIMER3_BASE,TIMER_A); //Enable Timer 3A again
    while((TIMER3_RIS_R & TIMER_TIMA_TIMEOUT) == 0){}; //Wait until Timer 3A Timeout
    TIMER3_ICR_R=0x1; //Clear timeout
    
    //Return values of lights to its previous state
    GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_0,rns); //Red NS
    GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_1,yns); //Yellow NS
    GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_2,gns); //Green NS
    GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_3,rpns); //Red pd NS
    GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_4,gpns); //Green pd NS
    
    
  }
  // Pin 4 is the switch for EW
  else if((GPIOIntStatus(GPIO_PORTF_BASE,true)&0x10)!=0){
    // Save current State
    int32_t rew=GPIOPinRead(GPIO_PORTA_BASE,GPIO_PIN_2); //Red EW
    int32_t yew=GPIOPinRead(GPIO_PORTA_BASE,GPIO_PIN_3); //Yellow EW
    int32_t gew=GPIOPinRead(GPIO_PORTA_BASE,GPIO_PIN_4); //Green EW
    int32_t rpew=GPIOPinRead(GPIO_PORTA_BASE,GPIO_PIN_5); //Red pd EW
    int32_t gpew=GPIOPinRead(GPIO_PORTA_BASE,GPIO_PIN_6); //Green pd EW
    
    // Turn pedestrian lights green and street lights red
    GPIOPinWrite(GPIO_PORTA_BASE,GPIO_PIN_3,0); //Yellow EW OFF
    GPIOPinWrite(GPIO_PORTA_BASE,GPIO_PIN_4,0); //Green EW OFF
    GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_5,0); //Red pd EW OFF
    GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_2,GPIO_PIN_2); //Red EW ON
    GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_6,GPIO_PIN_6); //Green pd EW ON
    
    //Delay for 2 seconds
    TimerEnable(TIMER3_BASE,TIMER_A); //Enable Timer 3A
    while((TIMER3_RIS_R & TIMER_TIMA_TIMEOUT) == 0){}; //Wait until Timer 3A Timeout
    TIMER3_ICR_R=0x1; //Clear timeout
    TimerEnable(TIMER3_BASE,TIMER_A); //Enable Timer 3A again
    while((TIMER3_RIS_R & TIMER_TIMA_TIMEOUT) == 0){}; //Wait until Timer 3A Timeout
    TIMER3_ICR_R=0x1; //Clear timeout
    
    //Return values of lights to its previous state
    GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_2,rew); //Red EW
    GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_3,yew); //Yellow EW
    GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_4,gew); //Green EW
    GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_5,rpew); //Red pd EW
    GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_6,gpew); //Green pd EW
    
  }
  
  
  //Bonus 2
  GPIOIntClear(GPIO_PORTF_BASE, GPIO_PIN_0); //Clear Interrupt for Pin 0
  GPIOIntClear(GPIO_PORTF_BASE, GPIO_PIN_4); // Clear Interrupt for Pin 4
  
  
  
  if (paused==1){ // Checks to see if a timer paused
    if(t==0){ //Identifies which timer is paused
      TimerEnable(TIMER0_BASE,TIMER_A); //Enables Timer 0A
    }
    else if (t==1){
      TimerEnable(TIMER1_BASE,TIMER_A); //Enables Timer 1A
    }
    else if (t==2){
      TimerEnable(TIMER2_BASE,TIMER_A); //Enables Timer 2A
    }
  }
}

