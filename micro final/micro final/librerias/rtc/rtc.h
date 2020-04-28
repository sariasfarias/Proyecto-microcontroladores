/*
 *
 * Created: 06-02-2018 22:00:08
 *  Author: familia
 */ 


#ifndef INCFILE1_H_
#define INCFILE1_H_

#define DS1307_ADDR (0x68<<1) //direccion del sensor
#define DS1307_I2CFLEURYPATH "../i2c/i2cmaster.h" //define the path to i2c fleury lib
#define DS1307_I2CINIT 1 //init i2c

extern void ds1307_init();
extern uint8_t ds1307_setdate(uint8_t anio, uint8_t mes, uint8_t dia, uint8_t hora, uint8_t minuto, uint8_t segundo);
extern void ds1307_getdate(uint8_t *dsem, uint8_t *anio, uint8_t *mes, uint8_t *dia, uint8_t *hora, uint8_t *minuto, uint8_t *segundo);

#endif /* INCFILE1_H_ */