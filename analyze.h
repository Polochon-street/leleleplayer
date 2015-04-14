#include <stdio.h>
#include <libavformat/avformat.h>
//#include <libavcodec/avcodec.h>

//int16_t *current_sample_array;
size_t size;
int nSamples;
int sample_rate;
int nb_bytes_per_sample;
int8_t *current_sample_array;
int8_t *next_sample_array;
int debug;
int channels;
int cli;
int planar;

struct song {
	float duration;
	int seconds;
	int minutes;
	char *artist;
	char *title;
	char *album;
	char *tracknumber;
};

struct song current_song;
struct song next_song;

float amp_sort(int16_t* array);
float envelope_sort(int16_t* array);
int8_t* audio_decode(struct song *, const char *file);
int analyze(char *filename, int8_t *);
float freq_sort(int16_t *array);
int gui(int argc, char **argv);
