#include <stddef.h>                     // Defines NULL
#include <stdbool.h>                    // Defines true
#include <stdlib.h>                     // Defines EXIT_FAILURE
#include "definitions.h"                // SYS function prototypes
#include <sys/kmem.h>


// *****************************************************************************
// *****************************************************************************
// Section: Main Entry Point
// *****************************************************************************
// *****************************************************************************
uint8_t message[8];
unsigned int canFifoMessageBuffer[40];
uint16_t adc_count;
float Acc1;
uint16_t adc_count2;
float Brake;

void ADCGet(){
    AD1CHS = 0x170000;
    ADC_InputSelect(ADC_MUX_A, ADC_INPUT_POSITIVE_AN23, ADC_INPUT_NEGATIVE_VREFL);
    ADC_SamplingStart();
    CORETIMER_DelayMs(50);
    ADC_ConversionStart();
    while(!ADC_ResultIsReady());
    adc_count = ADC_ResultGet(ADC_RESULT_BUFFER_0);
    
    ADC_InputSelect(ADC_MUX_A, ADC_INPUT_POSITIVE_AN27, ADC_INPUT_NEGATIVE_VREFL);
    ADC_SamplingStart();
    CORETIMER_DelayMs(50);
    ADC_ConversionStart();
    while(!ADC_ResultIsReady());
    adc_count2 = ADC_ResultGet(ADC_RESULT_BUFFER_0);
    
    Brake = Brake + 1;
    
}
int main ( void )
{
    /* Initialize all modules */
    SYS_Initialize ( NULL );

    while ( true )
    {
        CORETIMER_DelayMs(100);
        ADCGet();
        CANSend(adc_count, adc_count2);
        LED_Toggle();
        /* Maintain state machines of all polled MPLAB Harmony modules. */
        SYS_Tasks ( );
    }

    /* Execution should not come here during normal operation */

    return ( EXIT_FAILURE );
}


/*******************************************************************************
 End of File
*/

