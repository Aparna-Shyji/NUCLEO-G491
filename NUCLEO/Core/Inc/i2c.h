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
int write_i2c_device(int channel_num, int slave_addr, uint8_t reg,
                            uint8_t *write_buffer, int size);
int read_i2c_device(int channel_num, int slave_addr, uint8_t reg,
                           uint8_t *read_buffer, int size);



#ifdef __cplusplus
}
#endif

#endif /* INC_I2C_H_ */
