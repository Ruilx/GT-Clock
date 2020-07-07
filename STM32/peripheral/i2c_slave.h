#ifndef I2C_SLAVE_H
#define I2C_SLAVE_H

#include <list.h>

typedef const struct {
	void *(* const data)(unsigned int write, unsigned int id, unsigned int *segment, unsigned int *size);
	void (* const write_complete)(unsigned int id, unsigned int segment, unsigned int size, void *p);
} i2c_slave_reg_handler_t;

#define I2C_SLAVE_REG_HANDLER()	LIST_ITEM(i2c_slave_reg, i2c_slave_reg_handler_t)

#endif // I2C_SLAVE_H
