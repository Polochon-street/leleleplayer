#include "analyze.h"

void lelele_free_song(struct song *song) {
	if(song->artist) {
		free(song->artist);
		song->artist = NULL;
	}
	if(song->title) {
		free(song->title);
		song->title = NULL;
	}
	if(song->album) {
		free(song->album);
		song->album = NULL;
	}
	if(song->tracknumber) {
		free(song->tracknumber);
		song->tracknumber = NULL;
	}
	if(song->sample_array) {
		free(song->sample_array);
		song->sample_array = NULL;
	}
}

float lelele_analyze(char *filename, struct song *current_song, int debug, int analyze) { // add debug flag, and only decode flag
	float resnum;
	struct d2vector envelope_result;

	if(debug)
		printf("\nAnalyzing: %s\n\n", filename);

	if(audio_decode(filename, current_song) == 0) { // Decode audio track
		if(analyze) {
			envelope_result = envelope_sort(*current_song, debug); // Global envelope sort
			current_song->force_vector.x = envelope_result.x; // Tempo sort
			current_song->force_vector.y = amp_sort(*current_song, debug); // Amplitude sort
			current_song->force_vector.z = freq_sort(*current_song, debug); // Freq sort 
			current_song->force_vector.t = envelope_result.y; // Attack sort

			resnum = MAX(current_song->force_vector.x, 0) + current_song->force_vector.y + current_song->force_vector.z + MAX(current_song->force_vector.t, 0); 

			if(debug)
				printf("\n-> Final Result: %f\n", resnum);

			if(resnum > 0) {
				if(debug)
					printf("Loud\n");
				return 0;
			}
			if(resnum < 0) {
				if(debug)
					printf("Calm\n");
				return 1;
			}
			else {
				printf("Couldn't conclude\n");
				return 2;
			}
		}
		else
			return 2;
	}
	else {
		printf("Couldn't decode song\n");
		return 3;
	}
}

