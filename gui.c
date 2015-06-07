#include <stdio.h>
#include "gui.h"

static void destroy (GtkWidget *window, gpointer data) {

	gtk_main_quit ();
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

void playlist_queue(GtkTreeIter *iter_to_queue, GtkTreeModel *model_library, GtkTreeView *treeview_playlist, struct arguments *argument) {
	GtkTreeIter iter_playlist;
	GtkTreeModel *model_playlist;
	GtkTreeSelection *selection;
	gchar *tempfile;
	gfloat tempfloat;
	size_t i;

	gtk_list_store_append(argument->store_playlist, &iter_playlist);
	model_playlist = gtk_tree_view_get_model(treeview_playlist);
	selection = gtk_tree_view_get_selection(treeview_playlist);
	for(i = 0; i < COLUMNS; ++i) {
		if(i == FORCE) {
			gtk_tree_model_get(model_library, iter_to_queue, i, &tempfloat, -1);
			gtk_list_store_set(argument->store_playlist, &iter_playlist, i, tempfloat,-1);
		}
		else {
			gtk_tree_model_get(model_library, iter_to_queue, i, &tempfile, -1);
			gtk_list_store_set(argument->store_playlist, &iter_playlist, i, tempfile, -1);
		}
	}
	argument->playlist_count++;
}

void get_current_playlist_song(GtkTreeView *treeview_playlist, struct song *song, struct arguments *argument) {
	GtkTreeModel *model_playlist;
	GstFormat fmt = GST_FORMAT_TIME;
	gint64 current = -1;

	model_playlist = gtk_tree_view_get_model(treeview_playlist);
	gtk_tree_model_get(model_playlist, &(argument->iter_playlist), AFILE, &song->filename, TRACKNUMBER, &song->tracknumber, TRACK, &song->title, ALBUM, &song->album,
		ARTIST, &song->artist, -1);
	if((!gst_element_query_duration(argument->current_song.playbin, fmt,
			&song->duration))) 
		g_printerr("Couldn't query current duration.\n");
}

gboolean get_next_playlist_song(GtkTreeView *treeview_playlist, struct song *song, struct arguments *argument) {
	GtkTreeModel *model_playlist;

	model_playlist = gtk_tree_view_get_model(treeview_playlist);

	if(argument->random) {
		return get_random_playlist_song(GTK_TREE_VIEW(argument->treeview_playlist), &(argument->current_song), argument);
	}
	else {
		if(gtk_tree_model_iter_next(model_playlist, &(argument->iter_playlist))) {
			gtk_tree_model_get(model_playlist, &(argument->iter_playlist), AFILE, &song->filename, TRACKNUMBER, &song->tracknumber, 
				TRACK, &song->title, ALBUM, &song->album, ARTIST, &song->artist, -1);
			return TRUE;
		}
		else
			return FALSE;
	}
}

gboolean get_random_playlist_song(GtkTreeView *treeview_playlist, struct song *song, struct arguments *argument) {
	GtkTreeModel *model_playlist;
	int i, rand;

	model_playlist = gtk_tree_view_get_model(treeview_playlist);
	rand = g_random_int_range(0, argument->playlist_count);
	
	gtk_tree_model_get_iter_first(model_playlist, &(argument->iter_playlist));

	for(i = 0; i < rand-1; ++i)
		gtk_tree_model_iter_next(model_playlist, &(argument->iter_playlist));

	if(gtk_tree_model_iter_next(model_playlist, &(argument->iter_playlist))) {
		gtk_tree_model_get(model_playlist, &(argument->iter_playlist), AFILE, &song->filename,
			TRACKNUMBER, &song->tracknumber, TRACK, &song->title, ALBUM, &song->album,
			ARTIST, &song->artist, -1);
		return TRUE;
	}
	else {
		return FALSE;
	}
}

gboolean get_previous_playlist_song(GtkTreeView *treeview_playlist, struct song *song, struct arguments *argument) {
	GtkTreeModel *model_playlist;

	model_playlist = gtk_tree_view_get_model(treeview_playlist);

	if(gtk_tree_model_iter_previous(model_playlist, &(argument->iter_playlist))) {
		gtk_tree_model_get(model_playlist, &(argument->iter_playlist), AFILE, &song->filename, TRACKNUMBER, &song->tracknumber,
		TRACK, &song->title, ALBUM, &song->album, ARTIST, &song->artist, -1);
		return TRUE;
	}
	else
		return FALSE;
}

static void row_activated(GtkTreeView *treeview, GtkTreePath *path, GtkTreeViewColumn *column, struct arguments *argument) {
	GtkTreeModel *model_library, *model_playlist;
	GtkTreeIter iter_library;
	gchar *songfile;
	
	//if(argument->iter.stamp != 0)
	//	gtk_list_store_set(((struct arguments*)argument)->store, &(((struct arguments*)argument)->iter), PLAYING, "", -1);
	
	model_library = gtk_tree_view_get_model(treeview);
	if(gtk_tree_model_get_iter(model_library, &iter_library, path)) {
		do {
			playlist_queue(&iter_library, model_library, GTK_TREE_VIEW(argument->treeview_playlist), argument);
		} while(gtk_tree_model_iter_next(model_library, &iter_library));

		gtk_tree_model_get_iter_first(gtk_tree_view_get_model(GTK_TREE_VIEW(argument->treeview_playlist)), &(argument->iter_playlist));
		get_current_playlist_song(GTK_TREE_VIEW(argument->treeview_playlist), &argument->current_song, argument);
		//gtk_tree_model_get(model_playlist, &argument->iter_playlist, AFILE, &(argument->current_song.filename), TRACKNUMBER, &(argument->current_song.tracknumber), TRACK, &(argument->current_song.title),
		//	ALBUM, &(argument->current_song.album), ARTIST, &(argument->current_song.artist), -1);

		// TODO 

		play_song(argument);
		//refresh_ui(argument->current_song.playbin, argument);

		argument->bartag = g_timeout_add_seconds(1, refresh_progressbar, argument);
		//free(songfile);
	}
}

static void continue_track(GstElement *playbin, struct arguments *argument) {
	//char *tempfile;
	GtkTreeModel *model_playlist;

	model_playlist = gtk_tree_view_get_model(GTK_TREE_VIEW(argument->treeview_playlist));
	
	get_next_playlist_song(GTK_TREE_VIEW(argument->treeview_playlist), &argument->current_song, argument);
	queue_song(argument);
	/*if(argument->random) {
		if(get_random_playlist_song(GTK_TREE_VIEW(argument->treeview_playlist), &argument->current_song, argument)) {
			queue_song(argument);
		}	
	}
	else {
		if(get_next_playlist_song(GTK_TREE_VIEW(argument->treeview_playlist), &argument->current_song, argument))
			queue_song(argument);
	}*/

	//argument->bartag = g_timeout_add_seconds(1, refresh_progressbar, argument);
}

static void previous_track(struct arguments *argument) { 
	GtkTreeModel *model_playlist;
	//char *tempfile;
	//int buffers;

	model_playlist = gtk_tree_view_get_model(GTK_TREE_VIEW(argument->treeview_playlist));
	if(get_previous_playlist_song(GTK_TREE_VIEW(argument->treeview_playlist), &argument->current_song, argument))
		play_song(argument);
}

void queue_song(struct arguments *argument) {
	g_object_set(argument->current_song.playbin, "uri", g_filename_to_uri(argument->current_song.filename, NULL, NULL)); 
//	gst_element_set_state(argument->current_song.playbin, GST_STATE_PLAYING);
}

void play_song(struct arguments *argument) {
	gst_element_set_state(argument->current_song.playbin, GST_STATE_NULL);
	g_object_set(argument->current_song.playbin, "uri", g_filename_to_uri(argument->current_song.filename, NULL, NULL)); 
	gst_element_set_state(argument->current_song.playbin, GST_STATE_PLAYING);
}

void pause_song(struct arguments *argument) {
	gst_element_set_state(argument->current_song.playbin, GST_STATE_PAUSED);
	//argument->bartag = g_timeout_add_seconds(1, refresh_progressbar, argument);
	gtk_button_set_image((GtkButton*)(argument->toggle_button), gtk_image_new_from_file("./play.svg"));
	gtk_list_store_set(argument->store_playlist, &(argument->iter_playlist), PLAYING, "▍▍", -1);

	gst_element_set_state(argument->current_song.playbin, GST_STATE_PAUSED);
}

static void toggle_random(GtkWidget *button, struct arguments *argument) {
	argument->random = (argument->random == 1 ? 0 : 1);
}

static void toggle(GtkWidget *button, struct arguments *argument) { 
	GtkTreeSelection *selection;
	GtkTreeModel *model;
	GtkTreePath *path;
	GtkTreeIter iter;
	
	if(argument->current_song.state == GST_STATE_NULL) {
		selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(argument->treeview_library));
		gtk_tree_view_get_model((GtkTreeView*)(argument->treeview_library));
		if(gtk_tree_selection_get_selected(selection, &model, &iter)) {
				path = gtk_tree_model_get_path(model, &iter);

				row_activated(GTK_TREE_VIEW(argument->treeview_library), path, NULL,argument);
		}		
	}
	else if(argument->current_song.state == GST_STATE_PLAYING)
		pause_song(argument);
	else if(argument->current_song.state == GST_STATE_PAUSED) 
		play_song(argument);
}

static void next(GtkWidget *button, struct arguments *argument) {
	get_next_playlist_song(GTK_TREE_VIEW(argument->treeview_playlist), &argument->current_song, argument);
	play_song(argument);
}

static void previous(GtkWidget *button, struct arguments *argument) {
	float timef;
    int bytes;
    //alGetSourcei(argument->source, AL_BYTE_OFFSET, &bytes);

    if(argument->current_song.nSamples > 0) {
		timef = bytes / (float)(argument->current_song.sample_rate * argument->current_song.channels * argument->current_song.nb_bytes_per_sample);
		//alSourceStop(argument->source);

		if(timef >= 1.0f) {
			//alSourcePlay(argument->source);
		}
		else {
			previous_track(argument);
			//alSourcePlay(argument->source);
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
		line[strcspn(line, "\n")] = '\0';
		if((resnum = analyze(line, &song)) != 0) {

			fprintf(library, "%s\n%s\n%s\n%s\n%s\n%f\n", line, song.tracknumber, song.title, song.album, song.artist, resnum);
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
	vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
	hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
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

		config_folder_changed(argument->folder, dialog);
		display_library(GTK_TREE_VIEW(argument->treeview), argument->store_library);
	} 
	gtk_widget_destroy(dialog); 

	argument->window = window_temp;
}

static void state_changed(GstBus *bus, GstMessage *msg, struct arguments *argument) {
	GstState old_state, new_state, pending_state;
	if(GST_MESSAGE_SRC(msg) == GST_OBJECT(argument->current_song.playbin)) {
			gst_message_parse_state_changed(msg, &old_state, &new_state, &pending_state);
    		argument->current_song.state = new_state; 
	}
}

static void refresh_ui(GstBus *bus, GstMessage *msg, struct arguments *argument) {
	GtkTreeSelection *selection;
	GtkTreePath *path;
	GtkTreeViewColumn *column;
	GtkTreeModel *model_playlist;
	GtkTreeIter temp_iter;
	gchar *tempfile;

	gint64 test;
	GstFormat fmt = GST_FORMAT_TIME;
	gst_element_query_duration(argument->current_song.playbin, fmt,
		&(argument->current_song.duration));
		gst_element_query_position(argument->current_song.playbin, 
		fmt, &(argument->current_song.current));

	if(model_playlist = gtk_tree_view_get_model(GTK_TREE_VIEW(argument->treeview_playlist))) {
		selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(argument->treeview_playlist));
		path = gtk_tree_model_get_path(model_playlist, &argument->iter_playlist);
		gtk_tree_view_get_column(GTK_TREE_VIEW(argument->treeview_playlist), PLAYING);
 		//gtk_tree_model_get(model_playlist, &(argument->iter_playlist), AFILE, &tempfile, -1);

		gtk_tree_model_get_iter_first(gtk_tree_view_get_model(GTK_TREE_VIEW(argument->treeview_playlist)), &(temp_iter));

		do {
			gtk_list_store_set(argument->store_playlist, &(temp_iter), PLAYING, "", -1);
		}
		while(gtk_tree_model_iter_next(model_playlist, &(temp_iter)));
		gtk_button_set_image(GTK_BUTTON(argument->toggle_button), gtk_image_new_from_file("./pause.svg"));
		gtk_list_store_set(argument->store_playlist, &(argument->iter_playlist), PLAYING, "▶", -1);
		gtk_tree_selection_select_iter(selection, &(argument->iter_playlist));
		gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(argument->treeview_playlist), path, column, 1, 0.2, 1);
	
		gtk_label_set_text(GTK_LABEL(argument->album_label), argument->current_song.album);
		gtk_label_set_text(GTK_LABEL(argument->artist_label), argument->current_song.artist);
		gtk_label_set_text(GTK_LABEL(argument->title_label), argument->current_song.title);
	}
	g_signal_handler_block(argument->progressbar, argument->progressbar_update_signal_id);
	gtk_adjustment_configure(argument->adjust, 0, 0, argument->current_song.duration/GST_SECOND, 
		1, 1, 1);
	//	gtk_adjustment_set_value(argument->adjust, argument->current_song.current/GST_SECOND);
	gtk_adjustment_changed(argument->adjust);
	g_signal_handler_unblock(argument->progressbar, argument->progressbar_update_signal_id);

	//refresh_progressbar(argument);
}

static gboolean refresh_progressbar(gpointer pargument) {
	struct arguments *argument = (struct arguments*)pargument;
	GstFormat fmt = GST_FORMAT_TIME;
	gint64 current = -1;
	
	if(argument->current_song.state < GST_STATE_PAUSED) {
		return TRUE;
	}
	
	if(gst_element_query_position(argument->current_song.playbin, 
		fmt, &(argument->current_song.current))) {
		g_signal_handler_block(argument->progressbar, argument->progressbar_update_signal_id);
		gtk_adjustment_configure(argument->adjust, argument->current_song.current/GST_SECOND, 0, argument->current_song.duration/GST_SECOND, 
			1, 1, 1);
	//	gtk_adjustment_set_value(argument->adjust, argument->current_song.current/GST_SECOND);
		gtk_adjustment_changed(argument->adjust);
		g_signal_handler_unblock(argument->progressbar, argument->progressbar_update_signal_id);
	}
	return TRUE;
}

static void slider_changed(GtkRange *progressbar, struct arguments *argument) {
	if(!gst_element_seek(argument->current_song.playbin, 1.0, GST_FORMAT_TIME, GST_SEEK_FLAG_FLUSH, 
		GST_SEEK_TYPE_SET, gtk_adjustment_get_value(argument->adjust)*GST_SECOND, GST_SEEK_TYPE_NONE,
		GST_CLOCK_TIME_NONE)) 
		printf("Seek failed!\n");
} 

static void setup_tree_view(GtkWidget *treeview) {
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;

	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes("", renderer, "text", PLAYING, NULL);
	gtk_tree_view_column_set_sort_column_id(column, PLAYING);
	gtk_tree_view_column_set_sizing(column, GTK_TREE_VIEW_COLUMN_FIXED);
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
	column = gtk_tree_view_column_new_with_attributes("Force", renderer, "text", TEXTFORCE, NULL);
	gtk_tree_view_column_set_resizable (column, TRUE);
	gtk_tree_view_column_set_sort_column_id(column, TEXTFORCE);
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
	float tempforcef;

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

			tempforcef = atof(tempforce);
			
			if(atof(tempforce) > 0)
				strcpy(tempforce, "Loud");
			else if(atof(tempforce) < 0)
				strcpy(tempforce, "Calm");
			else
				strcpy(tempforce, "Can't conclude");

			gtk_list_store_append(store, &iter);
			gtk_list_store_set(store, &iter, PLAYING, "", TRACKNUMBER, temptracknumber, TRACK, temptrack, ALBUM, tempalbum, ARTIST, tempartist, FORCE, tempforcef, TEXTFORCE, tempforce, AFILE, tempfile, -1);
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

gint sort_force(GtkTreeModel *model, GtkTreeIter *a, GtkTreeIter *b, gpointer userdata) {
	gfloat force1, force2;
	gtk_tree_model_get(model, a, FORCE, &force1, -1);
	gtk_tree_model_get(model, b, FORCE, &force2, -1);

	if(force1 > force2)
		return 1;
	else if(force1 < force2)
		return -1;
	else
		return 0;
}

int main(int argc, char **argv) {
	struct arguments argument;
	struct arguments *pargument = &argument;
	struct pref_arguments pref_arguments;

	GtkWidget *window, *treeview_library, *treeview_playlist, *library_panel, *playlist_panel, *vboxv, *progressbar,
		*playbox, *randombox, *repeat_button, *random_button, *lelele_button, *labelbox, *next_button, *previous_button, *menubar, *edit, *editmenu, 
		*preferences, *libplaypane;
	GtkTreeSortable *sortable;
	GtkTreeIter iter;

	GstBus *bus;

	pargument->lelelerandom = 0;
	pargument->random = 0;
	pargument->first = 1;
	pargument->playlist_count = 0;
	pargument->iter_library.stamp = 0;
	pargument->current_song.duration = GST_CLOCK_TIME_NONE;
	pargument->elapsed = g_timer_new();
	pargument->current_song.state = GST_STATE_NULL;

	gtk_init(&argc, &argv);
	
	gst_init(&argc, &argv);
	
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(window), "lelele player");
	gtk_widget_set_size_request(window, 900, 500);

	//treeview = gtk_tree_view_new();

	pargument->current_song.playbin = gst_element_factory_make("playbin", "playbin");
	if(!pargument->current_song.playbin)
		g_printerr("Not all elements could be created.\n");
	bus = gst_element_get_bus(pargument->current_song.playbin);
	gst_bus_add_signal_watch(bus);

	library_panel = gtk_scrolled_window_new(NULL, NULL);
	playlist_panel = gtk_scrolled_window_new(NULL, NULL);

	pargument->store_library = gtk_list_store_new(COLUMNS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_FLOAT, G_TYPE_STRING, G_TYPE_STRING);
	sortable = GTK_TREE_SORTABLE(pargument->store_library);
	gtk_tree_sortable_set_sort_column_id(sortable, TRACKNUMBER, GTK_SORT_ASCENDING);
	treeview_library = gtk_tree_view_new_with_model(GTK_TREE_MODEL(pargument->store_library));
	gtk_tree_sortable_set_sort_func(sortable, TRACKNUMBER, sort_iter_compare_func, NULL, NULL); 
	gtk_tree_sortable_set_sort_func(sortable, TEXTFORCE, sort_force, NULL, NULL);
	setup_tree_view(treeview_library);	
	pargument->treeview_library = treeview_library;
	
	pargument->store_playlist = gtk_list_store_new(COLUMNS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_FLOAT, G_TYPE_STRING, G_TYPE_STRING);
	treeview_playlist = gtk_tree_view_new();
	treeview_playlist = gtk_tree_view_new_with_model(GTK_TREE_MODEL(pargument->store_playlist));
	setup_tree_view(treeview_playlist);
	gtk_tree_view_set_reorderable(GTK_TREE_VIEW(treeview_playlist), TRUE);
	pargument->treeview_playlist = treeview_playlist;
	//gtk_tree_view_set_model(GTK_TREE_VIEW(treeview), GTK_TREE_MODEL(store));

	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(library_panel), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(playlist_panel), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	pargument->elapsed = g_timer_new();
	playbox = gtk_button_box_new(GTK_ORIENTATION_HORIZONTAL);
	randombox= gtk_button_box_new(GTK_ORIENTATION_HORIZONTAL);
	pargument->toggle_button = gtk_button_new_with_label("▶▮▮");
	random_button = gtk_toggle_button_new();
	gtk_button_set_image((GtkButton*)random_button, gtk_image_new_from_file("random.svg"));
	repeat_button = gtk_toggle_button_new();
	gtk_button_set_image((GtkButton*)repeat_button , gtk_image_new_from_file("repeat.svg"));
	lelele_button = gtk_toggle_button_new();
	gtk_button_set_image((GtkButton*)lelele_button, gtk_image_new_from_file("lelelerandom.svg"));
	next_button = gtk_button_new();
	gtk_button_set_image((GtkButton*)next_button, gtk_image_new_from_file("next.svg"));
	previous_button = gtk_button_new();
	gtk_button_set_image((GtkButton*)previous_button, gtk_image_new_from_file("previous.svg"));
	pargument->toggle_button = gtk_button_new();
	gtk_button_set_image((GtkButton*)pargument->toggle_button, gtk_image_new_from_file("play.svg"));
	gtk_window_set_icon_from_file(GTK_WINDOW(window), "lelele.svg", NULL);
	pargument->adjust = (GtkAdjustment*)gtk_adjustment_new(0, 0, 100, 1, 1, 1);
	pargument->progressbar = gtk_scale_new(GTK_ORIENTATION_HORIZONTAL, pargument->adjust);
	menubar = gtk_menu_bar_new();
	edit = gtk_menu_item_new_with_label("Edit");
	editmenu = gtk_menu_new();
	gtk_scale_set_draw_value((GtkScale*)pargument->progressbar, FALSE);
	vboxv = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
	labelbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
	libplaypane = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
	pargument->title_label = gtk_label_new("");
	pargument->album_label = gtk_label_new("");
	gtk_label_set_markup(GTK_LABEL(pargument->album_label), "<span foreground=\"grey\">No song currently playing</span>");
	pargument->artist_label = gtk_label_new("");

	gtk_menu_item_set_submenu(GTK_MENU_ITEM(edit), editmenu);
	gtk_menu_shell_append(GTK_MENU_SHELL(menubar), edit);

	preferences = gtk_menu_item_new_with_label("Preferences");
	gtk_menu_shell_append(GTK_MENU_SHELL(editmenu), preferences);

	pref_arguments.window = window;
	pref_arguments.treeview = treeview_library;
	pref_arguments.store_library = pargument->store_library;

	/* Signal management */
	//g_signal_connect(G_OBJECT(pargument->current_song.playbin), "audio-tags-changed", G_CALLBACK(tags_cb), pargument);
	g_signal_connect(G_OBJECT(bus), "message::state-changed", G_CALLBACK(state_changed), pargument);
	g_signal_connect(G_OBJECT(pargument->current_song.playbin), "about-to-finish", G_CALLBACK(continue_track), pargument);
	g_signal_connect(G_OBJECT(bus), "message::stream-start", G_CALLBACK(refresh_ui), pargument);
	g_signal_connect(G_OBJECT(pargument->toggle_button), "clicked", G_CALLBACK(toggle), pargument);
	g_signal_connect(G_OBJECT(random_button), "clicked", G_CALLBACK(toggle_random), pargument);
	g_signal_connect(G_OBJECT(next_button), "clicked", G_CALLBACK(next), pargument);
	g_signal_connect(G_OBJECT(previous_button), "clicked", G_CALLBACK(previous), pargument);
	g_signal_connect(G_OBJECT(preferences), "activate", G_CALLBACK(preferences_callback), &pref_arguments);
	g_signal_connect(G_OBJECT(treeview_library), "row-activated", G_CALLBACK(row_activated), pargument);
	pargument->progressbar_update_signal_id = g_signal_connect(G_OBJECT(pargument->progressbar), 
		"value-changed", G_CALLBACK(slider_changed), pargument);
	g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(destroy), NULL);	

	gtk_container_add(GTK_CONTAINER(library_panel), treeview_library);
	gtk_container_add(GTK_CONTAINER(playlist_panel), treeview_playlist);

	gtk_box_set_homogeneous(GTK_BOX(vboxv), FALSE);
	
	/* Add objects to the box */
	gtk_box_pack_start(GTK_BOX(vboxv), menubar, FALSE, FALSE, 1);
	gtk_box_pack_start(GTK_BOX(vboxv), labelbox, FALSE, FALSE, 1);
		gtk_box_pack_start(GTK_BOX(labelbox), pargument->title_label, FALSE, FALSE, 1);
		gtk_box_pack_start(GTK_BOX(labelbox), pargument->album_label, FALSE, FALSE, 1);
		gtk_box_pack_start(GTK_BOX(labelbox), pargument->artist_label, FALSE, FALSE, 1);
	gtk_box_pack_start(GTK_BOX(vboxv), playbox, FALSE, FALSE, 1);
		gtk_button_box_set_layout(GTK_BUTTON_BOX(playbox), GTK_BUTTONBOX_CENTER);
		gtk_box_pack_start(GTK_BOX(playbox), previous_button, FALSE, FALSE, 1);
		gtk_box_pack_start(GTK_BOX(playbox), pargument->toggle_button, FALSE, FALSE, 1);
		gtk_box_pack_start(GTK_BOX(playbox), next_button, FALSE, FALSE, 1);
		gtk_button_box_set_child_non_homogeneous(GTK_BUTTON_BOX(playbox), pargument->toggle_button, TRUE);
		gtk_button_box_set_child_non_homogeneous(GTK_BUTTON_BOX(playbox), next_button, TRUE);
		gtk_button_box_set_child_non_homogeneous(GTK_BUTTON_BOX(playbox), previous_button, TRUE);
		gtk_box_set_spacing(GTK_BOX(playbox), 5);
	gtk_box_pack_start(GTK_BOX(vboxv), randombox, FALSE, FALSE, 1);
		gtk_button_box_set_layout(GTK_BUTTON_BOX(randombox), GTK_BUTTONBOX_CENTER);
		gtk_box_pack_start(GTK_BOX(randombox), repeat_button, FALSE, FALSE, 1);
		gtk_box_pack_start(GTK_BOX(randombox), lelele_button, FALSE, FALSE, 1);
		gtk_box_pack_start(GTK_BOX(randombox), random_button, FALSE, FALSE, 1);
		gtk_button_box_set_child_non_homogeneous(GTK_BUTTON_BOX(randombox), repeat_button, TRUE);
		gtk_button_box_set_child_non_homogeneous(GTK_BUTTON_BOX(randombox), lelele_button, TRUE);
		gtk_button_box_set_child_non_homogeneous(GTK_BUTTON_BOX(randombox), random_button, TRUE);
		gtk_box_set_spacing(GTK_BOX(randombox), 5);
	gtk_box_pack_start(GTK_BOX(vboxv), pargument->progressbar, FALSE, FALSE, 1);
	gtk_box_pack_start(GTK_BOX(vboxv), libplaypane, TRUE, TRUE, 1);
		gtk_paned_add1(GTK_PANED(libplaypane), library_panel);
		gtk_paned_add2(GTK_PANED(libplaypane), playlist_panel);
		gtk_paned_set_position(GTK_PANED(libplaypane), 500);


	gtk_container_add(GTK_CONTAINER(window), vboxv);

	/* temporary */
	display_library(GTK_TREE_VIEW(treeview_library), pargument->store_library);
	/* temporary */

	gtk_widget_show_all(window);

	gtk_main();
	return 0;
}
