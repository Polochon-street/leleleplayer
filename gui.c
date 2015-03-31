#include <stdio.h>
#include "gui.h"

enum {
	TRACK = 0,
	ALBUM,
	ARTIST,
	FORCE,
	AFILE,
	COLUMNS
};

static void setup_tree_view(GtkWidget *treeview) {
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;

	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes("Track", renderer, "text", TRACK, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);

	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes("Album", renderer, "text", ALBUM, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);

	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes("Artist", renderer, "text", ARTIST, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);

	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes("Force", renderer, "text", FORCE, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);

	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes("File", renderer, "text", AFILE, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);

}

static void row_activated(GtkTreeView *treeview, GtkTreePath *path, GtkTreeViewColumn *column, gpointer data) {
	GtkTreeModel *model;
	GtkTreeIter iter;
	char *tempfile;

	model = gtk_tree_view_get_model(treeview);
	if(gtk_tree_model_get_iter(model, &iter, path)) {
		gtk_tree_model_get(model, &iter, AFILE, &tempfile, -1);
	}
	printf("%s\n", tempfile);
}

int main(int argc, char **argv) {
	FILE *library;
	library = fopen("library.txt", "r");
	char tempfile[1000];
	char temptrack[1000];
	char tempalbum[1000];
	char tempartist[1000];
	char tempforce[1000];
	struct arguments argument;
	struct arguments *pargument = &argument;	

	GtkWidget *window, *treeview, *scrolled_win;
	GtkListStore *store;
	GtkTreeIter iter;
	size_t i = 0;

	gtk_init(&argc, &argv);
	
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(window), "lelele player");
	gtk_widget_set_size_request(window, 800, 500);	

	treeview = gtk_tree_view_new();
	setup_tree_view(treeview);

	g_signal_connect(G_OBJECT(treeview), "row-activated", G_CALLBACK(row_activated), NULL);

	store = gtk_list_store_new(COLUMNS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);

	while(fgets(tempfile, 1000, library) != NULL) {
		tempfile[strcspn(tempfile, "\n")] = '\0';
		fgets(temptrack, 1000, library);
		temptrack[strcspn(temptrack, "\n")] = '\0';
		fgets(tempalbum, 1000, library);
		tempalbum[strcspn(tempalbum, "\n")] = '\0';
		fgets(tempartist, 1000, library);
		tempartist[strcspn(tempartist, "\n")] = '\0';
		fgets(tempforce, 1000, library);
		tempforce[strcspn(tempforce, "\n")] = '\0';
		if(atoi(tempforce) == 0)
			strcpy(tempforce, "Calm");
		else if(atoi(tempforce) == 1)
			strcpy(tempforce, "Loud");
		else
			strcpy(tempforce, "Can't conclude");

		gtk_list_store_append(store, &iter);
		gtk_list_store_set(store, &iter, TRACK, temptrack, ALBUM, tempalbum, ARTIST, tempartist, FORCE, tempforce, AFILE, tempfile, -1);
	}

	gtk_tree_view_set_model(GTK_TREE_VIEW(treeview), GTK_TREE_MODEL(store));
	g_object_unref(store);

	scrolled_win = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_win), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

	gtk_container_add(GTK_CONTAINER(scrolled_win), treeview);
	gtk_container_add(GTK_CONTAINER(window), scrolled_win);

	gtk_widget_show_all(window);

	gtk_main();
	return 0;
}
