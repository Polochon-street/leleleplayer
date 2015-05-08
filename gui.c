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

void free_song(struct song *song) {
	free(song->sample_array);
	free(song->artist);
	free(song->title);
	free(song->album);
	free(song->tracknumber);
}

void explore(GDir *dir, char *folder, FILE *list) {
	const gchar *file;

	while((dir != NULL) && (file = g_dir_read_name(dir))) {
		if( g_file_test(g_build_path("/", folder, file, NULL), G_FILE_TEST_IS_REGULAR) && ( g_str_has_suffix(file, "flac") || g_str_has_suffix(file, "mp3") || g_str_has_suffix(file, "ogg") ) ) 
			g_fprintf(list, "%s\n", g_build_path("/", folder, file, NULL));	
		else if(g_file_test(g_build_path("/", folder, file, NULL), G_FILE_TEST_IS_DIR))
			explore(g_dir_open(g_build_path("/", folder, file, NULL), 0, NULL), g_build_path("/", folder, file, NULL), list);
	}
	if (file == NULL) {
		g_dir_close(dir);
	}
}

void set_next_song(struct arguments *argument) {
	GtkTreeModel *model;
	gchar *tempfile;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(argument->treeview));
	if(gtk_tree_model_iter_next(model, &(argument->iter))) {
			gtk_tree_model_get(model, &(argument->iter), AFILE, &tempfile, -1);
			audio_decode(tempfile, &argument->next_song); 
			bufferize(argument->next_song, argument);
			g_free(tempfile);
	}
	else
		argument->next_song.nSamples = 0;
}

void unqueue_buffer(int buffer, ALuint *source) {
	int buffers;

	do {
		alGetSourcei(*source, AL_BUFFERS_PROCESSED, &buffers);
	}
	while(buffers == 0);

	alSourceUnqueueBuffers(*source, 1, &buffer);
	alDeleteBuffers(1, &buffer);
}

static void row_activated(GtkTreeView *treeview, GtkTreePath *path, GtkTreeViewColumn *column, struct arguments *argument) {
	GtkTreeModel *model;
	GtkTreeIter iter;
	char *tempfile;
	
	alDeleteSources(1, &argument->source);
	alGenSources(1, &argument->source);
	
	if(argument->playing_iter.stamp != 0)
		gtk_list_store_set(((struct arguments*)argument)->store, &(((struct arguments*)argument)->playing_iter), PLAYING, "", -1);

	model = gtk_tree_view_get_model(treeview);
	if(gtk_tree_model_get_iter(model, &iter, path)) {
		gtk_tree_model_get(model, &iter, AFILE, &tempfile, -1);
		argument->playing_iter = iter;

		if(audio_decode(tempfile, &argument->current_song) == 0) {
			gtk_list_store_set(argument->store, &(argument->playing_iter), PLAYING, "▶", -1);
			gtk_adjustment_configure(argument->adjust, 0, 0, argument->current_song.duration, 1, 1, 1);
			gtk_adjustment_changed(argument->adjust);

			bufferize(argument->current_song, argument);
			argument->buffer_old = argument->buffer;

			play_song(argument->current_song, argument);
			// <insert set title func here>
			free_song(&argument->current_song);
			argument->iter = iter;
			set_next_song(argument);
		}
	}
}

static gboolean continue_track(gpointer pargument) {
	struct arguments *argument = (struct arguments*)pargument;
	GtkTreeModel *model;
	int buffers;

	argument->current_song = argument->next_song;
	gtk_list_store_set(argument->store, &(argument->playing_iter), PLAYING, "", -1);
	argument->playing_iter = argument->iter;
	gtk_list_store_set(argument->store, &(argument->playing_iter), PLAYING, "▶", -1);

	// <insert set title func here>

	free_song(&argument->current_song);
	argument->buffer_old = argument->buffer;
	model = gtk_tree_view_get_model((GtkTreeView *)(argument->treeview));
	unqueue_buffer(argument->buffer_old, &argument->source);

	gtk_adjustment_configure(argument->adjust, 0, 0, argument->current_song.duration, 1, 1, 1);
	gtk_adjustment_changed(argument->adjust);

	set_next_song(argument);
}

static void previous_track(struct arguments *argument) { 
	GtkTreeModel *model;
	char *tempfile;
	int buffers;

	gtk_list_store_set(argument->store, &(argument->playing_iter), PLAYING, "", -1);
	argument->playing_iter = argument->iter;

	model = gtk_tree_view_get_model((GtkTreeView *)(argument->treeview));

	unqueue_buffer(argument->buffer_old, &argument->source);
	unqueue_buffer(argument->buffer, &argument->source);

	if(gtk_tree_model_iter_previous(model, &(argument->iter))) {
		if(gtk_tree_model_iter_previous(model, &(argument->iter))) {
			gtk_list_store_set(argument->store, &(argument->iter), PLAYING, "▶", -1);
			gtk_tree_model_get(model, &(argument->iter), AFILE, &tempfile, -1);
			audio_decode(tempfile, &argument->current_song);
			bufferize(argument->current_song, argument);
			gtk_adjustment_configure(argument->adjust, 0, 0, argument->current_song.duration, 1, 1, 1);
			gtk_adjustment_changed(argument->adjust);
			argument->playing_iter = argument->iter;
		}
	}
	set_next_song(argument);
	argument->buffer_old = argument->buffer;
//	next_sample_array = audio_decode(&argument->next_song, tempfile);
}

int bufferize(struct song song, struct arguments *argument) {
	ALenum format;
	int i;
	float *float_samples;

	if(argument->first == 1) {
		InitOpenAL();
		alSourcei(argument->source, AL_BUFFER, 0);
		alDeleteSources(1, &argument->source);
		alGenSources(1, &argument->source);
	}
	argument->first = 0;
	alGenBuffers(1, &(argument->buffer));

	if(song.nb_bytes_per_sample == 1 && song.channels == 1)
		format = AL_FORMAT_MONO8;
	else if(song.nb_bytes_per_sample == 1 && song.channels == 2)
		format = AL_FORMAT_STEREO8;
	else if(song.nb_bytes_per_sample == 2 && song.channels == 1)
		format = AL_FORMAT_MONO16;
	else if(song.nb_bytes_per_sample == 2 && song.channels == 2)
		format = AL_FORMAT_STEREO16;
	else if(song.nb_bytes_per_sample == 4 && song.channels == 1)
		format = AL_FORMAT_MONO_FLOAT32;
	else if(song.nb_bytes_per_sample == 4 && song.channels == 2) {
		float_samples = malloc(song.nSamples*song.nb_bytes_per_sample);
		for(i = 0; i <= song.nSamples; ++i)
			float_samples[i] = ((int32_t*)song.sample_array)[i] / (float)0x7fffffff;
		format = AL_FORMAT_STEREO_FLOAT32;  
		if(song.nSamples % 2)
			song.nSamples--;
		alBufferData(argument->buffer, format, float_samples, song.nSamples * song.nb_bytes_per_sample, song.sample_rate);
		alSourceQueueBuffers(argument->source, 1, &(argument->buffer));
		return 0;
	}

	for(; ((int16_t*)song.sample_array)[song.nSamples-1] == 0; --song.nSamples)
		;

	if(song.nSamples % 2)
		song.nSamples--;

	alBufferData(argument->buffer, format, song.sample_array, song.nSamples * song.nb_bytes_per_sample, song.sample_rate);
	alSourceQueueBuffers(argument->source, 1, &(argument->buffer));
	return 0;
}

void play_song(struct song song, struct arguments *argument) {
	float timef;
	int bytes;

	alGetSourcei(argument->source, AL_BYTE_OFFSET, &bytes);
	timef = bytes / (float)(song.sample_rate * song.channels * song.nb_bytes_per_sample);

	alSourcePlay(argument->source);
	gtk_button_set_image((GtkButton*)(argument->toggle_button), gtk_image_new_from_file("./pause.svg"));
	gtk_list_store_set(argument->store, &(argument->playing_iter), PLAYING, "▶", -1);
	
	if(argument->tag != 0)
		g_source_remove(argument->tag);

	argument->bartag = g_timeout_add_seconds(1, timer_progressbar, argument);
	argument->tag = g_timeout_add_seconds(song.duration - timef, continue_track, argument);
}

void pause_song(struct arguments *argument) {
	alSourcePause(argument->source);
	argument->bartag = g_timeout_add_seconds(1, timer_progressbar, argument);
	gtk_button_set_image((GtkButton*)(argument->toggle_button), gtk_image_new_from_file("./play.svg"));
	gtk_list_store_set(argument->store, &(argument->playing_iter), PLAYING, "▎▎" -1);
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
		play_song(argument->current_song, argument);
	}
}

static void next(GtkWidget *button, struct arguments *argument) {
	if(argument->next_song.nSamples > 0) {
		alSourceStop(argument->source);
		continue_track(argument);
		alSourcePlay(argument->source);
	}
	else
		gtk_list_store_set(argument->store, &(argument->playing_iter), PLAYING, "", -1);
}

static void previous(GtkWidget *button, struct arguments *argument) {
	float timef;
    int bytes;
    alGetSourcei(argument->source, AL_BYTE_OFFSET, &bytes);

    if(argument->current_song.nSamples > 0) {
		timef = bytes / (float)(argument->current_song.sample_rate * argument->current_song.channels * argument->current_song.nb_bytes_per_sample);
		alSourceStop(argument->source);

		if(timef >= 1.0f) {
			alSourcePlay(argument->source);
		}
		else {
			previous_track(argument);
			alSourcePlay(argument->source);
		}
	}
}

static void config_folder_changed (gchar *folder, GtkWidget *parent) {
	GtkWidget *progressbar, *progressdialog, *area;
	progressbar = gtk_progress_bar_new();
	progressdialog = gtk_dialog_new_with_buttons("Loading...", GTK_WINDOW(parent), 0, NULL);
	area = gtk_dialog_get_content_area(GTK_DIALOG(progressdialog));
	struct song song;
	GDir *dir = g_dir_open (folder, 0, NULL);
	FILE *list;
	FILE *library;
	list = fopen("list.txt", "w+");
	library = fopen("library.txt", "w");
	float resnum_temp; //int
	explore(dir, folder, list);
	int nblines = 0;
	float resnum = 0;
	ssize_t len = 0;
	char line[1000];
	int count = 0;

	fseek(list, 0, SEEK_SET);

	while(fgets(line, 1000, list) != NULL)
		nblines++;

	fseek(list, 0, SEEK_SET);

	gtk_progress_bar_set_ellipsize(GTK_PROGRESS_BAR(progressbar), PANGO_ELLIPSIZE_END);
	gtk_box_set_homogeneous(GTK_BOX(area), TRUE);
	gtk_widget_set_size_request(progressbar, 300, 20);
	gtk_box_pack_start(GTK_BOX(area), progressbar, TRUE, TRUE, 0);
	gtk_progress_bar_set_show_text(GTK_PROGRESS_BAR(progressbar), 1);
	//gtk_container_add(GTK_CONTAINER(area), progressbar);
	gtk_widget_show_all(progressdialog);

	song.sample_array = song.title = song.artist = song.album = song.tracknumber = NULL;
	while (fgets(line, 1000, list) != NULL) {
		printf("%s\n", line);
		line[strcspn(line, "\n")] = '\0';
		if((resnum = analyze(line, &song)) != 0) {

			fprintf(library, "%s\n%f\n%s\n%s\n%s\n%f\n", line, resnum, song.title, song.album, song.artist, resnum);
			gtk_progress_bar_set_text(GTK_PROGRESS_BAR(progressbar), song.title);
			free_song(&song);
		}
		count++;
		gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progressbar), (float)count/(float)nblines);

		while(gtk_events_pending())
			gtk_main_iteration();
	}
	
	gtk_widget_destroy(progressdialog);

	fclose(list);
	g_remove("list.txt");
	fclose(library);
}

void folder_chooser(GtkWidget *button, struct pref_arguments *argument) {
	GtkWidget *dialog;
	gint res;
	GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER;

	dialog = gtk_file_chooser_dialog_new("Choose library folder", GTK_WINDOW(argument->window),
		action, "Cancel", GTK_RESPONSE_CANCEL, "Open", GTK_RESPONSE_ACCEPT, NULL);

	res = gtk_dialog_run(GTK_DIALOG(dialog));

	if(res == GTK_RESPONSE_ACCEPT) {
		GtkFileChooser *chooser;
		chooser = GTK_FILE_CHOOSER(dialog);
		argument->folder = gtk_file_chooser_get_filename(chooser);
	}
	else
		argument->folder = NULL;
	gtk_widget_destroy(dialog);
	gtk_entry_set_text((GtkEntry*)argument->library_entry, argument->folder); 
}
	

static void preferences_callback(GtkMenuItem *preferences, struct pref_arguments *argument) {
	GtkWidget *dialog, *label, *area, *vbox, *hbox, *library_entry, *browse_button, *alignement, *window_temp;
	GtkDialogFlags flags = GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT;	
//	GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER;
	gint res;
	char *folder;

	label = gtk_label_new("");
	gtk_label_set_markup(GTK_LABEL(label), "<span weight=\"bold\">Select library location:</span>");
	alignement = gtk_alignment_new(0, 0.5, 0, 0);
	gtk_container_add(GTK_CONTAINER(alignement), label);
	browse_button = gtk_button_new_with_label("Browse...");
	library_entry = gtk_entry_new();
	//chooser = gtk_file_chooser_dialog_new("Browse...", GTK_WINDOW(dialog), GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER, "Save", GTK_RESPONSE_ACCEPT, NULL);
	vbox = gtk_vbox_new(TRUE, 5);
	hbox = gtk_hbox_new(TRUE, 5);
//	dialog = gtk_file_chooser_dialog_new("Browse library", GTK_WINDOW(argument->window), action, "Cancel", GTK_RESPONSE_CANCEL, "_Save", GTK_RESPONSE_ACCEPT, NULL);
	dialog = gtk_dialog_new_with_buttons("Preferences", GTK_WINDOW(argument->window), flags, "Cancel", GTK_RESPONSE_REJECT, "Save", GTK_RESPONSE_ACCEPT, NULL);
	area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));

	g_signal_connect(G_OBJECT(browse_button), "clicked", G_CALLBACK(folder_chooser), argument);
	argument->library_entry = library_entry;
	window_temp = argument->window;
	argument->window = dialog;

	gtk_box_set_homogeneous(GTK_BOX(vbox), TRUE);
	gtk_box_set_homogeneous(GTK_BOX(hbox), FALSE);

	gtk_box_pack_start(GTK_BOX(vbox), alignement, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
		gtk_box_pack_start(GTK_BOX(hbox), library_entry, TRUE, TRUE, 0);
		gtk_box_pack_start(GTK_BOX(hbox), browse_button, FALSE, FALSE, 0);
	gtk_container_add(GTK_CONTAINER(area), vbox);
	//chooser = GTK_FILE_CHOOSER(dialog);

	gtk_widget_set_size_request(dialog, 400, 120);
	gtk_widget_show_all(dialog);
	res = gtk_dialog_run(GTK_DIALOG(dialog));

	if(argument->folder!= NULL && res == GTK_RESPONSE_ACCEPT) {
	//	GtkFileChooser *chooser = GTK_FILE_CHOOSER(argument->chooser);
		//folder = gtk_file_chooser_get_current_folder(dialog);
		//gtk_entry_set_text((GtkEntry*)library_entry, gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(argument->chooser)));
		printf("%s\n", argument->folder);

		config_folder_changed(argument->folder, dialog);
		display_library(GTK_TREE_VIEW(argument->treeview), argument->store);
	} 
	gtk_widget_destroy(dialog); 

	argument->window = window_temp;
}

static int timer_progressbar(gpointer pargument) {
	struct arguments *argument = (struct arguments*)pargument;
	int time;
	float timef;
	int bytes;
	int buffers;

	alGetSourcei(argument->source, AL_BYTE_OFFSET, &bytes);
	
	timef = bytes / (float)(argument->current_song.sample_rate * argument->current_song.channels * argument->current_song.nb_bytes_per_sample);

	alGetSourcei(argument->source, AL_BUFFERS_PROCESSED, &buffers);

	alGetSourcei(argument->source, AL_SOURCE_STATE, &argument->status);
	alGetSourcei(argument->source, AL_SEC_OFFSET, &time);
	if(argument->status == AL_PLAYING) {
		gtk_adjustment_set_value (argument->adjust, timef);
		gtk_adjustment_changed(argument->adjust);
		g_source_remove(argument->bartag);

		argument->bartag = g_timeout_add_seconds(1, timer_progressbar, argument);
	}
	return 0;
}

static void slider_changed(GtkRange *progressbar, struct arguments *argument) {
	int time;
	float timef;
	int bytes;
	alGetSourcei(argument->source, AL_SEC_OFFSET, &time);
	alGetSourcei(argument->source, AL_BYTE_OFFSET, &bytes);
	
	timef = bytes / (float)(argument->current_song.sample_rate * argument->current_song.channels * argument->current_song.nb_bytes_per_sample);

	if(fabs(gtk_adjustment_get_value(argument->adjust) - timef) > 0.005f) {
		alSourcei(argument->source, AL_BYTE_OFFSET, argument->current_song.channels * (int) ((gdouble) gtk_adjustment_get_value(argument->adjust)
 			* ((gdouble) (argument->current_song.sample_rate * argument->current_song.nb_bytes_per_sample))));
		g_source_remove(argument->tag);
		argument->tag = g_timeout_add_seconds((int) (((gdouble)argument->current_song.duration) - ((gdouble)gtk_adjustment_get_value(argument->adjust)))
				, continue_track, argument);
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

void display_library(GtkTreeView *treeview, GtkListStore *store) {
	GtkTreeIter iter;
	FILE *library;
	size_t i = 0;
	char tempfile[1000];
	char temptrack[1000];
	char tempalbum[1000];
	char tempartist[1000];
	char temptracknumber[1000];
	char tempforce[1000];

	gtk_list_store_clear(store);

	if((library = fopen("library.txt", "r")) != NULL) {
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
			if(atoi(tempforce) > 0)
				strcpy(tempforce, "Loud");
			else if(atoi(tempforce) < 0)
				strcpy(tempforce, "Calm");
			else
				strcpy(tempforce, "Can't conclude");

			gtk_list_store_append(store, &iter);
			gtk_list_store_set(store, &iter, PLAYING, "", TRACKNUMBER, temptracknumber, TRACK, temptrack, ALBUM, tempalbum, ARTIST, tempartist, FORCE, tempforce, AFILE, tempfile, -1);
		}
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
	struct pref_arguments pref_arguments;

	GtkWidget *window, *treeview, *scrolled_win, *vboxv, *vboxh, *progressbar, *buttons_table, *next_button, *previous_button, 
		*menubar, *edit, *editmenu, *preferences;
	GtkAccelGroup *group;
	GtkTreeSortable *sortable;
	GtkTreeIter iter;

	pargument->first = 1;	
	pargument->status = 0;
	pargument->playing_iter.stamp = 0;
	pargument->tag = 0;
	pargument->current_song.nSamples = 0;
	pargument->next_song.nSamples = 0;
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
	gtk_button_set_image((GtkButton*)next_button, gtk_image_new_from_file("next.svg"));
	previous_button = gtk_button_new();
	gtk_button_set_image((GtkButton*)previous_button, gtk_image_new_from_file("previous.svg"));
	pargument->toggle_button = gtk_button_new();
	gtk_button_set_image((GtkButton*)pargument->toggle_button, gtk_image_new_from_file("play.svg"));
	gtk_window_set_icon_from_file(GTK_WINDOW(window), "lelele.svg", NULL);
	buttons_table = gtk_table_new(2, 1, FALSE);
	pargument->adjust = (GtkAdjustment*)gtk_adjustment_new(0, 0, 100, 1, 1, 1);
	progressbar = gtk_hscale_new(pargument->adjust);
	group = gtk_accel_group_new();
	menubar = gtk_menu_bar_new();
	edit = gtk_menu_item_new_with_label("Edit");
	editmenu = gtk_menu_new();
	gtk_scale_set_draw_value((GtkScale*)progressbar, FALSE);
	vboxv = gtk_vbox_new(TRUE, 5);
	vboxh = gtk_hbox_new(TRUE, 5);

	gtk_menu_item_set_submenu(GTK_MENU_ITEM(edit), editmenu);
	gtk_menu_shell_append(GTK_MENU_SHELL(menubar), edit);

	preferences = gtk_menu_item_new_with_label("Preferences");
	gtk_menu_shell_append(GTK_MENU_SHELL(editmenu), preferences);

	pref_arguments.window = window;
	pref_arguments.treeview = treeview;
	pref_arguments.store = pargument->store;

	/* Signal management */
	g_signal_connect(G_OBJECT(pargument->toggle_button), "clicked", G_CALLBACK(toggle), pargument);
	g_signal_connect(G_OBJECT(next_button), "clicked", G_CALLBACK(next), pargument);
	g_signal_connect(G_OBJECT(previous_button), "clicked", G_CALLBACK(previous), pargument);
	g_signal_connect(G_OBJECT(preferences), "activate", G_CALLBACK(preferences_callback), &pref_arguments);
	g_signal_connect(G_OBJECT(treeview), "row-activated", G_CALLBACK(row_activated), pargument);
	g_signal_connect(G_OBJECT(progressbar), "value-changed", G_CALLBACK(slider_changed), pargument);
	g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(destroy), NULL);	

	gtk_container_add(GTK_CONTAINER(scrolled_win), treeview);

	gtk_box_set_homogeneous(GTK_BOX(vboxv), FALSE);
	gtk_box_set_homogeneous(GTK_BOX(vboxh), FALSE);
	
	/* Add objects to the box */
	gtk_box_pack_start(GTK_BOX(vboxv), menubar, FALSE, FALSE, 1);
	//gtk_window_add_accel_group(GTK_BOX(vboxh), group);
	gtk_table_attach(GTK_TABLE(buttons_table), vboxh, 0, 2, 0, 2, TRUE, TRUE, 0, 0);
		gtk_box_pack_start(GTK_BOX(vboxh), previous_button, TRUE, TRUE, 1);
		gtk_box_pack_start(GTK_BOX(vboxh), pargument->toggle_button, TRUE, FALSE, 1);
		gtk_box_pack_start(GTK_BOX(vboxh), next_button, TRUE, TRUE, 1);
	gtk_box_pack_start(GTK_BOX(vboxv), buttons_table, FALSE, TRUE, 1);
	//gtk_box_pack_start(GTK_BOX(vboxv), vboxh, FALSE, FALSE, 1);
	gtk_box_pack_start(GTK_BOX(vboxv), progressbar, FALSE, FALSE, 1);
	gtk_box_pack_start(GTK_BOX(vboxv), scrolled_win, TRUE, TRUE, 1);


	gtk_container_add(GTK_CONTAINER(window), vboxv);

	/* temporary */
	display_library(GTK_TREE_VIEW(treeview), pargument->store);
	/* temporary */

	gtk_widget_show_all(window);

	gtk_main();
	return 0;
}
