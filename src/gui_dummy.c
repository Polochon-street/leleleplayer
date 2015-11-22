#include <stdio.h>
#include <bliss.h>

int main(int argc, char **argv) {
	char *filename = argv[1];
	int result;
	struct bl_song song;
	song.artist = song.title = song.album = song.tracknumber = song.sample_array = NULL;

	if(argc <= 1) {
		printf("Usage: ./analyse [--debug] AUDIO_FILE\nReturns 1 if the song is calm enough to sleep, 0 otherwise\n");
			return -1;
	}
	else if(argc == 3) {
		if(strcmp(argv[1], "--debug") == 0 || strcmp(argv[1], "-d") == 0) {
			filename = argv[2];
			result = bl_analyze(filename, &song);
		}
		else {
			printf("Usage: ./analyse [--debug] AUDIO_FILE\nReturns 1 if the song is calm enough to sleep, 0 otherwise\n");
			return 2;
		}	
	}
	else if(argc == 2) {
		result = bl_analyze(filename, &song);
	}
	else {
		printf("Usage: ./analyse [--debug] AUDIO_FILE\nReturns 1 if the song is calm enough to sleep, 0 otherwise\n");
		return 2;
	}

	bl_free_song(&song);
	return result;
}

