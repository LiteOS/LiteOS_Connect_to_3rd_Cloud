#include "stm32f4xx_empl_drv.h"
#include "mpu6050.h"
#include "cmsis_os.h"
#include "i2c.h"
#include <string.h>




static unsigned char stm32f4xx_empl_i2c_wb[MAX_WRITE_SIZE];

int stm32f4xx_i2c_write (unsigned char slave_addr,
                         unsigned char reg_addr, 
                         unsigned char length, 
                         unsigned char const *data)
{	
	
	
	 
Sensors_I2C_WriteRegister(slave_addr,
                                        reg_addr,
                                       length, 
                                       data);
	/*
	if (length + 1 > MAX_WRITE_SIZE)
		return -1;
	
	stm32f4xx_empl_i2c_wb[0] = reg_addr;
	memcpy (stm32f4xx_empl_i2c_wb + 1, data, length);
	
  while (__HAL_I2C_GET_FLAG(&hi2c1, I2C_FLAG_BUSY) == SET) ;
	
	HAL_I2C_Master_Transmit (&hi2c1, slave_addr << 1, stm32f4xx_empl_i2c_wb, length + 1, 1000);
	*/
	return 0;
}

													
int stm32f4xx_i2c_read  (unsigned char slave_addr,
                         unsigned char reg_addr, 
                         unsigned char length, 
                         unsigned char *data)
{
	
	Sensors_I2C_ReadRegister(slave_addr,
                                        reg_addr,
                                       length, 
                                       data);
	/*
  while (__HAL_I2C_GET_FLAG(&hi2c1, I2C_FLAG_BUSY) == SET) ;
	
	HAL_I2C_Master_Transmit (&hi2c1, slave_addr << 1, &reg_addr, 1, 1000);
	HAL_I2C_Master_Receive  (&hi2c1, slave_addr << 1, data, length, 1000);
	*/
	return 0;
}


void stm32f4xx_delay_ms  (unsigned long num_ms)
{
	//HAL_Delay(num_ms);
	osDelay(num_ms);
}


void stm32f4xx_get_ms    (unsigned long *count)
{
	//(*count) = HAL_GetTick();
}


void stm32f4xx_log_i (char const *s, ...)
{
	return;
}


void stm32f4xx_log_e (char const *s, ...)
{
	return;
}
