#include <stdio.h>
#include "pigpio.h"
#include <unistd.h>
#include <stdlib.h>
#include "stdarg.h"
#include "string.h"
#include <math.h>
	
#define TLS3001_GPIO 25
#define PIXELS 126

#define T1 3
#define T0 3

#define MANCH_BIT_1 {{ 1 << TLS3001_GPIO, 0, T1 }, { 0, 1 << TLS3001_GPIO, T0 }} 
#define MANCH_BIT_0 {{ 0, 1 << TLS3001_GPIO, T0 }, { 1 << TLS3001_GPIO, 0, T1 }}

#define p1 		{ 1 << TLS3001_GPIO, 0, 20 },
#define p2		{ 0, 1 << TLS3001_GPIO, 40 },
#define p3		{ 1 << TLS3001_GPIO, 0, 10 },
#define p4		{ 0, 1 << TLS3001_GPIO, 100 },

typedef struct
{
	uint32_t id_manch_0;
	uint32_t id_manch_1;
	uint32_t t0;
	uint32_t t1;	
	uint32_t gpio;
} manch_bit_wave_s;

manch_bit_wave_s manch_bit_wave = { 0 };


typedef struct
{
	uint64_t message;
	uint32_t num_bits;
	char data_chain[39];
} manch_message_wave1;

typedef struct
{
	uint64_t message;
	uint32_t num_bits;
	uint32_t wave_id;
} manch_message_wave;

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

static void _make_bitwaves1();
static void _make_bitwaves();

//static void _make_chain(manch_message_wave *self);
static void _combine_chains(int arg_num, ...);
int send_chain(char *chain, uint16_t chain_length);

void tx_send(manch_message_wave *self);

int main(int argc, char *argv[])
{
	char sz[] = "Hello, World!\n";	/* Hover mouse over "sz" while debugging to see its contents */
	printf("%s", sz);	
	fflush(stdout); /* <============== Put a breakpoint here */

	uint16_t i, pulses, micros;
	uint32_t bit;
	
	uint32_t sync_delay_us = (uint32_t)(PIXELS * 28.34);
	
	uint8_t delay_x = 0;
	uint8_t delay_y = ceil((187.0 / 256.0)*PIXELS);

	
	char sync_data_chain[PIXELS + 2 + 4];
	char sync_data_delay[4] = { 255, 2, delay_x, delay_y };
	char data_chain[PIXELS+1];
	
	pulses = 0;
	
	gpioInitialise();
	time_sleep(1);
	gpioSetMode(manch_bit_wave.gpio, PI_OUTPUT);
	
	//gpioSetPullUpDown(manch_bit_wave.gpio, PI_PUD_DOWN);
	//gpioWrite(manch_bit_wave.gpio, PI_LOW);
	
	manch_bit_wave.t1 = 3;
	manch_bit_wave.t0 = 3;
	manch_bit_wave.gpio = TLS3001_GPIO;
	
	//-----------------------------------
	

	/*
	gpioPulse_t wf[] =
	{
		{ 1 << TLS3001_GPIO, 0, 2 },
		{ 0, 1 << TLS3001_GPIO, 4 },
		{ 1 << TLS3001_GPIO, 0, 3 },
		{ 0, 1 << TLS3001_GPIO, 6 },
		{ 1 << TLS3001_GPIO, 0, 10 },
		{ 0, 1 << TLS3001_GPIO, 5 },
	};

	//gpioSetMode(manch_bit_wave.gpio, PI_OUTPUT);
	
	
	
	pulses = gpioWaveAddGeneric(6, wf);
	micros = gpioWaveGetMicros();
	
	test1.wave_id = gpioWaveCreate();
	*/
	//-----------------------------------
	uint64_t temp_mess = 0;
	//_make_bitwaves(&test1);
	gpioWaveClear();
	_make_bitwaves(&message_resetdata);
	_make_bitwaves(&message_syncdata);
	_make_bitwaves(&message_startdata);
	
	data_chain[0] = message_startdata.wave_id;
	sync_data_chain[0] = message_syncdata.wave_id;
	memcpy(&sync_data_chain[1], &sync_data_delay[0], sizeof(sync_data_delay));
	sync_data_chain[5] = message_startdata.wave_id;
	
	unsigned int color[3][3] = { 
		{3000, 0, 0},
		{0, 3000, 0},
		{0, 0, 3000}
		};
	
	for (i = 0; i < PIXELS; i++)
	{
		message_pixel_colors[i].num_bits = 39;
		pixel_data_array[i].red = color[(i % 3)][0];
		pixel_data_array[i].green = color[(i % 3)][1];
		pixel_data_array[i].blue = color[(i % 3)][2];
		
		
		temp_mess = (((uint64_t)pixel_data_array[i].red << 26) | ((uint64_t)pixel_data_array[i].green << 13) | ((uint64_t)pixel_data_array[i].blue));
		message_pixel_colors[i].message = temp_mess & ~((uint64_t)0x4002001000);
		_make_bitwaves(&message_pixel_colors[i]);
		
		//Add to chain
		data_chain[i + 1] = message_pixel_colors[i].wave_id;
		//sync_data_chain[i + 1 + 5] = message_pixel_colors[i].wave_id;
	}
	
	memcpy(&sync_data_chain[6], &data_chain[1], sizeof(char)*PIXELS);
	
	
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
	
	/*
	while (1)
	{

		//tx_send(&);
		
		//send_chain(message_resetdata.data_chain, message_resetdata.num_bits);
		gpioWaveTxSend(test1.wave_id, PI_WAVE_MODE_REPEAT_SYNC);

		// Transmit for 30 seconds.

		//sleep(30);

		//gpioWaveTxStop();

		time_sleep(0.1);
		
	}
	*/
	
	uint32_t loop_cnt = 0;
	
	/*
	gpioWaveTxSend(message_syncdata.wave_id, PI_WAVE_MODE_ONE_SHOT);
	while (gpioWaveTxBusy())
	gpioDelay(sync_delay_us);
	*/
	//send_chain(data_chain, PIXELS + 1);
	
	for(size_t i = 0 ; i < 10 ; i++)
	{	
		gpioWaveTxSend(message_resetdata.wave_id, PI_WAVE_MODE_ONE_SHOT);
		while (gpioWaveTxBusy())
		gpioDelay(1000);
	
		gpioWaveChain(sync_data_chain, PIXELS + 2 + 4);
		while (gpioWaveTxBusy())
		gpioDelay(2500);
		
		while (loop_cnt < 30)
		{
			gpioWaveChain(data_chain, PIXELS + 1);
			//send_chain(data_chain, PIXELS + 1);
			while(gpioWaveTxBusy())
			gpioDelay(1500);
			loop_cnt++;
		}	 
	}

	
	gpioTerminate();
	
	return 0;
}

static uint32_t RGB_to_int64(pixel_data_s *pixel_RGB_pointer)
{
	uint32_t message_32 = 0;
	
	return message_32;
}


static void _make_bitwaves(manch_message_wave *self)
{
	uint32_t bit_pulses = (self->num_bits) * 2;
	
	//Generates the basic waveforms needed to transmit codes.

	gpioPulse_t pulse_array[bit_pulses+1];
	
	//memset(&pulse, sizeof(pulse), 0);// clear full struct
	
	int i, pulses = 0;
	uint64_t bit;
	uint16_t t_off, micros, DMA_blocks = 0;
			
	bit = ((uint64_t)1 << (self->num_bits - 1));

	//gpioWaveClear();
	
	for(i = 0 ; i < (bit_pulses) ; (i = i + 2))
	{
		if (self->message & bit) 
		{
			//If bit == 1
			pulse_array[i].gpioOn = 1 << manch_bit_wave.gpio;
			pulse_array[i].gpioOff = 0;
			pulse_array[i].usDelay = manch_bit_wave.t1;
			
			pulse_array[i + 1].gpioOn = 0;
			pulse_array[i + 1].gpioOff = 1 << manch_bit_wave.gpio;
			pulse_array[i + 1].usDelay = manch_bit_wave.t0;
		
		}
		else 
		{
			//if bit == 0
			pulse_array[i].gpioOn = 0;
			pulse_array[i].gpioOff = 1 << manch_bit_wave.gpio;
			pulse_array[i].usDelay = manch_bit_wave.t0;
			
			pulse_array[i + 1].gpioOn = 1 << manch_bit_wave.gpio;
			pulse_array[i + 1].gpioOff = 0;
			pulse_array[i + 1].usDelay = manch_bit_wave.t1;
		}
		bit >>= 1;

	}
	
	//Make sure last pulse is set to zero
	pulse_array[bit_pulses].gpioOn = 0;
	pulse_array[bit_pulses].gpioOff = 1 << manch_bit_wave.gpio;
	pulse_array[bit_pulses].usDelay = manch_bit_wave.t0;

	
	pulses = gpioWaveAddGeneric((bit_pulses+1), pulse_array);
	micros = gpioWaveGetMicros();
	
	if (pulses == (bit_pulses+1))
	{
		self->wave_id = gpioWaveCreate();
	}

	
}

static void _make_bitwaves1()
{
	
	//Generates the basic waveforms needed to transmit codes.
	
	
	gpioWaveAddGeneric(2,(gpioPulse_t[])
	   { { 1 << manch_bit_wave.gpio, 0, manch_bit_wave.t1 }, { 0, 1 << manch_bit_wave.gpio, manch_bit_wave.t0 } });

	manch_bit_wave.id_manch_1 = gpioWaveCreate();

	gpioWaveAddGeneric(2,(gpioPulse_t[])
	   { { 0, 1 << manch_bit_wave.gpio, manch_bit_wave.t0 }, { 1 << manch_bit_wave.gpio, 0, manch_bit_wave.t1 } });

	manch_bit_wave.id_manch_0 = gpioWaveCreate();
	
}

static void _make_chain(manch_message_wave1 *self)
{
	/*
	Transmits the code (using the current settings of repeats,
	bits, gap, short, and long pulse length).
	*/
	int i, p = 0;
	uint32_t bit;

	/* Loop start
	chain[p++] = self->_amble;
	chain[p++] = 255;
	chain[p++] = 0;
	*/
	
	bit = ((uint32_t)1 << (self->num_bits - 1));

	for (i = 0; i < self->num_bits; i++)
	{
		if (self->message & bit) 
		{
			self->data_chain[p] = manch_bit_wave.id_manch_1;	
		}
		else 
		{
			self->data_chain[p] = manch_bit_wave.id_manch_0;
		}
		bit >>= 1;
		p++;
	}
/*
	chain[p++] = self->_amble;
	//Loop repeat
	chain[p++] = 255;	
	chain[p++] = 1;
	chain[p++] = self->repeats;
	chain[p++] = 0;
*/


}


static void _combine_chains(int arg_num, ...)
{
	int i, num, sum;
	va_list valist;
	/* initialize valist for num number of arguments */
	va_start(valist, arg_num);
	
	for (i = 0; i < num; i++) {
		sum += va_arg(valist, int);
	}
}


int send_chain(char *chain,	uint16_t chain_length)
{
	int ret;
	
	uint32_t repeats;
	//Busy wait (for prevoius transmission)
	while(gpioWaveTxBusy()) gpioDelay(150);
	
	ret = gpioWaveChain(chain, chain_length);
}

/*
_433D_tx_t *_433D_tx(int pi, int gpio)
{
	_433D_tx_t *self;

	self = malloc(sizeof(_433D_tx_t));

	if (!self) return NULL;

	self->pi = pi;
	self->gpio = gpio;
	self->repeats = 6;
	self->bits = 24;
	self->gap = 9000;
	self->t0 = 300;
	self->t1 = 900;

	_make_waves(self);

	set_mode(pi, gpio, PI_OUTPUT);

	return self;
}*/