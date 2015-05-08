#include "analyze.h"

#include <stdbool.h>
#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>
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
	AFILE,
	COLUMNS
};

struct arguments {
	ALuint source;
	ALint status;
	ALuint buffer;
	ALuint next_buffer;
	ALuint buffer_old;
	//ALint source;
	struct song current_song;
	struct song next_song;
	int tag;
	int first;
	int bartag;
	int playlist_count;
	char **playlist;
	GTimer *elapsed;
	GtkWidget *treeview;
	GtkWidget *toggle_button;
	//GtkWidget *next_button;
	//GtkWidget *previous_button;
	GtkTreePath *path;
	GtkTreeIter playing_iter;
	GtkTreeIter iter;
	GtkListStore *store;
	GtkTreeViewColumn *column;
//	int endless_check;
//	int continue_count;
//	GtkWidget *label, *album_label, *title_label, *artist_label;
	GtkAdjustment *adjust;
};

struct pref_arguments {
        GtkWidget *window;
        GtkWidget *treeview;
		gchar *folder;
        GtkListStore *store;
		GtkWidget *library_entry;
};

static void setup_tree_view(GtkWidget *);
static gboolean continue_track(gpointer);
static int timer_progressbar(gpointer);
static void row_activated(GtkTreeView *, GtkTreePath *, GtkTreeViewColumn *, struct arguments *);
static void config_folder_changed(gchar *, GtkWidget *);
static void toggle(GtkWidget *, struct arguments *);
static void next(GtkWidget *, struct arguments *);
static void previous(GtkWidget *, struct arguments *);
static void destroy(GtkWidget *, gpointer);
static void ShutdownOpenAL(void);
static void preferences_callback(GtkMenuItem *preferences, struct pref_arguments *);
bool InitOpenAL(void);
int bufferize(struct song, struct arguments *);
void pause_song(struct arguments *);
void play_song(struct song, struct arguments *);
void free_song(struct song *);
void explore(GDir *dir, char *folder, FILE *list);
void folder_chooser(GtkWidget *, struct pref_arguments *);
void display_library(GtkTreeView *, GtkListStore *);
void set_next_song(struct arguments *);
void unqueue_buffer(int , ALuint *);
