#include <math.h>
#include <libavcodec/avfft.h>
#include "analyse.h"

#define WIN_BITS 9 // 9
#define WIN_SIZE (1 << WIN_BITS)

#define GRAVE_INF 2
#define GRAVE_SUP 4
#define AIGU_INF 17
#define AIGU_SUP 104

float freq_sort(int16_t *cheat_array) {
	float hann_window[WIN_SIZE];
	FFTSample *spectre_moy;
	float tab_bandes[4];
	float tab_sum;
	int nFrames;
	int16_t *sample_array16;
	int16_t *p16;
	int32_t *sample_array32;
	int32_t *p32;
	 
	int d, iFrame;
	size_t i;
	FFTSample* x;
	RDFTContext* fft;
	float freq;
	float pas_freq;
	FILE *file1;
	FILE *file2;
	FILE *file3;
	
	if (debug) {
		file1 = fopen("file_freq1.txt", "w");
		file2 = fopen("file_freq2.txt", "w");
		file3 = fopen("yolo.raw", "w");
	}

	float peak;
	char resnum_freq = 0;

	if (nb_bytes_per_sample == 2) {
		sample_array16 = (int16_t*)malloc(size);
		p16 = sample_array16;
		for (i = 0; i < nSamples; i+=2) {
			*(p16++) = (cheat_array[i]+cheat_array[i+1])/2;
		}
	}
	else if (nb_bytes_per_sample == 4) {
		sample_array32 = (int32_t*)malloc(size);
		p32 = sample_array32;
		for (i = 0; i < nSamples; i+=2) {	
			*(p32++) = (((int32_t*)cheat_array)[i]+((int32_t*)cheat_array)[i+1])/2;
		}
	}
	
	if (debug) {
		if (nb_bytes_per_sample == 2)
			fwrite(sample_array16, 1, nSamples, file3);
		else if (nb_bytes_per_sample == 4)
			fwrite(sample_array32, 1, size, file3);
	}

	peak=0;
	for(i = 0; i < WIN_SIZE; ++i)
		hann_window[i] = .5f - .5f*cos(2*M_PI*i/(WIN_SIZE-1));

	/* FFT init */

	spectre_moy = (FFTSample*)av_malloc((WIN_SIZE/2)*sizeof(FFTSample));
	spectre_moy = (FFTSample*)av_malloc((WIN_SIZE*sizeof(FFTSample)));

	for(i=0;i<=WIN_SIZE/2;++i)
		spectre_moy[i] = 0.0f;

	for(i=0;i<11;++i)
		tab_bandes[i] = 0.0f;

	nSamples/=2; // Only one channel

	if(nSamples%WIN_SIZE > 0)
		nSamples = (nSamples/WIN_SIZE+1)*WIN_SIZE; //in order for the fft to work

	nFrames = nSamples/WIN_SIZE;

	x = (FFTSample*)av_malloc(WIN_SIZE*sizeof(FFTSample));
	
	fft = av_rdft_init(WIN_BITS, DFT_R2C);

	/* End of FFT init */
	/* FFT computation */

	for(i=0, iFrame = 0; iFrame<nFrames;i+=WIN_SIZE, iFrame++) {
		if (nb_bytes_per_sample == 2) {
			for(d = 0; d < WIN_SIZE; d++)
				x[d] = (float)(sample_array16[i+d])*hann_window[d];
		}

		else if (nb_bytes_per_sample == 4) {
			for(d = 0; d < WIN_SIZE; d++)
				x[d] = (float)(sample_array32[i+d])*hann_window[d];
		}

		av_rdft_calc(fft, x);
		
		for(d = 1; d < WIN_SIZE/2; ++d) { // 1?
			float re = x[d*2];
			float im = x[d*2+1];
			float raw = re*re + im*im;
			spectre_moy[d] += raw;
		} 
		spectre_moy[0] = x[0]*x[0];
	}	

	for(d=1;d<=WIN_SIZE/2;++d) {
		spectre_moy[d] /= WIN_SIZE;
		spectre_moy[d] = sqrt(spectre_moy[d]);
		peak = spectre_moy[d] < peak ? peak : spectre_moy[d];
	}

	for(d=1;d<=WIN_SIZE/2;++d) {
		float x = spectre_moy[d]/peak;
		spectre_moy[d] = 20*log10(x)-3;
	}

	pas_freq = sample_rate/WIN_SIZE;

	if (debug)
		for(d=1;d<WIN_SIZE/2;++d) {
			freq+=pas_freq;
			fprintf(file1, "%d\n", freq);
			fprintf(file2, "%f\n", spectre_moy[d]);
			if(freq > 20000) // 10000 pour tracer
			break;
		}

	tab_bandes[0] = (spectre_moy[1]+spectre_moy[2])/2;
	tab_bandes[1] = (spectre_moy[3]+spectre_moy[4])/2;
	for(i=5;i<=30;++i)
		tab_bandes[2]+=spectre_moy[i];
	tab_bandes[2]/=(29-4);
	for(i=31;i<=59;++i)
		tab_bandes[3]+=spectre_moy[i];
	tab_bandes[3]/=(58-30);
	for(i=60;i<=117;++i)
		tab_bandes[4]+=spectre_moy[i];
	tab_bandes[4]/=(116-59);
	tab_sum = tab_bandes[3] + tab_bandes[4] - (tab_bandes[1] + tab_bandes[0]);

	if (tab_sum > -47.)
		resnum_freq = 2;
	else if (tab_sum > -51.)
		resnum_freq = 1;
	else if (tab_sum > -55)
		resnum_freq = -1;
	else
		resnum_freq = -2;

if (debug) {
	printf("-> Debug fréquentiel\n");
	printf("Basses fréquences: %f\n", tab_bandes[0]);
	printf("Moyennes-basses fréquences: %f\n", tab_bandes[1]);
	printf("Moyennes fréquences: %f\n", tab_bandes[2]); // Marche bien pour Combichrist (?) (27.1 = no strict) TODO
	printf("Moyennes-hautes fréquences: %f\n", tab_bandes[3]);
	printf("Hautes fréquences: %f\n", tab_bandes[4]);
	printf("Critères: fort > -47 > -51 > -55 > doux\n");
	printf("Somme: %f\n", tab_sum);
	printf("Résultat fréquences: %d\n", resnum_freq);
}

	return (resnum_freq);
}
