#include "analyze.h"

#include <stdbool.h>
#include <AL/al.h>
#include <AL/alc.h>
#include <gtk/gtk.h>
#include <glib/gprintf.h>
#include <glib/gstdio.h>
#include <glib.h>

static void destroy (GtkWidget*, gpointer);
static void folder_changed (GtkFileChooser*, GtkLabel*);
static void file_changed (GtkFileChooser*, GtkLabel*);
void explore(GDir *dir, char *folder);
int play(GtkWidget*, gpointer data);
bool InitOpenAL(void);
void ShutdownOpenAL(void);

ALint status;
ALuint source;

