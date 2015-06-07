#include "analyze.h"

#include <stdbool.h>
#include <gtk/gtk.h>
#include <glib/gprintf.h>
#include <glib/gerror.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <glib/gstdio.h>
#include <glib.h>

enum {
	PLAYING = 0,
	TRACKNUMBER,
	TRACK,
	ALBUM,
	ARTIST,
	FORCE,
	TEXTFORCE,
	AFILE,
	COLUMNS
};

struct arguments {
	//ALint source;
	gboolean lelelerandom;
	gboolean random;
	struct song current_song;
	int first;
	int bartag;
	int playlist_count;
	gulong progressbar_update_signal_id;
	GTimer *elapsed;
	GtkWidget *treeview_library;
	GtkWidget *progressbar;
	GtkWidget *treeview_playlist;
	GtkWidget *toggle_button;
	//GtkWidget *next_button;
	//GtkWidget *previous_button;
	GtkTreePath *path;
	GtkTreeIter iter_playlist;
	GtkTreeIter iter_library;
	GtkListStore *store_library;
	GtkListStore *store_playlist;
	GtkTreeViewColumn *column;
	GtkWidget *album_label, *title_label, *artist_label;
	GtkAdjustment *adjust;
};

struct pref_arguments {
        GtkWidget *window;
        GtkWidget *treeview;
		gchar *folder;
        GtkListStore *store_library;
		GtkWidget *library_entry;
};

static void setup_tree_view(GtkWidget *);
static void continue_track(GstElement *, struct arguments *);
static gboolean refresh_progressbar(gpointer);
static void row_activated(GtkTreeView *, GtkTreePath *, GtkTreeViewColumn *, struct arguments *);
static void config_folder_changed(gchar *, GtkWidget *);
static void toggle(GtkWidget *, struct arguments *);
static void next(GtkWidget *, struct arguments *);
static void previous(GtkWidget *, struct arguments *);
static void destroy(GtkWidget *, gpointer);
static void preferences_callback(GtkMenuItem *preferences, struct pref_arguments *);
static void state_changed(GstBus *, GstMessage *, struct arguments *);
static void refresh_ui(GstBus *, GstMessage *, struct arguments *);
static void toggle_lelele(GtkWidget *button, struct arguments *);
static void toggle_random(GtkWidget *button, struct arguments *);
int bufferize(struct song, struct arguments *);
void pause_song(struct arguments *);
void play_song(struct arguments *);
void queue_song(struct arguments *);
void free_song(struct song *);
void explore(GDir *dir, char *folder, FILE *list);
void folder_chooser(GtkWidget *, struct pref_arguments *);
void display_library(GtkTreeView *, GtkListStore *);
void playlist_queue(GtkTreeIter *, GtkTreeModel *, GtkTreeView *, struct arguments *);
void get_current_playlist_song(GtkTreeView *, struct song *, struct arguments *);
gboolean get_next_playlist_song(GtkTreeView *, struct song *, struct arguments *);
gboolean get_random_playlist_song(GtkTreeView *, struct song *, struct arguments *);
gboolean get_previous_playlist_song(GtkTreeView *, struct song *, struct arguments *);
