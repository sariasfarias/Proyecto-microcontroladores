#define F_CPU 16000000
#define BAUD 9600
#define LTHRES 500 // limite del dia y la noche

#include <stdlib.h>
#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "i2c/i2cmaster.h"
#include "rtc/rtc.h"

void mi_UART_Init( unsigned int);
uint8_t mi_putc(char);
uint8_t mi_getchar(void);
#define getc() mi_getc()
#define putc(x) mi_putc(x)
/********************/
int NOCHE=0;
int Porton=0, Puerta=0;
/********UART********/
void mi_UART_Init( unsigned int ubrr)
{
	UBRR0 = F_CPU/16/ubrr-1;				// Configura baudrate. Ver en sección UART de datasheet
	UCSR0B = (1<<RXEN0)|(1<<TXEN0);			// Habilita bits TXEN0 y RXEN0
	UCSR0C = (1<<USBS0)|(3<<UCSZ00);		// USBS0=1 2 bits stop, UCSZxx=3 8 bits
}
uint8_t mi_putc(char c)
{
	while(!(UCSR0A & (1<<UDRE0)) ); // Espera mientras el bit UDRE0=0 (buffer de transmisión ocupado)
	UDR0 = c;						// Cuando se desocupa, UDR0 puede recibir el nuevo dato c a trasmitir
	return 0;
}

uint8_t mi_getc()
{
	while ( !(UCSR0A & (1<<RXC0)) );//Espera mientras el bit RXC0=0 (recepción incompleta)
	return UDR0;					//Cuando se completa, se lee UDR0
}
uint8_t enviarCadena(char* cadena){
	while(*cadena !=0x00){
		putc(*cadena);
		cadena++;
	}
	return 0;
}
/********ADC********/ 
void adc_init(){
	ADMUX |=(1<<REFS0);//setting the reference of ADC
	ADCSRA |=(1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);
}
uint16_t adc_read(uint8_t ch)
{
	ADMUX &= 0xf0;
	ADMUX |= ch;
	ADCSRA |= (1<<ADSC);
	while(ADCSRA & (1<<ADSC));
	return (ADC);
}
/********LDR********/
void LDR(){
	uint16_t adc_result;
	adc_result = adc_read(0);
	char buf[20];
	
	sprintf(buf, "LDR : %d",adc_result);
	enviarCadena(buf);
	if (adc_result < LTHRES){ //Noche
		PORTD |= (1<<PIND3);
		NOCHE=1;
		enviarCadena("\r\nLuces de patio encendidas\r\n");
	}
	else{						//DIA
		PORTD &= ~(1<<PIND3);
		NOCHE=0;
		enviarCadena("\r\nLuces de patio apagadas\r\n");
	}
}
/********SERVO********/
void servo()
{
	if (Puerta==1)
	{
		enviarCadena("\r\nPuerta abierta\r\n");
		OCR1B = 420;
		_delay_ms(2000);
		OCR1B = 200 ;
		enviarCadena("\r\nPuerta cerrada\r\n");
		Puerta=0;
	}
	if (Porton==1)
	{
		enviarCadena("\r\nPorton abierto\r\n");
		OCR1A = 420;
		_delay_ms(2000);
		OCR1A = 200 ;
		enviarCadena("\r\nPorton cerrado\r\n");
		Porton=0;
	}
}
/********SIR********/
ISR (INT0_vect){
	
	if (NOCHE==1)
	{
		PORTD &=~ (1<<PIND0);
		enviarCadena("\r\nLuz de garage prendida\r\n");
	}
	Porton=1;
	servo();
}


/********BOTONES********/
void boton(){
	//elimino el ruido al pulsar el boton
	if (bit_is_clear(PINC,PINC1)) // se apreto el boton?
	{
		_delay_ms(20);
		if (bit_is_clear(PINC,PINC1)){
			
			PORTB ^= (1<<PIND0);//alterna el led
			while(bit_is_clear(PINC,PINC1)); // espera a que el ususario deje de pulsar
		}
	}
	if (bit_is_clear(PINC,PINC2)) // se apreto el boton?
	{
		_delay_ms(20);
		if (bit_is_clear(PINC,PINC2)){
			
			PORTB ^= (1<<PIND3);//alterna el led
			while(bit_is_clear(PINC,PINC2)); // espera a que el ususario deje de pulsar
		}
	}
	if (bit_is_clear(PINC,PINC3)) // se apreto el boton?
	{
		_delay_ms(20);
		if (bit_is_clear(PINC,PINC3)){
			
			PORTB ^= (1<<PIND4);//alterna el led
			while(bit_is_clear(PINC,PINC3)); // espera a que el ususario deje de pulsar
		}
	}
	if (bit_is_clear(PIND,PIND6)) // se apreto el boton?
	{
		_delay_ms(20);
		if (bit_is_clear(PIND,PIND6)){
			
			PORTB ^= (1<<PIND5);//alterna el led
			while(bit_is_clear(PIND,PIND6)); // espera a que el ususario deje de pulsar
		}
	}
}
/********MAIN********/
int main(void) {
	char xhorario[16];
	uint8_t aux[16];
	char c,x1,x;
	uint8_t n1,cant=0;
	uint8_t diaAlarma[7]= {0};

	//entrada
	DDRD &=~ (1<<DDD2);//SIR
	PORTD |= (1<<PIND2);
	DDRB &=~ (1<<DDD6);//pulsador
	PORTB |= (1<<PIND6);
	DDRC &=~ (1<<DDC1); //pulsador
	PORTC |=  (1<<PINC1);
	DDRC &=~ (1<<DDC2); //pulsador
	PORTC |=  (1<<PINC2);
	DDRC &=~ (1<<DDC3); //pulsador
	PORTC |=  (1<<PINC3);
	// salida
	DDRD |= (1<<DDD0);//RIEGO
	PORTD &=~ (1<<PIND0);
	DDRD |= (1<<DDD1); //LUZ GARAGE
	PORTD &=~ (1<<PIND1);
	DDRD |= (1<<DDD3); //PATIO
	PORTD &=~ (1<<PIND3);
	DDRD |= (1<<DDD4); //BANIO
	PORTD &=~ (1<<PIND4);
	DDRD |= (1<<DDD5); //PIEZA
	PORTD &=~ (1<<PIND5);
	
	//init uart
	mi_UART_Init(BAUD);
	//ADC
	adc_init();
	//INTERRUPCIONES
	EICRA |= _BV(ISC11);
	EIMSK |= _BV(INT0) | _BV(INT1);
	sei();
	
	//SERVO
	DDRB |= (1<<PINB1) | (1<<PINB2);
	TCCR1A |= (1 << WGM11) | (1 << COM1A1) | (1<<COM1B1);
	TCCR1B |= (1 << WGM12) | (1 << WGM13) |(1<<CS10)|(1<<CS11);
	ICR1 = 4999;	
	OCR1A = 200 ;
	OCR1B = 200 ;
	//init ds1307
	ds1307_init();
	sei();
	char buf[50];
	char* days[7]= {"Domingo","Lunes","Martes","Miercoles","Jueves","Viernes","Sabado"};
	
	uint8_t anio = 0;
	uint8_t mes = 0;
	uint8_t dia = 0;
	uint8_t hora = 0, horaAlarma1=0, horaAlarma2=0;
	uint8_t minuto = 0, minutoAlarma1=0, minutoAlarma2=0;
	uint8_t segundo = 0;
	uint8_t sem=0;
	
	_delay_ms(10);
	enviarCadena("Desea configurar la hora?\r\n");
	_delay_ms(30);
	enviarCadena(" 1. SI - 2. NO \r\n");
	
	x= mi_getc();

	switch(x){
		case '1':
		enviarCadena("\nIngrese fecha con el siguiente formato\r\n");
		enviarCadena("dd.mm.aa\r\n");
		_delay_ms(30);
		
		for (int i=0;i<8;i++)
		{
			xhorario[i]=mi_getc();
		}
		for (int i=0;i<8;i++)
		{
			putc(xhorario[i]);
		}
		for (int i=0;i<8;i++)
		{
			aux[i]=xhorario[i]-'0';
		}
		dia=aux[0]*10+aux[1];
		mes=aux[3]*10+aux[4];
		anio=aux[6]*10+aux[7];
		
		enviarCadena("\nIngrese hora con el siguiente formato\r\n");
		enviarCadena("hh.mm.ss\r\n");
		_delay_ms(30);
		
		for (int i=8;i<16;i++)
		{
			xhorario[i]=mi_getc();
		}
		for (int i=8;i<16;i++)
		{
			putc(xhorario[i]);
		}
		for (int i=8;i<16;i++)
		{
			aux[i]=xhorario[i]-'0';
		}
		hora=aux[8]*10+aux[9];
		minuto=aux[11]*10+aux[12];
		segundo=aux[14]*10+aux[15];
		
		ds1307_setdate(anio, mes, dia, hora, minuto, segundo);
		
		break;
		case '2':
		break;
		default:
		enviarCadena("valor invalido\r\n");
		break;
	}
	
	_delay_ms(10);
	enviarCadena("\nDesea configurar horario del sistema de riego?\r\n");
	_delay_ms(30);
	enviarCadena(" 1. SI - 2. NO \r\n");
	x= mi_getc();
	
	switch(x){
		case '1':
		enviarCadena("\nIngrese cantidad de veces a la semana\r\n");
		enviarCadena("Si desea todos los dias ingrese 7\r\n");
		
		c=mi_getc();
		cant= c - '0';
		
		enviarCadena("Ingrese\r\n");
		enviarCadena("0. Domingo\r\n");
		enviarCadena("1. Lunes\r\n");
		enviarCadena("2. Martes\r\n");
		enviarCadena("3. Miercoles\r\n");
		enviarCadena("4. Jueves\r\n");
		enviarCadena("5. Viernes\r\n");
		enviarCadena("6. Sabado\r\n");
		if (cant==7)
		{
			for (int i=0; i<cant; i++)
			{
				diaAlarma[i]=i;
			}
		}
		else
		{
			for (int i=0; i<cant; i++)
			{
				x1=getc();
				putc(x1);
				n1= x1 - '0';
				diaAlarma[i]=n1;
			}
		}
		
		enviarCadena("\nIngrese hora de inicio con el siguiente formato\r\n");
		enviarCadena("hh.mm\r\n");
		_delay_ms(30);
		
		for (int i=8;i<13;i++)
		{
			xhorario[i]=mi_getc();
		}
		for (int i=8;i<13;i++)
		{
			putc(xhorario[i]);
		}
		for (int i=8;i<13;i++)
		{
			aux[i]=xhorario[i]-'0';
		}
		horaAlarma1=aux[8]*10+aux[9];
		minutoAlarma1=aux[11]*10+aux[12];
		segundo=0;
		
		enviarCadena("\nIngrese hora de fin con el siguiente formato\r\n");
		enviarCadena("hh.mm\r\n");
		_delay_ms(30);
		
		for (int i=8;i<13;i++)
		{
			xhorario[i]=mi_getc();
		}
		for (int i=8;i<13;i++)
		{
			putc(xhorario[i]);
		}
		for (int i=8;i<13;i++)
		{
			aux[i]=xhorario[i]-'0';
		}
		horaAlarma2=aux[8]*10+aux[9];
		minutoAlarma2=aux[11]*10+aux[12];
		segundo=0;
		
		break;
		case '2':
		break;
		default:
		enviarCadena("valor invalido\r\n");
		break;
	}
	
	while(1) {
		
			
		ds1307_getdate(&sem, &anio, &mes, &dia, &hora, &minuto, &segundo);
		
		sprintf(buf, "%3s  %d/%d/20%d %d:%d:%d",days[sem],dia, mes,  anio, hora, minuto, segundo);
		enviarCadena(buf);
		enviarCadena("\r\n");
		for (int i=0; i<cant; i++)
		{
			if (diaAlarma[i]==sem)
			{
				if ((hora>= horaAlarma1 && minuto>=minutoAlarma1) && ( hora <=horaAlarma2 && minuto <= minutoAlarma2))
				{
					enviarCadena("\r\nRiego encendido\r\n");
					PORTD |= (1<<PIND0);
					
				}else{
					PORTD &=~(1<<PIND0);
				}
			}
		}
		
		_delay_ms(1000);
		LDR();		
		if (bit_is_clear(PIND,PIND6)) // se apreto el boton?
		{
			_delay_ms(20);
			
			if (bit_is_clear(PIND,PIND6)){
				
				Puerta=1;
				servo();
				if (NOCHE==1)
				{
					enviarCadena("Luz de garage apagada \n");
					PORTD = (1<<PIND3);
					_delay_ms(1000);
				}
				while(bit_is_clear(PIND,PIND6)); // espera a que el ususario deje de pulsar
			}
		}
		 
		enviarCadena("-----------------------------------------");
		//boton();
		

	}
}

