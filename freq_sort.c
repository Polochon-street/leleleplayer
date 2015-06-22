#include <math.h>
#include <libavcodec/avfft.h>
#include "analyze.h"

#define WIN_BITS 9 // 9
#define WIN_SIZE (1 << WIN_BITS)

#define GRAVE_INF 2
#define GRAVE_SUP 4
#define AIGU_INF 17
#define AIGU_SUP 104

float freq_sort(struct song song) {
	float hann_window[WIN_SIZE];
	int Samples;
	FFTSample *spectre_moy;
	float tab_bandes[4];
	float tab_sum;
	int nFrames;
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
	float resnum_freq = 0;

	peak=0;
	for(i = 0; i < WIN_SIZE; ++i)
		hann_window[i] = .5f - .5f*cos(2*M_PI*i/(WIN_SIZE-1));

	spectre_moy = (FFTSample*)av_malloc((WIN_SIZE*sizeof(FFTSample)));

	for(i=0;i<=WIN_SIZE/2;++i)
		spectre_moy[i] = 0.0f;

	for(i=0;i<11;++i)
		tab_bandes[i] = 0.0f;
	
	Samples = song.nSamples;
	Samples /= song.channels; // Only one channel


	if(Samples%WIN_SIZE > 0)
		Samples -= Samples%WIN_SIZE;

	nFrames = Samples/WIN_SIZE;

	x = (FFTSample*)av_malloc(WIN_SIZE*sizeof(FFTSample));
	
	fft = av_rdft_init(WIN_BITS, DFT_R2C);

	for(i=0, iFrame = 0; iFrame < nFrames; i+=song.channels*WIN_SIZE, iFrame++) {
		if (song.nb_bytes_per_sample == 2) {
			for(d = 0; d < WIN_SIZE; d++)
				x[d] = (float)((((int16_t*)song.sample_array)[i+2*d] + ((int16_t*)song.sample_array)[i+2*d+1])/2)*hann_window[d]; 
		}

		else if (song.nb_bytes_per_sample == 4) {
			for(d = 0; d < WIN_SIZE; d++) 
				x[d] = (float)(( ((int32_t*)song.sample_array)[i+2*d] + ((int32_t*)song.sample_array)[i+2*d+1] ) / 2)*hann_window[d];
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

	pas_freq = song.sample_rate/WIN_SIZE;

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
	tab_sum = tab_bandes[4] + tab_bandes[3] + tab_bandes[2] - tab_bandes[0] - tab_bandes[1];

	if (tab_sum > -66.1)
		resnum_freq = 2;
	else if (tab_sum > -68.)
		resnum_freq = 1;
	else if (tab_sum > -71)
		resnum_freq = -1;
	else
		resnum_freq = -2;
	
	resnum_freq = ((float)1/3)*tab_sum + ((float)68/3);
	
	if (debug) {
		printf("-> Freq debug\n");
		printf("Low frequencies: %f\n", tab_bandes[0]);
		printf("Mid-low frequencices: %f\n", tab_bandes[1]);
		printf("Mid frequencies: %f\n", tab_bandes[2]); // Marche bien pour Combichrist (?) (27.1 = no strict) TODO
		printf("Mid-high frequencies: %f\n", tab_bandes[3]);
		printf("High frequencies: %f\n", tab_bandes[4]);
		printf("Criterion: Loud > -66.1 > -68 > -71 > Calm\n");
		printf("Sum: %f\n", tab_sum);
		printf("Freq result: %f\n", resnum_freq);
	}
	//resnum_freq = -2*(tab_sum + 68.0f)/(tab_sum - 68.0f); 
	return (resnum_freq); 
}
