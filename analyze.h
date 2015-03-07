#include <stdio.h>
#include <libavformat/avformat.h>
//#include <libavcodec/avcodec.h>

//int16_t *current_sample_array;
size_t size;
int nSamples;
int duration;
int sample_rate;
int nb_bytes_per_sample;
int8_t *current_sample_array;
int debug;
int cli;
int planar;

float amp_sort(int16_t* array);
float envelope_sort(int16_t* array);
int8_t* audio_decode(int8_t*array, const char *file);
int analyze(char *filename);
float freq_sort(int16_t *array);
int gui(int argc, char **argv);
