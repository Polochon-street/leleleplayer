#include "analyze.h"
#include <libavformat/coucou/internal.h>

int audio_decode(const char *filename, struct song *song) { // decode the track
	AVCodec *codec = NULL;
	AVCodecContext *c = NULL;
	AVFormatContext *pFormatCtx;
	
	int i, d, e;
	int len;
	int planar;
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
		printf("Couldn't open file: %s, %d\n", filename, errno);
		song->nSamples = 0;
		return 1;
	}

	if(avformat_find_stream_info(pFormatCtx, NULL) < 0) {
		printf("Couldn't find stream information\n");
		song->nSamples = 0;
		return 1;
	} 
	
	audioStream = av_find_best_stream(pFormatCtx, AVMEDIA_TYPE_AUDIO, -1, -1, &codec, 0);
	c = pFormatCtx->streams[audioStream]->codec;
	
	if (!codec) {
		printf("Codec not found!\n");
		song->nSamples = 0;
		return 1;
	}

	if(avcodec_open2(c, codec, NULL) < 0) {
		printf("Could not open codec\n");
		song->nSamples = 0;
		return 1;
	}
	
	song->sample_rate = c->sample_rate;
	song->duration = pFormatCtx->duration/AV_TIME_BASE;
	size = (((uint64_t)(pFormatCtx->duration)*(uint64_t)song->sample_rate)/(uint64_t)AV_TIME_BASE)*c->channels*av_get_bytes_per_sample(c->sample_fmt);
	song->nSamples = (((uint64_t)(pFormatCtx->duration)*(uint64_t)song->sample_rate)/(uint64_t)AV_TIME_BASE)*c->channels;
	song->sample_array = malloc(size);

	for(i = 0; i < size; ++i)
		song->sample_array[i] = 0;

	beginning = song->sample_array;
	index = 0;

	planar = av_sample_fmt_is_planar(c->sample_fmt);
	song->nb_bytes_per_sample = av_get_bytes_per_sample(c->sample_fmt);

	song->channels = c->channels;
	
	song->artist = song->tracknumber = song->title = song->album = NULL;

	/* Get tags */
	if(av_dict_get(pFormatCtx->metadata, "track", NULL, AV_DICT_IGNORE_SUFFIX) != NULL) {
		song->tracknumber = malloc(strlen(av_dict_get(pFormatCtx->metadata, "TRACK", NULL, AV_DICT_IGNORE_SUFFIX)->value) + 1);
		strcpy(song->tracknumber, av_dict_get(pFormatCtx->metadata, "TRACK", NULL, AV_DICT_IGNORE_SUFFIX)->value);
		song->tracknumber[strcspn(song->tracknumber, "/")] = '\0';
	}
	else {
		song->tracknumber = malloc(1*sizeof(char));
		strcpy(song->tracknumber, "");
	}

	if(av_dict_get(pFormatCtx->metadata, "title", NULL, AV_DICT_IGNORE_SUFFIX) != NULL) {
		song->title = malloc(strlen(av_dict_get(pFormatCtx->metadata, "TITLE", NULL, AV_DICT_IGNORE_SUFFIX)->value) + 1);
		strcpy(song->title, av_dict_get(pFormatCtx->metadata, "TITLE", NULL, AV_DICT_IGNORE_SUFFIX)->value);
	}
	else {
		song->title = malloc(12*sizeof(char));
		strcpy(song->title, "<no title>");
	}

	if(av_dict_get(pFormatCtx->metadata, "ARTIST", NULL, AV_DICT_IGNORE_SUFFIX) != NULL) {
		song->artist= malloc(strlen(av_dict_get(pFormatCtx->metadata, "ARTIST", NULL, AV_DICT_IGNORE_SUFFIX)->value) + 1);
		strcpy(song->artist, av_dict_get(pFormatCtx->metadata, "ARTIST", NULL, AV_DICT_IGNORE_SUFFIX)->value);
	}
	else {
		song->artist= malloc(12*sizeof(char));
		strcpy(song->artist, "<no artist>");
	}

	if(av_dict_get(pFormatCtx->metadata, "ALBUM", NULL, AV_DICT_IGNORE_SUFFIX) != NULL) {
		song->album= malloc(strlen(av_dict_get(pFormatCtx->metadata, "ALBUM", NULL, AV_DICT_IGNORE_SUFFIX)->value) + 1);
		strcpy(song->album, av_dict_get(pFormatCtx->metadata, "ALBUM", NULL, AV_DICT_IGNORE_SUFFIX)->value);
	}
	else {
		song->album= malloc(11*sizeof(char));
		strcpy(song->album, "<no album>");
	}

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
	
			if(len < 0)
				avpkt.size = 0;

			av_free_packet(&avpkt);

			/* interesting part: copying decoded data into a huge array */
			/* flac has a different behaviour from mp3, hence the planar condition */
			if(got_frame) {
				size_t data_size = av_samples_get_buffer_size(NULL, c->channels, decoded_frame->nb_samples, c->sample_fmt, 1); 

				if(index*song->nb_bytes_per_sample + data_size > size) {
					beginning = realloc(beginning, (size += data_size));
					song->nSamples += data_size/song->nb_bytes_per_sample;
				}
				int8_t *p = beginning+index*song->nb_bytes_per_sample;
				if(planar == 1) {
					for(i = 0; i < decoded_frame->nb_samples*song->nb_bytes_per_sample; i += song->nb_bytes_per_sample) { 
						for(e = 0; e < c->channels; ++e)
							for(d = 0; d < song->nb_bytes_per_sample; ++d) 
								*(p++) = ((int8_t*)(decoded_frame->extended_data[e]))[i+d];
					}
					index += data_size/song->nb_bytes_per_sample;
				}
				else if(planar == 0) {
					memcpy(index*song->nb_bytes_per_sample + beginning, decoded_frame->extended_data[0], data_size);
	        		index += data_size/song->nb_bytes_per_sample; 
				}
			}
		}
		else
			av_free_packet(&avpkt); /* Dropping the damn album cover */
	}
	song->sample_array = beginning;

	/* cleaning memory */
	avpkt.data = NULL;
	avpkt.size = 0;

	do {
		avcodec_decode_audio4(c, decoded_frame, &got_frame, &avpkt);
	} while(got_frame); 

	avcodec_close(c);
	av_frame_unref(decoded_frame);
	av_frame_free(&decoded_frame);
	av_free_packet(&avpkt);
	avformat_close_input(&pFormatCtx);

	return 0;
} 
