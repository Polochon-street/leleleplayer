#include "analyze.h"

float analyze (char *filename, struct song *current_song) {
	float resnum_freq;
	float resnum_amp;
	float resnum_env;
	int i, d;
	float resnum;

	if(audio_decode(filename, current_song) == 0) { // Decode audio track
		current_song->force_vector.x = envelope_sort(*current_song); // Attack sort // TODO better implementation of final sort
		current_song->force_vector.y = amp_sort(*current_song); // Amplitude sort
		current_song->force_vector.z = freq_sort(*current_song); // Freq sort 
		resnum = current_song->force_vector.x + current_song->force_vector.y + current_song->force_vector.z; 

		return resnum;
		if(debug)
			printf("FINAL RESULT: %f\n", resnum);
	}
	else
		return 0;
}

