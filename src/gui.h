#include "analyze.h"

#include <stdbool.h>
#include <gtk/gtk.h>
#include <glib/gprintf.h>
#include <glib/gerror.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <glib/gstdio.h>
#include <glib.h>
#include <limits.h>
#define GST_USE_UNSTABLE_API
#include <gst/gl/gl.h>
enum {
	COLUMN_ARTIST = 0,
	NUM_COLS_ARTIST
};

enum {
	PLAYING = 0,
	TRACKNUMBER,
	TRACK,
	ALBUM,
	ARTIST,
	FORCE,
	FORCE_TEMPO,
	FORCE_AMP,
	FORCE_FREQ,
	FORCE_ATK,
	TEXTFORCE,
	AFILE,
	COLUMNS
};

struct arguments {
	gboolean lelelerandom;
	gboolean random;
	gboolean repeat;
	struct song current_song;
	int first;
	int bartag;
	int playlist_count;
	gulong progressbar_update_signal_id;
	GTimer *elapsed;
	GList *history;
	GtkWidget *treeview_library;
	GtkWidget *treeview_artist;
	GtkWidget *progressbar;
	GtkWidget *treeview_playlist;
	GtkWidget *playpause_button;
	GtkTreePath *path;
	GtkTreeIter iter_playlist;
	GtkTreeIter iter_library;
	GtkTreeIter iter_artist;
	GtkListStore *store_library;
	GtkListStore *store_playlist;
	GtkTreeStore *store_artist;
	GtkTreeViewColumn *column;
	GtkWidget *album_label, *title_label, *artist_label;
	GtkWidget *genre_label, *samplerate_label, *bitrate_label, *channels_label;
	gchar *str_genre, *str_samplerate, *str_channels, *str_bitrate;
	GtkAdjustment *adjust;
	GtkWidget *volume_scale;
	GtkWidget *video_window;
};

struct pref_arguments {
        GtkWidget *window;
        GtkWidget *treeview;
		gchar *folder;
        GtkListStore *store_library;
		GtkWidget *library_entry;
};

struct pref_folder_arguments {
	GAsyncQueue *msg_queue;
	char *line;
	FILE *list;
	FILE *library;
};

void setup_tree_view_renderer_play_lib(GtkWidget *);
/**
* Description: Sets up the treeview renderer with columns "playing", "tracknumber", "track",
* "album", "artist", "force", and sizes it properly
* Arguments: GtkWidget *treeview: The TreeView to set up
*/
void setup_tree_view_renderer_artist(GtkWidget *, GtkTreeStore *, GtkTreeModel *);
/**
* Description: Sets up the treeview renderer like this: artist->album->songs
* Arguments: GtkWidget *treeview: the TreeView to set up
* Arguments: GtkTreeStore *treestore: the treestore associated with the TreeView
* Arguments: GtkTreeModel *model_library: the model associated with the library treeview (NOT the artist treeview)
*/
void continue_track(GstElement *, struct arguments *);
/**
* Description: Queue next playlist song, and don't play it immediately: useful for gapless
* Arguments: struct arguments *argument: the global argument struct containing:
* -the songs history: history
* -the struct_song current_song to set up: current_song 
* -the playlist treeview (in order to instert songs there): treeview_playlist
* -the playlist iter (in order to know what is the next song)
* Arguments: GstElement *playbin: the playbin where the song is queued
*/
gboolean refresh_progressbar(gpointer);
/**
* Description: Refresh the GtkScale progress bar (the song progress bar)
* Arguments: struct arguments *argument: the global argument struct containing:
* -the song state: current_song.state
* -the progressbar widget: progressbar
* -the playbin, in order to get the time elapsed in the song: playbin
*/
void lib_row_activated(GtkTreeView *, GtkTreePath *, GtkTreeViewColumn *, struct arguments *);
/**
* Description: Callback function called when the user selects (double-click/enter) a song in the library
* Arguments: GtkTreeView *treeview: the library treeview
* Arguments: GtkTreePath *path: the path of the selected item in the treeview (useless)
* Arguments: GtkTreeViewColumn *column: the column of the selected item in the treeview
* Arguments: struct arguments *argument: the global argument struct containing:
* -the playlist treeview (in order to insert songs there): treeview_playlist
* -an history of played songs: history
* -the struct argument itself to pass to start_song()
*/
void playlist_row_activated(GtkTreeView *, GtkTreePath *, GtkTreeViewColumn *, struct arguments *);
/**
* Description: Callback function called when the user selects (double-click/enter) a song in the playlist
* Arguments: GtkTreeView *treeview: the playlist treeview
* Arguments: GtkTreePath *path: the path of the selected item in the treeview (useless)
* Arguments: GtkTreeViewColumn *column: the column of the selected item in the treeview
* Arguments: struct arguments *argument: the global argument struct containing:
* -the playlist iter to fill with the selected iter: iter_playlist
* -the struct argument itself to pass to start_song()
*/
void artist_row_activated(GtkTreeView *, GtkTreePath *, GtkTreeViewColumn *, struct arguments *);
/** 
* Description: Callback function called when the user selects (double-click/enter) a song in the « artist » tab
* Arguments: GtkTreeview *treeview: the artist treeview
* Arguments: GtkTreePath *path: the path of the selected item in the treeview (its « level » in the treeview)
* Arguments: GtkTreeViewColumn *column: the column of the selected item in the treeview
* Arguments: struct arguments *argument: the global argument struct containing:
* -the playlist treeview (in order to insert songs there): treeview_playlist
* -the library treeview (in order to look up for songs): treeview_library
* -the artist iter to fill with the selected iter: iter_artist
* -the struct argument itself to pass to start_song(), add_album_to_playlist() and add_artist_to_playlist()
* -the playlist iter to fill with the first song played: iter_playlist
*/
void toggle_playpause_button(GtkWidget *, struct arguments *);
/**
* Description: Callback function called when the « next » button is clicked
* Arguments: GtkWidget *button: useless
* Arguments: struct arguments *argument: the global argument struct containing:
* -the playlist treeview (in order to get the next playlist song): treeview_playlist
* -the playlist iter, in order to add its path to the history: iter_playlist
* -the history list, in order to add the previous song to it: history
* -the struct argument istelf to pass to start_song()
*/
void previous_buttonf(GtkWidget *, struct arguments *);
void destroy(GtkWidget *, gpointer);
void config_folder_changed(char *, GtkWidget *);
void preferences_callback(GtkMenuItem *preferences, struct pref_arguments *);
void analyze_thread(struct pref_folder_arguments *);
void state_changed(GstBus *, GstMessage *, struct arguments *);
void slider_changed(GtkRange *, struct arguments *);
void volume_scale_changed(GtkScaleButton*, struct arguments *);
void refresh_ui(GstBus *, GstMessage *, struct arguments *);
void refresh_ui_mediainfo(GstBus *, GstMessage *, struct arguments *);
void ui_playlist_changed(GtkTreeModel *, GtkTreePath *, GtkTreeIter *, GtkNotebook *);
void toggle_lelele(GtkWidget *button, struct arguments *);
void toggle_random(GtkWidget *button, struct arguments *);
int bufferize(struct song, struct arguments *);
float distance(struct d4vector, struct d4vector);
void pause_song(struct arguments *);
void start_song(struct arguments *);
void resume_song(struct arguments *);
void play_song(struct arguments *);
void queue_song(struct arguments *);
void free_song(struct song *);
void explore(GDir *dir, char *folder, FILE *list);
void folder_chooser(GtkWidget *, struct pref_arguments *);
void display_library(GtkTreeView *, GtkListStore *);
void playlist_queue(GtkTreeIter *, GtkTreeModel *, GtkTreeView *, struct arguments *);
void get_playlist_song(GtkTreeView *, struct song *, struct arguments *);
void clean_playlist(GtkTreeView *, struct arguments *);
gboolean filter_vis_features(GstPluginFeature *, gpointer);
gboolean get_next_playlist_song(GtkTreeView *, struct arguments *);
gboolean get_random_playlist_song(GtkTreeView *, struct arguments *);
gboolean get_lelelerandom_playlist_song(GtkTreeView *, struct arguments *);
gboolean get_previous_playlist_song(GtkTreeView *, struct arguments *);
gboolean add_album_to_playlist(gchar *, gchar *, struct arguments *);
gboolean add_artist_to_playlist(gchar *, struct arguments *);
gboolean play_playlist_song(gchar *, struct arguments *); 
