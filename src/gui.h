#include <stdbool.h>
#include <gtk/gtk.h>
#include <glib/gprintf.h>
#include <glib/gerror.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <glib/gstdio.h>
#include <glib.h>
#include <limits.h>
#define GST_USE_UNSTABLE_API
#include <gst/gst.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <libxml/xmlwriter.h>
#include <bliss.h>

#ifdef linux
#include <X11/Xlib.h>
#endif

enum {
	COLUMN_ARTIST = 0,
	NUM_COLS_ARTIST
};

enum {
	COLUMN_ALBUM = 0,
	NUM_COLS_ALBUM
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
	struct bl_song current_song;
	GstElement *playbin;
	GstElement *pipeline;
	GstElement *filesrc;
	GstElement *socketsrc;
	GstElement *decode;
	GstElement *sink;
	GstState state;
	gint64 duration;
	gint64 elapsed;
	int bartag;
	int playlist_count;
	gulong progressbar_update_signal_id;
	gulong tags_update_signal_id;
	gulong playlist_update_signal_id;
	gulong time_spin_update_signal_id;
	GList *history;
	GtkWidget *treeview_library;
	GtkWidget *treeview_artist;
	GtkWidget *treeview_album;
	GtkWidget *progressbar;
	GtkWidget *treeview_playlist;
	GtkWidget *playpause_button;
	GtkTreePath *path;
	GtkTreeRowReference *row_playlist;
	GtkTreeIter iter_library;
	GtkTreeIter iter_artist;
	GtkTreeIter iter_album;
	GtkListStore *store_library;
	GtkListStore *store_playlist;
	GtkTreeStore *store_artist;
	GtkTreeStore *store_album;
	GtkTreeViewColumn *column;
	GtkTreeModelFilter *library_filter, *artist_filter, *album_filter;
	gchar *search_entry_text;
	GtkWidget *album_label, *title_label, *artist_label;
	GtkWidget *genre_label, *samplerate_label, *bitrate_label, *channels_label;
	gchar *str_genre, *str_samplerate, *str_channels, *str_bitrate, *str_artist,
		*str_album, *str_title;
	GtkAdjustment *adjust;
	GtkWidget *volume_scale;
	GtkWidget *video_window;
	GTimer *sleep_timer;
	GtkWidget *time_spin;
	GCond queue_cond;
	GMutex queue_mutex;
	GtkWidget *libnotebook;
	GtkWidget *time_checkbox;
	gdouble timer_delay;
	gdouble vol;
	gchar *lib_path;
	GSettings *preferences;
	gboolean network_mode_set;
	GSocketConnection *connection_remote;
	const gchar *lllserver_address_char;
};

struct pref_arguments {
    GtkWidget *window;
    GtkWidget *treeview;
    GtkWidget *treeview_artist;
    GtkWidget *treeview_album;
	gboolean is_configured;
	gboolean lelele_scan;
	gboolean network_mode_set;
	const gchar *folder;
	const gchar *lllserver_address_char;
	int count;
	int nblines;
	gchar *lib_path;
    GtkListStore *store_library;
    GtkTreeStore *store_album;
    GtkTreeStore *store_artist;
	GtkWidget *library_entry, *spinner, *progress_label, *network_entry;
	GtkTreeModelSort *library_sort, *artist_sort, *album_sort;
	GtkTreeModelFilter *library_filter, *artist_filter, *album_filter;
	GSettings *preferences;
	gboolean terminate;
	gboolean erase;
	GAsyncQueue *msg_queue;
	GstElement *playbin;
};

struct tab_label {
	gchar *str;
	GtkWidget *label;
};

void setup_tree_view_renderer_play_lib(GtkWidget *);
/**
* Description: Sets up the treeview renderer with columns "playing", "tracknumber", "track",
* "album", "artist", "force", and sizes it properly
* Arguments: GtkWidget *treeview: The TreeView to set up
*/
void setup_tree_view_renderer_artist(GtkWidget *);
/**
* Description: Sets up the treeview renderer like this: artist->album->songs
* Arguments: GtkWidget *treeview: the TreeView to set up
*/
void setup_tree_view_renderer_album(GtkWidget *);
/**
* Description: Sets up the treeview renderer like this: album->songs
* Arguments: GtkWidget *treeview: the TreeView to set up
*/
void continue_track_cb(GstElement *, struct arguments *);
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
void lib_row_activated_cb(GtkTreeView *, GtkTreePath *, GtkTreeViewColumn *, struct arguments *);
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
void playlist_row_activated_cb(GtkTreeView *, GtkTreePath *, GtkTreeViewColumn *, struct arguments *);
/**
* Description: Callback function called when the user selects (double-click/enter) a song in the playlist
* Arguments: GtkTreeView *treeview: the playlist treeview
* Arguments: GtkTreePath *path: the path of the selected item in the treeview (useless)
* Arguments: GtkTreeViewColumn *column: the column of the selected item in the treeview
* Arguments: struct arguments *argument: the global argument struct containing:
* -the playlist iter to fill with the selected iter: iter_playlist
* -the struct argument itself to pass to start_song()
*/
void artist_row_activated_cb(GtkTreeView *, GtkTreePath *, GtkTreeViewColumn *, struct arguments *);
/** 
* Description: Callback function called when the user selects (double-click/enter) a song in the « artist » tab
* Arguments: GtkTreeview *treeview: the artist treeview
* Arguments: GtkTreePath *path: the path of the selected item in the treeview (its « level » in the treeview)
* Arguments: GtkTreeViewColumn *column: the column of the selected item in the treeview
* Arguments: struct arguments *argument: the global argument struct containing:
* -the playlist treeview in order to insert songs there: treeview_playlist
* -the library treeview in order to look up for songs: treeview_library
* -the artist iter to fill with the selected iter: iter_artist
* -the struct argument itself to pass to start_song(), add_album_to_playlist() and add_artist_to_playlist()
* -the playlist iter to fill with the first song played: iter_playlist
*/
void album_row_activated_cb(GtkTreeView *, GtkTreePath *, GtkTreeViewColumn *, struct arguments *);
void toggle_playpause_button_cb(GtkWidget *, struct arguments *);
/**
* Description: Callback function called when the « previous » button is clicked
* Arguments: GtkWidget *button: useless
* Arguments: struct arguments *argument: the global argument struct containing:
* -the playlist treeview in order to get the next playlist song: treeview_playlist
* -the playlist iter, in order to add its path to the history: iter_playlist
* -the history list, in order to add the previous song to it: history
* -the current song struct, in order to free it and reallocate it
* -the struct argument istelf to pass to start_song()
*/
void next_buttonf_cb(GtkWidget *, struct arguments *);
/**
* Description: Callback function called when the « previous » button is clicked
* Arguments: GtkWidget *button: useless
* Arguments: struct arguments *argument: the global argument struct containing:
* -the playlist treeview (in order to get the previous playlist song with get_previous_playlist_song(): treeview_playlist)
* -the current song struct, in order to free it and reallocate it
* -the struct argument itself to pass to start_song() and get_next_playlist_song()
*/
void previous_buttonf_cb(GtkWidget *, struct arguments *);
/**
* Description: Callback function called when trying to quit the player
* Arguments: struct pref_arguments *argument: the global preferences argument struct containing:
* -the playbin, in order to set its state to NULL, which is necessary when quitting: current_song.playbin
* -
*/
void destroy_cb(GtkWidget *, struct pref_arguments *);
/**
* Description: Callback function called when the « file->add file to playlist » button is clicked: adds an audio file to the playlist
* Arguments: GtkMenuItem *add_file: the button, used for getting the toplevel window for gtk_file_chooser_dialog_new()
* Arguments: struct arguments *argument: the global argument struct containing:
* -treeview_playlist, the playlist treeview, used to get the playlist model
* -iter_playlist: the playlist iter, in order to add the created iter to the playlist treeview (and add the first to the history)
* -store_playlist: the playlist GtkTreeStore, used to set the struct bl_song variables
* -playlist_count: the playlist count to increment for each song
* Arguments: gboolean erase: boolean in order ton know if the library folder has changed and must be erased or not
*/
void add_file_to_playlist_cb(GtkMenuItem *open, struct arguments *);
/**
* Description: Callback function called when the « file->Open... » button is clicked: plays an audio file 
* Arguments: GtkMenuItem *open: the button, used for getting the toplevel window for gtk_file_chooser_dialog_new()
* Arguments: struct arguments *argument: the global argument struct containing: 
* -treeview_playlist, the playlist treeview, used to get the playlist model
* -iter_playlist: the playlist iter, in order to add the created iter to the playlist treeview, and add it to the history
* -store_playlist: the playlist GtkTreeStore, used to set the struct bl_song variables
* -playlist_count: the playlist count to increment
*/
void open_audio_file_cb(GtkMenuItem *, struct arguments *);
void preferences_callback_cb(GtkMenuItem *, struct pref_arguments *);
void analyze_thread(struct pref_arguments *);
gboolean refresh_config_progressbar(struct pref_arguments *argument);
void state_changed_cb(GstBus *, GstMessage *, struct arguments *);
void slider_changed_cb(GtkRange *, struct arguments *);
void message_application_cb(GstBus *, GstMessage *, struct arguments *);
void tags_obtained_cb(GstElement *, gint, struct arguments *);
void volume_scale_changed_cb(GtkScaleButton*, gdouble, struct arguments *);
void refresh_ui_cb(GstBus *, GstMessage *, struct arguments *);
void refresh_ui_mediainfo(GstBus *, GstMessage *, struct arguments *);
void search_cb(GtkSearchEntry *, struct arguments *);
void end_of_playlist_cb(GstBus *, GstMessage *, struct arguments *);
void ui_playlist_changed_cb(GtkTreeModel *, GtkTreePath *, GtkTreeIter *, GtkNotebook *);
void toggle_lelele_cb(GtkWidget *button, struct arguments *);
void toggle_repeat_cb(GtkWidget *button, struct arguments *);
void toggle_random_cb(GtkWidget *button, struct arguments *);
void toggle_lelelescan_cb(GtkWidget *button, struct pref_arguments *);
void toggle_network_mode_cb(GtkWidget *button, struct pref_arguments *);
void changed_page_notebook_cb(GtkNotebook *, GtkWidget *, guint, gpointer data);
void remove_playlist_selection_from_playlist_cb(GtkWidget *, struct arguments *);
gboolean playlist_del_button_cb(GtkWidget *, GdkEventKey *, struct arguments *);
gboolean treeviews_right_click_cb(GtkWidget *, GdkEventButton *, struct arguments *);
void artist_popup_menu(GtkWidget *, GdkEventButton *event, struct arguments *);
void playlist_popup_menu(GtkWidget *, GdkEventButton *event, struct arguments *);
void library_popup_menu(GtkWidget *, GdkEventButton *event, struct arguments *);
void album_popup_menu(GtkWidget *, GdkEventButton *event, struct arguments *);
void add_library_selection_to_playlist_cb(GtkWidget *, struct arguments *);
void add_artist_selection_to_playlist_cb(GtkWidget *, struct arguments *);
void add_album_selection_to_playlist_cb(GtkWidget *, struct arguments *);
int bufferize(struct bl_song, struct arguments *);
void pause_song(struct arguments *);
void start_song(struct arguments *);
void resume_song(struct arguments *);
void play_song(struct arguments *);
void queue_song(struct arguments *);
void free_song(struct bl_song *);
void explore(GDir *dir, const gchar *folder, GList *list);
void folder_chooser_cb(GtkWidget *, struct pref_arguments *);
void display_library(GtkTreeView *, GtkListStore *, gchar *libfile);
void playlist_queue(GtkTreeIter *, GtkTreeModel *, GtkTreeView *, struct arguments *);
void get_playlist_song(GtkTreeView *, struct bl_song *, struct arguments *);
void clean_playlist(GtkTreeView *, struct arguments *);
void reset_ui(struct arguments *);
void display_album_tab(GtkWidget *, GtkTreeStore *, GtkTreeModel *, GtkTreeModelSort *, GtkTreeModelFilter *);
void display_artist_tab(GtkWidget *, GtkTreeStore *, GtkTreeModel *, GtkTreeModelSort *, GtkTreeModelFilter *);
void add_entry_artist_tab(GtkWidget *treeview, GtkTreeStore *treestore, GtkTreeModel *model_library, GtkTreeIter *);
void add_entry_album_tab(GtkWidget *treeview, GtkTreeStore *treestore, GtkTreeModel *model_library, GtkTreeIter *);
void time_checkbox_toggled_cb(GtkToggleButton *, struct arguments *);
float distance(struct force_vector_s, struct force_vector_s);
float cosine_distance(struct force_vector_s, struct force_vector_s);
void update_tab_label(GtkTreeModel *, GtkTreePath *, struct tab_label *);
void update_tab_label_a(GtkTreeModel *, GtkTreePath *, GtkTreeIter *, struct tab_label *);
gboolean lib_right_click(GtkWidget *, GdkEventButton *, struct arguments *);
gboolean artist_right_click(GtkWidget *, GdkEventButton *, struct arguments *);
gboolean playlist_right_click(GtkWidget *, GdkEventButton *, struct arguments *);
gboolean filter_vis_features(GstPluginFeature *, gpointer);
gboolean get_next_playlist_song(GtkTreeView *, struct arguments *);
gboolean get_random_playlist_song(GtkTreeView *, struct arguments *);
gboolean get_lelelerandom_playlist_song(GtkTreeView *, struct arguments *);
gboolean get_previous_playlist_song(GtkTreeView *, struct arguments *);
gboolean add_album_to_playlist(gchar *, gchar *, struct arguments *);
gboolean add_artist_to_playlist(gchar *, struct arguments *);
gboolean play_playlist_song(gchar *, struct arguments *); 
gboolean ui_init(struct arguments *);
gint sort_iter_compare_func(GtkTreeModel *, GtkTreeIter *, GtkTreeIter *, gpointer); 
gint sort_artist_album_tracks(GtkTreeModel *, GtkTreeIter *, GtkTreeIter *, gpointer);
gint sort_album_tracks(GtkTreeModel *, GtkTreeIter *, GtkTreeIter *, gpointer);
gint sort_force(GtkTreeModel *, GtkTreeIter *, GtkTreeIter *, gpointer);
gint sort_text(GtkTreeModel *, GtkTreeIter *, GtkTreeIter *, gpointer);
gint time_spin_output_cb(GtkSpinButton *, struct arguments *);
gint time_spin_input_cb(GtkSpinButton *, gdouble *, struct arguments *);
void time_spin_changed_cb(GtkSpinButton *, struct arguments *);
void toggle_playpause(struct arguments *);
gboolean lib_right_click(GtkWidget *, GdkEventButton *, struct arguments *);
gboolean filter_library(GtkTreeModel *, GtkTreeIter *, struct arguments *);
gboolean filter_artist(GtkTreeModel *, GtkTreeIter *, struct arguments *);
gboolean filter_album(GtkTreeModel *, GtkTreeIter *, struct arguments *);
int nb_rows_treeview(GtkTreeModel *);
gboolean tree_row_reference_get_iter(GtkTreeRowReference *, GtkTreeIter *);
GtkTreeRowReference *tree_iter_get_row_reference(GtkTreeModel *, GtkTreeIter *);
void remote_lllp_connected_cb(GObject *listener, GAsyncResult *res, gpointer pref_arguments);
void connection_established_lllserver_cb(GObject *, GAsyncResult *, gpointer);
void quit_lllserver_cb(GObject *, GAsyncResult *, gpointer);
void pad_added_handler_cb(GstElement *, GstPad *, gpointer);
void source_setup_cb(GstElement *, GstElement *, GSocketConnection **); 
gboolean uri_socket_src_plugin_init(GstPlugin *);
