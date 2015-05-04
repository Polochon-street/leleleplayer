#include "analyze.h"

int analyze (char *filename) {
	float resnum_freq;
	float resnum_amp;
	float resnum_env;
	int i, d;
	float resnum;
	struct song current_song;

	if(audio_decode(filename, &current_song) == 0) { // Decode audio track
		resnum_env = envelope_sort(current_song); // Attack sort // TODO better implementation of final sort
		resnum_amp = amp_sort(current_song); // Amplitude sort
		resnum_freq = freq_sort(current_song); // Freq sort 

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
	else
		return 3;
}

