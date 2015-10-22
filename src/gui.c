#include "gui.h"

void destroy(GtkWidget *window, struct arguments *argument) {
	gst_element_set_state(argument->current_song.playbin, GST_STATE_NULL);
	gtk_main_quit();
}

gboolean ignore_destroy(GtkWidget *window, gpointer data) {
	return TRUE;
}

void tags_obtained(GstElement *playbin, gint stream, struct arguments *argument) {
	GstStructure *structure;	
	
	structure = gst_structure_new_empty("tags");

	GstMessage *msg = gst_message_new_application(GST_OBJECT(playbin), structure);

	gst_element_post_message(argument->current_song.playbin, msg);
}

void artist_row_activated(GtkTreeView *treeview, GtkTreePath *path, GtkTreeViewColumn *column, struct arguments *argument) {
	GtkTreeSortable *sortable = GTK_TREE_SORTABLE(argument->store_library);
	gtk_tree_sortable_set_sort_column_id(sortable, TRACKNUMBER, GTK_SORT_ASCENDING);
	GtkTreeModel *model_artist = gtk_tree_view_get_model(treeview);
	GtkTreeModel *model_library = gtk_tree_view_get_model(GTK_TREE_VIEW(argument->treeview_library));
	GtkTreeModel *model_playlist = gtk_tree_view_get_model(GTK_TREE_VIEW(argument->treeview_playlist));
	gchar *songtitle;
	gchar *songalbum;
	gchar *songartist;
	gchar *temptitle;
	gchar *tempalbum;
	gchar *tempartist;
	GtkTreeIter lib_iter;
	gboolean valid;

	clean_playlist(GTK_TREE_VIEW(argument->treeview_playlist), argument);

	if(gtk_tree_path_get_depth(path) == 3) {
		if(!(gtk_tree_model_get_iter(model_artist, &(argument->iter_artist), path))) {
			printf("Error getting artist iter!\n");
			return;
		}
		gtk_tree_model_get(model_artist, &(argument->iter_artist), 0, &songtitle, -1);
		gtk_tree_path_up(path);
		gtk_tree_model_get_iter(model_artist, &(argument->iter_artist), path);
		gtk_tree_model_get(model_artist, &(argument->iter_artist), 0, &songalbum, -1);
		gtk_tree_path_up(path);
		gtk_tree_model_get_iter(model_artist, &(argument->iter_artist), path);
		gtk_tree_model_get(model_artist, &(argument->iter_artist), 0, &songartist, -1);
		songtitle = strstr(songtitle, "  ") + 2;

		add_album_to_playlist(songalbum, songartist, argument);
	
		play_playlist_song(songtitle, argument);
	}
	else if(gtk_tree_path_get_depth(path) == 2) {
		if(!(gtk_tree_model_get_iter(model_artist, &(argument->iter_artist), path))) {
			printf("Error getting artist iter!\n");
			return;
		}
		gtk_tree_model_get_iter(model_artist, &(argument->iter_artist), path);
		gtk_tree_model_get(model_artist, &(argument->iter_artist), 0, &songalbum, -1);
		gtk_tree_path_up(path);
		gtk_tree_model_get_iter(model_artist, &(argument->iter_artist), path);
		gtk_tree_model_get(model_artist, &(argument->iter_artist), 0, &songartist, -1);
		
		add_album_to_playlist(songalbum, songartist, argument);

		gtk_tree_model_get_iter_first(model_playlist, &argument->iter_playlist);
		start_song(argument);
	}
	else if(gtk_tree_path_get_depth(path) == 1) {
		if(!(gtk_tree_model_get_iter(model_artist, &(argument->iter_artist), path))) {
			printf("Error getting artist iter!\n");
			return;
		}
		gtk_tree_model_get(model_artist, &(argument->iter_artist), 0, &songartist, -1);

		add_artist_to_playlist(songartist, argument);
		
		gtk_tree_model_get_iter_first(model_playlist, &argument->iter_playlist);

		start_song(argument);
	}
}

void album_row_activated(GtkTreeView *treeview, GtkTreePath *path, GtkTreeViewColumn *column, struct arguments *argument) {
	GtkTreeSortable *sortable = GTK_TREE_SORTABLE(argument->store_library);
	gtk_tree_sortable_set_sort_column_id(sortable, TRACKNUMBER, GTK_SORT_ASCENDING);
	GtkTreeModel *model_album = gtk_tree_view_get_model(treeview);
	GtkTreeModel *model_library = gtk_tree_view_get_model(GTK_TREE_VIEW(argument->treeview_library));
	GtkTreeModel *model_playlist = gtk_tree_view_get_model(GTK_TREE_VIEW(argument->treeview_playlist));
	gchar *songtitle;
	gchar *songalbum;
	gchar *temptitle;
	gchar *tempalbum;
	GtkTreeIter lib_iter;
	gboolean valid;

	clean_playlist(GTK_TREE_VIEW(argument->treeview_playlist), argument);

	if(gtk_tree_path_get_depth(path) == 2) {
		if(!(gtk_tree_model_get_iter(model_album, &(argument->iter_album), path))) {
			printf("Error getting album iter!\n");
			return;
		}
		gtk_tree_model_get(model_album, &(argument->iter_album), 0, &songtitle, -1);
		gtk_tree_path_up(path);
		gtk_tree_model_get_iter(model_album, &(argument->iter_album), path);
		gtk_tree_model_get(model_album, &(argument->iter_album), 0, &songalbum, -1);
		songtitle = strstr(songtitle, "  ") + 2;

		add_album_to_playlist(songalbum, NULL, argument);
	
		play_playlist_song(songtitle, argument);
	}
	else if(gtk_tree_path_get_depth(path) == 1) {
		if(!(gtk_tree_model_get_iter(model_album, &(argument->iter_album), path))) {
			printf("Error getting album iter!\n");
			return;
		}
		gtk_tree_model_get(model_album, &(argument->iter_album), 0, &songalbum, -1);

		add_album_to_playlist(songalbum, NULL, argument);
		
		gtk_tree_model_get_iter_first(model_playlist, &argument->iter_playlist);

		start_song(argument);
	}
}


void playlist_row_activated(GtkTreeView *treeview, GtkTreePath *path, GtkTreeViewColumn *column, struct arguments *argument) {
	GtkTreeModel *model_playlist = gtk_tree_view_get_model(treeview);

	if(gtk_tree_model_get_iter(model_playlist, &(argument->iter_playlist), path)) {
		start_song(argument);
	}
}

void lib_row_activated(GtkTreeView *treeview, GtkTreePath *path, GtkTreeViewColumn *column, struct arguments *argument) {
	GtkTreeModel *model_library;
	GtkTreeModel *model_playlist;
	GtkTreeIter iter_library;

	model_library = gtk_tree_view_get_model(treeview);
	model_playlist = gtk_tree_view_get_model(GTK_TREE_VIEW(argument->treeview_playlist));

	clean_playlist(GTK_TREE_VIEW(argument->treeview_playlist), argument);
	gtk_tree_model_get_iter_first(model_library, &iter_library);
	do {
		playlist_queue(&iter_library, model_library, GTK_TREE_VIEW(argument->treeview_playlist), argument);
	} while(gtk_tree_model_iter_next(model_library, &iter_library));

	gtk_tree_model_get_iter(model_playlist, &(argument->iter_playlist), path);
	argument->history = g_list_prepend(argument->history, gtk_tree_model_get_string_from_iter(model_playlist, &argument->iter_playlist));

	start_song(argument);
}

void toggle_playpause(struct arguments *argument) {
	if(argument->current_song.state == GST_STATE_PLAYING)
		pause_song(argument);
	else if(argument->current_song.state == GST_STATE_PAUSED)
		resume_song(argument);
}

void toggle_random(GtkWidget *button, struct arguments *argument) {
	argument->random = (argument->random == 1 ? 0 : 1);
}

void toggle_lelele(GtkWidget *button, struct arguments *argument) {
	argument->lelelerandom = (argument->lelelerandom == 1 ? 0 : 1);
}

void toggle_repeat(GtkWidget *button, struct arguments *argument) {
	argument->repeat = (argument->repeat == 1 ? 0 : 1);
}

void toggle_playpause_button(GtkWidget *button, struct arguments *argument) { 
	GtkTreeSelection *selection;
	GtkTreeModel *model;
	GtkTreePath *path;
	GtkTreeIter iter;
	gchar *path_string;
	GList *path_list;
	gint page;
	
	page = gtk_notebook_get_current_page(GTK_NOTEBOOK(argument->libnotebook));

	if(argument->current_song.state == GST_STATE_NULL) {
		if(page == 0) {
			selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(argument->treeview_library));
			if((path_list = gtk_tree_selection_get_selected_rows(selection, &model))) {
				lib_row_activated(GTK_TREE_VIEW(argument->treeview_library), path_list->data, NULL, argument);
			}		
		}
		else if(page == 3) {
			selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(argument->treeview_playlist));
			if(gtk_tree_selection_get_selected(selection, &model, &iter)) {
				path = gtk_tree_model_get_path(model, &iter);
				playlist_row_activated(GTK_TREE_VIEW(argument->treeview_playlist), path, NULL, argument);
			}
		}
	}
	else
		toggle_playpause(argument);
}

void next_buttonf(GtkWidget *button, struct arguments *argument) {
	GtkTreeModel *model_playlist;
	model_playlist = gtk_tree_view_get_model(GTK_TREE_VIEW(argument->treeview_playlist));
	gchar *iter_string;
	
	if((iter_string = gtk_tree_model_get_string_from_iter(model_playlist, &argument->iter_playlist))) {
		argument->history = g_list_prepend(argument->history, iter_string);
		lelele_free_song(&argument->current_song);
		if(get_next_playlist_song(GTK_TREE_VIEW(argument->treeview_playlist), argument)) {
			start_song(argument);
		}
	}
}

void previous_buttonf(GtkWidget *button, struct arguments *argument) {
	lelele_free_song(&argument->current_song);
	if(get_previous_playlist_song(GTK_TREE_VIEW(argument->treeview_playlist), argument)) {
		start_song(argument);
	}
}

gboolean refresh_config_progressbar(struct pref_arguments *argument) {	
	GAsyncQueue *msg_queue = argument->msg_queue;
	gpointer msg = NULL;
	GtkTreeIter iter;
	int nblines = argument->nblines;
	int count = argument->count;
	gchar tempforce[17];
	struct song *song;
	GtkTreeModel *model_library;
	gtk_spinner_start(GTK_SPINNER(argument->spinner));

	do {
		if((msg != NULL)) {
			song = (struct song*)msg;
			if(song->resnum == 0)
				strcpy(tempforce, "Loud");
			else if(song->resnum == 1) {
				strcpy(tempforce, "Calm");
			}
			else
				strcpy(tempforce, "Can't conclude"); 
			
			model_library = gtk_tree_view_get_model(GTK_TREE_VIEW(argument->treeview));
			gtk_list_store_append(argument->store_library, &iter);			
			gtk_list_store_set(argument->store_library, &iter, PLAYING, "", TRACKNUMBER, song->tracknumber, TRACK, song->title, ALBUM, song->album, ARTIST, song->artist,
			 FORCE, (float)song->resnum, FORCE_TEMPO, song->force_vector.x, FORCE_AMP, song->force_vector.y, FORCE_FREQ, song->force_vector.z, FORCE_ATK, song->force_vector.t, TEXTFORCE, tempforce, AFILE, song->filename, -1);
			//display_artist_tab(argument->treeview_artist, argument->store_artist, model_library);
			//display_album_tab(argument->treeview_album, argument->store_album, model_library);
			lelele_free_song(msg);
			g_free(song->filename);
			msg = NULL; 
		}
  	} while(((msg = g_async_queue_try_pop(msg_queue)) != NULL));
	
	if(argument->terminate == TRUE) {
		gtk_spinner_stop(GTK_SPINNER(argument->spinner));
		argument->library_set = TRUE;
		return FALSE;
	}
	else
		return TRUE;
}

void analyze_thread(struct pref_arguments *argument) {
	GDir *dir = g_dir_open(argument->folder, 0, NULL);

	int listfd;
	FILE *list;
	FILE *library;
	gchar *template;

	if((listfd = g_file_open_tmp("XXXXXX", &template, NULL)) == -1) {
		g_warning("Couldn't write config file");
		return;
	}

	list = fdopen(listfd, "w+");

	if(argument->erase) {
		if(!(library = fopen(argument->lib_path, "w"))) {
			g_warning("Couldn't write library file");
			return;
		}
	}
	else {
		if(!(library = fopen(argument->lib_path, "a+"))) {
			g_warning("Couldn't write library file");
			return;
		}
	}
	
	explore(dir, argument->folder, list);
	int nblines = 0;
	char line[PATH_MAX];
	GAsyncQueue *msg_queue = g_async_queue_new();
	argument->msg_queue = msg_queue;
	char *msg;

	fseek(list, 0, SEEK_SET);

	while(fgets(line, PATH_MAX, list) != NULL)
		nblines++;

	argument->nblines = nblines;
	fseek(list, 0, SEEK_SET);

	gpointer msg_thread;
	gchar tempstring[PATH_MAX];
	FILE *library_read = fopen(argument->lib_path, "r");
	int resnum;
	argument->count = 0;
	gboolean found = FALSE;
	argument->terminate = FALSE;

	g_idle_add((GSourceFunc)refresh_config_progressbar, argument);

	while(fgets(line, PATH_MAX, list) != NULL) {
		line[strcspn(line, "\n")] = '\0';
		rewind(library_read);
		found = FALSE;
		while(fgets(tempstring, PATH_MAX, library_read)) {
			if(strstr(tempstring, line)) {	
				found = TRUE;
			}
		}
		if(found == FALSE) {
			struct song song;
			struct song msg_song;
			if((resnum = lelele_analyze(line, &song, 0, 1)) < 3) {
	//		fprintf(test, "%f %f %f\n", song.force_vector.x, song.force_vector.y, song.force_vector.z);
				fprintf(library, "%s\n%s\n%s\n%s\n%s\n%d\n%f\n%f\n%f\n%f\n", line, song.tracknumber, song.title, song.album, song.artist, resnum, song.force_vector.x,
					song.force_vector.y, song.force_vector.z, song.force_vector.t);
				msg_song = song;
				msg_song.tracknumber = g_malloc(strlen(song.tracknumber)+1);
				msg_song.tracknumber = strcpy(msg_song.tracknumber, song.tracknumber);
				msg_song.title = g_malloc(strlen(song.title)+1);
				msg_song.title = strcpy(msg_song.title, song.title);
				msg_song.album = g_malloc(strlen(song.album)+1);
				msg_song.album = strcpy(msg_song.album, song.album);	
				msg_song.artist= g_malloc(strlen(song.artist)+1);
				msg_song.artist = strcpy(msg_song.artist, song.artist);
				msg_song.sample_array = NULL;
				msg_song.resnum = (int)resnum;
				msg_song.filename = g_malloc(strlen(line)+1);
				msg_song.filename = strcpy(msg_song.filename, line);
				g_async_queue_push(msg_queue, (gpointer)&msg_song);
				lelele_free_song(&song);
				//fflush(library); 
			}
		}
		argument->count++;
	}
	argument->terminate = TRUE;
	g_async_queue_unref(msg_queue);
	msg_queue = NULL;
	g_close(listfd, NULL);
	fclose(list);
	g_remove(template);
	fclose(library);
	fclose(library_read);
	//fclose(test);
}

void config_folder_changed(const gchar *folder, gchar *library_path, GtkWidget *parent, gboolean erase) {
/*
	GtkWidget *progressbar, *progressdialog, *area;
	progressbar = gtk_progress_bar_new();
	progressdialog = gtk_dialog_new();
	gtk_window_set_transient_for(GTK_WINDOW(progressdialog), GTK_WINDOW(parent));
	if(g_file_test("../images/lelele.svg", G_FILE_TEST_EXISTS))
		gtk_window_set_icon_from_file(GTK_WINDOW(progressdialog), "../images/lelele.svg", NULL);
	else
		gtk_window_set_icon_from_file(GTK_WINDOW(progressdialog), "/usr/share/leleleplayer/icons/lelele.svg", NULL);
	area = gtk_dialog_get_content_area(GTK_DIALOG(progressdialog));
	GDir *dir = g_dir_open (folder, 0, NULL);
	int listfd;
	FILE *list;
	FILE *library;
	gchar *template;

	if((listfd = g_file_open_tmp("XXXXXX", &template, NULL)) == -1) {
		g_warning("Couldn't write config file");
		return;
	}

	list = fdopen(listfd, "w+");
	
	if(erase) {
		if(!(library = fopen(library_path, "w"))) {
			g_warning("Couldn't write library file");
			return;
		}
	}
	else {
		if(!(library = fopen(library_path, "a+"))) {
			g_warning("Couldn't write library file");
			return;
		}
	}
	
	explore(dir, folder, list);
	int nblines = 0;
	char line[PATH_MAX];
	int count = 0;
	GAsyncQueue *msg_queue = g_async_queue_new();
	char *msg;
	struct pref_folder_arguments argument;
	argument.progressbar = progressbar;
	argument.progressdialog = progressdialog;
	argument.lib_path = library_path;
	argument.folder = folder;

	fseek(list, 0, SEEK_SET);

	while(fgets(line, PATH_MAX, list) != NULL)
		nblines++;

	argument.nblines = nblines;
	fseek(list, 0, SEEK_SET)
	g_signal_connect(G_OBJECT(progressdialog), "delete-event", G_CALLBACK(ignore_destroy), NULL);

	gtk_progress_bar_set_ellipsize(GTK_PROGRESS_BAR(progressbar), PANGO_ELLIPSIZE_END);
	gtk_box_set_homogeneous(GTK_BOX(area), TRUE);
	gtk_widget_set_size_request(progressbar, 300, 20);
	gtk_box_pack_start(GTK_BOX(area), progressbar, TRUE, TRUE, 0);
	gtk_progress_bar_set_show_text(GTK_PROGRESS_BAR(progressbar), 1);
	//gtk_window_set_keep_above(GTK_WINDOW(progressdialog), TRUE);
	gtk_window_set_deletable(GTK_WINDOW(progressdialog), FALSE);
	gtk_window_set_modal(GTK_WINDOW(progressdialog), TRUE);
	gtk_widget_show_all(progressdialog);

	argument.msg_queue = msg_queue;
	argument.line = line;
	argument.list = list;
	argument.library = library;

	gtk_dialog_run(GTK_DIALOG(progressdialog));
	gtk_widget_destroy(progressdialog);*/
	/* WARING: can explode here */
	/*g_async_queue_unref(msg_queue);
	msg_queue = NULL;
	g_close(listfd, NULL);
	fclose(list);
	g_remove(template);
	fclose(library);*/
}

void folder_chooser(GtkWidget *button, struct pref_arguments *argument) {
	GtkWidget *dialog;
	gint res;
	GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER;

	dialog = gtk_file_chooser_dialog_new("Choose library folder", GTK_WINDOW(argument->window),
		action, "Cancel", GTK_RESPONSE_CANCEL, "Open", GTK_RESPONSE_ACCEPT, NULL);

	if(g_file_test("../images/lelele.svg", G_FILE_TEST_EXISTS))
		gtk_window_set_icon_from_file(GTK_WINDOW(dialog), "../images/lelele.svg", NULL);
	else
		gtk_window_set_icon_from_file(GTK_WINDOW(dialog), "/usr/share/leleleplayer/icons/lelele.svg", NULL);

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
	
void preferences_callback(GtkMenuItem *preferences, struct pref_arguments *argument) {
	GtkWidget *dialog, *label_library, *label_browse, *area, *vbox, *hbox, *labelbox, *library_entry, *browse_button, *window_temp, *complete_box;
	GtkDialogFlags flags = GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT;	
	gint res;
	const gchar *old_folder;

	label_library = gtk_label_new("");
	label_browse = gtk_label_new("");
	gtk_label_set_markup(GTK_LABEL(label_library), "<span weight=\"bold\">Library management:</span>");
	gtk_label_set_markup(GTK_LABEL(label_browse), "Select library location:");
	browse_button = gtk_button_new_with_label("Browse...");
	library_entry = gtk_entry_new();
	argument->folder = (gchar*)g_variant_get_data(g_settings_get_value(argument->preferences, "library"));
	gtk_entry_set_text((GtkEntry*)library_entry, argument->folder);
	vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
	hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
	labelbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
	dialog = gtk_dialog_new_with_buttons("Preferences", GTK_WINDOW(argument->window), flags, "Cancel", GTK_RESPONSE_REJECT, "Save", GTK_RESPONSE_ACCEPT, NULL);
	if(g_file_test("../images/lelele.svg", G_FILE_TEST_EXISTS))
		gtk_window_set_icon_from_file(GTK_WINDOW(dialog), "../images/lelele.svg", NULL);
	else
		gtk_window_set_icon_from_file(GTK_WINDOW(dialog), "/usr/share/leleleplayer/icons/lelele.svg", NULL);

	area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
	complete_box = gtk_check_button_new_with_label("LeleleScan (complete but longer) (not functionnal now)");
	gtk_window_set_default_size(GTK_WINDOW(dialog), 500, 100);
	gtk_window_set_resizable(GTK_WINDOW(dialog), FALSE);

	g_signal_connect(G_OBJECT(browse_button), "clicked", G_CALLBACK(folder_chooser), argument);
	argument->library_entry = library_entry;
	window_temp = argument->window;
	argument->window = dialog;

	gtk_box_set_homogeneous(GTK_BOX(vbox), TRUE);
	gtk_box_set_homogeneous(GTK_BOX(hbox), FALSE);
	
	gtk_box_pack_start(GTK_BOX(vbox), labelbox, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(labelbox), label_library, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
		gtk_box_pack_start(GTK_BOX(hbox), label_browse, FALSE, FALSE, 0);
		gtk_box_pack_start(GTK_BOX(hbox), library_entry, TRUE, TRUE, 0);
		gtk_box_pack_start(GTK_BOX(hbox), browse_button, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), complete_box, FALSE, FALSE, 0);
	gtk_container_add(GTK_CONTAINER(area), vbox);

	gtk_widget_set_size_request(dialog, 400, 120);
	gtk_widget_show_all(dialog);
	old_folder = argument->folder;
	res = gtk_dialog_run(GTK_DIALOG(dialog));

	if(argument->folder != NULL && res == GTK_RESPONSE_ACCEPT) {
		argument->folder = gtk_entry_get_text(GTK_ENTRY(library_entry));
		g_settings_set_value(argument->preferences, "library", g_variant_new_string(argument->folder));
		if(strcmp(old_folder, argument->folder)) {
			argument->erase = TRUE;
		}
		else {
			argument->erase = FALSE;
		}
		g_thread_new("analyze", (GThreadFunc)analyze_thread, argument);
	//	display_library(GTK_TREE_VIEW(argument->treeview), argument->store_library, argument->lib_path);
	}
	gtk_widget_destroy(dialog); 
	argument->window = window_temp;
}

void add_file_to_playlist(GtkMenuItem *add_file, struct arguments *argument) {
	struct song song;
	int resnum;
	GtkWidget *dialog;
	GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_OPEN;
	gint res;
	GSList *filelist;
	char *filename;
	GtkTreeModel *model_playlist;
	GtkTreeIter iter_playlist;
	model_playlist = gtk_tree_view_get_model(GTK_TREE_VIEW(argument->treeview_playlist));

	dialog = gtk_file_chooser_dialog_new("Open audio file(s)", GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(add_file))),
	action, "Cancel", GTK_RESPONSE_CANCEL, "Open", GTK_RESPONSE_ACCEPT, NULL);
	if(g_file_test("../images/lelele.svg", G_FILE_TEST_EXISTS))
		gtk_window_set_icon_from_file(GTK_WINDOW(dialog), "../images/lelele.svg", NULL);
	else
		gtk_window_set_icon_from_file(GTK_WINDOW(dialog), "/usr/share/leleleplayer/icons/lelele.svg", NULL);

	GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialog);

	gtk_file_chooser_set_select_multiple(chooser, TRUE);

	res = gtk_dialog_run(GTK_DIALOG(dialog));
	if(res == GTK_RESPONSE_ACCEPT) {
		if(argument->iter_playlist.stamp)
			argument->history = g_list_prepend(argument->history, gtk_tree_model_get_string_from_iter(model_playlist, &argument->iter_playlist));
		for(filelist = gtk_file_chooser_get_filenames(chooser); filelist; filelist = filelist->next) {
			GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialog);
			filename = (gchar*)filelist->data;
			if((resnum = lelele_analyze(filename, &song, 0, 0)) == 0)
				printf("Couldn't conclude\n");
			else {
				gtk_list_store_append(argument->store_playlist, &iter_playlist);
				gtk_list_store_set(argument->store_playlist, &iter_playlist, PLAYING, "", -1);
				gtk_list_store_set(argument->store_playlist, &iter_playlist, TRACKNUMBER, song.tracknumber, -1);
				gtk_list_store_set(argument->store_playlist, &iter_playlist, TRACK, song.title, -1);
				gtk_list_store_set(argument->store_playlist, &iter_playlist, ALBUM, song.album, -1);
				gtk_list_store_set(argument->store_playlist, &iter_playlist, ARTIST, song.artist, -1);		
				gtk_list_store_set(argument->store_playlist, &iter_playlist, FORCE_TEMPO, song.force_vector.x, -1);
				gtk_list_store_set(argument->store_playlist, &iter_playlist, FORCE_AMP, song.force_vector.y, -1);
				gtk_list_store_set(argument->store_playlist, &iter_playlist, FORCE_FREQ, song.force_vector.z, -1);
				gtk_list_store_set(argument->store_playlist, &iter_playlist, FORCE_ATK, song.force_vector.t, -1);
				if(resnum > 0)
					gtk_list_store_set(argument->store_playlist, &iter_playlist, TEXTFORCE, "Loud", -1);
				else if(resnum < 0)
					gtk_list_store_set(argument->store_playlist, &iter_playlist, TEXTFORCE, "Calm", -1);
				gtk_list_store_set(argument->store_playlist, &iter_playlist, AFILE, filename, -1);
				argument->playlist_count++;

				argument->iter_playlist = iter_playlist;
				lelele_free_song(&song);
			}
			g_free(filename);
		}
	}
	gtk_widget_destroy(dialog);
}

void open_audio_file(GtkMenuItem *open, struct arguments *argument) {
	struct song song;
	int resnum;
	GtkWidget *dialog;
	GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_OPEN;
	GtkTreeIter iter_playlist;
	gint res;
	GtkTreeModel *model_playlist;
	model_playlist = gtk_tree_view_get_model(GTK_TREE_VIEW(argument->treeview_playlist));

	dialog = gtk_file_chooser_dialog_new("Open audio file", GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(open))),
	action, "Cancel", GTK_RESPONSE_CANCEL, "Open", GTK_RESPONSE_ACCEPT, NULL);

	res = gtk_dialog_run(GTK_DIALOG(dialog));
	if(res == GTK_RESPONSE_ACCEPT) {
		if(argument->iter_playlist.stamp)
			argument->history = g_list_prepend(argument->history, gtk_tree_model_get_string_from_iter(model_playlist, &argument->iter_playlist));
		char *filename;
		GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialog);
		filename = gtk_file_chooser_get_filename(chooser);
		if((resnum = lelele_analyze(filename, &song, 0, 0)) == 0)
			printf("Couldn't conclude\n");
		else { // Create an iter_forge() function?
			gtk_list_store_append(argument->store_playlist, &iter_playlist);
			gtk_list_store_set(argument->store_playlist, &iter_playlist, PLAYING, "", -1);
			gtk_list_store_set(argument->store_playlist, &iter_playlist, TRACKNUMBER, song.tracknumber, -1);
			gtk_list_store_set(argument->store_playlist, &iter_playlist, TRACK, song.title, -1);
			gtk_list_store_set(argument->store_playlist, &iter_playlist, ALBUM, song.album, -1);
			gtk_list_store_set(argument->store_playlist, &iter_playlist, ARTIST, song.artist, -1);		
			gtk_list_store_set(argument->store_playlist, &iter_playlist, FORCE_TEMPO, song.force_vector.x, -1);
			gtk_list_store_set(argument->store_playlist, &iter_playlist, FORCE_AMP, song.force_vector.y, -1);
			gtk_list_store_set(argument->store_playlist, &iter_playlist, FORCE_FREQ, song.force_vector.z, -1);
			gtk_list_store_set(argument->store_playlist, &iter_playlist, FORCE_ATK, song.force_vector.t, -1);
			if(resnum > 0)
				gtk_list_store_set(argument->store_playlist, &iter_playlist, TEXTFORCE, "Loud", -1);
			else if(resnum < 0)
				gtk_list_store_set(argument->store_playlist, &iter_playlist, TEXTFORCE, "Calm", -1);
			else
				gtk_list_store_set(argument->store_playlist, &iter_playlist, TEXTFORCE, "Couldn't conclude", -1);
			gtk_list_store_set(argument->store_playlist, &iter_playlist, AFILE, filename, -1);
			argument->playlist_count++;

			argument->iter_playlist = iter_playlist;
			lelele_free_song(&song);
			start_song(argument);
		}
		g_free(filename);
	}	
	gtk_widget_destroy(dialog);
}

void reset_ui(struct arguments *argument) {
	GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(argument->treeview_playlist));

	gtk_button_set_image(GTK_BUTTON(argument->playpause_button), gtk_image_new_from_icon_name("media-playback-start-symbolic", GTK_ICON_SIZE_BUTTON));
	gtk_list_store_set(argument->store_playlist, &(argument->iter_playlist), PLAYING, "", -1);
	gst_element_set_state(argument->current_song.playbin, GST_STATE_NULL);

	if(argument->bartag)
		g_source_remove(argument->bartag);
	argument->bartag = 0;

	gtk_widget_set_sensitive(argument->progressbar, FALSE);
	g_signal_handler_block(argument->progressbar, argument->progressbar_update_signal_id);
	gtk_adjustment_configure(argument->adjust, 0, 0, argument->current_song.duration/GST_SECOND, 
		1, 1, 1);
	g_signal_handler_unblock(argument->progressbar, argument->progressbar_update_signal_id);

	gtk_tree_selection_unselect_all(selection);

	gtk_label_set_text(GTK_LABEL(argument->title_label), "");
	gtk_label_set_text(GTK_LABEL(argument->album_label), "");
	gtk_label_set_markup(GTK_LABEL(argument->album_label), "<span foreground=\"grey\">No song currently playing</span>");
	gtk_label_set_text(GTK_LABEL(argument->artist_label), ""); 
	gtk_label_set_text(GTK_LABEL(argument->genre_label), "Genre:");
	gtk_label_set_text(GTK_LABEL(argument->samplerate_label), "Sample rate:");
	gtk_label_set_text(GTK_LABEL(argument->bitrate_label), "Bitrate:");
	gtk_label_set_text(GTK_LABEL(argument->channels_label), "Channels:");

}

void refresh_ui(GstBus *bus, GstMessage *msg, struct arguments *argument) {
	GtkTreeSelection *selection;
	GtkTreePath *path;
	GtkTreeViewColumn *column;
	GtkTreeModel *model_playlist;
	GtkTreeIter temp_iter;
	
	GstFormat fmt = GST_FORMAT_TIME;

	if((model_playlist = gtk_tree_view_get_model(GTK_TREE_VIEW(argument->treeview_playlist)))) {
		selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(argument->treeview_playlist));
		path = gtk_tree_model_get_path(model_playlist, &argument->iter_playlist);
		column = gtk_tree_view_get_column(GTK_TREE_VIEW(argument->treeview_playlist), PLAYING);

		gtk_tree_model_get_iter_first(gtk_tree_view_get_model(GTK_TREE_VIEW(argument->treeview_playlist)), &(temp_iter));

		do {
			gtk_list_store_set(argument->store_playlist, &(temp_iter), PLAYING, "", -1);
		}
		while(gtk_tree_model_iter_next(model_playlist, &(temp_iter)));
		gtk_button_set_image(GTK_BUTTON(argument->playpause_button), gtk_image_new_from_icon_name("media-playback-pause-symbolic", GTK_ICON_SIZE_BUTTON));
		gtk_list_store_set(argument->store_playlist, &(argument->iter_playlist), PLAYING, "▶", -1);
		gtk_tree_selection_unselect_all(selection);
		gtk_tree_selection_select_iter(selection, &(argument->iter_playlist));
		gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(argument->treeview_playlist), path, column, 1, 0.3, 1);
	
		gtk_label_set_text(GTK_LABEL(argument->album_label), argument->current_song.album);
		gtk_label_set_text(GTK_LABEL(argument->artist_label), argument->current_song.artist);
		gtk_label_set_text(GTK_LABEL(argument->title_label), argument->current_song.title);	
	}
	while(!gst_element_query_duration(argument->current_song.playbin, fmt,
		&(argument->current_song.duration)))
		;
	gst_element_query_position(argument->current_song.playbin, 
		fmt, &(argument->current_song.current));

	g_signal_handler_block(argument->progressbar, argument->progressbar_update_signal_id);
	gtk_adjustment_configure(argument->adjust, 0, 0, argument->current_song.duration/GST_SECOND, 
		1, 1, 1);
	g_signal_handler_unblock(argument->progressbar, argument->progressbar_update_signal_id);
	refresh_progressbar(argument);
}

void message_application(GstBus *bus, GstMessage *msg, struct arguments *argument) {
	const GstStructure *structure = gst_message_get_structure(msg);
	if(!strcmp(gst_structure_get_name(structure), "tags")) {
		GstTagList *tags;
		gchar *str;
		gint samplerate = 0, channels = 0, bitrate;
		GstStructure *s;
		GstPad *pad;
		GstCaps *caps;

		g_signal_emit_by_name(argument->current_song.playbin, "get-audio-pad", 0, &pad);
		caps = gst_pad_get_current_caps(pad);

		s = gst_caps_get_structure(caps, 0);

		gst_structure_get_int(s, "channels", &channels);
		gst_structure_get_int(s, "rate", &samplerate);

		if(channels) {
			argument->str_channels = g_strdup_printf("Channels: %d", channels);
		}
		else {
			argument->str_channels = g_strdup("Number of channels not found");
		}

		if(samplerate) {
			argument->str_samplerate = g_strdup_printf("Sample rate: %dHz", samplerate);
		}
		else {
			argument->str_samplerate = g_strdup("Sample rate not found");
		}

		g_signal_emit_by_name(argument->current_song.playbin, "get-audio-tags", 0, &tags);
		if(tags) {
			if(gst_tag_list_get_string(tags, GST_TAG_GENRE, &str)) {
				argument->str_genre = g_strdup_printf("Genre: %s", str);
				g_free(str);
				str = NULL;
			}
			else {
				argument->str_genre = g_strdup("No genre found");
			}

			if(gst_tag_list_get_uint(tags, GST_TAG_BITRATE, &bitrate)) {
				argument->str_bitrate = g_strdup_printf("Bit rate: %dkB/s", bitrate/1000);
			}
			else {
				argument->str_bitrate = g_strdup("Bitrate not found");
			}
		}
		gst_tag_list_free(tags);

		gtk_label_set_text(GTK_LABEL(argument->channels_label), argument->str_channels);
		g_free(argument->str_channels);
		gtk_label_set_text(GTK_LABEL(argument->samplerate_label), argument->str_samplerate);
		g_free(argument->str_samplerate);
		argument->str_samplerate = NULL;
		argument->str_channels = NULL;
		gtk_label_set_text(GTK_LABEL(argument->genre_label), argument->str_genre);
		g_free(argument->str_genre);
		argument->str_genre = NULL;
		gtk_label_set_text(GTK_LABEL(argument->bitrate_label), argument->str_bitrate);
		g_free(argument->str_bitrate);
		argument->str_bitrate = NULL;
	}
	else if(!strcmp(gst_structure_get_name(structure), "next_song")) {
		GtkTreeModel *model_playlist;

		model_playlist = gtk_tree_view_get_model(GTK_TREE_VIEW(argument->treeview_playlist));
		
		argument->history = g_list_prepend(argument->history, gtk_tree_model_get_string_from_iter(model_playlist, &argument->iter_playlist));
		if(!argument->repeat) {
			lelele_free_song(&argument->current_song); 
			if(get_next_playlist_song(GTK_TREE_VIEW(argument->treeview_playlist), argument)) {
				GtkTreeModel *model_playlist;
	
				model_playlist = gtk_tree_view_get_model(GTK_TREE_VIEW(argument->treeview_playlist));
				gtk_tree_model_get(model_playlist, &(argument->iter_playlist), AFILE, &argument->current_song.filename, 
				TRACKNUMBER, &argument->current_song.tracknumber, TRACK, &argument->current_song.title, 
				ALBUM, &argument->current_song.album, ARTIST, &argument->current_song.artist, 
				FORCE, &argument->current_song.force, FORCE_TEMPO, &argument->current_song.force_vector.x, 
				FORCE_AMP, &argument->current_song.force_vector.y, FORCE_FREQ, &argument->current_song.force_vector.z, 
				FORCE_ATK, &argument->current_song.force_vector.t, -1);
				g_mutex_lock(&argument->queue_mutex);
			}
		}
	
		g_cond_signal(&argument->queue_cond);
		g_mutex_unlock(&argument->queue_mutex);
	}
}

void ui_playlist_changed(GtkTreeModel *playlist_model, GtkTreePath *path, GtkTreeIter *iter, GtkNotebook *libnotebook) {
	gtk_notebook_set_current_page(libnotebook, 3);
}

gboolean refresh_progressbar(gpointer pargument) {
	struct arguments *argument = (struct arguments*)pargument;
	GstFormat fmt = GST_FORMAT_TIME;

	if(argument->sleep_timer) {
		gdouble elapsed = g_timer_elapsed(argument->sleep_timer, NULL);
	
		GtkAdjustment *adjustment;
		if(argument->timer_delay - elapsed < -1.) {
			reset_ui(argument);	
			g_timer_destroy(argument->sleep_timer);
			argument->sleep_timer = NULL;
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(argument->time_checkbox), FALSE);
		}
		else {
			g_signal_handler_block(argument->time_spin, argument->time_spin_update_signal_id);
			adjustment = gtk_spin_button_get_adjustment(GTK_SPIN_BUTTON(argument->time_spin));
			gtk_adjustment_set_value(adjustment, argument->timer_delay - elapsed);
			g_signal_handler_unblock(argument->time_spin, argument->time_spin_update_signal_id);
			time_spin_output(GTK_SPIN_BUTTON(argument->time_spin), argument);
		}
	}

	if(argument->current_song.state < GST_STATE_PAUSED) {
		return TRUE;
	}
	
	if(gst_element_query_position(argument->current_song.playbin, 
		fmt, &(argument->current_song.current))) {
		g_signal_handler_block(argument->progressbar, argument->progressbar_update_signal_id);
		gtk_adjustment_configure(argument->adjust, argument->current_song.current/GST_SECOND, 0, argument->current_song.duration/GST_SECOND, 
			1, 1, 1);
		g_signal_handler_unblock(argument->progressbar, argument->progressbar_update_signal_id);
		return TRUE;
	}
	else
		return FALSE;
}

void slider_changed(GtkRange *progressbar, struct arguments *argument) {
	if(!gst_element_seek(argument->current_song.playbin, 1.0, GST_FORMAT_TIME, GST_SEEK_FLAG_FLUSH, 
		GST_SEEK_TYPE_SET, gtk_adjustment_get_value(argument->adjust)*GST_SECOND, GST_SEEK_TYPE_NONE,
		GST_CLOCK_TIME_NONE)) 
		g_warning("Seek failed!\n");
} 

void volume_scale_changed(GtkScaleButton* volume_scale, gdouble value, struct arguments *argument) {
	double vol = pow(value, 3);
	g_object_set(argument->current_song.playbin, "volume", vol, NULL);
}

void display_artist_tab(GtkWidget *treeview, GtkTreeStore *treestore, GtkTreeModel *model_library) {
	gtk_tree_store_clear(treestore);

	GtkTreeIter toplevel, child, lowlevel, tempiter_artist, tempiter, tempiter2;
	gboolean same_artists;
	gchar *tempartist1, *tempartist2;
	gchar *tempalbum1, *tempalbum2;
	gchar *temptrack;
	gchar *temptracknumber1, *temptracknumber2;
	GtkTreeModel *model_artist = gtk_tree_view_get_model(GTK_TREE_VIEW(treeview));
	tempartist1 = tempartist2 = tempalbum1 = tempalbum2 = temptrack = temptracknumber1 = temptracknumber2 = NULL;

	if(gtk_tree_model_get_iter_first(model_library, &tempiter_artist)) {
		do {
			gtk_tree_model_get(model_library, &tempiter_artist, ARTIST, &tempartist1, -1);
			if(g_strcmp0(tempartist1, tempartist2)) {
				same_artists = FALSE;
			 	gtk_tree_store_append(treestore, &toplevel, NULL);
  				gtk_tree_store_set(treestore, &toplevel,
                COLUMN_ARTIST, tempartist1, -1);
			
				g_free(tempartist1);
				g_free(tempartist2);
				tempartist1 = tempartist2 = NULL;

				gtk_tree_model_get(model_library, &tempiter_artist, ARTIST, &tempartist2, -1);
			}
			else
				same_artists = TRUE;

			gtk_tree_model_get(model_library, &tempiter_artist, ALBUM, &tempalbum1, -1);
			if(same_artists == FALSE || g_strcmp0(tempalbum1, tempalbum2)) {
				gtk_tree_store_append(treestore, &child, &toplevel);
  				gtk_tree_store_set(treestore, &child,
                COLUMN_ARTIST, tempalbum1, -1);

				g_free(tempalbum1);
				g_free(tempalbum2);
				tempalbum2 = tempalbum1 = NULL;

				gtk_tree_model_get(model_library, &tempiter_artist, ALBUM, &tempalbum2, -1);
			}
			gtk_tree_model_get(model_library, &tempiter_artist, TRACK, &temptrack, -1);
			gtk_tree_model_get(model_library, &tempiter_artist, TRACKNUMBER, &temptracknumber1, -1);
			gchar *song = g_strconcat(temptracknumber1, "  ", temptrack, NULL);

			if(gtk_tree_model_iter_children(model_artist, &tempiter, &child) == TRUE) {
				gboolean valid = TRUE;
				do {
					gtk_tree_model_get(model_artist, &tempiter, COLUMN_ARTIST, &temptracknumber2, -1);
					int i;
					for(i = 0; temptracknumber2[i] != ' '; ++i)
						;
					temptracknumber2[i] = '\0';
					if(atol(temptracknumber1) < atol(temptracknumber2)) {
						gtk_tree_store_insert_before(treestore, &tempiter2, &child, &tempiter);
						gtk_tree_store_set(treestore, &tempiter2, COLUMN_ARTIST, song, -1);
						break;
					}
				} while((valid = gtk_tree_model_iter_next(model_artist, &tempiter)));
				if(valid == FALSE) {
					gtk_tree_store_append(treestore, &tempiter, &child);
					gtk_tree_store_set(treestore, &tempiter, COLUMN_ARTIST, song, -1);
				}
			}	
			else {
				gtk_tree_store_append(treestore, &lowlevel, &child);
				gtk_tree_store_set(treestore, &lowlevel, COLUMN_ARTIST, song, -1);
			}

			g_free(temptrack);
			g_free(temptracknumber1);
			g_free(temptracknumber2);
			g_free(song);
			temptrack = temptracknumber1 = temptracknumber2 = song = NULL;
		} while(gtk_tree_model_iter_next(model_library, &tempiter_artist));
	}
}

void display_album_tab(GtkWidget *treeview, GtkTreeStore *treestore, GtkTreeModel *model_library) {
	gtk_tree_store_clear(treestore);

	GtkTreeIter toplevel, child, lowlevel, tempiter_album, tempiter, tempiter2;
	gchar *tempalbum1, *tempalbum2;
	gchar *temptrack;
	gchar *temptracknumber1, *temptracknumber2;

	GtkTreeModel *model_album = gtk_tree_view_get_model(GTK_TREE_VIEW(treeview));
	tempalbum1 = tempalbum2 = temptrack = temptracknumber1 = temptracknumber2 = NULL;

	if(gtk_tree_model_get_iter_first(model_library, &tempiter_album)) {
		do {
			gtk_tree_model_get(model_library, &tempiter_album, ALBUM, &tempalbum1, -1);
			if(g_strcmp0(tempalbum1, tempalbum2)) {
				gtk_tree_store_append(treestore, &toplevel, NULL);
  				gtk_tree_store_set(treestore, &toplevel,
                COLUMN_ARTIST, tempalbum1, -1);

				g_free(tempalbum1);
				g_free(tempalbum2);
				tempalbum2 = tempalbum1 = NULL;

				gtk_tree_model_get(model_library, &tempiter_album, ALBUM, &tempalbum2, -1);
			}
			gtk_tree_model_get(model_library, &tempiter_album, TRACK, &temptrack, -1);
			gtk_tree_model_get(model_library, &tempiter_album, TRACKNUMBER, &temptracknumber1, -1);
			gchar *song = g_strconcat(temptracknumber1, "  ", temptrack, NULL);

			if(gtk_tree_model_iter_children(model_album, &tempiter, &toplevel) == TRUE) {
				gboolean valid = TRUE;
				do {
					gtk_tree_model_get(model_album, &tempiter, COLUMN_ALBUM, &temptracknumber2, -1);
					int i;
					for(i = 0; temptracknumber2[i] != ' '; ++i)
						;
					temptracknumber2[i] = '\0';
					if(atol(temptracknumber1) < atol(temptracknumber2)) {
						gtk_tree_store_insert_before(treestore, &tempiter2, &toplevel, &tempiter);
						gtk_tree_store_set(treestore, &tempiter2, COLUMN_ALBUM, song, -1);
						break;
					}
				} while((valid = gtk_tree_model_iter_next(model_album, &tempiter)));
				if(valid == FALSE) {
					gtk_tree_store_append(treestore, &tempiter, &toplevel);
					gtk_tree_store_set(treestore, &tempiter, COLUMN_ALBUM, song, -1);
				}
			}	
			else {
				gtk_tree_store_append(treestore, &child, &toplevel);
				gtk_tree_store_set(treestore, &child, COLUMN_ALBUM, song, -1);
			}

			g_free(temptrack);
			g_free(temptracknumber1);
			g_free(temptracknumber2);
			g_free(song);
			temptrack = temptracknumber1 = temptracknumber2 = song = NULL;
		} while(gtk_tree_model_iter_next(model_library, &tempiter_album));
	}
}

void setup_tree_view_renderer_artist(GtkWidget *treeview, GtkTreeStore *treestore, GtkTreeModel *model_library) {
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;

	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes("Artist", renderer, "text", COLUMN_ARTIST, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
	g_object_set(renderer, "size", 13*PANGO_SCALE, NULL);
}

void setup_tree_view_renderer_album(GtkWidget *treeview, GtkTreeStore *treestore, GtkTreeModel *model_library) {
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;

	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes("Album", renderer, "text", COLUMN_ARTIST, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
	g_object_set(renderer, "size", 13*PANGO_SCALE, NULL);
}

void add_album_selection_to_playlist(GtkWidget *menuitem, struct arguments *argument) {
	GtkTreeSortable *sortable = GTK_TREE_SORTABLE(argument->store_library);
	gtk_tree_sortable_set_sort_column_id(sortable, TRACKNUMBER, GTK_SORT_ASCENDING);
	GtkTreeModel *model;
	GtkTreeIter iter;
	GtkTreePath *path;
	GtkTreeSelection *selection;
	GList *path_list;
	gchar *path_string;
	GtkWidget *page = gtk_notebook_get_nth_page(GTK_NOTEBOOK(argument->libnotebook), 3);
	GtkWidget *label = gtk_notebook_get_tab_label(GTK_NOTEBOOK(argument->libnotebook), page);

	GtkTreeModel *model_playlist = gtk_tree_view_get_model(GTK_TREE_VIEW(argument->treeview_playlist));

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(argument->treeview_album));

	if((path_list = gtk_tree_selection_get_selected_rows(selection, &model))) {
		g_signal_handler_block(model_playlist, argument->playlist_update_signal_id);
		while(path_list) {
			path = path_list->data;
			GtkTreeModel *model_album= gtk_tree_view_get_model(GTK_TREE_VIEW(argument->treeview_album));

			GtkTreeModel *model_library = gtk_tree_view_get_model(GTK_TREE_VIEW(argument->treeview_library));

			gchar *songtitle;
			gchar *songalbum;
			gchar *songartist;
			gchar *temptitle;
			gchar *tempalbum;
			GtkTreeIter lib_iter;
			gboolean valid;
			if(gtk_tree_path_get_depth(path) == 2) {
				if(!(gtk_tree_model_get_iter(model_album, &(argument->iter_album), path))) {
					printf("Error getting album iter!\n");
					return;
				}
				gtk_tree_model_get(model_album, &(argument->iter_album), 0, &songtitle, -1);
				gtk_tree_path_up(path);
				gtk_tree_model_get_iter(model_album, &(argument->iter_album), path);
				gtk_tree_model_get(model_album, &(argument->iter_album), 0, &songalbum, -1);
				songtitle = strstr(songtitle, "  ") + 2;
				gboolean valid;
	
				valid = gtk_tree_model_get_iter_first(model_library, &iter);
				while(valid) {
					gtk_tree_model_get(model_library, &iter, ALBUM, &tempalbum, TRACK, &temptitle, -1);
	
					if((!strcmp(tempalbum, songalbum)) 
					&& (!strcmp(temptitle, songtitle))) {
						playlist_queue(&iter, model_library, GTK_TREE_VIEW(argument->treeview_playlist), argument);
						break;
					}
					valid = gtk_tree_model_iter_next(model_library, &iter);
				}
			}
			else if(gtk_tree_path_get_depth(path) == 1) {
				if(!(gtk_tree_model_get_iter(model_album, &(argument->iter_album), path))) {
					printf("Error getting album iter!\n");
					return;
				}
			
				gtk_tree_model_get_iter(model_album, &(argument->iter_album), path);
				gtk_tree_model_get(model_album, &(argument->iter_album), 0, &songalbum, -1);

				add_album_to_playlist(songalbum, NULL, argument);

				gtk_tree_model_get_iter_first(model_playlist, &argument->iter_playlist);
			}
			path_list = path_list->next;
		}
		gtk_label_set_markup(GTK_LABEL(label), "<span foreground=\"red\">Playlist</span>");
	}
	g_signal_handler_unblock(model_playlist, argument->playlist_update_signal_id);
}

void add_artist_selection_to_playlist(GtkWidget *menuitem, struct arguments *argument) {
	GtkTreeSortable *sortable = GTK_TREE_SORTABLE(argument->store_library);
	gtk_tree_sortable_set_sort_column_id(sortable, TRACKNUMBER, GTK_SORT_ASCENDING);
	GtkTreeModel *model;
	GtkTreeIter iter;
	GtkTreePath *path;
	GtkTreeSelection *selection;
	GList *path_list;
	gchar *path_string;
	GtkWidget *page = gtk_notebook_get_nth_page(GTK_NOTEBOOK(argument->libnotebook), 3);
	GtkWidget *label = gtk_notebook_get_tab_label(GTK_NOTEBOOK(argument->libnotebook), page);

	GtkTreeModel *model_playlist = gtk_tree_view_get_model(GTK_TREE_VIEW(argument->treeview_playlist));

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(argument->treeview_artist));

	if((path_list = gtk_tree_selection_get_selected_rows(selection, &model))) {
		g_signal_handler_block(model_playlist, argument->playlist_update_signal_id);
		while(path_list) {
			path = path_list->data;
			GtkTreeModel *model_artist = gtk_tree_view_get_model(GTK_TREE_VIEW(argument->treeview_artist));
			GtkTreeModel *model_library = gtk_tree_view_get_model(GTK_TREE_VIEW(argument->treeview_library));

			gchar *songtitle;
			gchar *songalbum;
			gchar *songartist;
			gchar *temptitle;
			gchar *tempalbum;
			gchar *tempartist;
			GtkTreeIter lib_iter;
			gboolean valid;

			if(gtk_tree_path_get_depth(path) == 3) {
				if(!(gtk_tree_model_get_iter(model_artist, &(argument->iter_artist), path))) {
					printf("Error getting artist iter!\n");
					return;
				}
				gtk_tree_model_get(model_artist, &(argument->iter_artist), 0, &songtitle, -1);
				gtk_tree_path_up(path);
				gtk_tree_model_get_iter(model_artist, &(argument->iter_artist), path);
				gtk_tree_model_get(model_artist, &(argument->iter_artist), 0, &songalbum, -1);
				gtk_tree_path_up(path);
				gtk_tree_model_get_iter(model_artist, &(argument->iter_artist), path);
				gtk_tree_model_get(model_artist, &(argument->iter_artist), 0, &songartist, -1);
				songtitle = strstr(songtitle, "  ") + 2;
				gboolean valid;
	
				valid = gtk_tree_model_get_iter_first(model_library, &iter);
				while(valid) {
					gtk_tree_model_get(model_library, &iter, ALBUM, &tempalbum, ARTIST, &tempartist, TRACK, &temptitle, -1);
	
					if((!strcmp(tempalbum, songalbum)) 
					&& (!strcmp(tempartist, songartist))
					&& (!strcmp(temptitle, songtitle))) {
						playlist_queue(&iter, model_library, GTK_TREE_VIEW(argument->treeview_playlist), argument);
						break;
					}
					valid = gtk_tree_model_iter_next(model_library, &iter);
				}
			}
			else if(gtk_tree_path_get_depth(path) == 2) {
				if(!(gtk_tree_model_get_iter(model_artist, &(argument->iter_artist), path))) {
					printf("Error getting artist iter!\n");
					return;
				}
				gtk_tree_model_get_iter(model_artist, &(argument->iter_artist), path);
				gtk_tree_model_get(model_artist, &(argument->iter_artist), 0, &songalbum, -1);
				gtk_tree_path_up(path);
				gtk_tree_model_get_iter(model_artist, &(argument->iter_artist), path);
				gtk_tree_model_get(model_artist, &(argument->iter_artist), 0, &songartist, -1);
		
				add_album_to_playlist(songalbum, songartist, argument);

				gtk_tree_model_get_iter_first(model_playlist, &argument->iter_playlist);
			}
			else if(gtk_tree_path_get_depth(path) == 1) {
				if(!(gtk_tree_model_get_iter(model_artist, &(argument->iter_artist), path))) {
					printf("Error getting artist iter!\n");
					return;
				}
				gtk_tree_model_get(model_artist, &(argument->iter_artist), 0, &songartist, -1);

				add_artist_to_playlist(songartist, argument);
			}
		
			path_list = path_list->next;
		}
		gtk_label_set_markup(GTK_LABEL(label), "<span foreground=\"red\">Playlist</span>");
	}
	g_signal_handler_unblock(model_playlist, argument->playlist_update_signal_id);
}

void add_library_selection_to_playlist(GtkWidget *menuitem, struct arguments *argument) {
	GtkTreeModel *model;
	GtkTreeIter iter;
	GtkTreePath *path;
	GtkTreeSelection *selection;
	GList *path_list;
	gchar *path_string;
	GtkTreeModel *model_playlist;
	GtkTreeModel *model_library;
	GtkWidget *page = gtk_notebook_get_nth_page(GTK_NOTEBOOK(argument->libnotebook), 2);
	GtkWidget *label = gtk_notebook_get_tab_label(GTK_NOTEBOOK(argument->libnotebook), page);

	model_playlist = gtk_tree_view_get_model(GTK_TREE_VIEW(argument->treeview_playlist));
	model_library = gtk_tree_view_get_model(GTK_TREE_VIEW(argument->treeview_library));
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(argument->treeview_library));
	gtk_tree_view_get_model((GtkTreeView*)(argument->treeview_library));

	if((path_list = gtk_tree_selection_get_selected_rows(selection, &model))) {
		while(path_list) {
			path_string = gtk_tree_path_to_string((GtkTreePath*)path_list->data);
			gtk_tree_model_get_iter_from_string(model_library, &iter, path_string);
			g_signal_handler_block(model_playlist, argument->playlist_update_signal_id);
			playlist_queue(&iter, model, GTK_TREE_VIEW(argument->treeview_playlist), argument);
			g_signal_handler_unblock(model_playlist, argument->playlist_update_signal_id);
			g_free(path_string);
			path_list = path_list->next;
		}
		gtk_label_set_markup(GTK_LABEL(label), "<span foreground=\"red\">Playlist</span>");
	}
}

void remove_playlist_selection_from_playlist(GtkWidget *menuitem, struct arguments *argument) {
	GtkTreeModel *model;
	GtkTreeIter iter;
	GtkTreePath *path;
	GtkTreeSelection *selection;
	GList *path_list;
	gchar *path_string;
	gchar *tempstring;
	gchar *current_path_string;
	gboolean deleted = FALSE;
	GtkTreeModel *model_playlist;
	int i = 0;

	model_playlist = gtk_tree_view_get_model(GTK_TREE_VIEW(argument->treeview_playlist));
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(argument->treeview_playlist));
	current_path_string = gtk_tree_model_get_string_from_iter(model_playlist, &argument->iter_playlist);

	if((path_list = gtk_tree_selection_get_selected_rows(selection, &model))) {
		while(path_list) {
			tempstring = gtk_tree_path_to_string((GtkTreePath*)path_list->data);
			path_string = g_strdup_printf("%d", strtol(tempstring, NULL, 10) - i);
			gtk_tree_model_get_iter_from_string(model_playlist, &iter, path_string);
			if(!g_strcmp0(current_path_string, path_string))
				deleted = TRUE;
			g_signal_handler_block(model_playlist, argument->playlist_update_signal_id);
			gtk_list_store_remove(argument->store_playlist, &iter);
			g_signal_handler_unblock(model_playlist, argument->playlist_update_signal_id);
			g_free(path_string);
			g_free(tempstring);
			++i;
			path_list = path_list->next;	
		}
		if(deleted) {
			argument->iter_playlist = iter;
			start_song(argument);
		}
	}
}

void playlist_del_button(GtkWidget *treeview, GdkEventKey *event, struct arguments *argument) {
	if(event->type == GDK_KEY_PRESS && event->keyval == 0xffff)
		remove_playlist_selection_from_playlist(NULL, argument);
}

void playlist_popup_menu(GtkWidget *treeview, GdkEventButton *event, struct arguments *argument) {
	GtkWidget *menu, *item_remove_from_playlist;

    menu = gtk_menu_new();

    item_remove_from_playlist = gtk_menu_item_new_with_label("Remove this track(s) from playlist");

    g_signal_connect(item_remove_from_playlist, "activate", (GCallback)remove_playlist_selection_from_playlist, argument);

    gtk_menu_shell_append(GTK_MENU_SHELL(menu), item_remove_from_playlist);

    gtk_widget_show_all(menu);
    gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, NULL,
                   (event != NULL) ? event->button : 0,
                   gdk_event_get_time((GdkEvent*)event));
}

void library_popup_menu(GtkWidget *treeview, GdkEventButton *event, struct arguments *argument) {
    GtkWidget *menu, *item_add_to_playlist;

    menu = gtk_menu_new();

    item_add_to_playlist = gtk_menu_item_new_with_label("Add track(s) to playlist");

    g_signal_connect(item_add_to_playlist, "activate", (GCallback)add_library_selection_to_playlist,argument);

    gtk_menu_shell_append(GTK_MENU_SHELL(menu), item_add_to_playlist);

    gtk_widget_show_all(menu);
    gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, NULL,
                   (event != NULL) ? event->button : 0,
                   gdk_event_get_time((GdkEvent*)event));
}

void changed_page_notebook(GtkNotebook *notebook, GtkWidget *page, guint page_num, gpointer data) {
	if(page_num == 3) { // "Playlist" tab selected 
		GtkWidget *label = gtk_notebook_get_tab_label(GTK_NOTEBOOK(notebook), page);
		gtk_label_set_markup(GTK_LABEL(label), "Playlist");
	}
}

void album_popup_menu(GtkWidget *treeview, GdkEventButton *event, struct arguments *argument) {
	GtkWidget *menu, *item_add_to_playlist;

    menu = gtk_menu_new();

    item_add_to_playlist = gtk_menu_item_new_with_label("Add track(s) to playlist");

    g_signal_connect(item_add_to_playlist, "activate", (GCallback)add_album_selection_to_playlist, argument);

    gtk_menu_shell_append(GTK_MENU_SHELL(menu), item_add_to_playlist);

    gtk_widget_show_all(menu);
    gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, NULL,
                   (event != NULL) ? event->button : 0,
                   gdk_event_get_time((GdkEvent*)event));
}

void artist_popup_menu(GtkWidget *treeview, GdkEventButton *event, struct arguments *argument) {
	GtkWidget *menu, *item_add_to_playlist;

    menu = gtk_menu_new();

    item_add_to_playlist = gtk_menu_item_new_with_label("Add track(s) to playlist");

    g_signal_connect(item_add_to_playlist, "activate", (GCallback)add_artist_selection_to_playlist, argument);

    gtk_menu_shell_append(GTK_MENU_SHELL(menu), item_add_to_playlist);

    gtk_widget_show_all(menu);
    gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, NULL,
                   (event != NULL) ? event->button : 0,
                   gdk_event_get_time((GdkEvent*)event));
}

gboolean treeviews_right_click(GtkWidget *treeview, GdkEventButton *event, struct arguments *argument) {
	gboolean blank;
	if(event->type == GDK_BUTTON_PRESS && event->button == 3) {
		GtkTreeSelection *selection;

		selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));
	
		GtkTreePath *path;
	
		if(!(blank = !gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(treeview), (gint)event->x,
		(gint)event->y, &path, NULL, NULL, NULL))) {
			if(gtk_tree_selection_count_selected_rows(selection) <= 1) {
				gtk_tree_selection_unselect_all(selection);
				gtk_tree_selection_select_path(selection, path);
				gtk_tree_path_free(path);
			}
		}
		if(blank == FALSE) {
			if(treeview == argument->treeview_playlist)
				playlist_popup_menu(treeview, event, argument);
			else if(treeview == argument->treeview_artist)
				artist_popup_menu(treeview, event, argument);
			else if(treeview == argument->treeview_library)
				library_popup_menu(treeview, event, argument);
			else if(treeview == argument->treeview_album)
				album_popup_menu(treeview, event, argument);
			return TRUE;
		}
	}
	else
		return FALSE;
}

void time_spin_changed(GtkSpinButton *spin_button, struct arguments *argument) {
	time_spin_input(spin_button, &argument->timer_delay, argument);
}

gint time_spin_input(GtkSpinButton *spin_button, gdouble *new_val, struct arguments *argument) {
	const gchar *text;
	gchar **str;
	gboolean found = FALSE;
  	gint hours;
	gint minutes;
	gint seconds;
	gchar *endh;
	gchar *endm;
	gchar *ends;

	text = gtk_entry_get_text(GTK_ENTRY(spin_button));
	str = g_strsplit (text, ":", 3);

	if(g_strv_length (str) == 3) {
		hours = strtol(str[0], &endh, 10);
		minutes = strtol(str[1], &endm, 10);
		seconds = strtol(str[2], &ends, 10);
		if(!*endh && !*endm && !*ends &&
		0 <= hours && hours < 24 &&
		 0 <= minutes && minutes < 60 &&
		  0 <= seconds && seconds < 60) {
			*new_val = hours * 3600 + minutes*60 + seconds;
			found = TRUE;
		}
	}

	g_strfreev(str);

	if(!found) {
		*new_val = 0.0;
		return GTK_INPUT_ERROR;
	}
  	return TRUE;
}

gint time_spin_output(GtkSpinButton *spin_button, struct arguments *argument) {
	GtkAdjustment *adjustment;
	gchar *buf;
	gdouble hours;
	gdouble minutes;
	gdouble seconds;

	adjustment = gtk_spin_button_get_adjustment(spin_button);

	hours = gtk_adjustment_get_value(adjustment) / 3600.0;
	minutes = (hours - floor(hours)) * 60.0;
	seconds = (minutes - floor(minutes)) * 60.0;
	buf = g_strdup_printf ("%02.0f:%02.0f:%02.0f", floor(hours), floor(minutes), floor(seconds + 0.5));
	if(strcmp (buf, gtk_entry_get_text (GTK_ENTRY (spin_button))))
    	gtk_entry_set_text (GTK_ENTRY (spin_button), buf);

  	g_free (buf);

	return TRUE;
}

void time_checkbox_toggled(GtkToggleButton *togglebutton, struct arguments *argument) {
	gboolean mode = gtk_toggle_button_get_active(togglebutton);

	if(mode == TRUE && (argument->timer_delay != 0.)) {
		argument->sleep_timer = g_timer_new();
	}
	else if(argument->sleep_timer) {	
		g_timer_destroy(argument->sleep_timer);
		argument->sleep_timer = NULL;
	}
}

void setup_tree_view_renderer_play_lib(GtkWidget *treeview) {
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
}

void display_library(GtkTreeView *treeview, GtkListStore *store, gchar *libfile) {
	GtkTreeIter iter;
	FILE *library;
	char tempfile[PATH_MAX];
	char temptrack[PATH_MAX];
	char tempalbum[PATH_MAX];
	char tempartist[PATH_MAX];
	char temptracknumber_arr[PATH_MAX];
	gchar *temptracknumber;
	int nbr_temptracknumber;
	char tempforce[PATH_MAX];
	float tempforcef;
	char tempforce_env[PATH_MAX];
	float tempforce_envf;
	char tempforce_amp[PATH_MAX];
	float tempforce_ampf;
	char tempforce_freq[PATH_MAX];
	float tempforce_freqf;
	char tempforce_atk[PATH_MAX];
	float tempforce_atkf;

	gtk_list_store_clear(store);

	if((library = fopen(libfile, "r")) != NULL) {
		while(fgets(tempfile, PATH_MAX, library) != NULL) {
			tempfile[strcspn(tempfile, "\n")] = '\0';

			if(!fgets(temptracknumber_arr, PATH_MAX, library)
			|| !fgets(temptrack, PATH_MAX, library)
			|| !fgets(tempalbum, PATH_MAX, library)
			|| !fgets(tempartist, PATH_MAX, library)
			|| !fgets(tempforce, PATH_MAX, library)
			|| !fgets(tempforce_env, PATH_MAX, library)
			|| !fgets(tempforce_amp, PATH_MAX, library)
			|| !fgets(tempforce_freq, PATH_MAX, library)
			|| !fgets(tempforce_atk, PATH_MAX, library)) {
				printf("Wrong config file format\n");
				return;
			}
			
			int i;
			for(i = 0; g_ascii_isdigit(temptracknumber_arr[i]) == FALSE; ++i) {
				temptracknumber_arr[i] = '0';
			}
			temptracknumber = g_strdup_printf("%02d", strtol(temptracknumber_arr, NULL, 10));
			temptracknumber[strcspn(temptracknumber, "\n")] = '\0';

			temptrack[strcspn(temptrack, "\n")] = '\0';

			tempalbum[strcspn(tempalbum, "\n")] = '\0';

			tempartist[strcspn(tempartist, "\n")] = '\0';

			tempforce[strcspn(tempforce, "\n")] = '\0';
			tempforcef = atof(tempforce);

			tempforce_env[strcspn(tempforce_env, "\n")] = '\0';
			tempforce_envf = atof(tempforce_env);

			tempforce_amp[strcspn(tempforce_amp, "\n")] = '\0';
			tempforce_ampf = atof(tempforce_amp);

			tempforce_freq[strcspn(tempforce_freq, "\n")] = '\0';
			tempforce_freqf = atof(tempforce_freq);

			tempforce_atk[strcspn(tempforce_atk, "\n")] = '\0';
			tempforce_atkf = atof(tempforce_atk);

			if(atof(tempforce) == 0)
				strcpy(tempforce, "Loud");
			else if(atof(tempforce) == 1) {
				strcpy(tempforce, "Calm");
			}
			else
				strcpy(tempforce, "Can't conclude");
			
			gtk_list_store_append(store, &iter);
			gtk_list_store_set(store, &iter, PLAYING, "", TRACKNUMBER, temptracknumber, TRACK, temptrack, ALBUM, tempalbum, ARTIST, tempartist, FORCE, tempforcef, FORCE_TEMPO, tempforce_envf, FORCE_AMP, tempforce_ampf, FORCE_FREQ, tempforce_freqf, FORCE_ATK, tempforce_atkf, TEXTFORCE, tempforce, AFILE, tempfile, -1);
			g_free(temptracknumber);
		}
		fclose(library);
	}
}

int main(int argc, char **argv) {
	struct arguments argument;
	struct arguments *pargument = &argument;
	struct pref_arguments pref_arguments;

	GtkWidget *window, *treeview_library, *treeview_playlist, *treeview_artist, *treeview_album, *library_panel, *artist_panel, *album_panel, *playlist_panel, *vboxv,
		*playbox, *volumebox, *randombox, *repeat_button, *random_button, *lelele_button, *labelbox, *next_button, *previous_button, 
		*menubar, *file, *filemenu, *open, *add_file, *close, *edit, *editmenu, *preferences,
		*mediainfo_expander, *mediainfo_box, *mediainfo_labelbox, *area, *time_spin, *time_box, *time_checkbox, *analyze_spinner;
	GtkAdjustment *time_adjust;
	GSettingsSchema *schema;
	GSettingsSchemaSource *schema_source;
	gboolean library_set;
	
	GtkTreeModel *model_playlist;
	GtkTreeModel *model_library;
	GtkTreeSortable *sortable;
	const gchar *volume[5] = {
		"audio-volume-muted-symbolic",
		"audio-volume-high-symbolic",
		"audio-volume-low-symbolic",
		"audio-volume-medium-symbolic",
		NULL
	};

	GstElement *gtk_sink;
	GstBus *bus;
	
	#ifdef linux
		XInitThreads();
	#endif

	pargument->lelelerandom = 0;
	pargument->timer_delay = 0;
	pargument->random = 0;
	pargument->repeat = 0;
	pargument->first = 1;
	pargument->playlist_count = 0;
	pargument->iter_library.stamp = 0;
	pargument->iter_playlist.stamp = 0;
	pargument->current_song.duration = GST_CLOCK_TIME_NONE;
	pargument->current_song.state = GST_STATE_NULL;
	pargument->current_song.artist = pargument->current_song.title = pargument->current_song.album = pargument->current_song.tracknumber = NULL;
	pargument->str_genre = pargument->str_bitrate = pargument->str_samplerate = pargument->str_channels = NULL;
	pargument->history = NULL;
	pargument->current_song.sample_array = NULL;
	pargument->bartag = 0;
	pargument->sleep_timer = NULL;
	g_mutex_init(&pargument->queue_mutex);

	gtk_init(&argc, &argv);	
	gst_init(&argc, &argv);
	
	char lib_file[] = "library.txt";
	gchar *libdir;
	gchar *libfile;

	libdir = g_build_filename(g_get_home_dir(), ".local", "share", "leleleplayer", NULL);
	libfile = g_build_filename(libdir, "library.txt", NULL);

	if(g_mkdir_with_parents(libdir, 0755) != -1)
		pargument->lib_path = libfile;
	else
		pargument->lib_path = lib_file;	

	g_free(libdir);

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(window), "lelele player");
	gtk_widget_set_size_request(window, 900, 700);

	pargument->current_song.playbin = gst_element_factory_make("playbin", "playbin");
	if(!pargument->current_song.playbin)
		g_error("Not all elements could be created.\n");
	bus = gst_element_get_bus(pargument->current_song.playbin);
	gst_bus_add_signal_watch(bus);

/*	if((gtk_sink = gst_element_factory_make("gtkglsink", NULL))) {
		GstElement *video_sink;

		g_object_get(gtk_sink, "widget", &area, NULL);
	
		video_sink = gst_element_factory_make("glsinkbin", NULL);
		g_object_set(video_sink, "sink", gtk_sink, NULL);
	
		g_object_set(pargument->current_song.playbin, "video-sink", video_sink, NULL);
	} 
	if((gtk_sink = gst_element_factory_make("gtksink", NULL))) {
		g_object_get(gtk_sink, "widget", &area, NULL);

		g_object_set(pargument->current_song.playbin, "video-sink", gtk_sink, NULL);	
	} 

	GstElement *sink = gst_element_factory_make("ximagesink", "sink");
	g_object_set(gtk_sink, "video-sink", sink, NULL);


	GstElement *vis_plugin;
	GstElementFactory *selected_factory = NULL;
	GList *list, *walk;
	guint flags;
	list = gst_registry_feature_filter(gst_registry_get(), filter_vis_features, FALSE, NULL);

	for(walk = list; walk != NULL; walk = g_list_next(walk)) {
		const gchar *name;
		GstElementFactory *factory;

		factory = GST_ELEMENT_FACTORY(walk->data);
		name = gst_element_factory_get_longname(factory);

		if(selected_factory == NULL || g_str_has_prefix(name, "Waveform")) {
			selected_factory = factory;
		}
	}

	if(!selected_factory)
		g_printf("No visualization plugins found!\n");

	vis_plugin = gst_element_factory_create(selected_factory, NULL);

	if(!vis_plugin)
		g_printf("Couldn't create the visualizator factory element!\n");

	g_object_get(pargument->current_song.playbin, "flags", &flags, NULL);
	flags |= (1 << 3);
	g_object_set(pargument->current_song.playbin, "flags", flags, NULL);
	g_object_set(pargument->current_song.playbin, "vis-plugin", vis_plugin, NULL);
	g_object_set(pargument->current_song.playbin, "force-aspect-ratio", FALSE, NULL);*/


	library_panel = gtk_scrolled_window_new(NULL, NULL);
	playlist_panel = gtk_scrolled_window_new(NULL, NULL);
	artist_panel = gtk_scrolled_window_new(NULL, NULL);
	album_panel = gtk_scrolled_window_new(NULL, NULL);

	pargument->store_library = gtk_list_store_new(COLUMNS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_FLOAT, G_TYPE_FLOAT, G_TYPE_FLOAT, G_TYPE_FLOAT, G_TYPE_FLOAT, G_TYPE_STRING, G_TYPE_STRING);
	sortable = GTK_TREE_SORTABLE(pargument->store_library);
	gtk_tree_sortable_set_sort_column_id(sortable, TRACKNUMBER, GTK_SORT_ASCENDING);
	treeview_library = gtk_tree_view_new_with_model(GTK_TREE_MODEL(pargument->store_library));
	gtk_tree_sortable_set_sort_func(sortable, TRACKNUMBER, sort_iter_compare_func, NULL, NULL); 
	gtk_tree_sortable_set_sort_func(sortable, TEXTFORCE, sort_force, NULL, NULL);
	gtk_tree_sortable_set_sort_func(sortable, ARTIST, sort_artist_album_tracks, NULL, NULL);
	setup_tree_view_renderer_play_lib(treeview_library);	
	pargument->treeview_library = treeview_library;
	display_library(GTK_TREE_VIEW(treeview_library), pargument->store_library, pargument->lib_path);
	model_library = gtk_tree_view_get_model(GTK_TREE_VIEW(treeview_library));
	GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview_library));
	gtk_tree_selection_set_mode(selection, GTK_SELECTION_MULTIPLE);

	pargument->store_playlist = gtk_list_store_new(COLUMNS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_FLOAT, G_TYPE_FLOAT, G_TYPE_FLOAT, G_TYPE_FLOAT, G_TYPE_FLOAT, G_TYPE_STRING, G_TYPE_STRING);
	treeview_playlist = gtk_tree_view_new_with_model(GTK_TREE_MODEL(pargument->store_playlist));
	setup_tree_view_renderer_play_lib(treeview_playlist);
	gtk_tree_view_set_reorderable(GTK_TREE_VIEW(treeview_playlist), TRUE);
	gtk_tree_view_set_headers_clickable(GTK_TREE_VIEW(treeview_playlist), FALSE);
	pargument->treeview_playlist = treeview_playlist;
	model_playlist = gtk_tree_view_get_model(GTK_TREE_VIEW(treeview_playlist));
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview_playlist));
	gtk_tree_selection_set_mode(selection, GTK_SELECTION_MULTIPLE);

	pargument->store_artist = gtk_tree_store_new(NUM_COLS_ARTIST, G_TYPE_STRING);
	treeview_artist = gtk_tree_view_new_with_model(GTK_TREE_MODEL(pargument->store_artist));
	gtk_tree_sortable_set_sort_column_id(sortable, ARTIST, GTK_SORT_ASCENDING);
	setup_tree_view_renderer_artist(treeview_artist, pargument->store_artist, model_library);
	pargument->treeview_artist = treeview_artist;
	display_artist_tab(treeview_artist, pargument->store_artist, model_library);
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview_artist));
	gtk_tree_selection_set_mode(selection, GTK_SELECTION_MULTIPLE);

	pargument->store_album = gtk_tree_store_new(NUM_COLS_ALBUM, G_TYPE_STRING);
	treeview_album = gtk_tree_view_new_with_model(GTK_TREE_MODEL(pargument->store_album));
	gtk_tree_sortable_set_sort_column_id(sortable, ALBUM, GTK_SORT_ASCENDING);
	setup_tree_view_renderer_album(treeview_album, pargument->store_album, model_library);
	pargument->treeview_album = treeview_album;
	display_album_tab(treeview_album, pargument->store_album, model_library);
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview_album));
	gtk_tree_selection_set_mode(selection, GTK_SELECTION_MULTIPLE);


	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(library_panel), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(playlist_panel), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(artist_panel), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(album_panel), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	playbox = gtk_button_box_new(GTK_ORIENTATION_HORIZONTAL);
	randombox = gtk_button_box_new(GTK_ORIENTATION_HORIZONTAL);
	volumebox = gtk_button_box_new(GTK_ORIENTATION_HORIZONTAL);
	pargument->playpause_button= gtk_button_new();
	random_button = gtk_toggle_button_new();
	gtk_button_set_image(GTK_BUTTON(random_button), gtk_image_new_from_icon_name("media-playlist-shuffle-symbolic", GTK_ICON_SIZE_BUTTON));
	gtk_widget_set_tooltip_text(random_button, "Standard random button");
	repeat_button = gtk_toggle_button_new();
	gtk_button_set_image(GTK_BUTTON(repeat_button), gtk_image_new_from_icon_name("media-playlist-repeat-symbolic", GTK_ICON_SIZE_BUTTON));
	lelele_button = gtk_toggle_button_new();
	if(g_file_test("../images/lelelerandom.svg", G_FILE_TEST_EXISTS))
		gtk_button_set_image(GTK_BUTTON(lelele_button), gtk_image_new_from_file("../images/lelelerandom.svg"));
	else
		gtk_button_set_image(GTK_BUTTON(lelele_button), gtk_image_new_from_file("/usr/share/leleleplayer/icons/lelelerandom.svg"));

	gtk_widget_set_tooltip_text(lelele_button, "Random smoothly over songs");
	next_button = gtk_button_new();
	gtk_button_set_image(GTK_BUTTON(next_button), gtk_image_new_from_icon_name("media-skip-forward-symbolic", GTK_ICON_SIZE_BUTTON));
	previous_button = gtk_button_new();
	gtk_button_set_image(GTK_BUTTON(previous_button), gtk_image_new_from_icon_name("media-skip-backward-symbolic", GTK_ICON_SIZE_BUTTON));
	pargument->playpause_button = gtk_button_new();
	gtk_button_set_image(GTK_BUTTON(pargument->playpause_button), gtk_image_new_from_icon_name("media-playback-start-symbolic", GTK_ICON_SIZE_BUTTON));
	if(g_file_test("../images/lelele.svg", G_FILE_TEST_EXISTS))
		gtk_window_set_icon_from_file(GTK_WINDOW(window), "../images/lelele.svg", NULL);
	else
		gtk_window_set_icon_from_file(GTK_WINDOW(window), "/usr/share/leleleplayer/icons/lelele.svg", NULL);
	pargument->adjust = (GtkAdjustment*)gtk_adjustment_new(0, 0, 100, 1, 1, 1);
	pargument->progressbar = gtk_scale_new(GTK_ORIENTATION_HORIZONTAL, pargument->adjust);
	pargument->volume_scale = gtk_scale_button_new(GTK_ICON_SIZE_BUTTON, 0, 1, 0.1, volume);
	gtk_scale_set_draw_value((GtkScale*)pargument->progressbar, FALSE);
	gtk_widget_set_sensitive(pargument->progressbar, FALSE);
	vboxv = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
	time_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
	labelbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
	pargument->title_label = gtk_label_new("");
	pargument->album_label = gtk_label_new("");
	gtk_label_set_markup(GTK_LABEL(pargument->album_label), "<span foreground=\"grey\">No song currently playing</span>");
	pargument->artist_label = gtk_label_new("");
	pargument->genre_label = gtk_label_new("Genre:");
	pargument->samplerate_label = gtk_label_new("Sample rate:");
	pargument->bitrate_label = gtk_label_new("Bitrate:");
	pargument->channels_label = gtk_label_new("Channels:");
	pargument->libnotebook = gtk_notebook_new();
	mediainfo_expander = gtk_expander_new("Visualizer/Mediainfo");
	mediainfo_labelbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
	mediainfo_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
	//gtk_widget_set_size_request(area, -1, 50);
	menubar = gtk_menu_bar_new();
	if(schema_source = g_settings_schema_source_new_from_directory("..", NULL, FALSE, NULL))
		;
	else
		schema_source = g_settings_schema_source_new_from_directory("/usr/share/glib-2.0/schemas/", NULL, FALSE, NULL);
	schema = g_settings_schema_source_lookup(schema_source, "org.gnome.leleleplayer.preferences", FALSE);
	pref_arguments.preferences = g_settings_new_full(schema, NULL, NULL);
	library_set = (gboolean)g_settings_get_boolean(pref_arguments.preferences, "library-set");
	pref_arguments.lib_path = pargument->lib_path;
	time_adjust = gtk_adjustment_new(0, 0, 86399, 30, 60, 0);
	pargument->time_spin = gtk_spin_button_new(time_adjust, 1, 1);
	gtk_widget_set_tooltip_text(pargument->time_spin, "Ends the playing after a given time");
	gtk_spin_button_set_wrap(GTK_SPIN_BUTTON(pargument->time_spin), TRUE);
	time_checkbox = gtk_check_button_new_with_label("Enable sleep timer until ");
	gtk_widget_set_tooltip_text(time_checkbox, "Ends the playing after a given time");
	pargument->time_checkbox = time_checkbox;
	analyze_spinner = gtk_spinner_new();		

	file = gtk_menu_item_new_with_label("File");
	edit = gtk_menu_item_new_with_label("Edit");
	filemenu = gtk_menu_new();
	editmenu = gtk_menu_new();

	gtk_menu_item_set_submenu(GTK_MENU_ITEM(file), filemenu);
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(edit), editmenu);
	gtk_menu_shell_append(GTK_MENU_SHELL(menubar), file);
	gtk_menu_shell_append(GTK_MENU_SHELL(menubar), edit);

	open = gtk_menu_item_new_with_label("Open...");
	add_file = gtk_menu_item_new_with_label("Add file to playlist...");
	close = gtk_menu_item_new_with_label("Close");
	preferences = gtk_menu_item_new_with_label("Preferences");
	gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), open);
	gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), add_file);
	gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), close);
	gtk_menu_shell_append(GTK_MENU_SHELL(editmenu), preferences);

	pref_arguments.window = window;
	pref_arguments.treeview = treeview_library;
	pref_arguments.treeview_artist = treeview_artist;
	pref_arguments.treeview_album = treeview_album;
	pref_arguments.store_library = pargument->store_library;
	pref_arguments.store_artist = pargument->store_artist;
	pref_arguments.store_album = pargument->store_album;
	pref_arguments.spinner = analyze_spinner;

	/* Signal management */
	g_signal_connect(G_OBJECT(bus), "message::state-changed", G_CALLBACK(state_changed), pargument);
	g_signal_connect(G_OBJECT(pargument->current_song.playbin), "about-to-finish", G_CALLBACK(continue_track), pargument);
	g_signal_connect(G_OBJECT(bus), "message::stream-start", G_CALLBACK(refresh_ui), pargument);
	g_signal_connect(G_OBJECT(bus), "message::application", G_CALLBACK(message_application), pargument);
	g_signal_connect(G_OBJECT(pargument->current_song.playbin), "audio-tags-changed", G_CALLBACK(tags_obtained), pargument);
	g_signal_connect(G_OBJECT(pargument->playpause_button), "clicked", G_CALLBACK(toggle_playpause_button), pargument);
	g_signal_connect(G_OBJECT(pargument->volume_scale), "value-changed", G_CALLBACK(volume_scale_changed), pargument);
	g_signal_connect(G_OBJECT(random_button), "clicked", G_CALLBACK(toggle_random), pargument);
	g_signal_connect(G_OBJECT(repeat_button), "clicked", G_CALLBACK(toggle_repeat), pargument);
	g_signal_connect(G_OBJECT(lelele_button), "clicked", G_CALLBACK(toggle_lelele), pargument);
	g_signal_connect(G_OBJECT(next_button), "clicked", G_CALLBACK(next_buttonf), pargument);
	g_signal_connect(G_OBJECT(previous_button), "clicked", G_CALLBACK(previous_buttonf), pargument);
	g_signal_connect(G_OBJECT(preferences), "activate", G_CALLBACK(preferences_callback), &pref_arguments);
	g_signal_connect(G_OBJECT(open), "activate", G_CALLBACK(open_audio_file), pargument);
	g_signal_connect(G_OBJECT(add_file), "activate", G_CALLBACK(add_file_to_playlist), pargument);
	g_signal_connect(G_OBJECT(close), "activate", G_CALLBACK(destroy), pargument);
	g_signal_connect(G_OBJECT(treeview_library), "row-activated", G_CALLBACK(lib_row_activated), pargument);
	g_signal_connect(G_OBJECT(treeview_library), "button-press-event", G_CALLBACK(treeviews_right_click), pargument);
	g_signal_connect(G_OBJECT(treeview_artist), "button-press-event", G_CALLBACK(treeviews_right_click), pargument);
	g_signal_connect(G_OBJECT(treeview_album), "button-press-event", G_CALLBACK(treeviews_right_click), pargument);
	g_signal_connect(G_OBJECT(treeview_playlist), "button-press-event", G_CALLBACK(treeviews_right_click), pargument);
	g_signal_connect(G_OBJECT(treeview_playlist), "key-press-event", G_CALLBACK(playlist_del_button), pargument);
	g_signal_connect(G_OBJECT(treeview_playlist), "row-activated", G_CALLBACK(playlist_row_activated), pargument);
	g_signal_connect(G_OBJECT(treeview_artist), "row-activated", G_CALLBACK(artist_row_activated), pargument);
	g_signal_connect(G_OBJECT(treeview_album), "row-activated", G_CALLBACK(album_row_activated), pargument);
	g_signal_connect(G_OBJECT(time_checkbox), "toggled", G_CALLBACK(time_checkbox_toggled), pargument);
	g_signal_connect(G_OBJECT(pargument->time_spin), "input", G_CALLBACK(time_spin_input), pargument);
	g_signal_connect(G_OBJECT(pargument->time_spin), "output", G_CALLBACK(time_spin_output), pargument);
	pargument->time_spin_update_signal_id = g_signal_connect(G_OBJECT(pargument->time_spin), "value-changed", G_CALLBACK(time_spin_changed), pargument);
	pargument->playlist_update_signal_id = g_signal_connect(G_OBJECT(model_playlist), "row-inserted", G_CALLBACK(ui_playlist_changed), pargument->libnotebook);
	g_signal_connect(G_OBJECT(pargument->libnotebook), "switch-page", G_CALLBACK(changed_page_notebook), NULL);
	pargument->progressbar_update_signal_id = g_signal_connect(G_OBJECT(pargument->progressbar), 
		"value-changed", G_CALLBACK(slider_changed), pargument);
	g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(destroy), pargument);	

	gtk_container_add(GTK_CONTAINER(library_panel), treeview_library);
	gtk_container_add(GTK_CONTAINER(playlist_panel), treeview_playlist);
	gtk_container_add(GTK_CONTAINER(artist_panel), treeview_artist);
	gtk_container_add(GTK_CONTAINER(album_panel), treeview_album);

	gtk_box_set_homogeneous(GTK_BOX(vboxv), FALSE);
	gtk_box_set_homogeneous(GTK_BOX(time_box), FALSE);
	
	/* Add objects to the box */
	gtk_box_pack_start(GTK_BOX(vboxv), menubar, FALSE, FALSE, 1);
	gtk_box_pack_start(GTK_BOX(vboxv), labelbox, FALSE, FALSE, 1);
		gtk_box_pack_start(GTK_BOX(labelbox), pargument->title_label, FALSE, FALSE, 1);
		gtk_box_pack_start(GTK_BOX(labelbox), pargument->album_label, FALSE, FALSE, 1);
		gtk_box_pack_start(GTK_BOX(labelbox), pargument->artist_label, FALSE, FALSE, 1);
	gtk_box_pack_start(GTK_BOX(vboxv), playbox, FALSE, FALSE, 1);
		gtk_button_box_set_layout(GTK_BUTTON_BOX(playbox), GTK_BUTTONBOX_CENTER);
		gtk_box_pack_start(GTK_BOX(playbox), previous_button, FALSE, FALSE, 1);
		gtk_box_pack_start(GTK_BOX(playbox), pargument->playpause_button, FALSE, FALSE, 1);
		gtk_box_pack_start(GTK_BOX(playbox), next_button, FALSE, FALSE, 1);
		gtk_button_box_set_child_non_homogeneous(GTK_BUTTON_BOX(playbox), pargument->playpause_button, TRUE);
		gtk_button_box_set_child_non_homogeneous(GTK_BUTTON_BOX(playbox), next_button, TRUE);
		gtk_button_box_set_child_non_homogeneous(GTK_BUTTON_BOX(playbox), previous_button, TRUE);
		gtk_box_set_spacing(GTK_BOX(playbox), 5);
	gtk_box_pack_start(GTK_BOX(vboxv), randombox, FALSE, FALSE, 1);
		gtk_button_box_set_layout(GTK_BUTTON_BOX(randombox), GTK_BUTTONBOX_CENTER);
		gtk_box_pack_start(GTK_BOX(randombox), lelele_button, FALSE, FALSE, 1);
		gtk_box_pack_start(GTK_BOX(randombox), repeat_button, FALSE, FALSE, 1);
		gtk_box_pack_start(GTK_BOX(randombox), random_button, FALSE, FALSE, 1);
		gtk_button_box_set_child_non_homogeneous(GTK_BUTTON_BOX(randombox), repeat_button, TRUE);
		gtk_button_box_set_child_non_homogeneous(GTK_BUTTON_BOX(randombox), lelele_button, TRUE);
		gtk_button_box_set_child_non_homogeneous(GTK_BUTTON_BOX(randombox), random_button, TRUE);
		gtk_box_set_spacing(GTK_BOX(randombox), 5);
	gtk_box_pack_start(GTK_BOX(vboxv), volumebox, FALSE, FALSE, 1);
		gtk_button_box_set_layout(GTK_BUTTON_BOX(volumebox), GTK_BUTTONBOX_CENTER);
		gtk_box_pack_start(GTK_BOX(volumebox), pargument->volume_scale, FALSE, FALSE, 1);
		gtk_button_box_set_child_non_homogeneous(GTK_BUTTON_BOX(volumebox), pargument->volume_scale, TRUE);
	gtk_box_pack_start(GTK_BOX(vboxv), mediainfo_expander, FALSE, FALSE, 1);
	gtk_container_add(GTK_CONTAINER(mediainfo_expander), mediainfo_box);
		//gtk_box_pack_start(GTK_BOX(mediainfo_box), area, TRUE, TRUE, 1);
		gtk_box_pack_start(GTK_BOX(mediainfo_box), mediainfo_labelbox, FALSE, FALSE, 1);
			gtk_box_pack_start(GTK_BOX(mediainfo_labelbox), gtk_label_new("More information:"), FALSE, FALSE, 1);
			gtk_box_pack_start(GTK_BOX(mediainfo_labelbox), pargument->genre_label,FALSE, FALSE, 1);
			gtk_box_pack_start(GTK_BOX(mediainfo_labelbox), pargument->bitrate_label, FALSE, FALSE, 1);
			gtk_box_pack_start(GTK_BOX(mediainfo_labelbox), pargument->channels_label, FALSE, FALSE, 1);
			gtk_box_pack_start(GTK_BOX(mediainfo_labelbox), pargument->samplerate_label, FALSE, FALSE, 1);
	gtk_box_pack_start(GTK_BOX(vboxv), pargument->progressbar, FALSE, FALSE, 1);
	gtk_box_pack_start(GTK_BOX(vboxv), pargument->libnotebook, TRUE, TRUE, 1);
		gtk_notebook_append_page(GTK_NOTEBOOK(pargument->libnotebook), library_panel, gtk_label_new("Library"));
		gtk_notebook_append_page(GTK_NOTEBOOK(pargument->libnotebook), artist_panel, gtk_label_new("Artists"));
		gtk_notebook_append_page(GTK_NOTEBOOK(pargument->libnotebook), album_panel, gtk_label_new("Albums"));
		gtk_notebook_append_page(GTK_NOTEBOOK(pargument->libnotebook), playlist_panel, gtk_label_new("Playlist"));
	gtk_box_pack_start(GTK_BOX(vboxv), time_box, FALSE, FALSE, 1);
		gtk_box_pack_end(GTK_BOX(time_box), analyze_spinner, FALSE, FALSE, 5);
		gtk_box_pack_end(GTK_BOX(time_box), pargument->time_spin, TRUE, TRUE, 5);
		gtk_box_pack_end(GTK_BOX(time_box), time_checkbox, FALSE, FALSE, 5);

	gtk_scale_button_set_value(GTK_SCALE_BUTTON(pargument->volume_scale), 0.1);
	gtk_container_add(GTK_CONTAINER(window), vboxv);
	gtk_widget_show_all(window);

	if(library_set == FALSE) {
		preferences_callback(GTK_MENU_ITEM(preferences), &pref_arguments);
	}

	gtk_main();

	gst_object_unref(bus);
	gst_object_unref(pargument->current_song.playbin);

	return 0;
}
