#include "analyze.h"

#include <stdbool.h>
#include <AL/al.h>
#include <AL/alc.h>
#include <gtk/gtk.h>
#include <glib/gprintf.h>
#include <glib/gstdio.h>
#include <glib.h>

enum {
	TRACK = 0,
	ALBUM,
	ARTIST,
	FORCE,
	AFILE,
	COLUMNS
};

static void setup_tree_view(GtkWidget *);
static void row_activated(GtkTreeView *, GtkTreePath *, GtkTreeViewColumn *, gpointer);
static void destroy(GtkWidget *, gpointer);
