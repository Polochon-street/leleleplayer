#define INBUF_SIZE 4096
#define AUDIO_INBUF_SIZE 20480
#define AUDIO_REFILL_THRESH 4096

#include "analyse.h"

int16_t *audio_decode(int16_t* sample_array, const char *filename) { // decode the track
	AVCodec *codec = NULL;
	AVCodecContext *c = NULL;
	AVFormatContext *pFormatCtx;
	int i;
	int len;
	AVPacket avpkt;
	AVFrame *decoded_frame = NULL;
	int16_t *beginning;
	int got_frame;
	int audioStream;
	

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
	size = (pFormatCtx->duration)*(sample_rate)*c->channels*sizeof(int16_t)/AV_TIME_BASE;
	
	nSamples = (pFormatCtx->duration)*(sample_rate)*c->channels/AV_TIME_BASE;

	sample_array = (int16_t*)malloc(10*size*c->channels);
	

	beginning = sample_array;

	planar = av_sample_fmt_is_planar(c->sample_fmt);
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
				int data_size = av_samples_get_buffer_size(NULL, c->channels, decoded_frame->nb_samples, c->sample_fmt, 1); 
				int16_t *p = sample_array;
				if(planar == 1) {
					for(i = 0; i < data_size; i++) { 
						*(p++) = ((int16_t*)(decoded_frame->extended_data[0]))[i];
						*(p++) = ((int16_t*)(decoded_frame->extended_data[1]))[i];
					}
					sample_array+=data_size/2; // WHY /2? TODO
				}
				else if(planar == 0) {
				memcpy(sample_array, decoded_frame->extended_data[0], data_size);
	        	sample_array+=data_size/2;
				}
			} 
		}
	}
	/* cleaning memory */
	//fclose(f);
	avcodec_close(c);
	av_free(c);
	//avformat_close_input(&pFormatCtx);
	av_frame_free(&decoded_frame);
	return beginning;
} 
