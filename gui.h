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
	TRACKNUMBER = 0,
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
	ALuint buffer_old;
	//ALint source;
	int tag;
	gdouble offset;
	int first;
	int bartag;
	GTimer *elapsed;
	GtkWidget *treeview;
	GtkTreePath *path;
	GtkTreeIter iter;
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
static void destroy(GtkWidget *, gpointer);
static void ShutdownOpenAL(void);
bool InitOpenAL(void);
int bufferize(struct arguments *);
int play(GtkWidget *, struct arguments *);
