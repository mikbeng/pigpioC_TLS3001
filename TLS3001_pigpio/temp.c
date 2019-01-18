

/*
 
 
typedef struct
{
	uint64_t message;
	uint32_t num_bits;
	uint32_t wave_id;
} manch_message_wave;
 
static void _make_bitwaves(manch_message_wave *self)
{
	
	
	//Generates the basic waveforms needed to transmit codes.
	

	gpioPulse_t pulse_array[(self->num_bits) * 2];
	
	//memset(&pulse, sizeof(pulse), 0);// clear full struct
	
	int i, pulses = 0;
	uint32_t bit;
	uint16_t t_off, micros, DMA_blocks = 0;
			
	bit = ((uint32_t)1 << (self->num_bits - 1));

	gpioWaveClear();
	
	for (i = 0; i < (self->num_bits * 2); (i = i + 2))
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
	
	pulses = gpioWaveAddGeneric((self->num_bits) * 2, pulse_array);
	micros = gpioWaveGetMicros();
	
	if (pulses == (self->num_bits * 2))
	{
		self->wave_id = gpioWaveCreate();
	}

	
}*/