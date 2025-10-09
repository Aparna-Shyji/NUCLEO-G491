/*
 * i2c.h
 *
 *  Created on: Oct 9, 2025
 *      Author: Admin
 */

#ifndef INC_I2C_H_
#define INC_I2C_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

#define I2C_SUCCESS     0
#define I2C_FAILURE    -1
#define MAX_I2C_WRITE_DATA_BYTES  64

void init_i2c_device(void);


#ifdef __cplusplus
}
#endif

#endif /* INC_I2C_H_ */
