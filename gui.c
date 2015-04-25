#include <stdio.h>
#include "gui.h"

static void destroy (GtkWidget *window, gpointer data) {
	ShutdownOpenAL();
	gtk_main_quit ();
}

bool InitOpenAL(void) {
	ALCdevice *Device = alcOpenDevice(NULL);
	if(!Device) {
		printf("Error while opening the device\n");
		//exit(1);
	}
	
	ALCcontext *Context = alcCreateContext(Device, NULL);
	if(!Context) {
		printf("Error while creating context\n");
		//exit(1);
	}

	if(!alcMakeContextCurrent(Context)) {
		printf("Error while activating context\n");
	//	exit(1);
	}
	return 0;
}

static void ShutdownOpenAL(void) {
	ALCcontext *Context = alcGetCurrentContext();
	ALCdevice *Device = alcGetContextsDevice(Context);

	alcMakeContextCurrent(NULL);
	alcDestroyContext(Context);
	alcCloseDevice(Device);
}

static void row_activated(GtkTreeView *treeview, GtkTreePath *path, GtkTreeViewColumn *column, struct arguments *argument) {
	GtkTreeModel *model;
	GtkTreeIter iter;
	char *tempfile;
	
	alDeleteSources(1, &argument->source);
	alGenSources(1, &argument->source);

	gtk_list_store_set(((struct arguments*)argument)->store, &(((struct arguments*)argument)->playing_iter), PLAYING, "", -1);
	model = gtk_tree_view_get_model(treeview);
	if(gtk_tree_model_get_iter(model, &iter, path)) {
		gtk_tree_model_get(model, &iter, AFILE, &tempfile, -1);
	}
	argument->playing_iter = iter;
	gtk_list_store_set(argument->store, &(argument->playing_iter), PLAYING, "▶", -1);

	next_sample_array = audio_decode(&next_song, tempfile);
	current_sample_array = next_sample_array;
	current_song = next_song;

	gtk_adjustment_configure(argument->adjust, 0, 0, current_song.duration, 1, 1, 1);
	gtk_adjustment_changed(argument->adjust);
	bufferize(next_sample_array, argument);
	argument->buffer_old = argument->buffer;

	play_song(argument);
	free(current_sample_array);
	if(gtk_tree_model_iter_next(model, &iter)) {
		gtk_tree_model_get(model, &iter, AFILE, &tempfile, -1);
		next_sample_array = audio_decode(&next_song, tempfile);
		bufferize(next_sample_array, argument);
	}
	argument->iter = iter;
}


static gboolean continue_track(gpointer argument) {
	GtkTreeModel *model;
	char *tempfile;
	int buffers;

	((struct arguments*)argument)->offset = 0;

	alDeleteBuffers(1, &(((struct arguments*)argument)->buffer_old)); // ?
	current_sample_array = next_sample_array;
	current_song = next_song;
	gtk_list_store_set(((struct arguments*)argument)->store, &(((struct arguments*)argument)->playing_iter), PLAYING, "", -1);
	((struct arguments*)argument)->playing_iter = ((struct arguments*)argument)->iter;
	gtk_list_store_set(((struct arguments*)argument)->store, &(((struct arguments*)argument)->playing_iter), PLAYING, "▶", -1);

	free(current_sample_array);
	((struct arguments*)argument)->buffer_old = ((struct arguments*)argument)->buffer;
	model = gtk_tree_view_get_model((GtkTreeView *)(((struct arguments*)argument)->treeview));
	do {
		alGetSourcei(((struct arguments*)argument)->source, AL_BUFFERS_PROCESSED, &buffers);
	}
	while(buffers == 0);

	alSourceUnqueueBuffers(((struct arguments*)argument)->source, 1, &(((struct arguments*)argument)->buffer_old));
	gtk_adjustment_configure(((struct arguments*)argument)->adjust, 0, 0, current_song.duration, 1, 1, 1);
	gtk_adjustment_changed(((struct arguments*)argument)->adjust);

	if(gtk_tree_model_iter_next(model, &(((struct arguments*)argument)->iter))) {
			gtk_tree_model_get(model, &(((struct arguments *)argument)->iter), AFILE, &tempfile, -1);
			next_sample_array = audio_decode(&next_song, tempfile);
			bufferize(next_sample_array, argument);
	}
}

int bufferize(int8_t *sample_array, struct arguments *argument) {
	ALenum format;
	int i;
	if(argument->first == 1) {
		InitOpenAL();
		alSourcei(argument->source, AL_BUFFER, 0);
		alDeleteSources(1, &argument->source);
		alGenSources(1, &argument->source);
	}
	argument->first = 0;
	alGenBuffers(1, &(argument->buffer));

	if(nb_bytes_per_sample == 1 && channels == 1)
		format = AL_FORMAT_MONO8;
	else if(nb_bytes_per_sample == 1 && channels == 2)
		format = AL_FORMAT_STEREO8;
	else if(nb_bytes_per_sample == 2 && channels == 1)
		format = AL_FORMAT_MONO16;
	else if(nb_bytes_per_sample == 2 && channels == 2)
		format = AL_FORMAT_STEREO16;
	else if(nb_bytes_per_sample == 4 && channels == 1)
		format = AL_FORMAT_MONO_FLOAT32;
	else if(nb_bytes_per_sample == 4 && channels == 2)
		format = AL_FORMAT_STEREO_FLOAT32;

	for(; ((int16_t*)sample_array)[nSamples] == 0; --nSamples)
		;
	if(nSamples % 2)
		nSamples--;

	alBufferData(argument->buffer, format, sample_array, nSamples * nb_bytes_per_sample, sample_rate);
	alSourceQueueBuffers(argument->source, 1, &(argument->buffer));
}

void play_song(struct arguments *argument) {
	float timef;
	int bytes;

	timef = bytes / (float)(sample_rate * channels * nb_bytes_per_sample);
	alGetSourcei(((struct arguments*)argument)->source, AL_BYTE_OFFSET, &bytes);
	alSourcePlay(argument->source);
	gtk_button_set_image((GtkButton*)(argument->toggle_button), gtk_image_new_from_file("./pause.svg"));

	g_source_remove(argument->tag);
	argument->bartag = g_timeout_add_seconds(1, timer_progressbar, argument);
	argument->tag = g_timeout_add_seconds(current_song.duration - timef, continue_track, argument);
}

void pause_song(struct arguments *argument) {
	alSourcePause(argument->source);
	argument->bartag = g_timeout_add_seconds(1, timer_progressbar, argument);
	gtk_button_set_image((GtkButton*)(argument->toggle_button), gtk_image_new_from_file("./play.svg"));
}

static void toggle(GtkWidget *button, struct arguments *argument) { 
		GtkTreeSelection *selection;
		GtkTreeModel *model;
		GtkTreePath *path;
		GtkTreeIter iter;
			
		alGetSourcei(argument->source, AL_SOURCE_STATE, &(argument->status));
		
		if(argument->status == AL_STOPPED || argument->status == 0) {
			selection = gtk_tree_view_get_selection((GtkTreeView*)(argument->treeview));
			gtk_tree_view_get_model((GtkTreeView*)(argument->treeview));
			gtk_tree_selection_get_selected(selection, &model, &iter);
			path = gtk_tree_model_get_path(model, &iter);

			row_activated((GtkTreeView*)(argument->treeview), path, NULL,argument); 
		}
		if(argument->status == AL_PLAYING) {
			pause_song(argument);
		}	
		if(argument->status == AL_PAUSED) {
			play_song(argument);
		}
}

static void next(GtkWidget *button, struct arguments *argument) {
	alSourceStop(argument->source);
	continue_track(argument);
	alSourcePlay(argument->source);
}

static timer_progressbar(gpointer argument) {
	int time;
	float timef;
	int bytes;
	int buffers;

	alGetSourcei(((struct arguments*)argument)->source, AL_BYTE_OFFSET, &bytes);
	
	timef = bytes / (float)(sample_rate * channels * nb_bytes_per_sample);

	alGetSourcei(((struct arguments*)argument)->source, AL_BUFFERS_PROCESSED, &buffers);

	alGetSourcei(((struct arguments*)argument)->source, AL_SOURCE_STATE, &((struct arguments*)argument)->status);
	alGetSourcei(((struct arguments*)argument)->source, AL_SEC_OFFSET, &time);
	if(((struct arguments*)argument)->status == AL_PLAYING) {
		gtk_adjustment_set_value (((struct arguments*)argument)->adjust, timef);
		gtk_adjustment_changed(((struct arguments*)argument)->adjust);
		g_source_remove(((struct arguments*)argument)->bartag);

		((struct arguments*)argument)->bartag = g_timeout_add_seconds(1, timer_progressbar, argument);
	}
}

static void slider_changed(GtkRange *progressbar, struct arguments *argument) {
	int time;
	float timef;
	int bytes;
	alGetSourcei(((struct arguments*)argument)->source, AL_SEC_OFFSET, &time);
	alGetSourcei(((struct arguments*)argument)->source, AL_BYTE_OFFSET, &bytes);
	
	timef = bytes / (float)(sample_rate * channels * nb_bytes_per_sample);

	if(fabs(gtk_adjustment_get_value(argument->adjust) - timef) > 0.005f) {
		alSourcei(argument->source, AL_BYTE_OFFSET, channels * (int) ((gdouble) gtk_adjustment_get_value(argument->adjust) * ((gdouble) (sample_rate * nb_bytes_per_sample))));
		g_source_remove(argument->tag);
		argument->tag = g_timeout_add_seconds((int) (((gdouble)current_song.duration) - ((gdouble)gtk_adjustment_get_value(argument->adjust))), continue_track, argument);
		argument->offset = gtk_adjustment_get_value(argument->adjust);
	} 
} 

static void setup_tree_view(GtkWidget *treeview) {
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;

	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes("", renderer, "text", PLAYING, NULL);
	gtk_tree_view_column_set_sort_column_id(column, PLAYING);
	gtk_tree_view_column_set_sizing(column, GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_fixed_width(column, 20);
	gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);

	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes("N°", renderer, "text", TRACKNUMBER, NULL);
	gtk_tree_view_column_set_resizable (column, TRUE);
	gtk_tree_view_column_set_sort_column_id(column, TRACKNUMBER);
	gtk_tree_view_column_set_sizing(column, GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_fixed_width(column, 50);
	gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);

	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes("Track", renderer, "text", TRACK, NULL);
	gtk_tree_view_column_set_resizable (column, TRUE);
	gtk_tree_view_column_set_sort_column_id(column, TRACK);
	gtk_tree_view_column_set_sizing(column, GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_fixed_width(column, 300);
	gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);


	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes("Album", renderer, "text", ALBUM, NULL);
	gtk_tree_view_column_set_resizable (column, TRUE);
	gtk_tree_view_column_set_sort_column_id(column, ALBUM);
	gtk_tree_view_column_set_sizing(column, GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_fixed_width(column, 150);
	gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);

	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes("Artist", renderer, "text", ARTIST, NULL);
	gtk_tree_view_column_set_resizable (column, TRUE);
	gtk_tree_view_column_set_sort_column_id(column, ARTIST);
	gtk_tree_view_column_set_sizing(column, GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_fixed_width(column, 150);
	gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);

	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes("Force", renderer, "text", FORCE, NULL);
	gtk_tree_view_column_set_resizable (column, TRUE);
	gtk_tree_view_column_set_sort_column_id(column, FORCE);
	gtk_tree_view_column_set_sizing(column, GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_fixed_width(column, 70);
	gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);

	gtk_tree_view_columns_autosize(GTK_TREE_VIEW(treeview));
	/*renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes("File", renderer, "text", AFILE, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);*/
}

static void display_library(GtkTreeView *treeview, GtkTreeIter iter, GtkListStore *store) {
	FILE *library;
	size_t i = 0;
	char tempfile[1000];
	char temptrack[1000];
	char tempalbum[1000];
	char tempartist[1000];
	char temptracknumber[1000];
	char tempforce[1000];

	library = fopen("library.txt", "r");

	while(fgets(tempfile, 1000, library) != NULL) {
		tempfile[strcspn(tempfile, "\n")] = '\0';
		fgets(temptracknumber, 1000, library);
		temptracknumber[strcspn(temptracknumber, "\n")] = '\0';
		fgets(temptrack, 1000, library);
		temptrack[strcspn(temptrack, "\n")] = '\0';
		fgets(tempalbum, 1000, library);
		tempalbum[strcspn(tempalbum, "\n")] = '\0';
		fgets(tempartist, 1000, library);
		tempartist[strcspn(tempartist, "\n")] = '\0';
		fgets(tempforce, 1000, library);
		tempforce[strcspn(tempforce, "\n")] = '\0';
		if(atoi(tempforce) == 0)
			strcpy(tempforce, "Loud");
		else if(atoi(tempforce) == 1)
			strcpy(tempforce, "Calm");
		else
			strcpy(tempforce, "Can't conclude");

		gtk_list_store_append(store, &iter);
		gtk_list_store_set(store, &iter, PLAYING, "", TRACKNUMBER, temptracknumber, TRACK, temptrack, ALBUM, tempalbum, ARTIST, tempartist, FORCE, tempforce, AFILE, tempfile, -1);
	}
	//g_object_unref(store);
}

gint sort_iter_compare_func(GtkTreeModel *model, GtkTreeIter *a, GtkTreeIter *b, gpointer userdata) {
	gchar *track1, *track2;
	gtk_tree_model_get(model, a, TRACKNUMBER, &track1, -1);
	gtk_tree_model_get(model, b, TRACKNUMBER, &track2, -1);

	if (atof(track1) > atof(track2))
		return 1;
	else if(atof(track1) < atof(track2))
		return -1;
	else
		return 0;
}

int main(int argc, char **argv) {
	struct arguments argument;
	struct arguments *pargument = &argument;

	GtkWidget *window, *treeview, *scrolled_win, *vboxv, *vboxh, *progressbar, *buttons_table, *next_button, *previous_button;
	GtkTreeSortable *sortable;
	GtkTreeIter iter;

	pargument->first = 1;	
	pargument->status = 0;
	pargument->offset = 0;
	pargument->elapsed = g_timer_new();

	gtk_init(&argc, &argv);
	
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(window), "lelele player");
	gtk_widget_set_size_request(window, 900, 500);

	//treeview = gtk_tree_view_new();

	scrolled_win = gtk_scrolled_window_new(NULL, NULL);
	pargument->store = gtk_list_store_new(COLUMNS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
	sortable = GTK_TREE_SORTABLE(pargument->store);
	gtk_tree_sortable_set_sort_column_id(sortable, TRACKNUMBER, GTK_SORT_ASCENDING);
	treeview = gtk_tree_view_new_with_model(GTK_TREE_MODEL(pargument->store));
	gtk_tree_sortable_set_sort_func(sortable, TRACKNUMBER, sort_iter_compare_func, NULL, NULL); 
	setup_tree_view(treeview);
	pargument->treeview = treeview;

	//gtk_tree_view_set_model(GTK_TREE_VIEW(treeview), GTK_TREE_MODEL(store));


	//sorted_model.set_sort_column_id(1, Gtk.SortType.ASCENDING);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_win), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	pargument->elapsed = g_timer_new();

	pargument->toggle_button = gtk_button_new_with_label("▶▮▮");
	next_button = gtk_button_new();
	gtk_button_set_image((GtkButton*)next_button, gtk_image_new_from_file("./next.svg"));
	previous_button = gtk_button_new();
	gtk_button_set_image((GtkButton*)previous_button, gtk_image_new_from_file("./previous.svg"));
	pargument->toggle_button = gtk_button_new();
	gtk_button_set_image((GtkButton*)pargument->toggle_button, gtk_image_new_from_file("./play.svg"));
	buttons_table = gtk_table_new(2, 1, FALSE);
	pargument->adjust = (GtkAdjustment*)gtk_adjustment_new(0, 0, 100, 1, 1, 1);
	progressbar = gtk_hscale_new(pargument->adjust);
	gtk_scale_set_draw_value((GtkScale*)progressbar, FALSE);
	vboxv = gtk_vbox_new(TRUE, 5);
	vboxh = gtk_hbox_new(TRUE, 5);

	/* Signal management */
	g_signal_connect(G_OBJECT(pargument->toggle_button), "clicked", G_CALLBACK(toggle), pargument);
	g_signal_connect(G_OBJECT(next_button), "clicked", G_CALLBACK(next), pargument);
	g_signal_connect(G_OBJECT(treeview), "row-activated", G_CALLBACK(row_activated), pargument);
	g_signal_connect(G_OBJECT(progressbar), "value-changed", G_CALLBACK(slider_changed), pargument);
	g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(destroy), NULL);	

	gtk_container_add(GTK_CONTAINER(scrolled_win), treeview);

	gtk_box_set_homogeneous(GTK_BOX(vboxv), FALSE);
	gtk_box_set_homogeneous(GTK_BOX(vboxh), FALSE);
	
	/* Add objects to the box */
	gtk_table_attach(GTK_TABLE(buttons_table), vboxh, 0, 2, 0, 2, TRUE, TRUE, 0, 0);
		gtk_box_pack_start(GTK_BOX(vboxh), previous_button, TRUE, TRUE, 1);
		gtk_box_pack_start(GTK_BOX(vboxh), pargument->toggle_button, TRUE, FALSE, 1);
		gtk_box_pack_start(GTK_BOX(vboxh), next_button, TRUE, TRUE, 1);
	gtk_box_pack_start(GTK_BOX(vboxv), buttons_table, FALSE, TRUE, 1);
	//gtk_box_pack_start(GTK_BOX(vboxv), vboxh, FALSE, FALSE, 1);
	gtk_box_pack_start(GTK_BOX(vboxv), progressbar, FALSE, FALSE, 1);
	gtk_box_pack_start_defaults(GTK_BOX(vboxv), scrolled_win);


	gtk_container_add(GTK_CONTAINER(window), vboxv);

	/* temporary */
	display_library(GTK_TREE_VIEW(treeview), iter, pargument->store);
	/* temporary */

	gtk_widget_show_all(window);

	gtk_main();
	return 0;
}
