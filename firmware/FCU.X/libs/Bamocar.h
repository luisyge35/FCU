#ifndef  BAMOCAR_H
#define  BAMOCAR_H

#include <stdint.h>

#define BAMOCAR_RECEPTION_ADDR    (0x181)
#define BAMOCAR_TRANSMISION_ADDR  (0x201)

void BamocarStart();
void BamocarSendTorqueSignal();
void BamocarStop();

#endif
