#include "analyze.h"

float analyze (char *filename, struct song *current_song) {
	float resnum;
	struct d2vector envelope_result;

	if(audio_decode(filename, current_song) == 0) { // Decode audio track
		envelope_result = envelope_sort(*current_song); // Global envelope sort
		current_song->force_vector.x = envelope_result.x; // Tempo sort
		current_song->force_vector.y = amp_sort(*current_song); // Amplitude sort
		current_song->force_vector.z = freq_sort(*current_song); // Freq sort 
		current_song->force_vector.t = envelope_result.y;
		

		resnum = MAX(current_song->force_vector.x, 0) + current_song->force_vector.y + current_song->force_vector.z + MAX(current_song->force_vector.t, 0); 
		return resnum;
	}
	else
		return 0;
}

