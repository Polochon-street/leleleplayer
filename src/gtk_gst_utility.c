#define _GNU_SOURCE
#include "gui.h"
#include <string.h>

#ifdef WIN32
#include <shlwapi.h>
#define strcasestr StrStrI
#endif


gboolean filter_vis_features(GstPluginFeature *feature, gpointer data) {
	GstElementFactory *factory;
   
	if(!GST_IS_ELEMENT_FACTORY(feature))
    	return FALSE;
	factory = GST_ELEMENT_FACTORY(feature);
	if (!g_strrstr(gst_element_factory_get_klass(factory), "Visualization"))
    	return FALSE;

	return TRUE;
}

void explore(GDir *dir, const gchar *folder, GList *list) {
	const gchar *file;
	gchar *temppath;

	while((dir != NULL) && (file = g_dir_read_name(dir))) {
		temppath = g_build_path("/", folder, file, NULL);
		if( g_file_test(temppath, G_FILE_TEST_IS_REGULAR) && ( g_str_has_suffix(file, "flac") || g_str_has_suffix(file, "mp3") || g_str_has_suffix(file, "ogg") ) ) {
			list = g_list_append(list, temppath);
		}
		else if(g_file_test(temppath, G_FILE_TEST_IS_DIR)) {
			explore(g_dir_open(temppath, 0, NULL), temppath, list);
			g_free(temppath);
		}
	}
	if(file == NULL) {
		g_dir_close(dir);
	}
}

float distance(struct force_vector_s v1, struct force_vector_s v2) {
	float distance;
	distance = sqrt((v1.amplitude - v2.amplitude)*(v1.amplitude- v2.amplitude) + (v1.frequency - v2.frequency)*(v1.frequency- v2.frequency) +
		(v1.tempo - v2.tempo)*(v1.tempo - v2.tempo));
	return distance;
}


float cosine_distance(struct force_vector_s v1, struct force_vector_s v2) {
	float similarity;

    similarity = (v1.tempo*v2.tempo + v1.amplitude*v2.amplitude +
            v1.frequency*v2.frequency + v1.attack*v2.attack) / (
            sqrt(v1.tempo*v1.tempo + v1.amplitude*v1.amplitude +
                v1.frequency*v1.frequency + v1.attack*v1.attack) * 
            sqrt(v2.tempo*v2.tempo + v2.amplitude*v2.amplitude +
                v2.frequency*v2.frequency + v2.attack*v2.attack));

	return similarity;
}

gboolean add_artist_to_playlist(gchar *artist, struct arguments *argument) {
	gboolean valid;
	GtkTreeModel *model_library;
	GtkTreeModel *model_playlist;
	GtkTreeIter lib_iter;
	gchar *tempartist;
	GtkTreeSortable *sortable_library = GTK_TREE_SORTABLE(argument->store_library);

	gtk_tree_sortable_set_sort_column_id(sortable_library, ALBUM, GTK_SORT_ASCENDING);
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
		
		if(artist != NULL) {
			if((!g_strcmp0(tempalbum, album))
			&& (!g_strcmp0(tempartist, artist))) {
				playlist_queue(&lib_iter, model_library, GTK_TREE_VIEW(argument->treeview_playlist), argument);
			}
		}
		else
			if((!g_strcmp0(tempalbum, album)))
				playlist_queue(&lib_iter, model_library, GTK_TREE_VIEW(argument->treeview_playlist), argument);

		valid = gtk_tree_model_iter_next (model_library,
		&lib_iter);
	}
}

gboolean play_playlist_song(gchar *title, struct arguments *argument) {
	gboolean valid, result = FALSE;
	GtkTreeIter iter_playlist;
	GtkTreeModel *model_playlist;
	gchar *temptitle;
	
	model_playlist = gtk_tree_view_get_model(GTK_TREE_VIEW(argument->treeview_playlist));

	valid = gtk_tree_model_get_iter_first(model_playlist, &iter_playlist);

	while(valid) {
		gtk_tree_model_get(model_playlist, &iter_playlist, TRACK, &temptitle, -1);
		if((!strcmp(temptitle, title))) {
			result = TRUE;
			break;
		}
		valid = gtk_tree_model_iter_next(model_playlist,
			&iter_playlist);
	}

	argument->row_playlist = tree_iter_get_row_reference(model_playlist, &iter_playlist);

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
	GtkTreePath *path;
	gboolean valid = FALSE;
	model_playlist = gtk_tree_view_get_model(treeview_playlist);

	if(argument->lelelerandom) {
		return get_lelelerandom_playlist_song(GTK_TREE_VIEW(argument->treeview_playlist), argument);
	}
	else if(argument->random) {
		return get_random_playlist_song(GTK_TREE_VIEW(argument->treeview_playlist), argument);
	}
	else {
		GtkTreeIter test_iter;

		path = gtk_tree_row_reference_get_path(argument->row_playlist);
		gtk_tree_path_next(path);
	
		argument->row_playlist = gtk_tree_row_reference_new(model_playlist, path);
		valid = gtk_tree_model_get_iter(model_playlist, &test_iter, path);

		return valid;
	}
}

gboolean get_random_playlist_song(GtkTreeView *treeview_playlist, struct arguments *argument) {
	GtkTreeModel *model_playlist;
	GtkTreeIter iter_playlist;
	GList *temphistory;
	gchar *songstring;
	gchar *tempstring;
	int i, rand;
	gboolean found = true;

	model_playlist = gtk_tree_view_get_model(treeview_playlist);

	do {
		rand = g_random_int_range(0, argument->playlist_count);
		if(gtk_tree_model_get_iter_first(model_playlist, &iter_playlist)) {
			for(i = 0; i < rand - 1; ++i)
				gtk_tree_model_iter_next(model_playlist, &iter_playlist);
			songstring = gtk_tree_model_get_string_from_iter(model_playlist, &iter_playlist);
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
	
	
	if((argument->row_playlist = tree_iter_get_row_reference(model_playlist, &iter_playlist)))
		return TRUE;
	else
		return FALSE;
}

gboolean get_lelelerandom_playlist_song(GtkTreeView *treeview_playlist, struct arguments *argument) {
	GtkTreeModel *model_playlist;
	GtkTreeIter iter_playlist;
	gboolean network_succeeded = FALSE; 
	model_playlist = gtk_tree_view_get_model(GTK_TREE_VIEW(argument->treeview_playlist));
	struct force_vector_s current_force = argument->current_song.force_vector;

	if(argument->network_mode_set == TRUE) {
		GSocketClient *client;
		GSocketConnection *connection;
		GSocket *socket;
		GOutputStream *stream;
		GSocketListener *listener;
		GInputStream *address_stream;
		gsize count;

		client = g_socket_client_new();

		connection = g_socket_client_connect_to_host(client, argument->lllserver_address_char,
			1212, NULL, NULL);
		if(connection != NULL) {
			stream = g_io_stream_get_output_stream(G_IO_STREAM(connection));
			g_output_stream_write_all(stream, &current_force, sizeof(current_force),
				NULL, NULL, NULL);
	
			listener = g_socket_listener_new();
			g_socket_listener_add_inet_port(listener, 5353, NULL, NULL);
			
			address_stream = g_io_stream_get_input_stream(G_IO_STREAM(connection));
			g_input_stream_read_all(address_stream, &count, sizeof(gsize), NULL,
				NULL, NULL);
			gchar address_remote_player_char[count];
			g_input_stream_read_all(address_stream, &address_remote_player_char, count,
				NULL, NULL, NULL);

			g_socket_listener_add_inet_port(listener, 18322, NULL, NULL);
			connection = g_socket_listener_accept(listener, NULL, NULL, NULL);
			socket = g_socket_connection_get_socket(connection);

			printf("%p\n", socket);
			// TODO

			network_succeeded = TRUE;
		}
		else {
			network_succeeded = FALSE;
		}
	}

	if(network_succeeded = FALSE) {
		float treshold = 0.95;
		float treshold_distance = 4.0;
		gchar *tempstring;
	
		tree_row_reference_get_iter(argument->row_playlist, &iter_playlist);

		gtk_tree_model_get(model_playlist, &iter_playlist, TRACK, &tempstring, -1);
		int i = 0;
		do {
			//treshold -= 0.001;
			if(!get_random_playlist_song(treeview_playlist, argument))
				return FALSE;
			tree_row_reference_get_iter(argument->row_playlist, &iter_playlist);
			gtk_tree_model_get(model_playlist, &iter_playlist, TRACK, &argument->current_song.title, FORCE_TEMPO, &argument->current_song.force_vector.tempo, 
			FORCE_AMP, &argument->current_song.force_vector.amplitude, FORCE_FREQ, &argument->current_song.force_vector.frequency, 
			FORCE_ATK, &argument->current_song.force_vector.attack, -1);
			argument->current_song.force_vector.attack = current_force.attack = 0;
			argument->current_song.force_vector.tempo = current_force.tempo = 0;
			printf("Between %s and %s: %f, %f\n", tempstring, argument->current_song.title, bl_cosine_similarity(current_force, argument->current_song.force_vector), bl_distance(current_force, argument->current_song.force_vector));
		} while((bl_cosine_similarity(current_force, argument->current_song.force_vector) < treshold) ||
			 (bl_distance(current_force, argument->current_song.force_vector) > treshold_distance));
		return TRUE;
	}
}

gboolean get_previous_playlist_song(GtkTreeView *treeview_playlist, struct arguments *argument) {
	GtkTreeModel *model_playlist;
	GtkTreeIter iter_playlist;
	model_playlist = gtk_tree_view_get_model(treeview_playlist);

	if(argument->history && argument->history->next 
	&& gtk_tree_model_get_iter_from_string(model_playlist, &iter_playlist, (gchar*)argument->history->data)) {
		argument->history = g_list_remove(argument->history, argument->history->data);
		
		argument->row_playlist = tree_iter_get_row_reference(model_playlist, &iter_playlist);
		return TRUE;
	}
	else
		return FALSE;
}

void clean_playlist(GtkTreeView *treeview_playlist, struct arguments *argument) {
	gtk_list_store_clear(argument->store_playlist);
	argument->playlist_count = 0;
}

void continue_track_cb(GstElement *playbin, struct arguments *argument) {
	GstStructure *structure;
	gchar *uri;

	structure = gst_structure_new_empty("next_song");
	g_mutex_lock(&argument->queue_mutex);
	GstMessage *msg = gst_message_new_application(GST_OBJECT(playbin), structure);
	gst_element_post_message(argument->playbin, msg);

	g_cond_wait(&argument->queue_cond, &argument->queue_mutex);
	g_mutex_unlock(&argument->queue_mutex);

	if(argument->current_song.filename) {
		uri = g_filename_to_uri(argument->current_song.filename, NULL, NULL);
		g_object_set(argument->playbin, "uri", uri, NULL);
		g_free(uri);
	}
}

void queue_song(struct arguments *argument) {
	char *uri;
	GtkTreeIter iter_playlist;
	GtkTreeModel *model_playlist;

	model_playlist = gtk_tree_view_get_model(GTK_TREE_VIEW(argument->treeview_playlist));
	tree_row_reference_get_iter(argument->row_playlist, &iter_playlist);

	gtk_tree_model_get(model_playlist, &iter_playlist, AFILE, &argument->current_song.filename, 
	TRACKNUMBER, &argument->current_song.tracknumber, TRACK, &argument->current_song.title, 
	ALBUM, &argument->current_song.album, ARTIST, &argument->current_song.artist, 
	FORCE, &argument->current_song.force, FORCE_TEMPO, &argument->current_song.force_vector.tempo, 
	FORCE_AMP, &argument->current_song.force_vector.amplitude, FORCE_FREQ, &argument->current_song.force_vector.frequency, 
	FORCE_ATK, &argument->current_song.force_vector.attack, -1);

	uri = g_filename_to_uri(argument->current_song.filename, NULL, NULL);
	g_object_set(argument->playbin, "uri", uri, NULL);

	g_free(uri); 
}

void start_song(struct arguments *argument) {
	char *uri;
	GtkTreeModel *model_playlist;
	GtkTreePath *path_playlist;
	GtkTreeIter iter_playlist;
	
	model_playlist = gtk_tree_view_get_model(GTK_TREE_VIEW(argument->treeview_playlist));

	tree_row_reference_get_iter(argument->row_playlist, &iter_playlist);

	gtk_tree_model_get(model_playlist, &iter_playlist, AFILE, &argument->current_song.filename, 
	TRACKNUMBER, &argument->current_song.tracknumber, TRACK, &argument->current_song.title, 
	ALBUM, &argument->current_song.album, ARTIST, &argument->current_song.artist, 
	FORCE, &argument->current_song.force, FORCE_TEMPO, &argument->current_song.force_vector.tempo, 
	FORCE_AMP, &argument->current_song.force_vector.amplitude, FORCE_FREQ, &argument->current_song.force_vector.frequency, 
	FORCE_ATK, &argument->current_song.force_vector.attack, -1);

	/*gtk_tree_model_get(model_playlist, &(argument->iter_playlist), AFILE, &argument->current_song.filename, 
	TRACKNUMBER, &argument->current_song.tracknumber, TRACK, &argument->current_song.title, 
	ALBUM, &argument->current_song.album, ARTIST, &argument->current_song.artist, 
	FORCE, &argument->current_song.force, FORCE_TEMPO, &argument->current_song.force_vector.tempo, 
	FORCE_AMP, &argument->current_song.force_vector.amplitude, FORCE_FREQ, &argument->current_song.force_vector.frequency, 
	FORCE_ATK, &argument->current_song.force_vector.attack, -1);*/

	gtk_widget_set_sensitive(argument->progressbar, TRUE);
	uri = g_filename_to_uri(argument->current_song.filename, NULL, NULL);

	gst_element_set_state(argument->playbin, GST_STATE_NULL);
	g_object_set(argument->playbin, "uri", uri, NULL);
	gst_element_set_state(argument->playbin, GST_STATE_PLAYING);
	if(argument->bartag)
		g_source_remove(argument->bartag);
	argument->bartag = g_timeout_add_seconds(1, refresh_progressbar, argument);

	g_free(uri);
}

void pause_song(struct arguments *argument) {
	GtkTreeIter iter_playlist;
	tree_row_reference_get_iter(argument->row_playlist, &iter_playlist);

	gtk_button_set_image(GTK_BUTTON(argument->playpause_button), gtk_image_new_from_icon_name("media-playback-start-symbolic", GTK_ICON_SIZE_BUTTON));
	gtk_list_store_set(argument->store_playlist, &iter_playlist, PLAYING, "▍▍", -1);
	gst_element_set_state(argument->playbin, GST_STATE_PAUSED);
}

void resume_song(struct arguments *argument) {
	GtkTreeIter iter_playlist;
	tree_row_reference_get_iter(argument->row_playlist, &iter_playlist);
	gtk_button_set_image(GTK_BUTTON(argument->playpause_button), gtk_image_new_from_icon_name("media-playback-pause-symbolic", GTK_ICON_SIZE_BUTTON));
	gtk_list_store_set(argument->store_playlist, &iter_playlist, PLAYING, "▶",-1);
	gst_element_set_state(argument->playbin, GST_STATE_PLAYING);
}

void state_changed_cb(GstBus *bus, GstMessage *msg, struct arguments *argument) {
	GstState old_state, new_state, pending_state;

	if(GST_MESSAGE_SRC(msg) == GST_OBJECT(argument->playbin)) {
		gst_message_parse_state_changed(msg, &old_state, &new_state, &pending_state);
    	argument->state = new_state;
	}
}

gint sort_iter_compare_func(GtkTreeModel *model, GtkTreeIter *a, GtkTreeIter *b, gpointer userdata) {
	gchar *track1, *track2;

	gtk_tree_model_get(model, a, TRACKNUMBER, &track1, -1);
	gtk_tree_model_get(model, b, TRACKNUMBER, &track2, -1);

	if(track1 && track2) {
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
}

gint sort_album_tracks(GtkTreeModel *model, GtkTreeIter *a, GtkTreeIter *b, gpointer userdata) {
	gchar *album1, *album2;
	gchar *track1, *track2;
	int retval;

	gtk_tree_model_get(model, a, ALBUM, &album1, -1);
	gtk_tree_model_get(model, b, ALBUM, &album2, -1);
	gtk_tree_model_get(model, a, TRACKNUMBER, &track1, -1);
	gtk_tree_model_get(model, b, TRACKNUMBER, &track2, -1);

	if(album1 && album2 && track1 && track2) {
		if(strcmp(album1, album2) > 0) {
			retval = 1;
		}
		else if(strcmp(album1, album2) < 0)
			retval = -1;
		else {
			if(atof(track1) > atof(track2) > 0)
				retval = 1;
			else if(atof(track1) < atof(track2))
				retval = -1;
			else 
				retval = 0;
		}
	}
	retval = 0;

	g_free(album1);
	g_free(album2);
	g_free(track1);
	g_free(track2);
	
	return retval;
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

	if(artist1 && artist2 && album1 && album2 && track1 && track2) {
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
	}
	g_free(artist1);
	g_free(artist2);
	g_free(album1);
	g_free(album2);
	g_free(track1);
	g_free(track2);
	
	return retval;
}

gint sort_text(GtkTreeModel *model, GtkTreeIter *a, GtkTreeIter *b, gpointer userdata) {
	gint retval;
	
	gchar *string1, *string2;

	gtk_tree_model_get(model, a, 0, &string1, -1);
	gtk_tree_model_get(model, b, 0, &string2, -1);

	if(string1 && string2) {
		if(strcmp(string1, string2) > 0)
			retval = 1;
		else if(strcmp(string1, string2) < 0)
			retval = -1;
		else
			retval = 0;
	}
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

gboolean filter_library(GtkTreeModel *model_library, GtkTreeIter *iter, struct arguments *argument) {
	gchar *compstr = argument->search_entry_text;
	gchar *track, *artist, *album;
	gboolean visible = FALSE;
	gtk_tree_model_get(model_library, iter, TRACK, &track, ALBUM, &album, ARTIST, &artist, -1);

	if(compstr != NULL) {
		if(track && album && artist && (strcasestr(track, compstr) || strcasestr(album, compstr) || strcasestr(artist, compstr)))  {
			visible = TRUE;
		}
	}
	else
		visible = TRUE;

	g_free(track);
	g_free(artist);
	g_free(album);
	
	return visible;
}

gboolean filter_artist(GtkTreeModel *model_artist, GtkTreeIter *iter, struct arguments *argument) {
	gchar *compstr = argument->search_entry_text;
	gchar *artist;
	GtkTreeIter tempiter;
	gboolean visible = FALSE;
	GtkTreePath *artist_path = gtk_tree_model_get_path(model_artist, iter);

	gtk_tree_model_get(model_artist, iter, COLUMN_ARTIST, &artist, -1);

	if(compstr != NULL) {
		if(gtk_tree_path_get_depth(artist_path) == 1) {
			if(artist && compstr && (strcasestr(artist, compstr))) {
				visible = TRUE;
			}
		}
		else if(gtk_tree_path_get_depth(artist_path) == 2) {
			gtk_tree_path_up(artist_path);
			gtk_tree_model_get_iter(model_artist, &tempiter,artist_path);
			gtk_tree_model_get(model_artist, &tempiter,COLUMN_ARTIST, &artist, -1);
			if(artist && compstr &&(strcasestr(artist, compstr))) {
				visible = TRUE;
			}
		}
		else if(gtk_tree_path_get_depth(artist_path) == 3) {
			gtk_tree_path_up(artist_path);
			gtk_tree_path_up(artist_path);
			gtk_tree_model_get_iter(model_artist, &tempiter, artist_path);
			gtk_tree_model_get(model_artist, &tempiter, COLUMN_ARTIST, &artist, -1);
			if(artist && compstr &&(strcasestr(artist, compstr))) {
				visible = TRUE;
			}
		}
	}
	else
		visible = TRUE;

	g_free(artist);
	
	return visible;
}

gboolean filter_album(GtkTreeModel *model_album, GtkTreeIter *iter, struct arguments *argument) {
	gchar *compstr = argument->search_entry_text;
	gchar *album;
	GtkTreeIter tempiter;
	gboolean visible = FALSE;
	GtkTreePath *album_path = gtk_tree_model_get_path(model_album, iter);

	gtk_tree_model_get(model_album, iter, COLUMN_ALBUM, &album, -1);
	
	if(compstr != NULL) {
		if(gtk_tree_path_get_depth(album_path) == 1) {
			if(album && compstr && (strcasestr(album, compstr))) {
				visible = TRUE;
			}
		}
		else if(gtk_tree_path_get_depth(album_path) == 2) {
			gtk_tree_path_up(album_path);
			gtk_tree_model_get_iter(model_album, &tempiter,album_path);
			gtk_tree_model_get(model_album, &tempiter,COLUMN_ALBUM, &album, -1);
			if(album && compstr && (strcasestr(album, compstr))) {
				visible = TRUE;
			}
		}
	}
	else
		visible = TRUE;

	g_free(album);
	
	return visible;
}

int nb_rows_treeview(GtkTreeModel *model) {
	gboolean valid = FALSE;
	GtkTreeIter iter;
	int count = 0;
	gchar *tempchar;

	valid = gtk_tree_model_get_iter_first(model, &iter);

	while(valid == TRUE) {
		count++;
		valid = gtk_tree_model_iter_next(model, &iter);
	}

	return count;
}

gboolean tree_row_reference_get_iter(GtkTreeRowReference *reference, GtkTreeIter *iter) {
	GtkTreePath *path;
	GtkTreeModel *model;
	gboolean result = FALSE;

	model = gtk_tree_row_reference_get_model(reference);
	path = gtk_tree_row_reference_get_path(reference);
	result = gtk_tree_model_get_iter(model, iter, path);
	
	return result;
}

GtkTreeRowReference *tree_iter_get_row_reference(GtkTreeModel *model, GtkTreeIter *iter) {
	GtkTreePath *path;
	GtkTreeRowReference *reference;
	gboolean result = FALSE;

	path = gtk_tree_model_get_path(model, iter);
	reference = gtk_tree_row_reference_new(model, path);

	result = gtk_tree_row_reference_valid(reference);
	if(result == TRUE)
		return reference;
	else
		return NULL;
}
