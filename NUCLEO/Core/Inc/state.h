/*
 * state.h
 *
 *  Created on: Oct 4, 2025
 *      Author: Admin
 */

#ifndef INC_STATE_H_
#define INC_STATE_H_

#include "type.h"

typedef enum{
    VEHICLE_MODE_INVALID = 0,
    VEHICLE_MODE_MOTION,
    VEHICLE_MODE_HALT,
    VEHICLE_MODE_SLEEP
} AIS140_VEHICLE_MODE;



#endif /* INC_STATE_H_ */
