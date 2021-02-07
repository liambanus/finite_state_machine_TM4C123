// ***** 1. Pre-processor Directives Section *****

#include "TExaS.h"
#include "tm4c123gh6pm.h"
//#include "PLL.h" //The Texas init takes care of these. By configuring and activating PLL
//you can run it at 80MHz: Add files PLL.c and PLL.h and call PLL_Init() in main then SysT =80MHz.
//#include "SysTick.h"

#define LIGHT                   (*((volatile unsigned long *)0x400050FC))
//should work without these as theyre in tmc.h
#define GPIO_PORTB_OUT          (*((volatile unsigned long *)0x400050FC)) // bits 5-0
#define GPIO_PORTB_DIR_R        (*((volatile unsigned long *)0x40005400))
#define GPIO_PORTB_AFSEL_R      (*((volatile unsigned long *)0x40005420))
#define GPIO_PORTB_DEN_R        (*((volatile unsigned long *)0x4000551C))
#define GPIO_PORTB_AMSEL_R      (*((volatile unsigned long *)0x40005528))
#define GPIO_PORTB_PCTL_R       (*((volatile unsigned long *)0x4000552C))
#define GPIO_PORTE_IN           (*((volatile unsigned long *)0x4002400C)) // bits 1-0
#define SENSOR                  (*((volatile unsigned long *)0x4002400C))

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
//void PortE_Init(void);
//void PortB_Init(void);
// ***** 3. Subroutines Section *****

int main(void){
  TExaS_Init(SW_PIN_PE210, LED_PIN_PB543210,ScopeOff); // activate grader and set system clock to 80 MHz
    PortBEF_Init();

  EnableInterrupts();
  while(1){

  }
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



GPIO_PORTF_AMSEL_R &= 0x00;        // 2) disable analog function
  GPIO_PORTF_PCTL_R &= 0x00000000;   // 3) GPIO clear bit PCTL
  //GPIO_PORTF_DIR_R &= ~0x10;         // 4.1) PF4 input,
  GPIO_PORTF_DIR_R |= 0x0A;          // 4.2) PF3,1 output
  GPIO_PORTF_AFSEL_R &= 0x00;        // 5) no alternate function
  GPIO_PORTF_PUR_R |= 0x10;          // 6) enable pullup resistor on PF4
  //you don't need all of them enabled just 1,3
    GPIO_PORTF_DEN_R |= 0x0A;          // 7) enable digital pins PF1,3
  //GPIO_PORTF_DEN_R |= 0x1E;          // 7) enable digital pins PF4-PF1
    //GPIO_PORTF_DATA_R = (GPIO_PORTF_DATA_R|0x8);


}
