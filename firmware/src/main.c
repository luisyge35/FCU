#include <stddef.h>                     // Defines NULL
#include <stdbool.h>                    // Defines true
#include <stdlib.h>                     // Defines EXIT_FAILURE
#include "definitions.h"                // SYS function prototypes
#include <sys/kmem.h>

#define ADC_PIN_MAP(pin)

typedef enum{
    IDLE = 0,
    RUNNING = 1,
    ERROR = 2
} SMState;

#define TICKMILISECOND  (CORE_TIMER_FREQUENCY/1000)*50 
#define BAMOCAR_RX_ADDR (0x182)
#define TORQUE_REGID    (0x30)
#define ACC_1           (ADC_INPUT_POSITIVE_AN23)
#define ACC_2           (ADC_INPUT_POSITIVE_AN27)

// Private functions
static void SetState(uint8_t newState);
void CAN_1_Handler( void );
uint32_t ADCGet(uint8_t channel);

volatile uint16_t acceleratorValue;
volatile uint16_t acceleratorValue2;
uint8_t data[8] = {0};
uint32_t tick = 0;
uint32_t tickGet = 0;
volatile uint8_t state = IDLE;

void __ISR(_CAN_1_VECTOR, ipl1SOFT) CAN_1_Handler (void)
{
    SetState(RUNNING);
    CAN1_InterruptHandler();
}

void SetState(uint8_t newState){
    state = newState;
}
    
uint32_t ADCGet(uint8_t channel){
    
    ADC_InputSelect(ADC_MUX_A, channel, ADC_INPUT_NEGATIVE_VREFL);
    ADC_SamplingStart();
    CORETIMER_DelayMs(10);
    ADC_ConversionStart();
    while(!ADC_ResultIsReady());
    return ADC_ResultGet(ADC_RESULT_BUFFER_0);
    /*
    ADC_InputSelect(ADC_MUX_A, ADC_INPUT_POSITIVE_AN27, ADC_INPUT_NEGATIVE_VREFL);
    ADC_SamplingStart();
    CORETIMER_DelayMs(50);
    ADC_ConversionStart();
    while(!ADC_ResultIsReady());
    acceleratorValue2 = ADC_ResultGet(ADC_RESULT_BUFFER_0);*/
}

int main ( void )
{
    SYS_Initialize ( NULL );

    while ( true )
    {
        switch(state){
            case IDLE:
                break;
            case RUNNING:
                if (CORETIMER_CounterGet() - tick > TICKMILISECOND){
                    tick = CORETIMER_CounterGet();
                    acceleratorValue = (uint16_t)ADCGet(ACC_1);
                    acceleratorValue2 = (uint16_t)ADCGet(ACC_2);
                    LED_Toggle();
                    
                    data[0] = (uint8_t)((acceleratorValue & 0xFF00) >> 8);
                    data[1] = (uint8_t)(acceleratorValue & 0x00FF);
                    data[2] = (uint8_t)((acceleratorValue2 & 0xFF00) >> 8);
                    data[3] = (uint8_t)(acceleratorValue2 & 0x00FF);
                    CANSendBuffer(BAMOCAR_RX_ADDR, 5, TORQUE_REGID, data);
                }
                break;
            default: 
                break;
        }

        /*
        CORETIMER_DelayMs(100);
        ADCGet(ACC_1);
        
        data[0] = (uint8_t)((acceleratorValue & 0xFF00) >> 8);
        data[1] = (uint8_t)(acceleratorValue & 0x00FF);
        data[2] = (uint8_t)((acceleratorValue2 & 0xFF00) >> 8);
        data[3] = (uint8_t)(acceleratorValue2 & 0x00FF);
        CANSendBuffer(BAMOCAR_RX_ADDR, 5, TORQUE_REGID, data);
        
        LED_Toggle();*/
        
        /*if(C1FIFOINT1bits.RXNEMPTYIF == 1){
            //CANSendBuffer(BAMOCAR_RX_ADDR, 3, TORQUE_REGID, data);
            LED_Toggle();
            C1FIFOCON1SET = 0x2000;
        }*/
        
        SYS_Tasks ( );
    }

    /* Execution should not come here during normal operation */

    return ( EXIT_FAILURE );
}


/*******************************************************************************
 End of File
*/

