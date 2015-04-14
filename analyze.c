#include "analyze.h"

int analyze (char *filename, int8_t *current_sample_array) {
	float resnum_freq;
	float resnum_amp;
	float resnum_env;
	int i, d;
	float resnum;

	debug = 1;

	current_sample_array = audio_decode(&current_song, filename); // Decode audio track
//	resnum_env = envelope_sort((int16_t*)current_sample_array); // Attack sort // TODO better implementation of final sort
//	resnum_amp = amp_sort((int16_t*)current_sample_array); // Amplitude sort
	resnum_freq = freq_sort((int16_t*)current_sample_array); // Freq sort 

	resnum = resnum_amp + resnum_freq + resnum_env; 

	if(debug)
		printf("FINAL RESULT: %f\n", resnum);
	if(resnum == 0) {
		if(debug)
			printf("Can't conclude...\n");
		return 2;
	}
	else if(resnum > 0) {
		if(debug)
			printf("Loud\n");
		return 0;
	}
	else if(resnum < 0) {
		if(debug)
			printf("Calm\n");
		return 1;
	}	
}

