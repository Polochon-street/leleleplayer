#include "analyse.h"

int main (int argc, char **argv) {
	int8_t *current_sample_array;
	float resnum_freq;
	float resnum_amp;
	float resnum_env;
	int i, d;
	float resnum;
	char *filename = argv[1];

	debug = 0;
	if(argc <= 1) {
		printf("Usage: ./analyse [--debug] AUDIO_FILE\nReturns 1 if the song is calm enough to sleep, 0 otherwise\n");
		return -1;
	}

	if(argc == 3) {
		if(strcmp(argv[1], "--debug") == 0 || strcmp(argv[1], "-d") == 0) {
			filename = argv[2];
			debug = 1;
		}
		else {
			printf("Usage: ./analyse [--debug] AUDIO_FILE\nReturns 1 if the song is calm enough to sleep, 0 otherwise\n");
			return 2;
		}
	}

	current_sample_array = audio_decode(current_sample_array, filename); // Decode audio track
	resnum_env = envelope_sort((int16_t*)current_sample_array); // Attack sort // TODO better implementation of final sort
	resnum_amp = amp_sort((int16_t*)current_sample_array); // Amplitude sort
	resnum_freq = freq_sort((int16_t*)current_sample_array); // Freq sort 

	free(current_sample_array);
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

