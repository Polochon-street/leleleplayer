#include "analyze.h"

#include <stdbool.h>
#include <AL/al.h>
#include <AL/alc.h>
#include <gtk/gtk.h>
#include <glib/gprintf.h>
#include <glib/gstdio.h>
#include <glib.h>

struct arguments {
	ALint status;
	//ALint source;
	int tag;
	int first;
	int bartag;
	GTimer *elapsed;
	int endless_check;
	int continue_count;
	GtkWidget *label;
	GtkAdjustment *adjust;
};


static void destroy (GtkWidget*, gpointer);
static void folder_changed (GtkFileChooser*, struct arguments*);
static void file_changed (GtkFileChooser*, struct arguments*);
void explore(GDir *dir, char *folder);
int play(GtkWidget*, struct arguments*);
bool InitOpenAL(void);
void ShutdownOpenAL(void);

ALint status;
ALuint source;
