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
#define ACC_THRESHOLD       (50)
#define BAMOCAR_MAX_REF     (30000)
#define START_ID            (0)
#define START_REQUEST       (0xFF)
#define STOP_REQUEST        (0xAA)

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
CAN_TX_RX_MSG_BUFFER *rxMessage = NULL;
uint32_t messageId;

// CAN Message Handlers
void HandleStartMessage(uint8_t *message){
    switch(message[0]){
        case START_REQUEST:
            SetState(RUNNING);
            break;
        case STOP_REQUEST:
            SetState(IDLE);
            data[0] = 0;
            data[1] = 0;
            CANSendBuffer(BAMOCAR_RX_ADDR, 5, SPEED_REGID, data);
        default:
            break;
    }
}

void HandleCanMessage(){    
    rxMessage = (CAN_TX_RX_MSG_BUFFER *)PA_TO_KVA1(C1FIFOUA1);
    
    messageId = (rxMessage->msgSID & 0XFF);
    
    switch(messageId){
        case START_ID:
            HandleStartMessage(rxMessage->msgData);
            break;
        default:
            break;
    }
}

void __ISR(_CAN_1_VECTOR, ipl1SOFT) CAN_1_Handler (void)
{
    HandleCanMessage();
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
                    
                    if(acceleratorValue < ACC_THRESHOLD) acceleratorValue = 0;
                    
                    // Parse acc value
                    bamocarRef = acceleratorValue * BAMOCAR_MAX_REF / (ACC1_MAX_MEASURE - ACC1_BASE_MEASURE);
                    
                    // Get 2 complement
                    bamocarRef = ((~bamocarRef) + 1) & 0xFFFF;
                    
                    data[0] = (uint8_t)(bamocarRef & 0x00FF);
                    data[1] = (uint8_t)((bamocarRef & 0xFF00) >> 8);
                    
                    CANSendBuffer(BAMOCAR_RX_ADDR, 5, TORQUE_REGID, data);
                    
                    /*if(bamocarRef > ACC_THRESHOLD){
                        CANSendBuffer(BAMOCAR_RX_ADDR, 5, SPEED_REGID, data);
                    } else {
                        data[0] = 0;
                        data[1] = 0;
                        CANSendBuffer(BAMOCAR_RX_ADDR, 5, SPEED_REGID, data);
                    }*/
                    
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

