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
/*
#define _C1CON_REQOP_POSITION                    0x00000018
#define _C1CON_REQOP_MASK                        0x07000000
#define _C1CON_REQOP_LENGTH                      0x00000003*/
/*
void CanInit(){
    // Request to switch to config mode
    C1CON = _C1CON_REQOP_MASK;
    while(C1CON != 0x100);
    
    C1FIFOBA = KVA_TO_PA(canFifoMessageBuffer);
    
}
*/
int main ( void )
{
    /* Initialize all modules */
    SYS_Initialize ( NULL );

    while ( true )
    {
        CORETIMER_DelayMs(1000);
        CANSend();
        //CAN1_MessageTransmit(0x181, 8, message, 0, CAN_MSG_TX_DATA_FRAME);
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

