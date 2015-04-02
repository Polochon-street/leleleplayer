#include <stdio.h>
#include "gui.h"

static void destroy (GtkWidget *window, gpointer data) {
//	ShutdownOpenAL();
	gtk_main_quit ();
}

static void setup_tree_view(GtkWidget *treeview) {
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;

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
	gtk_tree_view_column_set_fixed_width(column, 200);
	gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);

	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes("Artist", renderer, "text", ARTIST, NULL);
	gtk_tree_view_column_set_resizable (column, TRUE);
	gtk_tree_view_column_set_sort_column_id(column, ARTIST);
	gtk_tree_view_column_set_sizing(column, GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_fixed_width(column, 200);
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

static void display_library(GtkTreeView *treeview, GtkTreeIter iter, GtkListStore *store) {
	FILE *library;
	size_t i = 0;
	char tempfile[1000];
	char temptrack[1000];
	char tempalbum[1000];
	char tempartist[1000];
	char tempforce[1000];

	library = fopen("library.txt", "r");

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
			strcpy(tempforce, "Loud");
		else if(atoi(tempforce) == 1)
			strcpy(tempforce, "Calm");
		else
			strcpy(tempforce, "Can't conclude");

		gtk_list_store_append(store, &iter);
		gtk_list_store_set(store, &iter, TRACK, temptrack, ALBUM, tempalbum, ARTIST, tempartist, FORCE, tempforce, AFILE, tempfile, -1);
	}
	g_object_unref(store);
}

int main(int argc, char **argv) {
	GtkWidget *window, *treeview, *scrolled_win, *vboxh;
	GtkListStore *store;
	GtkTreeIter iter;

	gtk_init(&argc, &argv);
	
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(window), "lelele player");
	gtk_widget_set_size_request(window, 800, 500);	

	treeview = gtk_tree_view_new();
	setup_tree_view(treeview);
	scrolled_win = gtk_scrolled_window_new(NULL, NULL);
	store = gtk_list_store_new(COLUMNS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
	gtk_tree_view_set_model(GTK_TREE_VIEW(treeview), GTK_TREE_MODEL(store));
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_win), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	vboxh = gtk_hbox_new(TRUE, 5);

	g_signal_connect(G_OBJECT(treeview), "row-activated", G_CALLBACK(row_activated), NULL);
	g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(destroy), NULL);

	gtk_container_add(GTK_CONTAINER(scrolled_win), treeview);
	gtk_box_pack_start_defaults(GTK_BOX(vboxh), scrolled_win);

	gtk_container_add(GTK_CONTAINER(window), vboxh);

	/* temporary */
	display_library(GTK_TREE_VIEW(treeview), iter, store);
	/* temporary */

	gtk_widget_show_all(window);

	gtk_main();
	return 0;
}
