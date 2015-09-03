#include <stdio.h>
#include "analyze.h"

int main(int argc, char **argv) {
	char *filename = argv[1];
	float result;

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
	struct song song;

	result = analyze(filename, &song);
	free_song(&song);
	if(debug)
		printf("\n-> Final Result: %f\n", result);

	if(result > 0) {
		if(debug)
			printf("Loud\n");
		return 0;
	}
	if(result < 0) {
		if(debug)
			printf("Calm\n");
		return 1;
	}
	else
		printf("Couldn't conclude\n");
}

