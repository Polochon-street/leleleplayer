#include "analyze.h"

#include <stdbool.h>
#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>
#include <gtk/gtk.h>
#include <glib/gprintf.h>
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
	int tag;
	gdouble offset;
	int first;
	int bartag;
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

static void setup_tree_view(GtkWidget *);
static gboolean continue_track(gpointer);
static timer_progressbar(gpointer);
static void row_activated(GtkTreeView *, GtkTreePath *, GtkTreeViewColumn *, struct arguments *);
static void toggle(GtkWidget *, struct arguments *);
static void next(GtkWidget *, struct arguments *);
static void destroy(GtkWidget *, gpointer);
static void ShutdownOpenAL(void);
bool InitOpenAL(void);
int bufferize(int8_t *, struct arguments *);
void pause_song(struct arguments *);
void play_song(struct arguments *);
