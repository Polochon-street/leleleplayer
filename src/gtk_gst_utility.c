#include "gui.h"

gboolean filter_vis_features(GstPluginFeature *feature, gpointer data) {
	GstElementFactory *factory;
   
	if(!GST_IS_ELEMENT_FACTORY(feature))
    	return FALSE;
	factory = GST_ELEMENT_FACTORY(feature);
	if (!g_strrstr(gst_element_factory_get_klass(factory), "Visualization"))
    	return FALSE;

	return TRUE;
}

float distance(struct d4vector v1, struct d4vector v2) {
	float distance;
	distance = sqrt((v1.x - v2.x)*(v1.x - v2.x) + (v1.y - v2.y)*(v1.y - v2.y) +
		(v1.z - v2.z)*(v1.z - v2.z));
	return distance;
}

void explore(GDir *dir, const gchar *folder, FILE *list) {
	const gchar *file;
	gchar *temppath;

	while((dir != NULL) && (file = g_dir_read_name(dir))) {
		temppath = g_build_path("/", folder, file, NULL);
		if( g_file_test(temppath, G_FILE_TEST_IS_REGULAR) && ( g_str_has_suffix(file, "flac") || g_str_has_suffix(file, "mp3") || g_str_has_suffix(file, "ogg") ) ) {
			g_fprintf(list, "%s\n", g_build_path("/", folder, file, NULL));
		}
		else if(g_file_test(temppath, G_FILE_TEST_IS_DIR))
			explore(g_dir_open(g_build_path("/", folder, file, NULL), 0, NULL), g_build_path("/", folder, file, NULL), list);
		g_free(temppath);
	}
	if (file == NULL) {
		g_dir_close(dir);
	}
}

gboolean add_artist_to_playlist(gchar *artist, struct arguments *argument) {
	gboolean valid;
	GtkTreeModel *model_library;
	GtkTreeModel *model_playlist;
	GtkTreeIter lib_iter;
	gchar *tempartist;

	model_playlist = gtk_tree_view_get_model(GTK_TREE_VIEW(argument->treeview_playlist));
	model_library = gtk_tree_view_get_model(GTK_TREE_VIEW(argument->treeview_library));

	valid = gtk_tree_model_get_iter_first(model_library, &lib_iter);
	while(valid) {
		gtk_tree_model_get(model_library, &lib_iter, ARTIST, &tempartist, -1);
			
   		if((!strcmp(tempartist, artist)))
			playlist_queue(&lib_iter, model_library, GTK_TREE_VIEW(argument->treeview_playlist), argument);
		valid = gtk_tree_model_iter_next(model_library,
		&lib_iter);
	}
}

gboolean add_album_to_playlist(gchar *album, gchar *artist, struct arguments *argument) {
	gboolean valid;
	GtkTreeModel *model_library;
	GtkTreeModel *model_playlist;
	GtkTreeIter lib_iter;
	gchar *tempalbum;
	gchar *tempartist;

	model_playlist = gtk_tree_view_get_model(GTK_TREE_VIEW(argument->treeview_playlist));
	model_library = gtk_tree_view_get_model(GTK_TREE_VIEW(argument->treeview_library));

	valid = gtk_tree_model_get_iter_first(model_library, &lib_iter);
	while(valid) {
		gtk_tree_model_get(model_library, &lib_iter, ALBUM, &tempalbum, ARTIST, &tempartist, -1);
		
   		if((!strcmp(tempalbum, album))
		&& (!strcmp(tempartist, artist))) {
			playlist_queue(&lib_iter, model_library, GTK_TREE_VIEW(argument->treeview_playlist), argument);
		}
		valid = gtk_tree_model_iter_next (model_library,
		&lib_iter);
	}
}

gboolean play_playlist_song(gchar *title, struct arguments *argument) {
	gboolean valid;
	GtkTreePath *path;
	GtkTreeModel *model_playlist;
	gchar *temptitle;
	
	model_playlist = gtk_tree_view_get_model(GTK_TREE_VIEW(argument->treeview_playlist));
	
	gint rows = gtk_tree_model_iter_n_children(GTK_TREE_MODEL(model_playlist), NULL);
	path = gtk_tree_path_new_from_indices(rows - 1, -1);
	valid = gtk_tree_model_get_iter(GTK_TREE_MODEL(model_playlist), &(argument->iter_playlist), path);

	while(valid) {
		gtk_tree_model_get(model_playlist, &(argument->iter_playlist), TRACK, &temptitle, -1);

   		if((!strcmp(temptitle, title)))
			break;
		valid = gtk_tree_model_iter_previous(model_playlist,
		&argument->iter_playlist);
	}

	start_song(argument);
}

void playlist_queue(GtkTreeIter *iter_to_queue, GtkTreeModel *model_library, GtkTreeView *treeview_playlist, struct arguments *argument) {
	GtkTreeIter iter_playlist;
	gchar *tempfile;
	gfloat tempfloat;
	size_t i;

	gtk_list_store_append(argument->store_playlist, &iter_playlist);
	for(i = 0; i < COLUMNS; ++i) {
		if(i == FORCE) {
			gtk_tree_model_get(model_library, iter_to_queue, i, &tempfloat, -1);
			gtk_list_store_set(argument->store_playlist, &iter_playlist, i, tempfloat,-1);
		}
		else if(i == FORCE_TEMPO) {
			gtk_tree_model_get(model_library, iter_to_queue, i, &tempfloat, -1);
			gtk_list_store_set(argument->store_playlist, &iter_playlist, i, tempfloat,-1);
		}
		else if(i == FORCE_AMP) {
			gtk_tree_model_get(model_library, iter_to_queue, i, &tempfloat, -1);
			gtk_list_store_set(argument->store_playlist, &iter_playlist, i, tempfloat,-1);
		}
		else if(i == FORCE_FREQ) {
			gtk_tree_model_get(model_library, iter_to_queue, i, &tempfloat, -1);
			gtk_list_store_set(argument->store_playlist, &iter_playlist, i, tempfloat,-1);
		}
		else if(i == FORCE_ATK) {
			gtk_tree_model_get(model_library, iter_to_queue, i, &tempfloat, -1);
			gtk_list_store_set(argument->store_playlist, &iter_playlist, i, tempfloat,-1);
		}
		else {
			gtk_tree_model_get(model_library, iter_to_queue, i, &tempfile, -1);
			gtk_list_store_set(argument->store_playlist, &iter_playlist, i, tempfile, -1);
			g_free(tempfile);
		}
	}
	argument->playlist_count++;
}

gboolean get_next_playlist_song(GtkTreeView *treeview_playlist, struct arguments *argument) {
	GtkTreeModel *model_playlist;
	GtkTreeIter tempiter;
	model_playlist = gtk_tree_view_get_model(treeview_playlist);

	if(argument->lelelerandom) {
		return get_lelelerandom_playlist_song(GTK_TREE_VIEW(argument->treeview_playlist), argument);
	}
	else if(argument->random) {
		return get_random_playlist_song(GTK_TREE_VIEW(argument->treeview_playlist), argument);
	}
	else {
		tempiter = argument->iter_playlist;
		if(gtk_tree_model_iter_next(model_playlist, &tempiter)) {
			argument->iter_playlist = tempiter;
			return TRUE;
		}
		else {
			return FALSE;
		}
	}
}

gboolean get_random_playlist_song(GtkTreeView *treeview_playlist, struct arguments *argument) {
	GtkTreeModel *model_playlist;
	GList *temphistory;
	gchar *songstring;
	gchar *tempstring;
	int i, rand;
	gboolean found = true;

	model_playlist = gtk_tree_view_get_model(treeview_playlist);

	do {
		rand = g_random_int_range(0, argument->playlist_count);
		if(gtk_tree_model_get_iter_first(model_playlist, &(argument->iter_playlist))) {
			for(i = 0; i < rand - 1; ++i)
				gtk_tree_model_iter_next(model_playlist, &(argument->iter_playlist));
			songstring = gtk_tree_model_get_string_from_iter(model_playlist, &(argument->iter_playlist));
			found = false;

			for(temphistory = argument->history; temphistory != NULL; temphistory = temphistory->next) {
				tempstring = (gchar*)temphistory->data;
				if(!g_strcmp0(tempstring, songstring)) {
					found = true;
					break;
				}
			}
			g_free(songstring);
		}
		else
			return FALSE;
	} while(found == true);

	return TRUE;
}

gboolean get_lelelerandom_playlist_song(GtkTreeView *treeview_playlist, struct arguments *argument) {
	GtkTreeModel *model_playlist;

	model_playlist = gtk_tree_view_get_model(GTK_TREE_VIEW(argument->treeview_playlist));
	struct d4vector current_force = argument->current_song.force_vector;
	float treshold = 0.45;
	int i = 0;
	do {
		treshold += 0.001;
		if(!get_random_playlist_song(treeview_playlist, argument))
			return FALSE;
		gtk_tree_model_get(model_playlist, &(argument->iter_playlist), FORCE_TEMPO, &argument->current_song.force_vector.x, 
		FORCE_AMP, &argument->current_song.force_vector.y, FORCE_FREQ, &argument->current_song.force_vector.z, 
		FORCE_ATK, &argument->current_song.force_vector.t, -1);
	} while(distance(current_force, argument->current_song.force_vector) >= treshold);
	return TRUE;
}

gboolean get_previous_playlist_song(GtkTreeView *treeview_playlist, struct arguments *argument) {
	GtkTreeModel *model_playlist;
	
	model_playlist = gtk_tree_view_get_model(treeview_playlist);

	if(argument->history && argument->history->next 
	&& gtk_tree_model_get_iter_from_string(model_playlist, &argument->iter_playlist, (gchar*)argument->history->data)) {
		argument->history = g_list_remove(argument->history, argument->history->data);
		return TRUE;
	}
	else
		return FALSE;
}

void clean_playlist(GtkTreeView *treeview_playlist, struct arguments *argument) {
	gtk_list_store_clear(argument->store_playlist);
	argument->playlist_count = 0;	
}

void continue_track(GstElement *playbin, struct arguments *argument) {
	GstStructure *structure;	
	gchar *uri;	

	structure = gst_structure_new_empty("next_song");
	g_mutex_lock(&argument->queue_mutex);
	GstMessage *msg = gst_message_new_application(GST_OBJECT(playbin), structure);

	gst_element_post_message(argument->current_song.playbin, msg);
	g_cond_wait(&argument->queue_cond, &argument->queue_mutex);
	g_mutex_unlock(&argument->queue_mutex);

	uri = g_filename_to_uri(argument->current_song.filename, NULL, NULL);
	g_object_set(argument->current_song.playbin, "uri", uri, NULL);
	g_free(uri); 

	// TODO: Wait until message_application ends!
}

void queue_song(struct arguments *argument) {
	char *uri;
	GtkTreeModel *model_playlist;

	model_playlist = gtk_tree_view_get_model(GTK_TREE_VIEW(argument->treeview_playlist));
	gtk_tree_model_get(model_playlist, &(argument->iter_playlist), AFILE, &argument->current_song.filename, 
	TRACKNUMBER, &argument->current_song.tracknumber, TRACK, &argument->current_song.title, 
	ALBUM, &argument->current_song.album, ARTIST, &argument->current_song.artist, 
	FORCE, &argument->current_song.force, FORCE_TEMPO, &argument->current_song.force_vector.x, 
	FORCE_AMP, &argument->current_song.force_vector.y, FORCE_FREQ, &argument->current_song.force_vector.z, 
	FORCE_ATK, &argument->current_song.force_vector.t, -1);

	uri = g_filename_to_uri(argument->current_song.filename, NULL, NULL);
	g_object_set(argument->current_song.playbin, "uri", uri, NULL);

	g_free(uri); 
}

void start_song(struct arguments *argument) {
	char *uri;
	GtkTreeModel *model_playlist;
	
	model_playlist = gtk_tree_view_get_model(GTK_TREE_VIEW(argument->treeview_playlist));

	gtk_tree_model_get(model_playlist, &(argument->iter_playlist), AFILE, &argument->current_song.filename, 
	TRACKNUMBER, &argument->current_song.tracknumber, TRACK, &argument->current_song.title, 
	ALBUM, &argument->current_song.album, ARTIST, &argument->current_song.artist, 
	FORCE, &argument->current_song.force, FORCE_TEMPO, &argument->current_song.force_vector.x, 
	FORCE_AMP, &argument->current_song.force_vector.y, FORCE_FREQ, &argument->current_song.force_vector.z, 
	FORCE_ATK, &argument->current_song.force_vector.t, -1);

	gtk_widget_set_sensitive(argument->progressbar, TRUE);
	uri = g_filename_to_uri(argument->current_song.filename, NULL, NULL);
	
	gst_element_set_state(argument->current_song.playbin, GST_STATE_NULL);
	g_object_set(argument->current_song.playbin, "uri", uri, NULL);
	gst_element_set_state(argument->current_song.playbin, GST_STATE_PLAYING);
	if(argument->bartag)
			g_source_remove(argument->bartag);
	argument->bartag = g_timeout_add_seconds(1, refresh_progressbar, argument);

	g_free(uri);
}

void pause_song(struct arguments *argument) {
	gtk_button_set_image(GTK_BUTTON(argument->playpause_button), gtk_image_new_from_icon_name("media-playback-start-symbolic", GTK_ICON_SIZE_BUTTON));
	gtk_list_store_set(argument->store_playlist, &(argument->iter_playlist), PLAYING, "▍▍", -1);
	gst_element_set_state(argument->current_song.playbin, GST_STATE_PAUSED);
}

void resume_song(struct arguments *argument) {
	gtk_button_set_image(GTK_BUTTON(argument->playpause_button), gtk_image_new_from_icon_name("media-playback-pause-symbolic", GTK_ICON_SIZE_BUTTON));
	gtk_list_store_set(argument->store_playlist, &(argument->iter_playlist), PLAYING, "▶",-1);
	gst_element_set_state(argument->current_song.playbin, GST_STATE_PLAYING);
}

void state_changed(GstBus *bus, GstMessage *msg, struct arguments *argument) {
	GstState old_state, new_state, pending_state;

	if(GST_MESSAGE_SRC(msg) == GST_OBJECT(argument->current_song.playbin)) {
		gst_message_parse_state_changed(msg, &old_state, &new_state, &pending_state);
    	argument->current_song.state = new_state;
	}
}

gint sort_iter_compare_func(GtkTreeModel *model, GtkTreeIter *a, GtkTreeIter *b, gpointer userdata) {
	gchar *track1, *track2;
	gtk_tree_model_get(model, a, TRACKNUMBER, &track1, -1);
	gtk_tree_model_get(model, b, TRACKNUMBER, &track2, -1);

	if (atof(track1) > atof(track2)) {
		g_free(track1);
		g_free(track2);
		return 1;
	}
	else if(atof(track1) < atof(track2)) {
		g_free(track1);
		g_free(track2);
		return -1;
	}
	else {
		g_free(track1);
		g_free(track2);
		return 0;
	}
}

gint sort_artist_album_tracks(GtkTreeModel *model, GtkTreeIter *a, GtkTreeIter *b, gpointer userdata) {
	gchar *artist1, *artist2;
	gchar *album1, *album2;
	gchar *track1, *track2;
	int retval;

	gtk_tree_model_get(model, a, ARTIST, &artist1, -1);
	gtk_tree_model_get(model, b, ARTIST, &artist2, -1);
	gtk_tree_model_get(model, a, ALBUM, &album1, -1);
	gtk_tree_model_get(model, b, ALBUM, &album2, -1);
	gtk_tree_model_get(model, a, TRACKNUMBER, &track1, -1);
	gtk_tree_model_get(model, b, TRACKNUMBER, &track2, -1);


	if(strcmp(artist1, artist2) > 0) {
		retval = 1;
	}
	else if(strcmp(artist1, artist2) < 0)
		retval = -1;
	else {
		if(strcmp(album1, album2) > 0)
			retval = 1;
		else if(strcmp(album1, album2) < 0)
			retval = -1;
		else {
			if(atof(track1) > atof(track2))
				retval = 1;
			else if(atof(track1) < atof(track2))
				retval = -1;
			else
				retval = 0;
		}
	}
	g_free(artist1);
	g_free(artist2);
	g_free(album1);
	g_free(album2);
	g_free(track1);
	g_free(track2);
	
	return retval;
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
