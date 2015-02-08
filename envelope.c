#include "analyse.h"
#include <libavcodec/avfft.h>

#define WIN_BITS 9
#define WIN_SIZE (1 << WIN_BITS)
#define max( a, b ) ( ((a) > (b)) ? (a) : (b) )
#define QUANT_FREQ 44100
#define SAMPLE_MAX 32768
#define DECR_SPEED ( (float)SAMPLE_MAX / ((1.0f/1.34f)*(float)QUANT_FREQ) )

float envelope_sort(int16_t* sample_array) {
	FFTSample *d_freq;
	FFTSample *x;
	RDFTContext *fft;

	int nFrames;
	FILE *file_env;
	size_t rbuf_head = 0;
	int16_t ringbuf[350*2];
	int16_t d_envelope;
	int16_t enveloppe;
	uint64_t atk =0;
	uint64_t time;
	float final;
	long attack = 0;
	float env, env_prev = 0;
	size_t i, d;
	int mSamples = nSamples/350;
	file_env = fopen("file_env.txt", "w");

	d_freq = (FFTSample*)av_malloc(WIN_SIZE*sizeof(FFTSample));

	for(i = 0; i <= WIN_SIZE; ++i)
		d_freq[i] = 0.0f;

	if(nSamples%WIN_SIZE > 0)
		nSamples = (nSamples/WIN_SIZE+1)*WIN_SIZE;

	nFrames = nSamples/WIN_SIZE;
	x = (FFTSample*)av_malloc(WIN_SIZE*sizeof(FFTSample));
	fft = av_rdft_init(WIN_BITS, DFT_R2C);
	d = 0;
	
	for(i = 0; i < nSamples; ++i) {
		//env = max(env_prev - DECR_SPEED, abs(sample_array[i]));
		
		env = max(env_prev - ((float)DECR_SPEED) * (0.1f + (env_prev/((float)32768))), abs(sample_array[i]));

		env_prev = env;
		time += env;	
		ringbuf[rbuf_head] = (int16_t)env;

		if( i > 1 && i % 350 == 0) {
			d_envelope = max(0, ringbuf[rbuf_head] - ringbuf[(rbuf_head+1) % (350*2)]);
			atk = max(d_envelope, atk);
		
			if(i > 0 && i < 126*60*350)
				//fprintf(file_env, "%d\n", d_envelope);

			if((i/350) % WIN_SIZE != 0)
				x[(i/350)%WIN_SIZE - 1] = (float)d_envelope;
			else {
				x[WIN_SIZE - 1] = (float)d_envelope;
			//	printf("Yolo\n");
				av_rdft_calc(fft, x);
		//		printf("Swag\n");
				for(d = 1; d < WIN_SIZE/2; ++d) {
					float re = x[d*2];
					float im = x[d*2+1];
					float raw = re*re + im*im;
					d_freq[d] += raw;
				}
			}
		}
		rbuf_head = (rbuf_head+1) % (350*2);
	}
	
	//printf("%lu\n", atk/(nSamples));
//	printf("%d\n", time);
	final = 0;
	for(i = 1; i < WIN_SIZE/2; ++i) {
	//	fprintf(file_env, "%f\n", d_freq[i]);
		final = max(final, d_freq[i]);
	}	

	final /= pow(10, 10);
	final = (final - 70.0f)/(final + 70.0f);
	final *= 2.0f;

	if(atk >= 30000)
		final += 2.0f;

	if(debug) {
		printf("%f\n", final);
		printf("%lu\n", atk);
	}

		
	//printf("SWAG: %f\n", (float)atk/nSamples);

	fclose(file_env);
	if(final > 0)
		return final;
	else
		return 0;
}
/*	enveloppe = calloc(nSamples, sizeof(int16_t));
	dEnveloppe = calloc(mSamples, sizeof(int16_t));

	for(i 

	for(i = 1; i < nSamples-1; ++i) 
		enveloppe[i] = max(enveloppe[i-1] - (int16_t)( ((float)DECR_SPEED)*(0.1f+((float)(enveloppe[i-1])/((float)32768))) ), abs(sample_array[i]));	
		//enveloppe[i] = max(enveloppe[i-1] - DECR_SPEED, abs(sample_array[i]));

	for(i = 1; i <= mSamples-1; i++) {
		dEnveloppe[i-1] = enveloppe[(i+1)*350] - enveloppe[(i-1)*350];
	}

	for(i = 44100*24; i < 44100*34;++i)
		fprintf(file_env, "%d\n", enveloppe[i]);
	
	for(i = 1; i < mSamples; ++i) 
		attack += ((dEnveloppe[i] - dEnveloppe[i-1] > 0) ? dEnveloppe[i] - dEnveloppe[i-1] : 0);
	
	printf("%d\n", attack/mSamples);*
} */
