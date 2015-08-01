#include <stdio.h>
#include <libavformat/avformat.h>
#include <gst/gst.h>
//#include <libavcodec/avcodec.h>

//int16_t *current_sample_array;
size_t size;
//int8_t *current_sample_array;
//int8_t *next_sample_array;
int debug;
int cli;

struct d4vector {
	float x;
	float y;
	float z;
	float t;
};

struct d3vector {
	float x;
	float y;
	float z;
};

struct d2vector {
	float x;
	float y;
};

struct song {
	float force;
	struct d4vector force_vector;
	int8_t* sample_array;
	int channels;
	int nSamples;
	int sample_rate;
	int nb_bytes_per_sample;
	int planar;
	GstElement *playbin;
	GstState state;
	gint64 duration;
	gint64 current;
	char *filename;
	char *artist;
	char *title;
	char *album;
	char *tracknumber;
};

//struct song current_song;
struct song next_song;

float amp_sort(struct song);
struct d2vector envelope_sort(struct song);
int audio_decode(const char *file, struct song *);
float analyze(char *filename, struct song *); 
float freq_sort(struct song);
int gui(int argc, char **argv);
