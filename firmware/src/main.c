#include <stddef.h>                     // Defines NULL
#include <stdbool.h>                    // Defines true
#include <stdlib.h>                     // Defines EXIT_FAILURE
#include "definitions.h"                // SYS function prototypes
#include <sys/kmem.h>

#define ADC_PIN_MAP(pin)

// *****************************************************************************
// *****************************************************************************
// Section: Main Entry Point
// *****************************************************************************
// *****************************************************************************
uint8_t message[8];
unsigned int canFifoMessageBuffer[40];
uint16_t acceleratorValue;
float Acc1;
uint16_t acceleratorValue2;

// Direccioens
#define BAMOCAR_RX_ADDR (0x182)
#define TORQUE_REGID    (0x30)

void ADCGet(){
    AD1CHS = 0x170000;
    ADC_InputSelect(ADC_MUX_A, ADC_INPUT_POSITIVE_AN23, ADC_INPUT_NEGATIVE_VREFL);
    ADC_SamplingStart();
    CORETIMER_DelayMs(50);
    ADC_ConversionStart();
    while(!ADC_ResultIsReady());
    acceleratorValue = ADC_ResultGet(ADC_RESULT_BUFFER_0);
    
    ADC_InputSelect(ADC_MUX_A, ADC_INPUT_POSITIVE_AN27, ADC_INPUT_NEGATIVE_VREFL);
    ADC_SamplingStart();
    CORETIMER_DelayMs(50);
    ADC_ConversionStart();
    while(!ADC_ResultIsReady());
    acceleratorValue2 = ADC_ResultGet(ADC_RESULT_BUFFER_0);    
}

int main ( void )
{
    uint8_t data[7] = {0};
    /* Initialize all modules */
    SYS_Initialize ( NULL );

    while ( true )
    {
        //CORETIMER_DelayMs(100);
        //ADCGet();
        
        //data[0] = (uint8_t)((acceleratorValue & 0xFF00) >> 8);
        //data[1] = (uint8_t)(acceleratorValue & 0x00FF);
        //CANSendBuffer(BAMOCAR_RX_ADDR, 3, TORQUE_REGID, data);
        
        //LED_Toggle();
        
        if(C1FIFOINT1bits.RXNEMPTYIF == 1){
            CANSendBuffer(BAMOCAR_RX_ADDR, 3, TORQUE_REGID, data);
            LED_Toggle();
        }

        SYS_Tasks ( );
    }

    /* Execution should not come here during normal operation */

    return ( EXIT_FAILURE );
}


/*******************************************************************************
 End of File
*/

