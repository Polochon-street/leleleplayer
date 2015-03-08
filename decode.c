#define INBUF_SIZE 4096
#define AUDIO_INBUF_SIZE 20480
#define AUDIO_REFILL_THRESH 4096

#include "analyze.h"

int8_t *audio_decode(int8_t* sample_array, const char *filename) { // decode the track
	AVCodec *codec = NULL;
	AVCodecContext *c = NULL;
	AVFormatContext *pFormatCtx;
	int i, d;
	int len;
	AVPacket avpkt;
	AVFrame *decoded_frame = NULL;
	int8_t *beginning;
	int got_frame;
	int audioStream;
	size_t index;

	av_register_all();
	av_init_packet(&avpkt);

	pFormatCtx = avformat_alloc_context();

	if(avformat_open_input(&pFormatCtx, filename, NULL, NULL) < 0) {
		printf("Couldn't open file\n");
		exit(1);
	}

	if(avformat_find_stream_info(pFormatCtx, NULL) < 0) {
		printf("Couldn't find stream information\n");
		exit(1);
	} 
	
	audioStream = av_find_best_stream(pFormatCtx, AVMEDIA_TYPE_AUDIO, -1, -1, &codec, 0);
	c = pFormatCtx->streams[audioStream]->codec;

	if (!codec) {
		printf("Codec not found!\n");
		exit(1);
	}

	if(avcodec_open2(c, codec, NULL) < 0) {
		printf("Could not open codec\n");
		exit(1);
	}

	sample_rate = c->sample_rate;
	duration = pFormatCtx->duration/AV_TIME_BASE+1;
	size = (((uint64_t)(pFormatCtx->duration)*(uint64_t)sample_rate)/(uint64_t)AV_TIME_BASE)*c->channels*av_get_bytes_per_sample(c->sample_fmt);
	nSamples = (((uint64_t)(pFormatCtx->duration)*(uint64_t)sample_rate)/(uint64_t)AV_TIME_BASE)*c->channels;
	sample_array = malloc(size);

	beginning = sample_array;
	index = 0;

	planar = av_sample_fmt_is_planar(c->sample_fmt);
	nb_bytes_per_sample = av_get_bytes_per_sample(c->sample_fmt);

	/* End of codec init */
	while(av_read_frame(pFormatCtx, &avpkt) >= 0) {
		if(avpkt.stream_index == audioStream) {
			got_frame = 0; 
	
			if(!decoded_frame) {
				if(!(decoded_frame = av_frame_alloc())) {
					printf("Could not allocate audio frame\n");
					exit(1);
				}
		
			}
			else 
				av_frame_unref(decoded_frame);

			len = avcodec_decode_audio4(c, decoded_frame, &got_frame, &avpkt);
	
			if(len < 0) {
				//printf("Error while decoding\n");
				//exit(1);
				avpkt.size=0;
			}
			/* interesting part: copying decoded data into a huge array */
			/* flac has a different behaviour from mp3, hence the planar condition */
			if(got_frame) {
				size_t data_size = av_samples_get_buffer_size(NULL, c->channels, decoded_frame->nb_samples, c->sample_fmt, 1); 

				if(index*nb_bytes_per_sample + data_size > size) {
					beginning = realloc(beginning, (size+=data_size));
					nSamples+=data_size/nb_bytes_per_sample;
				}
				int8_t *p = beginning+index*nb_bytes_per_sample;
				
				if(planar == 1) {
					for(i = 0; i < decoded_frame->nb_samples*nb_bytes_per_sample; i+=nb_bytes_per_sample) { 
						for(d = 0; d < nb_bytes_per_sample; ++d) 
							*(p++) = ((int8_t*)(decoded_frame->extended_data[0]))[i+d];
						for(d = 0; d < nb_bytes_per_sample; ++d) 
							*(p++) = ((int8_t*)(decoded_frame->extended_data[1]))[i+d];
					}
					index+=data_size/nb_bytes_per_sample;
				}
				else if(planar == 0) {
					memcpy(index*nb_bytes_per_sample+beginning, decoded_frame->extended_data[0], data_size);
	        		index+=data_size/nb_bytes_per_sample; 
				}
			} 
		}
	}
	/* cleaning memory */
	avcodec_close(c);
	av_free(c);
	//avformat_close_input(&pFormatCtx);
	av_frame_free(&decoded_frame);
	return beginning;
} 
