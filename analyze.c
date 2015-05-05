#include "analyze.h"

float analyze (char *filename, struct song *current_song) {
	float resnum_freq;
	float resnum_amp;
	float resnum_env;
	int i, d;
	float resnum;

	if(audio_decode(filename, current_song) == 0) { // Decode audio track
		resnum_env = envelope_sort(*current_song); // Attack sort // TODO better implementation of final sort
		resnum_amp = amp_sort(*current_song); // Amplitude sort
		resnum_freq = freq_sort(*current_song); // Freq sort 

		resnum = resnum_amp + resnum_freq + resnum_env; 

		return resnum;
		if(debug)
			printf("FINAL RESULT: %f\n", resnum);
	}
	else
		return 0;
}

