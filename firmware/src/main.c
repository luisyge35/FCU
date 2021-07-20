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

#define TICKMILISECOND      (CORE_TIMER_FREQUENCY/1000)*50 
#define BAMOCAR_RX_ADDR     (0x201)
#define TORQUE_REGID        (0x90)
#define SPEED_REGID         (0x31)
#define ACC_1               (ADC_INPUT_POSITIVE_AN23)
#define ACC_2               (ADC_INPUT_POSITIVE_AN27)
#define ACC1_BASE_MEASURE   (395)
#define ACC1_MAX_MEASURE    (640)
#define ACC_THRESHOLD       (10)
#define BAMOCAR_MAX_REF     (10000)

// Private functions
static void SetState(uint8_t newState);
void CAN_1_Handler( void );
uint32_t ADCGet(uint8_t channel);

uint16_t acceleratorValue;
volatile uint16_t acceleratorValue2;
uint16_t bamocarRef;
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
    uint16_t result;
    
    ADC_InputSelect(ADC_MUX_A, channel, ADC_INPUT_NEGATIVE_VREFL);
    ADC_SamplingStart();
    CORETIMER_DelayMs(10);
    ADC_ConversionStart();
    while(!ADC_ResultIsReady());
    result = ADC_ResultGet(ADC_RESULT_BUFFER_0) - ACC1_BASE_MEASURE;
    if(result < ACC1_MAX_MEASURE){
        return result;
    } else {
        return 0;
    }
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
                    // Parse acc value
                    bamocarRef = acceleratorValue * BAMOCAR_MAX_REF / (ACC1_MAX_MEASURE - ACC1_BASE_MEASURE);
                    
                    //data[0] = (uint8_t)((acceleratorValue & 0xFF00) >> 8);
                    //data[1] = (uint8_t)(acceleratorValue & 0x00FF);
                    
                    data[0] = (uint8_t)(bamocarRef & 0x00FF);
                    data[1] = (uint8_t)((bamocarRef & 0xFF00) >> 8);
                    
                    
                    if(bamocarRef > ACC_THRESHOLD){
                        CANSendBuffer(BAMOCAR_RX_ADDR, 5, SPEED_REGID, data);
                    } else {
                        data[0] = 0;
                        data[1] = 0;
                        CANSendBuffer(BAMOCAR_RX_ADDR, 5, SPEED_REGID, data);
                    }
                    
                    LED_Toggle();

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

