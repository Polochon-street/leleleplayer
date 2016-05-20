#include <gio/gio.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <libxml/xmlwriter.h>
#include <bliss.h>

void convert_library_to_list_store(GtkListStore *, gchar *);
void lllp_connected_cb(GObject *, GAsyncResult *, gpointer); 
void lllp_next_song_cb(GObject *, GAsyncResult *, gpointer); 
void lllp_quit_cb(GObject *, GAsyncResult *, gpointer); 

enum {
	INET_ADDRESS,
	AFILE,
	FORCE_AMP,
	FORCE_FREQ,
	FORCE_TEMPO,
	FORCE_ATK,
};

/*gint test(struct leleleplayer *lllp_array) {
	gboolean valid = FALSE;
	GtkTreeIter iter;
	gchar *tempfile;
	int i = 0;

	while(lllp_array[i].address != NULL) {
		valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(lllp_array[i].store_library), &iter);

		while(valid) {
			gtk_tree_model_get(GTK_TREE_MODEL(lllp_array[i].store_library), &iter, AFILE, &tempfile, -1);
			valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(lllp_array[i].store_library), &iter);
		}
		i++;
		printf("%s\n", tempfile);
	}

	return TRUE;
}*/

int main (int argc, char **argv) {
	gtk_init(&argc, &argv);

	GtkListStore *store_library = gtk_list_store_new(6, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_FLOAT, G_TYPE_FLOAT, G_TYPE_FLOAT, G_TYPE_FLOAT);
	GSocketListener *listener;
	GSocketListener *listener_next_song;
	GSocketListener *listener_quit;
	gsize i;

	listener = g_socket_listener_new();
	listener_next_song = g_socket_listener_new();
	listener_quit = g_socket_listener_new();
	
	g_socket_listener_add_inet_port(listener_next_song, 12512, NULL, NULL);
	g_socket_listener_add_inet_port(listener, 1292, NULL, NULL);
	g_socket_listener_add_inet_port(listener_quit, 11912, NULL, NULL);

	g_socket_listener_accept_async(listener, NULL, lllp_connected_cb, store_library);
	g_socket_listener_accept_async(listener_next_song, NULL, lllp_next_song_cb, store_library);
	g_socket_listener_accept_async(listener_quit, NULL, lllp_quit_cb, store_library);
//	g_timeout_add(4000, test, lllp_array);

	gtk_main();
	return 0;
}

void lllp_next_song_cb(GObject *listener, GAsyncResult *res, gpointer pstore_library) {
	GtkListStore *store_library = GTK_LIST_STORE(pstore_library);
	gboolean valid = FALSE;
	GSocketConnection *connection;
	GSocketClient *player_client;
	GSocketAddress *address;
	GSocket *socket;
	GInputStream *stream;
	GOutputStream *output_stream;
	GInetAddress *inet_address;
	GInetAddress *player_with_the_song;
	GtkTreeIter iter;
	gchar *player_with_the_song_char;
	struct force_vector_s force_vector;
	gchar *remote_file;
	gint random_number;
	gsize count = 0;
	int i;
	struct force_vector_s rand_force_vector;

	connection = g_socket_listener_accept_finish(G_SOCKET_LISTENER(listener), res, NULL, NULL);
	address = g_socket_connection_get_remote_address(connection, NULL);
	inet_address = g_inet_socket_address_get_address(G_INET_SOCKET_ADDRESS(address));

	socket = g_socket_connection_get_socket(connection);
	stream = g_io_stream_get_input_stream(G_IO_STREAM(connection));

	g_input_stream_read_all(stream, &force_vector, sizeof(force_vector), NULL, NULL, NULL);
	
	valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(store_library), &iter);
	while(valid == TRUE) {
		valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(store_library), &iter);
		count++;
	}

	gint rand;
	float distance = 100, old_distance = 100;
	float treshold = 0.95;
	float treshold_distance = 4.0;

	do {
		/* TO FUNCTIONALIZE */
		rand = g_random_int_range(0, count);

		if(gtk_tree_model_get_iter_first(GTK_TREE_MODEL(store_library), &iter)) {
			for(i = 0; i < rand - 1; ++i) {
				gtk_tree_model_iter_next(GTK_TREE_MODEL(store_library), &iter);
			}
			gtk_tree_model_get(GTK_TREE_MODEL(store_library), &iter, FORCE_AMP, &rand_force_vector.amplitude,
				FORCE_ATK, &rand_force_vector.attack, FORCE_FREQ, &rand_force_vector.frequency,
				FORCE_TEMPO, &rand_force_vector.tempo, -1);
		}
		treshold -= 0.01;
		treshold_distance += 0.01;
	}  while((bl_cosine_similarity(rand_force_vector, force_vector) < treshold) ||
		((distance = bl_distance(rand_force_vector, force_vector)) > treshold_distance));
	
	gtk_tree_model_get(GTK_TREE_MODEL(store_library), &iter, AFILE, &remote_file, INET_ADDRESS, &player_with_the_song_char, -1);
	player_with_the_song = g_inet_address_new_from_string(player_with_the_song_char);

	if(g_inet_address_equal(player_with_the_song, inet_address) == TRUE) {
		rand_force_vector.attack = rand_force_vector.frequency = rand_force_vector.tempo = rand_force_vector.amplitude = 0;
		player_client = g_socket_client_new();
		connection = g_socket_client_connect_to_host(player_client, g_inet_address_to_string(inet_address), 5353, NULL, NULL);
		output_stream = g_io_stream_get_output_stream(G_IO_STREAM(connection));
		g_output_stream_write_all(output_stream, &rand_force_vector, sizeof(rand_force_vector), &count, NULL, NULL);
		g_io_stream_close(G_IO_STREAM(connection), NULL, NULL);

		g_socket_listener_accept_async(G_SOCKET_LISTENER(listener), NULL, lllp_next_song_cb, store_library);
	
		return;
	}
	player_client = g_socket_client_new();
	connection = g_socket_client_connect_to_host(player_client, g_inet_address_to_string(inet_address), 5353, NULL, NULL);
	output_stream = g_io_stream_get_output_stream(G_IO_STREAM(connection));
	g_output_stream_write_all(output_stream, &rand_force_vector, sizeof(rand_force_vector), &count, NULL, NULL);
	g_io_stream_close(G_IO_STREAM(connection), NULL, NULL);

	player_with_the_song_char = g_inet_address_to_string(player_with_the_song);
	connection = g_socket_client_connect_to_host(player_client, player_with_the_song_char, 19144, NULL, NULL);
	printf("%p\n", connection);
	output_stream = g_io_stream_get_output_stream(G_IO_STREAM(connection));
	count = strlen(g_inet_address_to_string(inet_address)) + 1;
	g_output_stream_write_all(output_stream, &count, sizeof(gsize), NULL, NULL, NULL);
	g_output_stream_write_all(output_stream, g_inet_address_to_string(inet_address), count, NULL, NULL, NULL);
	count = strlen(remote_file) + 1;
	g_output_stream_write_all(output_stream, &count, sizeof(gsize), NULL, NULL, NULL);
	g_output_stream_write_all(output_stream, remote_file, count, NULL, NULL, NULL);
	g_output_stream_close(output_stream, NULL, NULL);

	g_socket_listener_accept_async(G_SOCKET_LISTENER(listener), NULL, lllp_next_song_cb, store_library);
}

void lllp_connected_cb(GObject *listener, GAsyncResult *res, gpointer pstore_library) {
	GtkListStore *store_library = GTK_LIST_STORE(pstore_library);
	GSocketConnection *connection;
	GSocketAddress *address;
	gchar *address_char;
	GInputStream *stream;
	GFile *output_file;
	GOutputStream *file_stream;
	gsize count;
	gboolean valid = FALSE;


	connection = g_socket_listener_accept_finish(G_SOCKET_LISTENER(listener), res, NULL, NULL);
	address = g_socket_connection_get_remote_address(connection, NULL);
	address_char = g_inet_address_to_string(g_inet_socket_address_get_address(G_INET_SOCKET_ADDRESS(address)));
	printf("%s\n", address_char);

	output_file = g_file_new_for_path(address_char);
	g_file_delete(output_file, NULL, NULL);
	file_stream = (GOutputStream*)g_file_replace(output_file, NULL, FALSE, G_FILE_CREATE_REPLACE_DESTINATION, NULL, NULL);

	stream = g_io_stream_get_input_stream(G_IO_STREAM(connection));
	g_output_stream_splice(file_stream, stream, 
		G_OUTPUT_STREAM_SPLICE_CLOSE_SOURCE & G_OUTPUT_STREAM_SPLICE_CLOSE_TARGET, NULL, NULL);
	g_output_stream_close(file_stream, NULL, NULL);

	convert_library_to_list_store(store_library, address_char);

	printf("%s CONNECTED\n", address_char);

	g_socket_listener_accept_async(G_SOCKET_LISTENER(listener), NULL, lllp_connected_cb, store_library);
}

void lllp_quit_cb(GObject *listener, GAsyncResult *res, gpointer pstore_library) {
	GtkListStore *store_library = GTK_LIST_STORE(pstore_library);
	gsize i;
	GtkTreeIter iter;
	gboolean valid;
	GSocketConnection *connection;
	GSocketAddress *remote_socket_addr;
	GInetAddress *remote_addr;
	gchar *remote_addr_char;
	gchar *tempchar;

	connection = g_socket_listener_accept_finish(G_SOCKET_LISTENER(listener), res, NULL, NULL);
	remote_socket_addr = g_socket_connection_get_remote_address(connection, NULL);
	remote_addr = g_inet_socket_address_get_address((GInetSocketAddress *)(remote_socket_addr));
	remote_addr_char = g_inet_address_to_string(remote_addr);

	if(connection != NULL) {
		valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(store_library), &iter);
		while(valid == TRUE) {
			gtk_tree_model_get(GTK_TREE_MODEL(store_library), &iter, INET_ADDRESS, &tempchar, -1);
			if(g_strcmp0(tempchar, remote_addr_char) == 0)
				gtk_list_store_remove(store_library, &iter);
			valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(store_library), &iter);
		}
		
		printf("LLLP %s DISCONNECTED\n", remote_addr_char);
	}
	g_socket_listener_accept_async(G_SOCKET_LISTENER(listener), NULL, lllp_quit_cb, store_library);
}

void convert_library_to_list_store(GtkListStore *store_library, gchar *libfile) {
	gboolean valid = FALSE;
	GtkTreeIter iter;
	xmlDocPtr library;
	xmlNodePtr cur;
	xmlNodePtr child;
	xmlChar *tempfile = NULL;
	xmlChar *tempforce_env = NULL;
	float tempforce_envf;
	xmlChar *tempforce_amp = NULL;
	float tempforce_ampf;
	xmlChar *tempforce_freq = NULL;
	float tempforce_freqf;
	xmlChar *tempforce_atk = NULL;
	float tempforce_atkf;

	valid = g_file_test(libfile, G_FILE_TEST_EXISTS);
	
	if(valid == TRUE) {
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
						if((!xmlStrcmp(child->name, (const xmlChar *)"filename"))) {
							tempfile = xmlNodeGetContent(child);
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

					gtk_list_store_append(store_library, &iter);
					gtk_list_store_set(store_library, &iter, INET_ADDRESS, libfile, AFILE, tempfile, FORCE_AMP, tempforce_ampf, FORCE_FREQ, tempforce_freqf, FORCE_TEMPO, tempforce_envf, FORCE_ATK, tempforce_atkf, -1);
					xmlFree(tempfile);
					xmlFree(tempforce_amp);
					xmlFree(tempforce_freq);
					xmlFree(tempforce_env);
					xmlFree(tempforce_atk);
					tempforce_amp = tempfile = tempforce_freq = tempforce_env = tempforce_atk = NULL;
				}
			}
			xmlFreeDoc(library);
		}
		else
			fprintf(stderr, "Couldn't parse %s.\n", libfile);
	}
	else
		fprintf(stderr, "Couldn't open library file: %s, because it doesn't exists.\n", libfile);
}
