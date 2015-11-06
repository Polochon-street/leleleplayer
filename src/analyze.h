#include <stdio.h>
#include <libavformat/avformat.h>
#include <gst/gst.h>

size_t size;
int cli;

struct d4vector {
	float x; // Tempo rating
	float y; // Amplitude rating
	float z; // Frequency rating
	float t; // Attack rating
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
	int bitrate;
	int nb_bytes_per_sample;
	GstElement *playbin;
	GstState state;
	gint64 duration;
	gint64 current;
	int resnum;
	char *filename;
	char *artist;
	char *title;
	char *album;
	char *tracknumber;
	char *genre;
};

float amp_sort(struct song, int debug);
struct d2vector envelope_sort(struct song, int debug);
int audio_decode(const char *file, struct song *, int analyze);
float lelele_analyze(char *filename, struct song *, int debug, int analyze); 
float freq_sort(struct song, int debug);
int gui(int argc, char **argv);
void lelele_free_song(struct song *);
