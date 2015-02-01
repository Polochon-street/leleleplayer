#include "analyse.h"

#define max( a, b ) ( ((a) > (b)) ? (a) : (b) )
#define QUANT_FREQ 44100
#define DECR_SPEED ( 32768 / (int)( ((float)1/2)*QUANT_FREQ)) 

float envelope_sort(int16_t* sample_array) {
	int i, g;
	FILE *file_env;
	int16_t *enveloppe;
	int16_t *dEnveloppe;
	float peaklength = 0;
	int passe;
	enveloppe = (int16_t*)malloc(nSamples*sizeof(int16_t));
	dEnveloppe = (int16_t*)malloc(nSamples*sizeof(int16_t));

	for(i = 0; i < nSamples-1; ++i)
		enveloppe[i] = 0;
	
	file_env = fopen("file_env.txt", "w");
	for(i = 1; i < nSamples-1; ++i) {
		/* if(enveloppe[i-1] - DECR_SPEED > abs(sample_array[i]))
			enveloppe[i] = enveloppe[i-1];
		else
			enveloppe[i] = abs(sample_array[i]); */
		//printf("%d, %d, %d\n", enveloppe[i-1] - DECR_SPEED, enveloppe[i], sample_array[i]);
		enveloppe[i] = max(enveloppe[i-1] - DECR_SPEED, abs(sample_array[i]));	
	}	

/*	for(i = 0; i < nSamples-1; ++i)
		enveloppe_temp[i] = enveloppe[i];

	for(g=0;g<=passe;++g) {
		enveloppe_smooth[0]=enveloppe_temp[0];
		enveloppe_smooth[1]=(float)1/4*(enveloppe_temp[0]+2*enveloppe_temp[1]+enveloppe_temp[2]);
		enveloppe_smooth[2]=(float)1/9*(enveloppe_temp[0]+2*enveloppe_temp[1]+3*enveloppe_temp[2]+2*enveloppe_temp[3]+enveloppe_temp[4]);
		for(i=3;i<=nSamples-5;++i)
			enveloppe_smooth[i]=(float)1/27*(enveloppe_temp[i-3]+enveloppe_temp[i-2]*3+6*enveloppe_temp[i-1]+7*enveloppe_temp[i]+6*enveloppe_temp[i+1]+enveloppe_temp[i+2]*3+enveloppe_temp[i+3]);
		for(i=3;i<=nSamples-5;++i)
			enveloppe_temp[i]=enveloppe_smooth[i];
	} */
	
	for(i = 1; i <= nSamples-1; ++i)
		dEnveloppe[i-1] = enveloppe[i+1] - enveloppe[i-1]; 
	for(i = 1; i <= nSamples-1; ++i)
		if(dEnveloppe[i] >= 500) {
			++peaklength;
			while(dEnveloppe[i] >= 500)
				++i;
		}

	peaklength/=nSamples;
	printf("%f\n", peaklength);

	for(i = 44100*24; i < 44100*36;++i)
		fprintf(file_env, "%d\n", dEnveloppe[i]);
}
