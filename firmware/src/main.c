#include <stddef.h>                     // Defines NULL
#include <stdbool.h>                    // Defines true
#include <stdlib.h>                     // Defines EXIT_FAILURE
#include "definitions.h"                // SYS function prototypes
#include <sys/kmem.h>

#define ADC_PIN_MAP(pin)

#undef DEBUG

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
#define BRAKE               (ADC_INPUT_POSITIVE_AN29)

// ACC1
#define ACC1_BASE_MEASURE    (450)
#define ACC1_MAX_BYTES       (806)
#define ACC1_THRESHOLD       (550)
// ACC2
#define ACC2_THRESHOLD       (172)  
// BRAKE
#define BRAKE_THRESHOLD      (105)

#define ACC_MAX_MEASURE     (250)

#define BAMOCAR_MAX_REF     (32000)
#define START_ID            (0)
#define START_REQUEST       (0xFF)
#define STOP_REQUEST        (0xAA)

#define SENSOR_VALUES_CAN_ID (0x234)

#define APPS_THRESHOLD       (20)
#define DASH_MAX_MEASURE     (100)

// Private functions
static void SetState(uint8_t newState);
void CAN_1_Handler( void );
uint32_t ADCGet(uint8_t channel);

int16_t acceleratorValue;
int16_t acceleratorValue2;
int16_t brakeValue;
int32_t bamocarRef;
uint8_t data[8] = {0};
uint32_t tick = 0;
uint32_t tickGet = 0;
volatile uint8_t state = IDLE;
CAN_TX_RX_MSG_BUFFER *rxMessage = NULL;
uint32_t messageId;
bool sendRef;

// CAN Message Handlers
void HandleStartMessage(uint8_t *message){
    switch(message[0]){
        case START_REQUEST:
            SetState(RUNNING);
            sendRef = true;
            break;
        case STOP_REQUEST:
            sendRef = false;
            SetState(IDLE);
            data[0] = TORQUE_REGID;
            data[1] = 0;
            data[2] = 0;
            CANSendBuffer(BAMOCAR_RX_ADDR, 5, data);
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
    result = ADC_ResultGet(ADC_RESULT_BUFFER_0);
}

int main ( void )
{
    SYS_Initialize ( NULL );
    
    state = RUNNING;

    while ( true )
    {
        switch(state){
            case IDLE:
                break;
            case RUNNING:
                if (CORETIMER_CounterGet() - tick > TICKMILISECOND){
                    
                    tick = CORETIMER_CounterGet();
                    
                    // APPS
                    acceleratorValue = (uint16_t)ADCGet(ACC_1);
                    acceleratorValue -= ACC1_THRESHOLD;
                    if(acceleratorValue < 0) acceleratorValue = 0;
                    
                    acceleratorValue2 = (uint16_t)ADCGet(ACC_2);
                    acceleratorValue2 -= ACC2_THRESHOLD;
                    if(acceleratorValue2 < 0) acceleratorValue2 = 0;
                    
                    //if(acceleratorValue2 - acceleratorValue > APPS_THRESHOLD) acceleratorValue = 0;
                    
                    // BRAKE
                    brakeValue = (uint16_t)ADCGet(BRAKE);
                    brakeValue -= BRAKE_THRESHOLD;
                    if(brakeValue < 0) brakeValue = 0;
#ifndef DEBUG                
                    // Parse acc value
                    bamocarRef = acceleratorValue * BAMOCAR_MAX_REF / (ACC1_MAX_BYTES);
                    
                    // Get 2 complement
                    bamocarRef = ((~bamocarRef) + 1) & 0xFFFF;
                    
                    data[0] = TORQUE_REGID;
                    data[1] = (uint8_t)(bamocarRef & 0x00FF);
                    data[2] = (uint8_t)((bamocarRef & 0xFF00) >> 8);
                    
                    if(sendRef){
                        CANSendBuffer(BAMOCAR_RX_ADDR, 5, data);
                    }
                              
                    // Propagate sensor values
                    acceleratorValue = acceleratorValue * 100 / ACC_MAX_MEASURE;
                    data[0] = acceleratorValue;
                    data[1] = brakeValue;
                    CANSendBuffer(0x234, 2, data);
#endif

                    
#ifdef DEBUG
                    data[0] = (uint8_t)(acceleratorValue & 0x00FF);
                    data[1] = (uint8_t)((acceleratorValue & 0xFF00) >> 8);
                    data[2] = (uint8_t)(acceleratorValue2 & 0x00FF);
                    data[3] = (uint8_t)((acceleratorValue2 & 0xFF00) >> 8);
                    data[4] = (uint8_t)(brakeValue & 0x00FF);
                    data[5] = (uint8_t)((brakeValue & 0xFF00) >> 8);
                    
                    CANSendBuffer(0x234, 6, data);
#endif                 

                    
                    LED_Toggle();

                }
                break;
            default: 
                break;
        }
        
        SYS_Tasks ( );
    }

    /* Execution should not come here during normal operation */

    return ( EXIT_FAILURE );
}


/*******************************************************************************
 End of File
*/

