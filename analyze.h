#include <stdio.h>
#include <libavformat/avformat.h>
//#include <libavcodec/avcodec.h>

//int16_t *current_sample_array;
size_t size;
//int8_t *current_sample_array;
//int8_t *next_sample_array;
int debug;
int cli;

struct song {
	int8_t* sample_array;
	int channels;
	int nSamples;
	int sample_rate;
	int nb_bytes_per_sample;
	int planar;
	float duration;
	char *artist;
	char *title;
	char *album;
	char *tracknumber;
};

//struct song current_song;
struct song next_song;

float amp_sort(struct song);
float envelope_sort(struct song);
int audio_decode(const char *file, struct song *);
float analyze(char *filename, struct song *); 
float freq_sort(struct song);
int gui(int argc, char **argv);
