/*
 *
 * Created: 06-02-2018 21:51:39
 *  Author: familia
 */ 
#include <inttypes.h>
#include <compat/twi.h>
#include "i2cmaster.h"
#ifndef F_CPU
#define F_CPU 4000000UL
#endif
#define SCL_CLOCK  10000L

void i2c_init(void)
{	
	TWSR = 0;                         
	TWBR = ((F_CPU/SCL_CLOCK)-16)/2;  

}
unsigned char i2c_start(unsigned char address)
{
	uint8_t   twst;
	TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN);
	while(!(TWCR & (1<<TWINT)));
	twst = TW_STATUS & 0xF8;
	if ( (twst != TW_START) && (twst != TW_REP_START)) return 1;

	// enviando direccion 
	TWDR = address;
	TWCR = (1<<TWINT) | (1<<TWEN);

	// esperar a que se complete a transmicion y devolver un nack o un ack
	while(!(TWCR & (1<<TWINT)));

	//chequea valor del estatos de twi
	twst = TW_STATUS & 0xF8;
	if ( (twst != TW_MT_SLA_ACK) && (twst != TW_MR_SLA_ACK) ) return 1;

	return 0;

}
void i2c_start_wait(unsigned char address)
{
	uint8_t   twst;


	while ( 1 )
	{
		TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN);

		while(!(TWCR & (1<<TWINT)));
		twst = TW_STATUS & 0xF8;
		if ( (twst != TW_START) && (twst != TW_REP_START)) continue;
		
		TWDR = address;
		TWCR = (1<<TWINT) | (1<<TWEN);
		
		while(!(TWCR & (1<<TWINT)));
		
		twst = TW_STATUS & 0xF8;
		if ( (twst == TW_MT_SLA_NACK )||(twst ==TW_MR_DATA_NACK) )
		{
			TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWSTO);
			
			while(TWCR & (1<<TWSTO));
			
			continue;
		}
		break;
	}

}
unsigned char i2c_rep_start(unsigned char address)
{
	return i2c_start( address );
}
void i2c_stop(void)
{
	/* send stop condition */
	TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWSTO);
	
	// wait until stop condition is executed and bus released
	while(TWCR & (1<<TWSTO));

}
unsigned char i2c_write( unsigned char data )
{
	uint8_t   twst;
	
	// envia datos al sensor
	TWDR = data;
	TWCR = (1<<TWINT) | (1<<TWEN);

	// espera
	while(!(TWCR & (1<<TWINT)));

	twst = TW_STATUS & 0xF8;
	if( twst != TW_MT_DATA_ACK) return 1;
	return 0;

	}
unsigned char i2c_readAck(void)
{
	TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWEA);
	while(!(TWCR & (1<<TWINT)));

	return TWDR;

	}
unsigned char i2c_readNak(void)
{
	TWCR = (1<<TWINT) | (1<<TWEN);
	while(!(TWCR & (1<<TWINT)));
	
    return TWDR;

}



