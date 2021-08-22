#include "Bamocar.h"
#include "peripheral/can/plib_can1.h"


#define TORQUE_REGID (0x31)


void BamocarStart(){
    
}

void BamocarSendTorqueSignal(uint16_t torque){
    uint8_t buffer[2];
    
    buffer[0] = (uint8_t)(torque & 0xFF);
    buffer[1] = (uint8_t)((torque & 0xFF00) >> 8);
    
    //CANSendBuffer(BAMOCAR_TRANSMISION_ADDR, 3, TORQUE_REGID, buffer);
}

void BamocarStop(){

}


