#include "analyze.h"

float analyze (char *filename, struct song *current_song) {
	float resnum;

	if(audio_decode(filename, current_song) == 0) { // Decode audio track
		current_song->force_vector.x = envelope_sort(*current_song); // Attack sort // TODO better implementation of final sort
		current_song->force_vector.y = amp_sort(*current_song); // Amplitude sort
		current_song->force_vector.z = freq_sort(*current_song); // Freq sort 

		resnum = MAX(current_song->force_vector.x, 0) + current_song->force_vector.y + current_song->force_vector.z; 
		return resnum;
	}
	else
		return 0;
}

