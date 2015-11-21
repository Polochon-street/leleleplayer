#include "gui.h"

void toggle_playpause(struct arguments *argument) {
	if(argument->state == GST_STATE_PLAYING)
		pause_song(argument);
	else if(argument->state == GST_STATE_PAUSED)
		resume_song(argument);
}

gboolean refresh_config_progressbar(struct pref_arguments *argument) {	
	GAsyncQueue *msg_queue = argument->msg_queue;
	gpointer msg = NULL;
	gboolean valid;
	gboolean found = FALSE;
	GtkTreeIter iter, tempiter;
	int nblines = argument->nblines;
	int count = argument->count;
	gchar tempforce[17];
	gchar *progression;
	gchar *tempfile;
	struct bl_song *song;
	GtkTreeModel *model_library;
	gtk_spinner_start(GTK_SPINNER(argument->spinner));
	GtkTreeSortable *sortable_library = GTK_TREE_SORTABLE(argument->store_library);
	GtkTreeSortable *sortable_artist = GTK_TREE_SORTABLE(argument->store_artist);
	GtkTreeSortable *sortable_album = GTK_TREE_SORTABLE(argument->store_album);
	gtk_tree_sortable_set_sort_column_id(sortable_library, ARTIST, GTK_SORT_ASCENDING);
	gtk_tree_sortable_set_sort_column_id(sortable_artist, COLUMN_ARTIST, GTK_SORT_ASCENDING);


	model_library = gtk_tree_view_get_model(GTK_TREE_VIEW(argument->treeview));
	progression = g_strdup_printf("<span weight=\"bold\">%d/%d</span>", count, nblines);
	gtk_label_set_markup(GTK_LABEL(argument->progress_label), progression);
	g_free(progression);

	do {
		if((msg != NULL)) {
			song = (struct bl_song*)msg;
			if(song->resnum == 0)
				strcpy(tempforce, "Loud");
			else if(song->resnum == 1) {
				strcpy(tempforce, "Calm");
			}
			else
				strcpy(tempforce, "Can't conclude"); 
			
			valid = gtk_tree_model_get_iter_first(model_library, &tempiter);
			while(valid) {
				gtk_tree_model_get(model_library, &tempiter, AFILE, &tempfile, -1); 
				if(!g_strcmp0(tempfile, song->filename)) {
					found = TRUE;
					break;
				}
				valid = gtk_tree_model_iter_next(model_library, &tempiter);
			}
			if(found == TRUE) {
				if(song->resnum == 2)
					gtk_list_store_set(argument->store_library, &tempiter, PLAYING, "", TRACKNUMBER, song->tracknumber, TRACK, song->title, ALBUM, song->album, ARTIST, song->artist,
			 			FORCE, (float)song->resnum, AFILE, song->filename, -1);
				else
					gtk_list_store_set(argument->store_library, &tempiter, PLAYING, "", TRACKNUMBER, song->tracknumber, TRACK, song->title, ALBUM, song->album, ARTIST, song->artist,
			 			FORCE, (float)song->resnum, FORCE_TEMPO, song->force_vector.x, FORCE_AMP, song->force_vector.y, FORCE_FREQ, song->force_vector.z, FORCE_ATK, song->force_vector.t, TEXTFORCE, tempforce, AFILE, song->filename, -1);
			}
			else {
				gtk_list_store_append(argument->store_library, &iter);			
				gtk_list_store_set(argument->store_library, &iter, PLAYING, "", TRACKNUMBER, song->tracknumber, TRACK, song->title, ALBUM, song->album, ARTIST, song->artist,
			 	FORCE, (float)song->resnum, FORCE_TEMPO, song->force_vector.x, FORCE_AMP, song->force_vector.y, FORCE_FREQ, song->force_vector.z, FORCE_ATK, song->force_vector.t, TEXTFORCE, tempforce, AFILE, song->filename, -1);
				add_entry_artist_tab(argument->treeview_artist, argument->store_artist, model_library, &iter);
				add_entry_album_tab(argument->treeview_album, argument->store_album, model_library, &iter);
			}
			bl_free_song(msg);
			g_free(song->filename);
			msg = NULL; 
		}
  	} while(((msg = g_async_queue_try_pop(msg_queue)) != NULL));
	
	if(argument->terminate == TRUE) {
		g_async_queue_unref(msg_queue);
		msg_queue = NULL;
		gtk_spinner_stop(GTK_SPINNER(argument->spinner));
		argument->library_set = TRUE;
		gtk_label_set_text(GTK_LABEL(argument->progress_label), "");
		return FALSE;
	}
	else
		return TRUE;
}

void analyze_thread(struct pref_arguments *argument) {
	xmlKeepBlanksDefault(0);
	GList *list, *l = NULL;
	xmlDocPtr library;
	xmlNodePtr cur, child, cur_find, child_find;
	xmlTextWriterPtr library_writer;
	int tempresnum = 0;
	int i;
	gchar *temptracknumber;

	list = g_list_alloc();

	if(argument->erase) {
		library_writer = xmlNewTextWriterFilename(argument->lib_path, 0);
		if(library_writer == NULL) {
			g_warning("Couldn't write library file");
			return;
		}
		xmlTextWriterStartDocument(library_writer, NULL, "UTF-8", NULL);
		xmlTextWriterStartElement(library_writer, "lelelelibrary");
		xmlTextWriterEndElement(library_writer);
		xmlTextWriterEndDocument(library_writer);
		xmlFreeTextWriter(library_writer);
		library = xmlParseFile(argument->lib_path);
	}
	else {
		library = xmlParseFile(argument->lib_path);
		if(library == NULL)
			g_warning("Couldn't open library file; creating a new");
		library_writer = xmlNewTextWriterFilename(argument->lib_path, 0);
		if(library_writer == NULL) {
			g_warning("Couldn't write library file");
			return;
		}
		xmlTextWriterStartDocument(library_writer, NULL, "UTF-8", NULL);
		xmlTextWriterStartElement(library_writer, "lelelelibrary");
		xmlTextWriterEndElement(library_writer);
		xmlTextWriterEndDocument(library_writer);
		xmlFreeTextWriter(library_writer);
		library = xmlParseFile(argument->lib_path);

	}
	cur = xmlDocGetRootElement(library);

	GDir *dir = g_dir_open(argument->folder, 0, NULL);
	explore(dir, argument->folder, list);
	list = list->next;

	int nblines = 0;
	gchar *tempstring;
	GAsyncQueue *msg_queue = g_async_queue_new();
	argument->msg_queue = msg_queue;
	char *msg;

	for(l = list; l != NULL; l = l->next) 
		nblines++;
	
	argument->nblines = nblines;

	gpointer msg_thread;
	int resnum;
	argument->count = 0;
	gboolean found = FALSE;
	argument->terminate = FALSE;

	g_idle_add((GSourceFunc)refresh_config_progressbar, argument);

	for(l = list; l != NULL; l = l->next) {
		((gchar*)l->data)[strcspn((gchar*)l->data, "\n")] = '\0';
		found = FALSE;

		cur_find = xmlDocGetRootElement(library);
		cur_find = cur_find->children;
		while(cur_find != NULL) {
			if((!xmlStrcmp(cur_find->name, (const xmlChar *)"song"))) {
				for(child_find = cur_find->children; child_find != NULL; child_find = child_find->next) {
					if((!xmlStrcmp(child_find->name, (const xmlChar *)"filename"))) {
						tempstring = xmlNodeGetContent(child_find->children);
						if(!g_strcmp0(tempstring, l->data)) {
							found = TRUE;
						}
						free(tempstring);
					}	
					if((!xmlStrcmp(child_find->name, (const xmlChar *)"analyze-resnum"))) {
						tempstring = xmlNodeGetContent(child_find->children);
						tempresnum = atoi(tempstring);
						g_free(tempstring);
					}
				}
				if(found == TRUE)
					break;
			}
			cur_find = cur_find->next;
		}
		printf("%d, %s\n", ( argument->lelele_scan && ( (tempresnum == 2) || (found == FALSE) ) ), l->data);
		struct bl_song song;
		struct bl_song msg_song;
		child = xmlNewTextChild(cur, NULL, "song", NULL);
		int tempint;
		gchar *amplitude, *freq, *tempo, *atk, *resnum_s;
		if((resnum = bl_analyze(l->data, &song, 0, ( argument->lelele_scan && ( (tempresnum == 2) || !found ) ) )) < 3) {
			for(i = 0; (song.tracknumber[i] != '\0') && (g_ascii_isdigit(song.tracknumber[i]) == FALSE); ++i)
				song.tracknumber[i] = '0';
			if(song.tracknumber[i] != '\0') {
				tempint = strtol(song.tracknumber, NULL, 10);
				temptracknumber = g_strdup_printf("%02d", tempint);
				g_free(song.tracknumber);
				song.tracknumber = temptracknumber;
			}
			resnum_s = g_strdup_printf("%d", resnum);
			amplitude = g_strdup_printf("%f", song.force_vector.y);
			freq = g_strdup_printf("%f", song.force_vector.z);
			tempo = g_strdup_printf("%f", song.force_vector.x);
			atk = g_strdup_printf("%f", song.force_vector.t);
	
			if(found == FALSE) {
				xmlNewTextChild(child, NULL, "filename", l->data);
				xmlNewTextChild(child, NULL, "tracknumber", song.tracknumber);
				xmlNewTextChild(child, NULL, "title", song.title);
				xmlNewTextChild(child, NULL, "album", song.album);
				xmlNewTextChild(child, NULL, "artist", song.artist);
				xmlNewTextChild(child, NULL, "analyze-resnum", resnum_s);
				xmlNewTextChild(child, NULL, "analyze-amplitude", amplitude);
				xmlNewTextChild(child, NULL, "analyze-freq", freq);
				xmlNewTextChild(child, NULL, "analyze-tempo", tempo);
				xmlNewTextChild(child, NULL, "analyze-atk", atk);
			}
			else {
				if((!xmlStrcmp(cur_find->name, (const xmlChar *)"song"))) {
					for(child_find = cur_find->children; child_find != NULL; child_find = child_find->next) {
						if((!xmlStrcmp(child_find->name, (const xmlChar *)"title"))) 
							xmlNodeSetContent(child_find, song.title);
						else if((!xmlStrcmp(child_find->name, (const xmlChar *)"artist")))
							xmlNodeSetContent(child_find, song.artist);
						else if((!xmlStrcmp(child_find->name, (const xmlChar *)"album")))
							xmlNodeSetContent(child_find, song.album);
						else if((!xmlStrcmp(child_find->name, (const xmlChar *)"tracknumber")))
							xmlNodeSetContent(child_find, song.tracknumber);	
						if(tempresnum == 2) {
							if((!xmlStrcmp(child_find->name, (const xmlChar *)"analyze-resnum"))) {
								xmlNodeSetContent(child_find, resnum_s);
							}
							else if((!xmlStrcmp(child_find->name, (const xmlChar *)"analyze-amplitude"))) {
								xmlNodeSetContent(child_find, amplitude);
							}
							else if((!xmlStrcmp(child_find->name, (const xmlChar *)"analyze-freq"))) {
								xmlNodeSetContent(child_find, freq);
							}
							else if((!xmlStrcmp(child_find->name, (const xmlChar *)"analyze-tempo"))) {
								xmlNodeSetContent(child_find, tempo);
							}
							else if((!xmlStrcmp(child_find->name, (const xmlChar *)"analyze-atk"))) {
								xmlNodeSetContent(child_find, atk);
							}		
						}
					}
				}
			}
			g_free(amplitude);
			g_free(resnum_s);
			g_free(freq);
			g_free(tempo);
			g_free(atk);
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
			msg_song.filename = g_malloc(strlen(l->data)+1);
			msg_song.filename = strcpy(msg_song.filename, l->data);
			g_async_queue_push(msg_queue, (gpointer)&msg_song);
			bl_free_song(&song);
			xmlSaveFormatFile(argument->lib_path, library, 1);
			//fflush(library); 
		}
		argument->count++;
	}
	argument->terminate = TRUE;
	g_list_free_full(list, g_free);
	xmlSaveFormatFile(argument->lib_path, library, 1);
	xmlFreeDoc(library);	
	//fclose(test);
}

void reset_ui(struct arguments *argument) {
	gboolean valid;
	GtkTreeIter iter;
	GtkTreeModel *model_playlist = gtk_tree_view_get_model(GTK_TREE_VIEW(argument->treeview_playlist));

	GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(argument->treeview_playlist));

	gst_element_set_state(argument->playbin, GST_STATE_NULL);
	gtk_button_set_image(GTK_BUTTON(argument->playpause_button), gtk_image_new_from_icon_name("media-playback-start-symbolic", GTK_ICON_SIZE_BUTTON));
	
	valid = gtk_tree_model_get_iter_first(model_playlist, &iter);
	while(valid) {
		gtk_list_store_set(argument->store_playlist, &iter, PLAYING, "", -1);
		valid = gtk_tree_model_iter_next(model_playlist, &iter);
	}

	if(argument->bartag)
		g_source_remove(argument->bartag);
	argument->bartag = 0;

	gtk_widget_set_sensitive(argument->progressbar, FALSE);
	g_signal_handler_block(argument->progressbar, argument->progressbar_update_signal_id);
	gtk_adjustment_configure(argument->adjust, 0, 0, argument->duration/GST_SECOND, 
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
			time_spin_output_cb(GTK_SPIN_BUTTON(argument->time_spin), argument);
		}
	}

	if(argument->state < GST_STATE_PAUSED) {
		return TRUE;
	}
	
	if(gst_element_query_position(argument->playbin, 
		fmt, &(argument->current))) {
		g_signal_handler_block(argument->progressbar, argument->progressbar_update_signal_id);
		gtk_adjustment_configure(argument->adjust, argument->current/GST_SECOND, 0, argument->duration/GST_SECOND, 
			1, 1, 1);
		g_signal_handler_unblock(argument->progressbar, argument->progressbar_update_signal_id);
		return TRUE;
	}
	else
		return FALSE;
}

void add_entry_album_tab(GtkWidget *treeview, GtkTreeStore *treestore, GtkTreeModel *model_library, GtkTreeIter *iter) {
	GtkTreeIter toplevel, child, lowlevel, tempiter_album, tempiter, tempiter2, tempiter_library, iter_album, iter_track;
	tempiter_library = *iter;
	gboolean same_albums = FALSE;
	gboolean valid;
	gchar *tempalbum1, *tempalbum2;
	gchar *temptrack;
	gchar *temptracknumber1, *temptracknumber2;

	GtkTreeModel *model_album = gtk_tree_view_get_model(GTK_TREE_VIEW(treeview));
	tempalbum1 = tempalbum2 = temptrack = temptracknumber1 = temptracknumber2 = NULL;

	gtk_tree_model_get(model_library, &tempiter_library, ALBUM, &tempalbum1,
	TRACK, &temptrack,
	TRACKNUMBER, &temptracknumber1,
	-1);
	gchar *song = g_strconcat(temptracknumber1, "  ", temptrack, NULL);	

	if(gtk_tree_model_get_iter_first(model_album, &iter_album) == TRUE) {
		do {
			gtk_tree_model_get(model_album, &iter_album, COLUMN_ALBUM, &tempalbum2, -1);
			if(!g_strcmp0(tempalbum1, tempalbum2)) {
				same_albums = TRUE;
				break;
			}
			g_free(tempalbum2);
			tempalbum2 = NULL;
		} while(gtk_tree_model_iter_next(model_album, &iter_album));
		if(same_albums == FALSE) {
			gtk_tree_store_append(treestore, &toplevel, NULL);
  			gtk_tree_store_set(treestore, &toplevel,
            COLUMN_ARTIST, tempalbum1, -1);
			
			gtk_tree_store_append(treestore, &child, &toplevel);
			gtk_tree_store_set(treestore, &child, COLUMN_ARTIST, song, -1);
		}
		else {
			if(gtk_tree_model_iter_children(model_album, &iter_track, &iter_album) == TRUE) {
				gtk_tree_store_append(treestore, &iter_track, &iter_album);
				gtk_tree_store_set(treestore, &iter_track, COLUMN_ARTIST, song, -1);
			}
		}
	}
	else {
		gtk_tree_store_append(treestore, &toplevel, NULL);
  		gtk_tree_store_set(treestore, &toplevel,
        COLUMN_ARTIST, tempalbum1, -1);
			
		gtk_tree_store_append(treestore, &child, &toplevel);
		gtk_tree_store_set(treestore, &child, COLUMN_ARTIST, song, -1);
	}

	g_free(temptrack);
	g_free(temptracknumber1);
	g_free(temptracknumber2);
	g_free(song);
	temptrack = temptracknumber1 = temptracknumber2 = song = NULL;
}

void add_entry_artist_tab(GtkWidget *treeview, GtkTreeStore *treestore, GtkTreeModel *model_library, GtkTreeIter *iter) {
	GtkTreeIter toplevel, child, lowlevel, tempiter_library, iter_artist, iter_album, iter_track;
	tempiter_library = *iter;
	gboolean same_artists = FALSE;
  	gboolean same_albums = FALSE;
	gboolean valid;
	gchar *tempartist1, *tempartist2;
	gchar *tempalbum1, *tempalbum2;
	gchar *temptrack;
	gchar *temptracknumber1, *temptracknumber2;
	GtkTreeModel *model_artist = gtk_tree_view_get_model(GTK_TREE_VIEW(treeview));
	tempartist1 = tempartist2 = tempalbum1 = tempalbum2 = temptrack = temptracknumber1 = temptracknumber2 = NULL;

	gtk_tree_model_get(model_library, &tempiter_library, ARTIST, &tempartist1, 
	ALBUM, &tempalbum1, 
	TRACK, &temptrack, 
	TRACKNUMBER, &temptracknumber1, -1);
	gchar *song = g_strconcat(temptracknumber1, "  ", temptrack, NULL);

	if(gtk_tree_model_get_iter_first(model_artist, &iter_artist) == TRUE) {
		do {
			gtk_tree_model_get(model_artist, &iter_artist, COLUMN_ARTIST, &tempartist2, -1);
			if(!g_strcmp0(tempartist1, tempartist2)) {
				same_artists = TRUE;
				break;
			}
			g_free(tempartist2);
			tempartist2 = NULL;
		} while(gtk_tree_model_iter_next(model_artist, &iter_artist));
		if(same_artists == FALSE) {
			gtk_tree_store_append(treestore, &toplevel, NULL);
  			gtk_tree_store_set(treestore, &toplevel,
			COLUMN_ARTIST, tempartist1, -1);
		
			gtk_tree_store_append(treestore, &child, &toplevel);
			gtk_tree_store_set(treestore, &child,
			COLUMN_ARTIST, tempalbum1, -1);

			gtk_tree_store_append(treestore, &lowlevel, &child);
			gtk_tree_store_set(treestore, &lowlevel, COLUMN_ARTIST, song, -1);
		}
		else {
			if(gtk_tree_model_iter_children(model_artist, &iter_album, &iter_artist) == TRUE) {
				do {
					gtk_tree_model_get(model_artist, &iter_album, COLUMN_ARTIST, &tempalbum2, -1);
					if(!g_strcmp0(tempalbum1, tempalbum2)) {
						same_albums = TRUE;
						break;
					}
					g_free(tempalbum2);
					tempalbum2 = NULL;
				} while(valid = gtk_tree_model_iter_next(model_artist, &iter_album));

				if(same_albums == FALSE) {
					gtk_tree_store_append(treestore, &child, &iter_artist);
					gtk_tree_store_set(treestore, &child,
   	   	 			COLUMN_ARTIST, tempalbum1, -1);

					gtk_tree_store_append(treestore, &lowlevel, &child);
					gtk_tree_store_set(treestore, &lowlevel, COLUMN_ARTIST, song, -1);
				}
				else {
					if(gtk_tree_model_iter_children(model_artist, &iter_track, &iter_album)) {
						gtk_tree_store_append(treestore, &iter_track, &iter_album);
						gtk_tree_store_set(treestore, &iter_track, COLUMN_ARTIST, song, -1);
					}
				}
			}
			else {
				gtk_tree_store_append(treestore, &child, &toplevel);
				gtk_tree_store_set(treestore, &child,
   	   	 		COLUMN_ARTIST, tempalbum1, -1);

				gtk_tree_store_append(treestore, &lowlevel, &child);
				gtk_tree_store_set(treestore, &lowlevel, COLUMN_ARTIST, song, -1);
			}	
		}
	}
	else {
		gtk_tree_store_append(treestore, &toplevel, NULL);
  		gtk_tree_store_set(treestore, &toplevel,
		COLUMN_ARTIST, tempartist1, -1);
		
		gtk_tree_store_append(treestore, &child, &toplevel);
		gtk_tree_store_set(treestore, &child,
		COLUMN_ARTIST, tempalbum1, -1);

		gtk_tree_store_append(treestore, &lowlevel, &child);
		gtk_tree_store_set(treestore, &lowlevel, COLUMN_ARTIST, song, -1);
	}

	temptrack = temptracknumber1 = temptracknumber2 = song = NULL;
	g_free(tempartist1);
	g_free(tempartist2);
	tempartist1 = tempartist2 = NULL;
	g_free(tempalbum1);
	g_free(tempalbum2);
	tempalbum2 = tempalbum1 = NULL;
	g_free(temptrack);
	g_free(temptracknumber1);
	g_free(temptracknumber2);
	g_free(song);
}

void display_artist_tab(GtkWidget *treeview, GtkTreeStore *treestore, GtkTreeModel *model_library) {
	gtk_tree_store_clear(treestore);
	GtkTreeIter tempiter_artist;

	if(gtk_tree_model_get_iter_first(model_library, &tempiter_artist)) {
		do
			add_entry_artist_tab(treeview, treestore, model_library, &tempiter_artist);
		while(gtk_tree_model_iter_next(model_library, &tempiter_artist));
	}
}

void display_album_tab(GtkWidget *treeview, GtkTreeStore *treestore, GtkTreeModel *model_library) {
	gtk_tree_store_clear(treestore);

	GtkTreeIter tempiter_album;

	if(gtk_tree_model_get_iter_first(model_library, &tempiter_album)) {
		do 
			add_entry_album_tab(treeview, treestore, model_library, &tempiter_album);
		while(gtk_tree_model_iter_next(model_library, &tempiter_album));
	}
}

void setup_tree_view_renderer_artist(GtkWidget *treeview, GtkTreeStore *treestore, GtkTreeModel *model_library) {
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;

	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes("Artist", renderer, "text", COLUMN_ARTIST, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
	gtk_tree_view_column_set_sort_column_id(column, COLUMN_ARTIST);
	g_object_set(renderer, "size", 13*PANGO_SCALE, NULL);
}

void setup_tree_view_renderer_album(GtkWidget *treeview, GtkTreeStore *treestore, GtkTreeModel *model_library) {
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;

	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes("Album", renderer, "text", COLUMN_ARTIST, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
	gtk_tree_view_column_set_sort_column_id(column, COLUMN_ALBUM);
	g_object_set(renderer, "size", 13*PANGO_SCALE, NULL);
}

void playlist_popup_menu(GtkWidget *treeview, GdkEventButton *event, struct arguments *argument) {
	GtkWidget *menu, *item_remove_from_playlist;

    menu = gtk_menu_new();

    item_remove_from_playlist = gtk_menu_item_new_with_label("Remove this track(s) from playlist");

    g_signal_connect(item_remove_from_playlist, "activate", (GCallback)remove_playlist_selection_from_playlist_cb, argument);

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

    g_signal_connect(item_add_to_playlist, "activate", (GCallback)add_library_selection_to_playlist_cb, argument);

    gtk_menu_shell_append(GTK_MENU_SHELL(menu), item_add_to_playlist);

    gtk_widget_show_all(menu);
    gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, NULL,
                   (event != NULL) ? event->button : 0,
                   gdk_event_get_time((GdkEvent*)event));
}

void album_popup_menu(GtkWidget *treeview, GdkEventButton *event, struct arguments *argument) {
	GtkWidget *menu, *item_add_to_playlist;

    menu = gtk_menu_new();

    item_add_to_playlist = gtk_menu_item_new_with_label("Add track(s) to playlist");

    g_signal_connect(item_add_to_playlist, "activate", (GCallback)add_album_selection_to_playlist_cb, argument);

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

    g_signal_connect(item_add_to_playlist, "activate", (GCallback)add_artist_selection_to_playlist_cb, argument);

    gtk_menu_shell_append(GTK_MENU_SHELL(menu), item_add_to_playlist);

    gtk_widget_show_all(menu);
    gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, NULL,
                   (event != NULL) ? event->button : 0,
                   gdk_event_get_time((GdkEvent*)event));
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
	xmlDocPtr library;
	xmlNodePtr cur;
	xmlNodePtr child;
	xmlChar *tempfile = NULL;
	xmlChar *temptrack = NULL;
	xmlChar *tempalbum = NULL;
	xmlChar *tempartist = NULL;
	xmlChar *temptracknumber = NULL;
	xmlChar *tempforce = NULL;
	gchar forceresult[20];
	float tempforcef;
	xmlChar *tempforce_env = NULL;
	float tempforce_envf;
	xmlChar *tempforce_amp = NULL;
	float tempforce_ampf;
	xmlChar *tempforce_freq = NULL;
	float tempforce_freqf;
	xmlChar *tempforce_atk = NULL;
	float tempforce_atkf;

	gtk_list_store_clear(store);

	library = xmlParseFile(libfile);

	if(library != NULL) {
		cur = xmlDocGetRootElement(library);
	
		if(cur == NULL) {
			printf("Library file is empty\n");
			xmlFreeDoc(library);
			return;
		}
		
		if(xmlStrcmp(cur->name, (const xmlChar *)"lelelelibrary")) {
			printf("Library file of the wrong type, root node != lelelelibrary");
			xmlFreeDoc(library);
			return;
		}
		for(cur = cur->children; cur != NULL; cur = cur->next) {
			if((!xmlStrcmp(cur->name, (const xmlChar *)"song")) && cur->children != NULL ) {
				for(child = cur->children; child != NULL; child = child->next) {
					if(child->type != XML_ELEMENT_NODE)
						continue;
					if((!xmlStrcmp(child->name, (const xmlChar *)"title"))) {
						temptrack = xmlNodeGetContent(child);
					}
					else if((!xmlStrcmp(child->name, (const xmlChar *)"artist")))
						tempartist = xmlNodeGetContent(child);
					else if((!xmlStrcmp(child->name, (const xmlChar *)"album")))
						tempalbum = xmlNodeGetContent(child);
					else if((!xmlStrcmp(child->name, (const xmlChar *)"tracknumber")))
						temptracknumber = xmlNodeGetContent(child);
					else if((!xmlStrcmp(child->name, (const xmlChar *)"filename"))) 
						tempfile = xmlNodeGetContent(child);
					else if((!xmlStrcmp(child->name, (const xmlChar *)"analyze-resnum"))) {
						tempforce = xmlNodeGetContent(child);
						tempforcef = atof(tempforce);
						if(tempforcef == 0)
							strcpy(forceresult, "Loud");
						else if(tempforcef == 1) 
							strcpy(forceresult, "Calm");
						else
							strcpy(forceresult, "Can't conclude");
					}
					else if((!xmlStrcmp(child->name, (const xmlChar *)"analyze-amplitude"))) {
						tempforce_amp = xmlNodeGetContent(child);
						tempforce_ampf = atof(tempforce_amp);
					}
					else if((!xmlStrcmp(child->name, (const xmlChar *)"analyze-freq"))) {
						tempforce_freq = xmlNodeGetContent(child);
						tempforce_freqf = atof(tempforce_freq);
					}
					else if((!xmlStrcmp(child->name, (const xmlChar *)"analyze-tempo"))) {
						tempforce_env = xmlNodeGetContent(child);
						tempforce_envf = atof(tempforce_env);
					}
					else if((!xmlStrcmp(child->name, (const xmlChar *)"analyze-atk"))) {
						tempforce_atk = xmlNodeGetContent(child);
						tempforce_atkf = atof(tempforce_atk);
					}
				}

				gtk_list_store_append(store, &iter);
				gtk_list_store_set(store, &iter, PLAYING, "", TRACKNUMBER, temptracknumber, TRACK, temptrack, ALBUM, tempalbum, ARTIST, tempartist, FORCE, tempforcef, FORCE_TEMPO, tempforce_envf, FORCE_AMP, tempforce_ampf, FORCE_FREQ, tempforce_freqf, FORCE_ATK, tempforce_atkf, TEXTFORCE, forceresult, AFILE, tempfile, -1);
				xmlFree(temptracknumber);
				xmlFree(temptrack);
				xmlFree(tempartist);
				xmlFree(tempalbum);
				xmlFree(tempfile);
				xmlFree(tempforce);
				xmlFree(tempforce_amp);
				xmlFree(tempforce_freq);
				xmlFree(tempforce_env);
				xmlFree(tempforce_atk);
				temptracknumber = temptrack = tempartist = tempalbum =  tempforce = tempforce_amp = tempfile = tempforce_freq = tempforce_env = tempforce_atk = NULL;
			}
		}
		xmlFreeDoc(library);
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
	GtkTreeSortable *sortable_library, *sortable_artist, *sortable_album;
	const gchar *volume[5] = {
		"audio-volume-muted-symbolic",
		"audio-volume-high-symbolic",
		"audio-volume-low-symbolic",
		"audio-volume-medium-symbolic",
		NULL
	};

	GstElement *gtk_sink;
	GstBus *bus;
	
/*	#ifdef linux
		XInitThreads();
	#endif */

	pargument->lelelerandom = 0;
	pargument->timer_delay = 0;
	pargument->random = 0;
	pargument->repeat = 0;
	pargument->first = 1;
	pargument->playlist_count = 0;
	pargument->iter_library.stamp = 0;
	pargument->iter_playlist.stamp = 0;
	pargument->duration = GST_CLOCK_TIME_NONE;
	pargument->state = GST_STATE_NULL;
	pargument->current_song.artist = pargument->current_song.title = pargument->current_song.album = pargument->current_song.tracknumber = NULL;
	pargument->str_genre = pargument->str_bitrate = pargument->str_samplerate = pargument->str_channels = NULL;
	pargument->history = NULL;
	pargument->current_song.sample_array = NULL;
	pargument->bartag = 0;
	pargument->sleep_timer = NULL;
	g_mutex_init(&pargument->queue_mutex);
	g_cond_init(&pargument->queue_cond);

	gtk_init(&argc, &argv);	
	gst_init(&argc, &argv);
	
	char lib_file[] = "library.xml";
	gchar *libdir;
	gchar *libfile;

	libdir = g_build_filename(g_get_home_dir(), ".local", "share", "leleleplayer", NULL);
	libfile = g_build_filename(libdir, "library.xml", NULL);

	if(g_mkdir_with_parents(libdir, 0755) != -1)
		pargument->lib_path = libfile;
	else
		pargument->lib_path = lib_file;	

	g_free(libdir);

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(window), "lelele player");
	gtk_widget_set_size_request(window, 900, 700);

	pargument->playbin = gst_element_factory_make("playbin", "playbin");
	if(!pargument->playbin)
		g_error("Not all elements could be created.\n");
	bus = gst_element_get_bus(pargument->playbin);
	gst_bus_add_signal_watch(bus);

	/*if((gtk_sink = gst_element_factory_make("gtkglsink", NULL))) {
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
	g_object_set(pargument->current_song.playbin, "force-aspect-ratio", FALSE, NULL); */


	library_panel = gtk_scrolled_window_new(NULL, NULL);
	playlist_panel = gtk_scrolled_window_new(NULL, NULL);
	artist_panel = gtk_scrolled_window_new(NULL, NULL);
	album_panel = gtk_scrolled_window_new(NULL, NULL);

	pargument->store_library = gtk_list_store_new(COLUMNS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_FLOAT, G_TYPE_FLOAT, G_TYPE_FLOAT, G_TYPE_FLOAT, G_TYPE_FLOAT, G_TYPE_STRING, G_TYPE_STRING);
	sortable_library = GTK_TREE_SORTABLE(pargument->store_library);
	treeview_library = gtk_tree_view_new_with_model(GTK_TREE_MODEL(pargument->store_library));
	gtk_tree_sortable_set_sort_func(sortable_library, TRACKNUMBER, sort_iter_compare_func, NULL, NULL); 
	gtk_tree_sortable_set_sort_func(sortable_library, TEXTFORCE, sort_force, NULL, NULL);
	gtk_tree_sortable_set_sort_func(sortable_library, ARTIST, sort_artist_album_tracks, NULL, NULL);
	gtk_tree_sortable_set_sort_func(sortable_library, ALBUM, sort_album_tracks, NULL, NULL);
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
	sortable_artist = GTK_TREE_SORTABLE(pargument->store_artist);
	gtk_tree_sortable_set_sort_column_id(sortable_library, ARTIST, GTK_SORT_ASCENDING);
	gtk_tree_sortable_set_sort_func(sortable_artist, COLUMN_ARTIST, sort_text, NULL, NULL); 
	gtk_tree_sortable_set_sort_column_id(sortable_artist, COLUMN_ARTIST, GTK_SORT_ASCENDING);
	setup_tree_view_renderer_artist(treeview_artist, pargument->store_artist, model_library);
	pargument->treeview_artist = treeview_artist;
	display_artist_tab(treeview_artist, pargument->store_artist, model_library);
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview_artist));
	gtk_tree_selection_set_mode(selection, GTK_SELECTION_MULTIPLE);

	pargument->store_album = gtk_tree_store_new(NUM_COLS_ALBUM, G_TYPE_STRING);
	treeview_album = gtk_tree_view_new_with_model(GTK_TREE_MODEL(pargument->store_album));
	sortable_album = GTK_TREE_SORTABLE(pargument->store_album);
	gtk_tree_sortable_set_sort_func(sortable_album, COLUMN_ALBUM, sort_text, NULL, NULL); 
	gtk_tree_sortable_set_sort_column_id(sortable_album, COLUMN_ALBUM, GTK_SORT_ASCENDING);
	gtk_tree_sortable_set_sort_column_id(sortable_library, ALBUM, GTK_SORT_ASCENDING);
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
	if(g_file_test("../images/lelele.png", G_FILE_TEST_EXISTS)) 
		gtk_window_set_icon_from_file(GTK_WINDOW(window), "../images/lelele.png", NULL);
	else
		gtk_window_set_icon_from_file(GTK_WINDOW(window), "/usr/share/leleleplayer/icons/lelele.png", NULL);
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
	pargument->preferences = pref_arguments.preferences;
	library_set = g_settings_get_boolean(pref_arguments.preferences, "library-set");
	pref_arguments.lelele_scan = g_settings_get_boolean(pref_arguments.preferences, "lelele-scan");
	pref_arguments.lib_path = pargument->lib_path;
	pargument->vol = g_settings_get_double(pref_arguments.preferences, "volume");
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
	pref_arguments.progress_label = gtk_label_new(NULL);

	/* Signal management */
	g_signal_connect(G_OBJECT(bus), "message::state-changed", G_CALLBACK(state_changed_cb), pargument);
	g_signal_connect(G_OBJECT(pargument->playbin), "about-to-finish", G_CALLBACK(continue_track_cb), pargument);
	g_signal_connect(G_OBJECT(bus), "message::stream-start", G_CALLBACK(refresh_ui_cb), pargument);
	g_signal_connect(G_OBJECT(bus), "message::eos", G_CALLBACK(end_of_playlist_cb), pargument);
	g_signal_connect(G_OBJECT(bus), "message::application", G_CALLBACK(message_application_cb), pargument);
	g_signal_connect(G_OBJECT(pargument->playbin), "audio-tags-changed", G_CALLBACK(tags_obtained_cb), pargument);
	g_signal_connect(G_OBJECT(pargument->playpause_button), "clicked", G_CALLBACK(toggle_playpause_button_cb), pargument);
	g_signal_connect(G_OBJECT(pargument->volume_scale), "value-changed", G_CALLBACK(volume_scale_changed_cb), pargument);
	g_signal_connect(G_OBJECT(random_button), "clicked", G_CALLBACK(toggle_random_cb), pargument);
	g_signal_connect(G_OBJECT(repeat_button), "clicked", G_CALLBACK(toggle_repeat_cb), pargument);
	g_signal_connect(G_OBJECT(lelele_button), "clicked", G_CALLBACK(toggle_lelele_cb), pargument);
	g_signal_connect(G_OBJECT(next_button), "clicked", G_CALLBACK(next_buttonf_cb), pargument);
	g_signal_connect(G_OBJECT(previous_button), "clicked", G_CALLBACK(previous_buttonf_cb), pargument);
	g_signal_connect(G_OBJECT(preferences), "activate", G_CALLBACK(preferences_callback_cb), &pref_arguments);
	g_signal_connect(G_OBJECT(open), "activate", G_CALLBACK(open_audio_file_cb), pargument);
	g_signal_connect(G_OBJECT(add_file), "activate", G_CALLBACK(add_file_to_playlist_cb), pargument);
	g_signal_connect(G_OBJECT(close), "activate", G_CALLBACK(destroy_cb), pargument);
	g_signal_connect(G_OBJECT(treeview_library), "row-activated", G_CALLBACK(lib_row_activated_cb), pargument);
	g_signal_connect(G_OBJECT(treeview_library), "button-press-event", G_CALLBACK(treeviews_right_click_cb), pargument);
	g_signal_connect(G_OBJECT(treeview_artist), "button-press-event", G_CALLBACK(treeviews_right_click_cb), pargument);
	g_signal_connect(G_OBJECT(treeview_album), "button-press-event", G_CALLBACK(treeviews_right_click_cb), pargument);
	g_signal_connect(G_OBJECT(treeview_playlist), "button-press-event", G_CALLBACK(treeviews_right_click_cb), pargument);
	g_signal_connect(G_OBJECT(treeview_playlist), "key-press-event", G_CALLBACK(playlist_del_button_cb), pargument);
	g_signal_connect(G_OBJECT(treeview_playlist), "row-activated", G_CALLBACK(playlist_row_activated_cb), pargument);
	g_signal_connect(G_OBJECT(treeview_artist), "row-activated", G_CALLBACK(artist_row_activated_cb), pargument);
	g_signal_connect(G_OBJECT(treeview_album), "row-activated", G_CALLBACK(album_row_activated_cb), pargument);
	g_signal_connect(G_OBJECT(time_checkbox), "toggled", G_CALLBACK(time_checkbox_toggled_cb), pargument);
	g_signal_connect(G_OBJECT(pargument->time_spin), "input", G_CALLBACK(time_spin_input_cb), pargument);
	g_signal_connect(G_OBJECT(pargument->time_spin), "output", G_CALLBACK(time_spin_output_cb), pargument);
	pargument->time_spin_update_signal_id = g_signal_connect(G_OBJECT(pargument->time_spin), "value-changed", G_CALLBACK(time_spin_changed_cb), pargument);
	pargument->playlist_update_signal_id = g_signal_connect(G_OBJECT(model_playlist), "row-inserted", G_CALLBACK(ui_playlist_changed_cb), pargument->libnotebook);
	g_signal_connect(G_OBJECT(pargument->libnotebook), "switch-page", G_CALLBACK(changed_page_notebook_cb), NULL);
	pargument->progressbar_update_signal_id = g_signal_connect(G_OBJECT(pargument->progressbar), 
		"value-changed", G_CALLBACK(slider_changed_cb), pargument);
	g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(destroy_cb), pargument);	

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
		gtk_box_pack_end(GTK_BOX(time_box), pref_arguments.progress_label, FALSE, FALSE, 1);
		gtk_box_pack_end(GTK_BOX(time_box), pargument->time_spin, TRUE, TRUE, 5);
		gtk_box_pack_end(GTK_BOX(time_box), time_checkbox, FALSE, FALSE, 5);

	gtk_scale_button_set_value(GTK_SCALE_BUTTON(pargument->volume_scale), pargument->vol);
	gtk_container_add(GTK_CONTAINER(window), vboxv);
	gtk_widget_show_all(window);

	if(library_set == FALSE) {
		preferences_callback_cb(GTK_MENU_ITEM(preferences), &pref_arguments);
	}

	gtk_main();

	gst_object_unref(bus);
	gst_object_unref(pargument->playbin);

	return 0;
}
