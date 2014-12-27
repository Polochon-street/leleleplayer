#include "analyse.h"
#define SIZE 32768
#define INT_INF 0
#define INT_SUP 2000
#define max( a, b ) ( ((a) > (b)) ? (a) : (b) )

float amp_sort(int16_t* sample_array) {
	int i, d, e, g;
	int histogram_count;
	float histogram[SIZE];
	float histogram_smooth[SIZE];
	float histogram_temp[SIZE];
	float histogram_integ;
	float moy, sd;
	int passe;
	int16_t* p16;
	FILE *file_amp; // prevents crumbling?
	int resnum_amp = 0;
	
	p16 = sample_array;
	for(i=0;i<SIZE;++i)
		histogram[i] = '\0';
	for(i=0;i<=SIZE;++i)
		histogram_smooth[i] = '\0';
	for(i=0;i<=SIZE;++i)
		histogram_temp[i] = '\0';

	passe = 300;
	histogram_count= 0;

	if (debug)
		file_amp = fopen("file_amp.txt", "w");

	for(d = 0; sample_array[d] == 0 ;++d)
		;
	for(e=nSamples-1; sample_array[e] == 0; --e)
		;
	
	for(i = d;i <= e; ++i) 
		++histogram[abs(*(p16++))];
	for(i=0;i<SIZE;++i)
		histogram_temp[i]=histogram[i];
	histogram_count+=e-d;

	for(i=0;i<SIZE;++i) {
		histogram[i]/=histogram_count;
		histogram[i]*=100.;
	}

	for(g=0;g<=passe;++g) {
		histogram_smooth[0]=histogram_temp[0];
		histogram_smooth[1]=(float)1/4*(histogram_temp[0]+2*histogram_temp[1]+histogram_temp[2]);
		histogram_smooth[2]=(float)1/9*(histogram_temp[0]+2*histogram_temp[1]+3*histogram_temp[2]+2*histogram_temp[3]+histogram_temp[4]);
		for(i=3;i<=SIZE-5;++i)
			histogram_smooth[i]=(float)1/27*(histogram_temp[i-3]+histogram_temp[i-2]*3+6*histogram_temp[i-1]+7*histogram_temp[i]+6*histogram_temp[i+1]+histogram_temp[i+2]*3+histogram_temp[i+3]);
		for(i=3;i<=SIZE-5;++i)
			histogram_temp[i]=histogram_smooth[i];
	}

	for(i=0;i<=SIZE;++i) {
		histogram_smooth[i]/=histogram_count; 
		histogram_smooth[i]*=100.;
		histogram_smooth[i]=fabs(histogram_smooth[i]);
	}

	for(i=0;i<=INT_SUP;++i)
		histogram_integ+=histogram_smooth[i];


	if(debug)
		for(i=0;i<SIZE;++i)
		fprintf(file_amp, "%d\n", sample_array[i]);

	if (histogram_integ < 25)
		resnum_amp = 2;
	else if (histogram_integ < 30)
		resnum_amp = 1;
	else if (histogram_integ < 35)
		resnum_amp = -1;
	else
		resnum_amp = -2;

	if (debug) {
	printf("-> Debug amplitudes\n");
	printf("Critères: fort < 25 < 30 < 35 < doux\n");
	printf("Intégration histogramme: %f\n", histogram_integ); 
	printf("Résultat amplitudes: %d\n", resnum_amp);
	}

	return (resnum_amp);
}
