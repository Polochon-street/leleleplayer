#include "analyse.h"

int main (int argc, char **argv) {
	int16_t *current_sample_array;
	int resnum_freq;
	int resnum_amp;
	int resnum_env;
	char resnum;
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

	resnum_amp = amp_sort(current_sample_array); // Amplitude sort
	resnum_freq = freq_sort(current_sample_array); // Freq sort
	resnum_env = envelope_sort(current_sample_array);
	resnum = resnum_amp + resnum_freq;
	
	if(debug)
		printf("RÉSULTAT FINAL: %d\n", resnum);
	if(resnum == 0 || resnum == -1) {
		if(resnum_env == 1)
			return 1;
		else
			return 0;
	}
	else if(resnum >= 1) {
		if(debug)
			printf("Fort\n");
		return 0;
	}
	else if(resnum < -1) {
		if(debug)
			printf("Doux\n");
		return 1;
	}
}

