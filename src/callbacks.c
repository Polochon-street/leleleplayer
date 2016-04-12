#include "gui.h"

void destroy_cb(GtkWidget *window, struct arguments *argument) {
	gst_element_set_state(argument->playbin, GST_STATE_NULL);
	gtk_main_quit();
}

void tags_obtained_cb(GstElement *playbin, gint stream, struct arguments *argument) {
	GstStructure *structure;	
	
	structure = gst_structure_new_empty("tags");

	GstMessage *msg = gst_message_new_application(GST_OBJECT(playbin), structure);

	gst_element_post_message(argument->playbin, msg);
}

void artist_row_activated_cb(GtkTreeView *treeview, GtkTreePath *path, GtkTreeViewColumn *column, struct arguments *argument) {
	GtkTreeSortable *sortable = GTK_TREE_SORTABLE(argument->store_library);
	gtk_tree_sortable_set_sort_column_id(sortable, TRACKNUMBER, GTK_SORT_ASCENDING);
	GtkTreeModel *model_artist = gtk_tree_view_get_model(treeview);
	GtkTreeModel *model_library = gtk_tree_view_get_model(GTK_TREE_VIEW(argument->treeview_library));
	GtkTreeModel *model_playlist = gtk_tree_view_get_model(GTK_TREE_VIEW(argument->treeview_playlist));
	GtkTreeIter iter_playlist;
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

		gtk_tree_model_get_iter_first(model_playlist, &iter_playlist);

		
		argument->row_playlist = tree_iter_get_row_reference(model_playlist, &iter_playlist);
		start_song(argument);
	}
	else if(gtk_tree_path_get_depth(path) == 1) {
		if(!(gtk_tree_model_get_iter(model_artist, &(argument->iter_artist), path))) {
			printf("Error getting artist iter!\n");
			return;
		}
		gtk_tree_model_get(model_artist, &(argument->iter_artist), 0, &songartist, -1);
		add_artist_to_playlist(songartist, argument);
		
		gtk_tree_model_get_iter_first(model_playlist, &iter_playlist);

		
		argument->row_playlist = tree_iter_get_row_reference(model_playlist, &iter_playlist);

		start_song(argument);
	}
}

void album_row_activated_cb(GtkTreeView *treeview, GtkTreePath *path, GtkTreeViewColumn *column, struct arguments *argument) {
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
	GtkTreeIter iter_playlist;
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
		
		gtk_tree_model_get_iter_first(model_playlist, &iter_playlist);
		argument->row_playlist = tree_iter_get_row_reference(model_playlist, &iter_playlist);
		start_song(argument);
	}
}


void playlist_row_activated_cb(GtkTreeView *treeview, GtkTreePath *path, GtkTreeViewColumn *column, struct arguments *argument) {
	GtkTreeModel *model_playlist = gtk_tree_view_get_model(treeview);

	
	if((argument->row_playlist = gtk_tree_row_reference_new(model_playlist, path))) {
		start_song(argument);
	}
}

void lib_row_activated_cb(GtkTreeView *treeview, GtkTreePath *path, GtkTreeViewColumn *column, struct arguments *argument) {
	GtkTreeModel *model_library;
	GtkTreeModel *model_playlist;
	GtkTreeIter iter_library;
	GtkTreePath *playlist_path;

	model_library = gtk_tree_view_get_model(treeview);
	model_playlist = gtk_tree_view_get_model(GTK_TREE_VIEW(argument->treeview_playlist));

	clean_playlist(GTK_TREE_VIEW(argument->treeview_playlist), argument);
	gtk_tree_model_get_iter_first(model_library, &iter_library);
	do {
		playlist_queue(&iter_library, model_library, GTK_TREE_VIEW(argument->treeview_playlist), argument);
	} while(gtk_tree_model_iter_next(model_library, &iter_library));

	playlist_path = gtk_tree_path_new_first();

	
	argument->row_playlist = gtk_tree_row_reference_new(model_playlist, path);
	argument->history = g_list_prepend(argument->history, gtk_tree_path_to_string(path));
	start_song(argument);
}

void update_tab_label(GtkTreeModel *model, GtkTreePath *treepath, struct tab_label *tab_label) {
	int count;
	gchar *tempchar;

	count = nb_rows_treeview(model);
	tempchar = g_strdup_printf("%s (%d)", tab_label->str, count);
	gtk_label_set_text(GTK_LABEL(tab_label->label), tempchar);
	g_free(tempchar);
}

void update_tab_label_a(GtkTreeModel *model, GtkTreePath *treepath, GtkTreeIter *iter, struct tab_label *tab_label) {
	update_tab_label(model, treepath, tab_label);
}

void toggle_random_cb(GtkWidget *button, struct arguments *argument) {
	argument->random = (argument->random == 1 ? 0 : 1);
}

void toggle_network_mode_cb(GtkWidget *button, struct pref_arguments *argument) {
	argument->network_mode_set = (argument->network_mode_set == 1 ? 0 : 1);
	gtk_widget_set_sensitive(argument->network_entry, argument->network_mode_set);
	g_settings_set_boolean(argument->preferences, "network-mode-set", argument->network_mode_set);
}

void toggle_lelelescan_cb(GtkWidget *button, struct pref_arguments *argument) {
	argument->lelele_scan = (argument->lelele_scan == 1 ? 0 : 1);
	g_settings_set_boolean(argument->preferences, "lelele-scan", argument->lelele_scan);
}
void toggle_lelele_cb(GtkWidget *button, struct arguments *argument) {
	argument->lelelerandom = (argument->lelelerandom == 1 ? 0 : 1);
}

void toggle_repeat_cb(GtkWidget *button, struct arguments *argument) {
	argument->repeat = (argument->repeat == 1 ? 0 : 1);
}

void toggle_playpause_button_cb(GtkWidget *button, struct arguments *argument) { 
	GtkTreeSelection *selection;
	GtkTreeModel *model;
	GtkTreePath *path;
	GtkTreeIter iter;
	gchar *path_string;
	GList *path_list;
	gint page;
	
	page = gtk_notebook_get_current_page(GTK_NOTEBOOK(argument->libnotebook));

	if(argument->state == GST_STATE_NULL) {
		if(page == 0) {
			selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(argument->treeview_library));
			if((path_list = gtk_tree_selection_get_selected_rows(selection, &model))) {
				lib_row_activated_cb(GTK_TREE_VIEW(argument->treeview_library), path_list->data, NULL, argument);
			}		
		}
		else if(page == 3) {
			selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(argument->treeview_playlist));
			if(gtk_tree_selection_get_selected(selection, &model, &iter)) {
				path = gtk_tree_model_get_path(model, &iter);
				playlist_row_activated_cb(GTK_TREE_VIEW(argument->treeview_playlist), path, NULL, argument);
			}
		}
	}
	else
		toggle_playpause(argument);
}

void next_buttonf_cb(GtkWidget *button, struct arguments *argument) {
	GtkTreeModel *model_playlist;
	model_playlist = gtk_tree_view_get_model(GTK_TREE_VIEW(argument->treeview_playlist));
	gchar *path_string;
	GtkTreePath *path;

	path = gtk_tree_row_reference_get_path(argument->row_playlist);

	if((path_string = gtk_tree_path_to_string(path))) {
		argument->history = g_list_prepend(argument->history, path_string);
		bl_free_song(&argument->current_song);
		if(get_next_playlist_song(GTK_TREE_VIEW(argument->treeview_playlist), argument)) {
			start_song(argument);
		}
	}
}

void previous_buttonf_cb(GtkWidget *button, struct arguments *argument) {
	bl_free_song(&argument->current_song);
	if(get_previous_playlist_song(GTK_TREE_VIEW(argument->treeview_playlist), argument)) {
		start_song(argument);
	}
}

void folder_chooser_cb(GtkWidget *button, struct pref_arguments *argument) {
	GtkWidget *dialog;
	gint res;
	GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER;

	dialog = gtk_file_chooser_dialog_new("Choose library folder", GTK_WINDOW(argument->window),
		action, "Cancel", GTK_RESPONSE_CANCEL, "Open", GTK_RESPONSE_ACCEPT, NULL);

	if(g_file_test("../images/lelele.png", G_FILE_TEST_EXISTS)) {
		gtk_window_set_icon_from_file(GTK_WINDOW(dialog), "../images/lelele.png", NULL);
	}
	else
		gtk_window_set_icon_from_file(GTK_WINDOW(dialog), "/usr/share/leleleplayer/icons/lelele.png", NULL);

	res = gtk_dialog_run(GTK_DIALOG(dialog));

	if(res == GTK_RESPONSE_ACCEPT) {
		GtkFileChooser *chooser;
		chooser = GTK_FILE_CHOOSER(dialog);
		argument->folder = gtk_file_chooser_get_filename(chooser);
		gtk_entry_set_text((GtkEntry*)argument->library_entry, argument->folder);
	}
	gtk_widget_destroy(dialog);
}
	
void preferences_callback_cb(GtkMenuItem *preferences, struct pref_arguments *argument) {
	GtkWidget *dialog, *label_library, *label_browse, *area, *vbox, *hbox_library, *hbox_network, *labelbox_library, *labelbox_network, *library_entry, *browse_button, *window_temp, *complete_box,
		*label_network, *network_box, *label_ip_set, *network_entry;
	GtkDialogFlags flags = GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT;
	gint res;
	const gchar *old_folder;

	label_library = gtk_label_new("");
	label_browse = gtk_label_new("");
	label_network = gtk_label_new("");
	label_ip_set = gtk_label_new("");
	gtk_label_set_markup(GTK_LABEL(label_library), "\t<span weight=\"bold\">Library management:</span>\n");
	gtk_label_set_markup(GTK_LABEL(label_browse), "Select library location:");
	gtk_label_set_markup(GTK_LABEL(label_ip_set), "Set leleleServer's IP address:");
	gtk_label_set_markup(GTK_LABEL(label_network), "\t<span weight=\"bold\">leleleNetwork Options:</span>\n");
	browse_button = gtk_button_new_with_label("Browse...");
	library_entry = gtk_entry_new();
	network_entry = gtk_entry_new();
	argument->folder = (gchar*)g_variant_get_data(g_settings_get_value(argument->preferences, "library"));
	argument->lllserver_address_char = (gchar*)g_variant_get_data(g_settings_get_value(argument->preferences, "network-mode-ip"));
	if(*(argument->folder) == '\0')
		argument->folder = g_get_user_special_dir(G_USER_DIRECTORY_MUSIC);
	if(*(argument->lllserver_address_char) == '\0')
		/* Type default IP here */;

	gtk_entry_set_text(GTK_ENTRY(library_entry), argument->folder);
	gtk_entry_set_text(GTK_ENTRY(network_entry), argument->lllserver_address_char);
	gtk_widget_set_sensitive(network_entry, argument->network_mode_set);
	vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
	hbox_library = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
	hbox_network = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
	labelbox_library = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
	labelbox_network = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
	dialog = gtk_dialog_new_with_buttons("Preferences", GTK_WINDOW(argument->window), flags, "Cancel", GTK_RESPONSE_REJECT, "Save", GTK_RESPONSE_ACCEPT, NULL);
	if(g_file_test("../images/leleleplayer.png", G_FILE_TEST_EXISTS))
		gtk_window_set_icon_from_file(GTK_WINDOW(dialog), "../images/leleleplayer.png", NULL);
	else
		gtk_window_set_icon_from_file(GTK_WINDOW(dialog), "/usr/share/leleleplayer/icons/leleleplayer.png", NULL);

	area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
	complete_box = gtk_check_button_new_with_label("LeleleScan (complete but longer) (checkbox not functional now)");
	network_box = gtk_check_button_new_with_label("Activate leleleNetwork (will allow you to access music from other leleleplayers with the lelelerandom button)");

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(complete_box), argument->lelele_scan);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(network_box), argument->network_mode_set);
	gtk_window_set_default_size(GTK_WINDOW(dialog), 500, 100);
	gtk_window_set_resizable(GTK_WINDOW(dialog), FALSE);

	g_signal_connect(G_OBJECT(browse_button), "clicked", G_CALLBACK(folder_chooser_cb), argument);
	g_signal_connect(G_OBJECT(complete_box), "toggled", G_CALLBACK(toggle_lelelescan_cb), argument);
	g_signal_connect(G_OBJECT(network_box), "toggled", G_CALLBACK(toggle_network_mode_cb), argument);
	argument->library_entry = library_entry;
	argument->network_entry = network_entry;
	window_temp = argument->window;
	argument->window = dialog;

	gtk_box_set_homogeneous(GTK_BOX(vbox), TRUE);
	gtk_box_set_homogeneous(GTK_BOX(hbox_library), FALSE);
	gtk_box_set_homogeneous(GTK_BOX(hbox_network), FALSE);
	
	gtk_box_pack_start(GTK_BOX(vbox), labelbox_library, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(labelbox_library), label_library, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox_library, FALSE, FALSE, 0);
		gtk_box_pack_start(GTK_BOX(hbox_library), label_browse, FALSE, FALSE, 0);
		gtk_box_pack_start(GTK_BOX(hbox_library), library_entry, TRUE, TRUE, 0);
		gtk_box_pack_start(GTK_BOX(hbox_library), browse_button, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), complete_box, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), labelbox_network, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(labelbox_network), label_network, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox_network, FALSE, FALSE, 0);
		gtk_box_pack_start(GTK_BOX(hbox_network), label_ip_set, FALSE, FALSE, 0);
		gtk_box_pack_start(GTK_BOX(hbox_network), network_entry, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), network_box, FALSE, FALSE, 0);

	
	gtk_container_add(GTK_CONTAINER(area), vbox);

	gtk_widget_set_size_request(dialog, 400, 120);
	gtk_widget_show_all(dialog);
	old_folder = argument->folder;
	res = gtk_dialog_run(GTK_DIALOG(dialog));

	if(argument->folder != NULL && res == GTK_RESPONSE_ACCEPT) {
		/* TODO: Error handling (if IP is invalid/folder doesn't exist) */
		argument->folder = g_strdup(gtk_entry_get_text(GTK_ENTRY(library_entry)));
		g_settings_set_value(argument->preferences, "library", g_variant_new_string(argument->folder));
		argument->lllserver_address_char = g_strdup(gtk_entry_get_text(GTK_ENTRY(network_entry)));
		g_settings_set_value(argument->preferences, "network-mode-ip", g_variant_new_string(argument->lllserver_address_char));
		if(strcmp(old_folder, argument->folder)) {
			gtk_list_store_clear(argument->store_library);
			gtk_tree_store_clear(argument->store_album);
			gtk_tree_store_clear(argument->store_artist);
			argument->erase = TRUE;
			g_thread_new("analyze", (GThreadFunc)analyze_thread, argument);
		}
		else {
			argument->erase = FALSE;
		}
		g_settings_set_boolean(argument->preferences, "is-configured", TRUE);
	}
	gtk_widget_destroy(dialog); 
	argument->window = window_temp;
}

void add_file_to_playlist_cb(GtkMenuItem *add_file, struct arguments *argument) {
	struct bl_song song;
	int resnum;
	GtkWidget *dialog;
	GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_OPEN;
	gint res;
	GSList *filelist;
	char *filename;
	GtkTreeModel *model_playlist;
	GtkTreeIter iter_playlist;
	model_playlist = gtk_tree_view_get_model(GTK_TREE_VIEW(argument->treeview_playlist));
	GtkTreePath *path;

	dialog = gtk_file_chooser_dialog_new("Open audio file(s)", GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(add_file))),
	action, "Cancel", GTK_RESPONSE_CANCEL, "Open", GTK_RESPONSE_ACCEPT, NULL);
	if(g_file_test("../images/lelele.png", G_FILE_TEST_EXISTS))
		gtk_window_set_icon_from_file(GTK_WINDOW(dialog), "../images/lelele.png", NULL);
	else
		gtk_window_set_icon_from_file(GTK_WINDOW(dialog), "/usr/share/leleleplayer/icons/lelele.png", NULL);

	GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialog);

	gtk_file_chooser_set_select_multiple(chooser, TRUE);

	res = gtk_dialog_run(GTK_DIALOG(dialog));
	if(res == GTK_RESPONSE_ACCEPT) {
		path = gtk_tree_row_reference_get_path(argument->row_playlist);
		argument->history = g_list_prepend(argument->history, gtk_tree_path_to_string(path));
		for(filelist = gtk_file_chooser_get_filenames(chooser); filelist; filelist = filelist->next) {
			GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialog);
			filename = (gchar*)filelist->data;
			if((resnum = bl_analyze(filename, &song)) == 0)
				printf("Couldn't conclude\n");
			else {
				gtk_list_store_append(argument->store_playlist, &iter_playlist);
				gtk_list_store_set(argument->store_playlist, &iter_playlist, PLAYING, "", -1);
				gtk_list_store_set(argument->store_playlist, &iter_playlist, TRACKNUMBER, song.tracknumber, -1);
				gtk_list_store_set(argument->store_playlist, &iter_playlist, TRACK, song.title, -1);
				gtk_list_store_set(argument->store_playlist, &iter_playlist, ALBUM, song.album, -1);
				gtk_list_store_set(argument->store_playlist, &iter_playlist, ARTIST, song.artist, -1);		
				gtk_list_store_set(argument->store_playlist, &iter_playlist, FORCE_TEMPO, song.force_vector.tempo, -1);
				gtk_list_store_set(argument->store_playlist, &iter_playlist, FORCE_AMP, song.force_vector.amplitude, -1);
				gtk_list_store_set(argument->store_playlist, &iter_playlist, FORCE_FREQ, song.force_vector.frequency, -1);
				gtk_list_store_set(argument->store_playlist, &iter_playlist, FORCE_ATK, song.force_vector.attack, -1);
				if(resnum > 0)
					gtk_list_store_set(argument->store_playlist, &iter_playlist, TEXTFORCE, "Loud", -1);
				else if(resnum < 0)
					gtk_list_store_set(argument->store_playlist, &iter_playlist, TEXTFORCE, "Calm", -1);
				gtk_list_store_set(argument->store_playlist, &iter_playlist, AFILE, filename, -1);
				argument->playlist_count++;

				
				argument->row_playlist = tree_iter_get_row_reference(GTK_TREE_MODEL(argument->store_playlist), &iter_playlist);
				bl_free_song(&song);
			}
			g_free(filename);
		}
	}
	gtk_widget_destroy(dialog);
}

void open_audio_file_cb(GtkMenuItem *open, struct arguments *argument) {
	struct bl_song song;
	int resnum;
	GtkWidget *dialog;
	GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_OPEN;
	GtkTreeIter iter_playlist;
	gint res;
	GtkTreeModel *model_playlist;
	model_playlist = gtk_tree_view_get_model(GTK_TREE_VIEW(argument->treeview_playlist));
	GtkTreePath *path;

	dialog = gtk_file_chooser_dialog_new("Open audio file", GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(open))),
	action, "Cancel", GTK_RESPONSE_CANCEL, "Open", GTK_RESPONSE_ACCEPT, NULL);

	res = gtk_dialog_run(GTK_DIALOG(dialog));
	if(res == GTK_RESPONSE_ACCEPT) {
		path = gtk_tree_row_reference_get_path(argument->row_playlist);
		argument->history = g_list_prepend(argument->history, gtk_tree_path_to_string(path));
		char *filename;
		GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialog);
		filename = gtk_file_chooser_get_filename(chooser);
		if((resnum = bl_analyze(filename, &song)) == 0)
			printf("Couldn't conclude\n");
		else { // Create an iter_forge() function?
			gtk_list_store_append(argument->store_playlist, &iter_playlist);
			gtk_list_store_set(argument->store_playlist, &iter_playlist, PLAYING, "", -1);
			gtk_list_store_set(argument->store_playlist, &iter_playlist, TRACKNUMBER, song.tracknumber, -1);
			gtk_list_store_set(argument->store_playlist, &iter_playlist, TRACK, song.title, -1);
			gtk_list_store_set(argument->store_playlist, &iter_playlist, ALBUM, song.album, -1);
			gtk_list_store_set(argument->store_playlist, &iter_playlist, ARTIST, song.artist, -1);		
			gtk_list_store_set(argument->store_playlist, &iter_playlist, FORCE_TEMPO, song.force_vector.tempo, -1);
			gtk_list_store_set(argument->store_playlist, &iter_playlist, FORCE_AMP, song.force_vector.amplitude, -1);
			gtk_list_store_set(argument->store_playlist, &iter_playlist, FORCE_FREQ, song.force_vector.frequency, -1);
			gtk_list_store_set(argument->store_playlist, &iter_playlist, FORCE_ATK, song.force_vector.attack, -1);
			if(resnum > 0) 
				gtk_list_store_set(argument->store_playlist, &iter_playlist, TEXTFORCE, "Loud", -1);
			else if(resnum < 0)
				gtk_list_store_set(argument->store_playlist, &iter_playlist, TEXTFORCE, "Calm", -1);
			else
				gtk_list_store_set(argument->store_playlist, &iter_playlist, TEXTFORCE, "Couldn't conclude", -1);
			gtk_list_store_set(argument->store_playlist, &iter_playlist, AFILE, filename, -1);
			argument->playlist_count++;

			
			argument->row_playlist = tree_iter_get_row_reference(GTK_TREE_MODEL(argument->store_playlist), &iter_playlist);
			bl_free_song(&song);
			start_song(argument);
		}
		g_free(filename);
	}	
	gtk_widget_destroy(dialog);
}

void end_of_playlist_cb(GstBus *bus, GstMessage *msg, struct arguments *argument) {
	reset_ui(argument);
}

void refresh_ui_cb(GstBus *bus, GstMessage *msg, struct arguments *argument) {
	GtkTreeSelection *selection;
	GtkTreePath *path;
	GtkTreeViewColumn *column;
	GtkTreeModel *model_playlist;
	GtkTreeIter temp_iter;
	GtkTreeIter iter_playlist;
	
	GstFormat fmt = GST_FORMAT_TIME;

	if((model_playlist = gtk_tree_view_get_model(GTK_TREE_VIEW(argument->treeview_playlist)))) {
		selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(argument->treeview_playlist));
		path = gtk_tree_row_reference_get_path(argument->row_playlist);
		gtk_tree_model_get_iter(model_playlist, &iter_playlist, path);
		column = gtk_tree_view_get_column(GTK_TREE_VIEW(argument->treeview_playlist), PLAYING);

		gtk_tree_model_get_iter_first(gtk_tree_view_get_model(GTK_TREE_VIEW(argument->treeview_playlist)), &(temp_iter));

		do {
			gtk_list_store_set(argument->store_playlist, &(temp_iter), PLAYING, "", -1);
		}
		while(gtk_tree_model_iter_next(model_playlist, &(temp_iter)));
		gtk_button_set_image(GTK_BUTTON(argument->playpause_button), gtk_image_new_from_icon_name("media-playback-pause-symbolic", GTK_ICON_SIZE_BUTTON));
		gtk_list_store_set(argument->store_playlist, &iter_playlist, PLAYING, "▶", -1);
		gtk_tree_selection_unselect_all(selection);
		gtk_tree_selection_select_iter(selection, &iter_playlist);
		gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(argument->treeview_playlist), path, column, 1, 0.3, 1);
	
		gtk_label_set_text(GTK_LABEL(argument->album_label), argument->current_song.album);
		gtk_label_set_text(GTK_LABEL(argument->artist_label), argument->current_song.artist);
		gtk_label_set_text(GTK_LABEL(argument->title_label), argument->current_song.title);	
	}
	while(!gst_element_query_duration(argument->playbin, fmt,
		&(argument->duration)))
		;
	gst_element_query_position(argument->playbin, 
		fmt, &(argument->elapsed));

	g_signal_handler_block(argument->progressbar, argument->progressbar_update_signal_id);
	gtk_adjustment_configure(argument->adjust, 0, 0, argument->duration/GST_SECOND, 
		1, 1, 1);
	g_signal_handler_unblock(argument->progressbar, argument->progressbar_update_signal_id);
	refresh_progressbar(argument);
}

void message_application_cb(GstBus *bus, GstMessage *msg, struct arguments *argument) {
	const GstStructure *structure = gst_message_get_structure(msg);
	if(!strcmp(gst_structure_get_name(structure), "tags")) {
/*
		GstTagList *tags;
		gchar *str;
		gint samplerate = 0, channels = 0, bitrate;
		GstStructure *s;
		GstPad *pad;
		GstCaps *caps;

		g_signal_emit_by_name(argument->playbin, "get-audio-pad", 0, &pad);
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

		g_signal_emit_by_name(argument->playbin, "get-audio-tags", 0, &tags);
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
		argument->str_bitrate = NULL;*/
	}
	else if(!strcmp(gst_structure_get_name(structure), "next_song")) {
		GtkTreeModel *model_playlist;
		GtkTreePath *path;
		g_mutex_lock(&argument->queue_mutex);
		model_playlist = gtk_tree_view_get_model(GTK_TREE_VIEW(argument->treeview_playlist));
		
		path = gtk_tree_row_reference_get_path(argument->row_playlist);
		argument->history = g_list_prepend(argument->history, gtk_tree_path_to_string(path));
		if(!argument->repeat) {
			bl_free_song(&argument->current_song); 
			if(get_next_playlist_song(GTK_TREE_VIEW(argument->treeview_playlist), argument)) {
				/*GtkTreeModel *model_playlist;
	
				model_playlist = gtk_tree_view_get_model(GTK_TREE_VIEW(argument->treeview_playlist));*/
				GtkTreeIter iter_playlist;
				tree_row_reference_get_iter(argument->row_playlist, &iter_playlist);

				gtk_tree_model_get(model_playlist, &iter_playlist, AFILE, &argument->current_song.filename, 
				TRACKNUMBER, &argument->current_song.tracknumber, TRACK, &argument->current_song.title, 
				ALBUM, &argument->current_song.album, ARTIST, &argument->current_song.artist, 
				FORCE, &argument->current_song.force, FORCE_TEMPO, &argument->current_song.force_vector.tempo, 
				FORCE_AMP, &argument->current_song.force_vector.amplitude, FORCE_FREQ, &argument->current_song.force_vector.frequency, 
				FORCE_ATK, &argument->current_song.force_vector.attack, -1);
			}
			else {
				argument->current_song.filename = NULL;
			}
		}
		g_mutex_unlock(&argument->queue_mutex);
		g_cond_signal(&argument->queue_cond);
	}
}

void ui_playlist_changed_cb(GtkTreeModel *playlist_model, GtkTreePath *path, GtkTreeIter *iter, GtkNotebook *libnotebook) {
	gtk_notebook_set_current_page(libnotebook, 3);
}

void slider_changed_cb(GtkRange *progressbar, struct arguments *argument) {
	if(!gst_element_seek(argument->playbin, 1.0, GST_FORMAT_TIME, GST_SEEK_FLAG_FLUSH, 
		GST_SEEK_TYPE_SET, gtk_adjustment_get_value(argument->adjust)*GST_SECOND, GST_SEEK_TYPE_NONE,
		GST_CLOCK_TIME_NONE)) 
		g_warning("Seek failed!\n");
} 

void volume_scale_changed_cb(GtkScaleButton* volume_scale, gdouble value, struct arguments *argument) {
	double vol = pow(value, 3);
	argument->vol = value;
	g_object_set(argument->playbin, "volume", argument->vol, NULL);
	g_settings_set_double(argument->preferences, "volume", value);
}

void add_album_selection_to_playlist_cb(GtkWidget *menuitem, struct arguments *argument) {
	GtkTreeSortable *sortable = GTK_TREE_SORTABLE(argument->store_library);
	gtk_tree_sortable_set_sort_column_id(sortable, TRACKNUMBER, GTK_SORT_ASCENDING);
	GtkTreeModel *model;
	GtkTreeIter iter, iter_playlist;
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

				gtk_tree_model_get_iter_first(model_playlist, &iter_playlist);

				
				argument->row_playlist = tree_iter_get_row_reference(model_playlist, &iter_playlist);
			}
			path_list = path_list->next;
		}
		gtk_label_set_markup(GTK_LABEL(label), "<span foreground=\"red\">Playlist</span>");
	}
	g_signal_handler_unblock(model_playlist, argument->playlist_update_signal_id);
}

void add_artist_selection_to_playlist_cb(GtkWidget *menuitem, struct arguments *argument) {
	GtkTreeSortable *sortable = GTK_TREE_SORTABLE(argument->store_library);
	gtk_tree_sortable_set_sort_column_id(sortable, TRACKNUMBER, GTK_SORT_ASCENDING);
	GtkTreeModel *model;
	GtkTreeIter iter, iter_playlist;
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

				gtk_tree_model_get_iter_first(model_playlist, &iter_playlist);

				
				argument->row_playlist = tree_iter_get_row_reference(model_playlist, &iter_playlist);
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

void add_library_selection_to_playlist_cb(GtkWidget *menuitem, struct arguments *argument) {
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

void remove_playlist_selection_from_playlist_cb(GtkWidget *menuitem, struct arguments *argument) {
	GtkTreeModel *model;
	GtkTreeIter iter;
	GtkTreePath *path;
	GtkTreeSelection *selection;
	gboolean valid;
	GList *path_list;
	gchar *path_string;
	gchar *tempstring;
	gchar *current_path_string;
	gboolean deleted = FALSE;
	GtkTreeModel *model_playlist;
	int i = 0;
	int path_string_i;

	model_playlist = gtk_tree_view_get_model(GTK_TREE_VIEW(argument->treeview_playlist));
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(argument->treeview_playlist));
	path = gtk_tree_row_reference_get_path(argument->row_playlist);
	current_path_string = gtk_tree_path_to_string(path);

	if((path_list = gtk_tree_selection_get_selected_rows(selection, &model))) {
		while(path_list) {
			tempstring = gtk_tree_path_to_string((GtkTreePath*)path_list->data);
			path_string_i = strtol(tempstring, NULL, 10) - i;
			path_string = g_strdup_printf("%d", path_string_i);
			gtk_tree_model_get_iter_from_string(model_playlist, &iter, path_string);
			if(!g_strcmp0(current_path_string, path_string))
				deleted = TRUE;
			g_signal_handler_block(model_playlist, argument->playlist_update_signal_id);
			gtk_list_store_remove(argument->store_playlist, &iter);
			g_signal_handler_unblock(model_playlist, argument->playlist_update_signal_id);
			valid = gtk_tree_model_get_iter_from_string(model_playlist, &iter, path_string);
			g_free(path_string);
			g_free(tempstring);
			++i;
			path_list = path_list->next;
		}
		if(deleted && valid) {
			argument->row_playlist = tree_iter_get_row_reference(model_playlist, &iter);
			start_song(argument);
		}
		if(!valid) 
			reset_ui(argument);
	}
}

gboolean playlist_del_button_cb(GtkWidget *treeview, GdkEventKey *event, struct arguments *argument) {
	guint state = event->state;
	gboolean ctrl_pressed = (state & GDK_CONTROL_MASK ? TRUE : FALSE);

	if((event->type == GDK_KEY_PRESS) && (event->keyval == GDK_KEY_Delete)
	&& !ctrl_pressed)
		remove_playlist_selection_from_playlist_cb(NULL, argument);
	else if((event->type == GDK_KEY_PRESS) && (event->keyval == GDK_KEY_a)
	&& ctrl_pressed) {
		GtkTreeSelection *selection;
	
		selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));
		gtk_tree_selection_select_all(selection);
	}
	return FALSE;
}

void changed_page_notebook_cb(GtkNotebook *notebook, GtkWidget *page, guint page_num, gpointer data) {
	if(page_num == 3) { // "Playlist" tab selected 
		GtkWidget *label = gtk_notebook_get_tab_label(GTK_NOTEBOOK(notebook), page);
		gtk_label_set_markup(GTK_LABEL(label), "Playlist");
	}
}

gboolean treeviews_right_click_cb(GtkWidget *treeview, GdkEventButton *event, struct arguments *argument) {
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

void time_spin_changed_cb(GtkSpinButton *spin_button, struct arguments *argument) {
	time_spin_input_cb(spin_button, &argument->timer_delay, argument);
}

gint time_spin_input_cb(GtkSpinButton *spin_button, gdouble *new_val, struct arguments *argument) {
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

gint time_spin_output_cb(GtkSpinButton *spin_button, struct arguments *argument) {
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

void time_checkbox_toggled_cb(GtkToggleButton *togglebutton, struct arguments *argument) {
	gboolean mode = gtk_toggle_button_get_active(togglebutton);

	if(mode == TRUE && (argument->timer_delay != 0.)) {
		argument->sleep_timer = g_timer_new();
	}
	else if(argument->sleep_timer) {
		g_timer_destroy(argument->sleep_timer);
		argument->sleep_timer = NULL;
	}
}


void search_cb(GtkSearchEntry *search_entry, struct arguments *argument) {
	argument->search_entry_text = (gchar*)gtk_entry_get_text(GTK_ENTRY(search_entry));

	gtk_tree_model_filter_refilter(argument->library_filter);
	gtk_tree_model_filter_refilter(argument->artist_filter);
	gtk_tree_model_filter_refilter(argument->album_filter);
}

void remote_lllp_connected_cb(GObject *listener, GAsyncResult *res, gpointer pargument) {
	struct pref_arguments *argument = (struct pref_arguments *)pargument;
	GSocketConnection *connection;
	GInputStream *server_stream;
	GSocketClient *client;
	GFile *output_file;
	GOutputStream *output_stream;
	GInputStream *file_stream;
	gsize count;

	connection = g_socket_listener_accept_finish(G_SOCKET_LISTENER(listener), res, NULL, NULL);
	
	server_stream = g_io_stream_get_input_stream(G_IO_STREAM(connection));
	g_input_stream_read_all(server_stream, &count, sizeof(gsize), NULL, NULL, NULL);
	gchar address_remote_player_char[count];
	g_input_stream_read_all(server_stream, &address_remote_player_char, count, NULL, NULL, NULL);

	g_input_stream_read_all(server_stream, &count, sizeof(gsize), NULL, NULL, NULL);
	gchar filename[count];
	g_input_stream_read_all(server_stream, &filename, count, NULL, NULL, NULL);

	client = g_socket_client_new();
	output_file = g_file_new_for_path(filename);
	file_stream = G_INPUT_STREAM(g_file_read(output_file, NULL, NULL));

	connection = g_socket_client_connect_to_host(client, address_remote_player_char, 18322, NULL, NULL); // 18322 = RCV

	output_stream = g_io_stream_get_output_stream(G_IO_STREAM(connection));
	g_output_stream_splice_async(output_stream, file_stream, 
		G_OUTPUT_STREAM_SPLICE_CLOSE_SOURCE & G_OUTPUT_STREAM_SPLICE_CLOSE_TARGET, 
		G_PRIORITY_DEFAULT, NULL, NULL, NULL);

	g_socket_listener_accept_async(G_SOCKET_LISTENER(listener), NULL, remote_lllp_connected_cb, pargument);
}

void pad_added_handler_cb(GstElement *decodebin, GstPad *pad, gpointer pargument) {
	struct arguments *argument = (struct arguments *)pargument;
	GstCaps *caps;
	GstStructure *str;
	GstPad *audiopad;

	/* only link once */
	audiopad = gst_element_get_static_pad (argument->sink, "sink");
	if (GST_PAD_IS_LINKED (audiopad)) {
		g_object_unref (audiopad);
		return;
	}

	/* check media type */
	caps = gst_pad_query_caps (pad, NULL);
	str = gst_caps_get_structure (caps, 0);
	if (!g_strrstr (gst_structure_get_name (str), "audio")) {
		gst_caps_unref (caps);
		gst_object_unref (audiopad);
		return;
	}
	gst_caps_unref (caps);

	/* link'n'play */
	gst_pad_link (pad, audiopad);

	g_object_unref (audiopad);
}
