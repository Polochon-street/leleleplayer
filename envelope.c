#include "analyse.h"
#define max( a, b ) ( ((a) > (b)) ? (a) : (b) )

void makeEnvelope(int16_t* sample_array, float* envelope, float attackS, float releaseS) {
	unsigned long i = 0;
	const float attack = pow(0.01f, 1.0f/(attackS*sample_rate));
	const float release = pow(0.01f, 1.0f/(releaseS*sample_rate));
	int16_t *p16;
	int d;
	
	p16 = sample_array;

	float e = 0;
	for(i = 0; i < nSamples; ++i) {
		float v = (float)abs((*(p16++)));
		float b = v > e ? attack : release;
		e = b*(e-v)+v;
		envelope[i] = e;
	}
}

void signalDownsample(float* dst, int dstLen, float* src, int srcLen) {
	unsigned long i, j;
	unsigned long ratio = srcLen/dstLen;

	for(j = 0; j < dstLen; ++j)
		dst[j] = 0.0f;

	for(i = 0; i < srcLen; ++i)
		dst[i/ratio] += src[i]/ratio;
}

void signalNormalize(float *signal, long length) {
	unsigned long i;
	float v;
	float m = 0.0f;
	for(i = 0; i < length; ++i) {
		float v = abs(signal[i]);
		if(v > m)
			m = v;
	}
	
	for(i = 0;i < length; ++i)
		signal[i] /= m;
}
		


float envelope_sort(int16_t* sample_array) {
	int i, d, e, g;
	float *envelope;
	int result = 0;
	FILE *file_env;
	const float analyzePrecision = 5.0f;
	unsigned long mSamples = analyzePrecision*nSamples/sample_rate;
	float downEnvelope[mSamples+1];
	float dDownEnvelope[mSamples-2];
	float dDownEnvelopeMax = 0.0f;
	float moy = 0;
	float peakLength = 0;
	
	file_env = fopen("file_env.txt", "w");

	envelope = (float*)malloc(nSamples*sizeof(float));

	makeEnvelope(sample_array, envelope, 5.0f, 10.0f);

	signalDownsample(downEnvelope, mSamples, envelope, nSamples);
	signalNormalize(downEnvelope, mSamples);

	for(i = 1; i < mSamples-1; ++i) {
		dDownEnvelope[i-1] = downEnvelope[i+1] - downEnvelope[i-1];
		if(dDownEnvelopeMax < fabs(dDownEnvelope[i-1]))
			dDownEnvelopeMax = dDownEnvelope[i-1];
	}

	for(i = 1; i < mSamples-1; ++i) {
		//dDownEnvelope[i-1] /= dDownEnvelopeMax;
		dDownEnvelope[i-1] = pow(dDownEnvelope[i-1], 2.0f)*(dDownEnvelope[i-1] > 0.0f ? 1.0f : -1.0f);
	}
		
	for(i = 1; i < mSamples-2; ++i) {
		moy += dDownEnvelope[i];
		fprintf(file_env, "%f\n", dDownEnvelope[i]);
	}

	for(i = 1; i < mSamples-2; ++i) {
		if(dDownEnvelope[i] >= 0.014) {
			for(d = i+1; dDownEnvelope[d] > 0; ++d) 
				peakLength++;
			for(d = i-1; dDownEnvelope[d] > 0; --d)
				peakLength++;
		}
	}			

	moy/=mSamples-1;

	if(moy*1000 > 1.)
		result+=2;
	if(peakLength/mSamples > 0.3)
		result+=2;

	if(debug) {
	printf("-> Debug attaque\n");
	printf("Moyenne dérivée enveloppe: %f\n", moy*1000);
	printf("Critère: fort > 1 > doux\n");
	printf("Proportion de pics: %f\n", peakLength/(mSamples-1));
	printf("Critère: fort > 0.2 > doux\n");
	printf("Envelope result: %d\n", result);
	}

	return result;
}


