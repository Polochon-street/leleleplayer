#include <stdio.h>
#include <libavformat/avformat.h>
//#include <libavcodec/avcodec.h>

//int16_t *current_sample_array;
size_t size;
int nSamples;
int duration;
int sample_rate;
int nb_bytes_per_sample;
int debug;
int planar;

float amp_sort(int16_t* array);
float envelope_sort(int16_t* array);
int16_t* audio_decode(int16_t *array, const char *file);
float freq_sort(int16_t *array);

