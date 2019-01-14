#include <stdio.h>
#include "pigpio.h"
#include <unistd.h>
#include <stdlib.h>
#include "stdarg.h"
	
#define TLS3001_GPIO 25
#define PIXELS 10

#define T1 3
#define T0 3

#define MANCH_BIT_1 {{ 1 << TLS3001_GPIO, 0, T1 }, { 0, 1 << TLS3001_GPIO, T0 }} 
#define MANCH_BIT_0 {{ 0, 1 << TLS3001_GPIO, T0 }, { 1 << TLS3001_GPIO, 0, T1 }}


typedef struct
{
	uint32_t id_manch_0;
	uint32_t id_manch_1;
	uint32_t t0;
	uint32_t t1;	
	uint32_t gpio;
} manch_bit_wave_s;

manch_bit_wave_s manch_bit_wave = { 0 };

/*
typedef struct
{
	uint64_t message;
	uint32_t num_bits;
	char wave_chain[MAX_];
} manch_message_wave;
*/


typedef struct
{
	uint64_t message;
	uint32_t num_bits;
	uint32_t wave_id;
} manch_message_wave;


static void _make_bitwaves(manch_message_wave *self)
{
	
	/*
	Generates the basic waveforms needed to transmit codes.
	*/

	gpioPulse_t pulse[(self->num_bits) * 2];
	
	//memset(&pulse, sizeof(pulse), 0);// clear full struct
	
	int i, pulses = 0;
	uint32_t bit;
	uint16_t t_off, micros, DMA_blocks = 0;
			
	bit = ((uint32_t)1 << (self->num_bits - 1));

	gpioSetMode(manch_bit_wave.gpio, PI_OUTPUT);
	gpioWaveClear();
	
	for (i = 0; i < (self->num_bits*2); (i=i+2))
	{
		if (self->message & bit) 
		{
			//If bit == 1
			/*
			pulse[i].gpioOn = 1 << manch_bit_wave.gpio;
			pulse[i].gpioOff = 0;
			pulse[i].usDelay = manch_bit_wave.t1;
			
			pulse[i+1].gpioOn = 0;
			pulse[i+1].gpioOff = 1 << manch_bit_wave.gpio;
			pulse[i+1].usDelay = manch_bit_wave.t0;
			*/
			pulses += gpioWaveAddGeneric(2,(gpioPulse_t[])
					{ { 1 << manch_bit_wave.gpio, 0, (manch_bit_wave.t1) }, { 0, 1 << manch_bit_wave.gpio, (manch_bit_wave.t0) } });
			//pulses += gpioWaveAddGeneric(2, (gpioPulse_t[]) MANCH_BIT_1);
		}
		else 
		{
			//if bit == 0
			/*
			pulse[i].gpioOn = 0;
			pulse[i].gpioOff = 1 << manch_bit_wave.gpio;
			pulse[i].usDelay = manch_bit_wave.t1;
			
			pulse[i + 1].gpioOn = 1 << manch_bit_wave.gpio;
			pulse[i + 1].gpioOff = 0;
			pulse[i + 1].usDelay = manch_bit_wave.t0;
			*/
			
			pulses += gpioWaveAddGeneric(2,(gpioPulse_t[])
					{ { 0, 1 << manch_bit_wave.gpio, (manch_bit_wave.t0) }, { 1 << manch_bit_wave.gpio, 0, (manch_bit_wave.t1) } });
			//pulses += gpioWaveAddGeneric(2, (gpioPulse_t[]) MANCH_BIT_0);
		}
		bit >>= 1;
		//t_off += 1;
	
	}
	
	//pulses = gpioWaveAddGeneric((self->num_bits) * 2, pulse);
	micros = gpioWaveGetMicros();
	
	if (pulses == (self->num_bits * 2))
	{
		self->wave_id = gpioWaveCreate();
	}

	
}


typedef struct
{
	unsigned int red : 12;
	unsigned int green : 12;
	unsigned int blue : 12;
}pixel_data_s;

pixel_data_s pixel_data_array[PIXELS];

manch_message_wave test1 = { 0b10, 2, 0 };
manch_message_wave message_resetdata = {0x7FFF4, 19, 0};
manch_message_wave message_syncdata = {0x3FFF8800, 30, 0};
manch_message_wave message_startdata = {0x7FFF2, 19, 0};
manch_message_wave message_pixel_colors[PIXELS];

//manch_message_wave color_data1 = { }

static void _make_bitwaves();
static void _make_chain(manch_message_wave *self);
static void _combine_chains(int arg_num, ...);

void tx_send(manch_message_wave *self);

int main(int argc, char *argv[])
{
	char sz[] = "Hello, World!\n";	/* Hover mouse over "sz" while debugging to see its contents */
	printf("%s", sz);	
	fflush(stdout); /* <============== Put a breakpoint here */

	uint16_t i, pulses, micros;
	
	gpioInitialise();
	
	manch_bit_wave.t1 = 3;
	manch_bit_wave.t0 = 3;
	manch_bit_wave.gpio = TLS3001_GPIO;
	
	gpioPulse_t wf[] =
	{
		{ 1 << TLS3001_GPIO, 0, 1 },
		{ 0, 1 << TLS3001_GPIO, 3 },
		{ 1 << TLS3001_GPIO, 0, 6 },
		{ 0, 1 << TLS3001_GPIO, 10 },
	};
	
	gpioSetMode(manch_bit_wave.gpio, PI_OUTPUT);

	gpioWaveClear();
	
	//gpioWaveAddNew();
	
	pulses = gpioWaveAddGeneric(4, wf);
	micros = gpioWaveGetMicros();
	
	test1.wave_id = gpioWaveCreate();
	
	_make_bitwaves(&test1);
	//_make_bitwaves(&message_resetdata);
	//_make_bitwaves(&message_syncdata);
	//_make_bitwaves(&message_startdata);
	

	/*
	_make_chain(&message_resetdata);
	_make_chain(&message_syncdata);
	_make_chain(&message_startdata);
	*/
	
	
	for (i = 0; i < PIXELS; i++)
	{
		//message_pixel_colors[i].message = RGB_to_int64(&pixel_data_array[i]);
		//_make_bitwaves(&message_pixel_colors[i]);
		
		//_make_chain(&message_pixel_colors[i]);
	}
	
	//_combine_chains((1 + PIXELS), &message_pixel_colors[0])
	
	
	while (1)
	{

		//tx_send(&);
		

		gpioWaveTxSend(test1.wave_id, PI_WAVE_MODE_REPEAT_SYNC);

		// Transmit for 30 seconds.

		//sleep(30);

		//gpioWaveTxStop();

		time_sleep(0.5);
		
	}

	
	return 0;
}

static uint32_t RGB_to_int64(pixel_data_s *pixel_RGB_pointer)
{
	uint32_t message_32 = 0;
	
	return message_32;
}

//
//static void _make_bitwaves()
//{
//	
//	//Generates the basic waveforms needed to transmit codes.
//	
//	
//	gpioWaveAddGeneric(2,(gpioPulse_t[])
//	   { { 1 << manch_bit_wave.gpio, 0, manch_bit_wave.t1 }, { 0, 1 << manch_bit_wave.gpio, manch_bit_wave.t0 } });
//
//	manch_bit_wave.id_manch_1 = gpioWaveCreate();
//
//	gpioWaveAddGeneric(2,(gpioPulse_t[])
//	   { { 0, 1 << manch_bit_wave.gpio, manch_bit_wave.t0 }, { 1 << manch_bit_wave.gpio, 0, manch_bit_wave.t1 } });
//
//	manch_bit_wave.id_manch_0 = gpioWaveCreate();
//	
//}
//
//static void _make_chain(manch_message_wave *self)
//{
//	/*
//	Transmits the code (using the current settings of repeats,
//	bits, gap, short, and long pulse length).
//	*/
//	int i, p = 0;
//	uint32_t bit;
//
//	/* Loop start
//	chain[p++] = self->_amble;
//	chain[p++] = 255;
//	chain[p++] = 0;
//	*/
//	
//	bit = ((uint32_t)1 << (self->num_bits - 1));
//
//	for (i = 0; i < self->num_bits; i++)
//	{
//		if (self->message & bit) 
//		{
//			self->wave_chain[p] = manch_bit_wave.id_manch_1;	
//		}
//		else 
//		{
//			self->wave_chain[p] = manch_bit_wave.id_manch_0;
//		}
//		bit >>= 1;
//		p++;
//	}
///*
//	chain[p++] = self->_amble;
//	//Loop repeat
//	chain[p++] = 255;	
//	chain[p++] = 1;
//	chain[p++] = self->repeats;
//	chain[p++] = 0;
//*/
//
//
//}
//
//static void _combine_chains(int arg_num, ...)
//{
//	
//	va_list valist;
//	/* initialize valist for num number of arguments */
//	va_start(valist, arg_num);
//	
//	for (i = 0; i < num; i++) {
//		sum += va_arg(valist, int);
//	}
//}
//
//
//int send_chain(char *chain,	uint16_t chain_length)
//{
//	int ret;
//	
//	uint32_t repeats;
//	//Busy wait (for prevoius transmission)
//	while(gpioWaveTxBusy()) {
//	}
//	
//	ret = gpioWaveChain(chain, chain_length);
//}
//
///*
//_433D_tx_t *_433D_tx(int pi, int gpio)
//{
//	_433D_tx_t *self;
//
//	self = malloc(sizeof(_433D_tx_t));
//
//	if (!self) return NULL;
//
//	self->pi = pi;
//	self->gpio = gpio;
//	self->repeats = 6;
//	self->bits = 24;
//	self->gap = 9000;
//	self->t0 = 300;
//	self->t1 = 900;
//
//	_make_waves(self);
//
//	set_mode(pi, gpio, PI_OUTPUT);
//
//	return self;
//}*/