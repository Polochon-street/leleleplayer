#include "analyse.h"
#include <libavcodec/avfft.h>

#define WIN_BITS 9
#define WIN_SIZE (1 << WIN_BITS)
#define max( a, b ) ( ((a) > (b)) ? (a) : (b) )

float envelope_sort(int16_t* sample_array) {
	FFTSample *d_freq;
	FFTSample *x;
	RDFTContext *fft;
	int nFrames;
	int precision = 350;
	uint64_t sample_max = pow(2, 8*nb_bytes_per_sample-1);
	float decr_speed = (float)((int)sample_max) / ((1.0f/1.34f)*(float)sample_rate);
	FILE *file_env;
	size_t rbuf_head = 0;
	int16_t ringbuf[precision*2];
	int16_t d_envelope;
	int16_t enveloppe;
	int64_t atk =0;
	float final = 0;
	float env, env_prev = 0;
	size_t i, d;
	file_env = fopen("file_env.txt", "w");

	d_freq = (FFTSample*)av_malloc(WIN_SIZE*sizeof(FFTSample));

	for(i = 0; i <= WIN_SIZE; ++i)
		d_freq[i] = 0.0f;

	if(nSamples%WIN_SIZE > 0)
		nSamples -= nSamples%WIN_SIZE; 

	nFrames = nSamples/WIN_SIZE;
	x = (FFTSample*)av_malloc(WIN_SIZE*sizeof(FFTSample));
	fft = av_rdft_init(WIN_BITS, DFT_R2C);
	
	for(i = 0; i < nSamples; ++i) {		
		env = max(env_prev - ((float)decr_speed) * (0.1f + (env_prev/((float)sample_max))), abs(sample_array[i]));
		env_prev = env;
		ringbuf[rbuf_head] = (int16_t)env;

		if( i > 1 && i % precision == 0) {
			d_envelope = max(0, ringbuf[rbuf_head] - ringbuf[(rbuf_head+1) % (precision*2)]);
			atk = max(d_envelope, atk);
		
			if((i/precision) % WIN_SIZE != 0)
				x[(i/precision)%WIN_SIZE - 1] = (float)d_envelope;
			else {
				x[WIN_SIZE - 1] = (float)d_envelope;
				av_rdft_calc(fft, x);
				for(d = 1; d < WIN_SIZE/2; ++d) {
					float re = x[d*2];
					float im = x[d*2+1];
					float raw = re*re + im*im;
					d_freq[d] += raw;
				}
			}
		}
		rbuf_head = (rbuf_head+1) % (precision*2);
	}
	
	if(debug) {
		for(i = 30; i < WIN_SIZE/2; ++i) {
			fprintf(file_env, "%f\n", d_freq[i]);
			final = max(final, d_freq[i]);
		}
	}
	else
		for(i = 30; i < WIN_SIZE/2; ++i) 
			final = max(final, d_freq[i]);

	final /= pow(10, 10);
	atk /= 1000;

	atk -= 25;
	atk = (int)atk > 0 ? atk : 0;
	final = ((float)1/4)*final - ((float)35/2);
	final = final > 0 ? final : 0;
	final += (float)((int)atk);


	if(debug) {
		printf("Atk: %f\n", (float)((int)atk));
		printf("Envelope result: %f\n", final);
	}

	fclose(file_env);
	return final;
}
