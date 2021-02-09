// ***** 1. Pre-processor Directives Section *****

#include "TExaS.h"
#include "tm4c123gh6pm.h"
//#include "PLL.h" //The Texas init takes care of these. By configuring and activating PLL
//you can run it at 80MHz: Add files PLL.c and PLL.h and call PLL_Init() in main then SysT =80MHz.
//#include "SysTick.h"

#define LIGHT                   (*((volatile unsigned long *)0x400050FC))
#define SENSOR                  (*((volatile unsigned long *)0x400243FC))
#define PLIGHT       (*((volatile unsigned long *)0x40025028))

//#define PLIGHT       (*((volatile unsigned long *)0x400253FC)) //whole reg
//#define PLIGHT       (*((volatile unsigned long *)0x40025010))
//#define PLIGHT       (*((volatile unsigned long *)0x40025012))    //gave 100002 after a write

//the values for each reg address just specify 'where' it is, don't
//get mixed up thinking an address ending in FC necessarily has dif 0s/1s
//stored there compared to one ending in 20 or 00.
//These addresses tell you "where to start" if you want to write to a reg
//and can be used to read-in specific bits - see 6.3

//should work without these as theyre in tmc.h?
#define GPIO_PORTB_DIR_R        (*((volatile unsigned long *)0x40005400))
#define GPIO_PORTB_AFSEL_R      (*((volatile unsigned long *)0x40005420))
#define GPIO_PORTB_DEN_R        (*((volatile unsigned long *)0x4000551C))
#define GPIO_PORTB_AMSEL_R      (*((volatile unsigned long *)0x40005528))
#define GPIO_PORTB_PCTL_R       (*((volatile unsigned long *)0x4000552C))


#define GPIO_PORTE_DIR_R        (*((volatile unsigned long *)0x40024400))
#define GPIO_PORTE_AFSEL_R      (*((volatile unsigned long *)0x40024420))
#define GPIO_PORTE_DEN_R        (*((volatile unsigned long *)0x4002451C))
#define GPIO_PORTE_AMSEL_R      (*((volatile unsigned long *)0x40024528))
#define GPIO_PORTE_PCTL_R       (*((volatile unsigned long *)0x4002452C))
#define SYSCTL_RCGC2_R          (*((volatile unsigned long *)0x400FE108))
#define SYSCTL_RCGC2_GPIOE      0x00000010  // port E Clock Gating Control
#define SYSCTL_RCGC2_GPIOB      0x00000002  // port B Clock Gating Control


// ***** 2. Global Declarations Section *****

// FUNCTION PROTOTYPES: Each subroutine defined
void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
void PortBEF_Init(void); //Not taken care of by Texas_init - make PF1/3 outputs
void PortF_Init(void);
void SysTick_Init(void);
void SysTick_Wait10ms(unsigned long delay);
void SysTick_Wait(unsigned long delay);
//void PortE_Init(void);
//void PortB_Init(void);
// ***** 3. Subroutines Section *****

// Linked data structure
struct State {
  unsigned long Out;  // 6-bit pattern to output port B
  unsigned long Out_p;  // 4-bit pattern to output port F
  unsigned long Time; // delay in 10ms units
  unsigned long Next[8];}; // next state for input patterns 0,1,2,3 etc
typedef const struct State STyp;
#define gow   0
#define waitw 1
#define gos   2
#define waits 3
#define gop 4
#define fla 5
#define gop2 6
#define fla2 7
#define gop3 8 //solid red?
//this is an array of the states specifying what light patterns to output
//how long to wait in that state and what to do based on the currentl

//lights are set based on FSM[S].output//if S = gow then it resolves to the 0th entry in this array of structs which says what lights to put on
//now, how long and what to do next if input is 000 Next is element 0 //(goW again), if input was 010 it should go to waitW so element 2 in it's Next[] array should be waitW and from there it goes to goS
    STyp FSM[9]={
  {0x0C,0x02,50,{gow,gow,waitw,waitw,waitw,waitw,waitw,waitw}},
//waitW
{0x14,0x02, 25,{gow,gow,gos,gos,gop,gop,gop,gop}},
{0x21,0x02,50,{gos,waits,gos,waits,waits,waits,waits,waits}},
//waitS
{0x22,0x02, 25,{gow,gow,gow,gow,gop,gow,gop,gow}},
//gop
{0x24,0x08,50,{gop,fla,fla,fla,gop,fla,fla,fla}},
//fla
{0x24, 0x0A,5,{gop2,gop2,gop2,gop2,gop2,gop2,gop2,gop2}},
//gop2
{0x24,0x08,25,{fla2,fla2,fla2,fla2,fla2,fla2,fla2,fla2}},
//fla2
{0x24,0x0A, 5,{gop3,gop3,gop3,gop3,gop3,gop3,gop3,gop3}},
//gop3, unsure if this state is necessary/correct
{0x24,0x02, 50,{gos,gow,gos,gos,gos,gow,gos,gos}}
/*
0x21 = 100001
OX0A = 1010
*/

};
unsigned long S;  // index to the current state
unsigned long Input;

int main(void){
  volatile unsigned long delay;
  TExaS_Init(SW_PIN_PE210, LED_PIN_PB543210,ScopeOff); // activate grader and set system clock to 80 MHz
    PortBEF_Init();
        PortF_Init();
    SysTick_Init();

  EnableInterrupts();
  S = gow;
  while(1){
    LIGHT = FSM[S].Out; // set lights
    PLIGHT = FSM[S].Out_p;//FSM[S].Out_p;
        SysTick_Wait10ms(FSM[S].Time);// wait for apprpriate time for current state
    Input = SENSOR; // read sensors to dictate next state
    S = FSM[S].Next[Input];

  }
}

void SysTick_Init(void){
  NVIC_ST_CTRL_R = 0;               // disable SysTick during setup
  NVIC_ST_CTRL_R = 0x00000005;      // enable SysTick with core clock
}

// The delay parameter is in units of the 80 MHz core clock. (12.5 ns)
void SysTick_Wait(unsigned long delay){
  NVIC_ST_RELOAD_R = delay-1;  // number of counts to wait
  NVIC_ST_CURRENT_R = 0;       // any value written to CURRENT clears
  while((NVIC_ST_CTRL_R&0x00010000)==0){ // wait for count flag
  }
}
// 800000*12.5ns equals 10ms
void SysTick_Wait10ms(unsigned long delay){
  unsigned long i;
  for(i=0; i<delay; i++){
    SysTick_Wait(800000);  // wait 10ms
  }
}

void PortF_Init(void){ volatile unsigned long delay;




  SYSCTL_RCGC2_R |= 0x00000020;     // 1) activate clock for Port F
  delay = SYSCTL_RCGC2_R;           // allow time for clock to start
  GPIO_PORTF_LOCK_R = 0x4C4F434B;   // 2) unlock GPIO Port F
  GPIO_PORTF_CR_R = 0x1F;           // allow changes to PF4-0
  // only PF0 needs to be unlocked, other bits can't be locked
  GPIO_PORTF_AMSEL_R = 0x00;        // 3) disable analog on PF
  GPIO_PORTF_PCTL_R = 0x00000000;   // 4) PCTL GPIO on PF4-0??
  GPIO_PORTF_DIR_R |= 0x0A;          // 4.2) PF3,1 output
  GPIO_PORTF_AFSEL_R = 0x00;        // 6) disable alt funct on PF7-0
  GPIO_PORTF_PUR_R = 0x11;          // enable pull-up on PF0 and PF4
  GPIO_PORTF_DEN_R = 0x1F;          // 7) enable digital I/O on PF4-0

}

void PortBEF_Init(void){ volatile unsigned long delay;
  //SYSCTL_RCGC2_R |= 0x00000020;      // 1) F clock
  //delay = SYSCTL_RCGC2_R;            // delay to allow clock to stabilize
  SYSCTL_RCGC2_R |= 0x32; // 1) B E+F
    delay = SYSCTL_RCGC2_R; // 2) no need to unlock

GPIO_PORTE_AMSEL_R &= ~0x03; // 3) disable analog function on PE1-0
GPIO_PORTE_PCTL_R &= ~0x000000FF; // 4) enable regular GPIO
GPIO_PORTE_DIR_R &= ~0x07; // 5) inputs on PE2-0
GPIO_PORTE_AFSEL_R &= ~0x03; // 6) regular function on PE1-0
GPIO_PORTE_DEN_R |= 0x07; // 7) enable digital on PE2-0

GPIO_PORTB_AMSEL_R &= ~0x3F; // 3) disable analog function on PB5-0
GPIO_PORTB_PCTL_R &= ~0x000000FF; // 4) enable regular GPIO
GPIO_PORTB_DIR_R |= 0x3F; // 5) outputs on PB5-0
GPIO_PORTB_AFSEL_R &= ~0x3F; // 6) regular function on PB1-0
GPIO_PORTB_DEN_R |= 0x3F; // 7) enable digital on PB5-0


/* //Code that led to strange behaviour
GPIO_PORTF_AMSEL_R &= 0x00;        // 2) disable analog function
  GPIO_PORTF_PCTL_R &= 0x00000000;   // 3) GPIO clear bit PCTL
  //GPIO_PORTF_DIR_R &= ~0x10;         // 4.1) PF4 input,

    //GPIO_PORTB_DIR_R |= 0x3F; // 5) outputs on PB5-0
    GPIO_PORTF_DIR_R |= 0x0A;          // 4.2) PF3,1 output
    //GPIO_PORTF_DIR_R = 0x0A;          // 4.2) PF3,1 output
  GPIO_PORTF_AFSEL_R &= 0x00;        // 5) no alternate function
  //GPIO_PORTF_PUR_R |= 0x10;          // 6) enable pullup resistor on PF4
  //you don't need all of them enabled just 1,3
    GPIO_PORTF_DEN_R |= 0x0A;          // 7) enable digital pins PF1,3
  //GPIO_PORTF_DEN_R |= 0x1E;          // 7) enable digital pins PF4-PF1
   //GPIO_PORTF_DATA_R = 0x00;// = (GPIO_PORTF_DATA_R|0x8);
*/

}
