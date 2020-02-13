#ifndef __LINUX_I2C_H__
#define __LINUX_I2C_H__

uint8_t _i2c_init(int i2c, int dev_addr);
uint8_t _i2c_close();
uint8_t _i2c_write(uint8_t* ptr, int16_t len);
uint8_t _i2c_read(uint8_t *ptr, int16_t len);
#endif
