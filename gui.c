#include <stdio.h>
#include "gui.h"

int tag;
GTimer *elapsed;

bool InitOpenAL(void) {
	ALCdevice *Device = alcOpenDevice(NULL);
	if(!Device) {
		printf("Error while opening the device\n");
		exit(1);
	}
	
	ALCcontext *Context = alcCreateContext(Device, NULL);
	if(!Context) {
		printf("Error while creating context\n");
		exit(1);
	}

	if(!alcMakeContextCurrent(Context)) {
		printf("Error while activating context\n");
		exit(1);
	}
	return 0;
}

void ShutdownOpenAL(void) {
	ALCcontext *Context = alcGetCurrentContext();
	ALCdevice *Device = alcGetContextsDevice(Context);

	alcMakeContextCurrent(NULL);
	alcDestroyContext(Context);
	alcCloseDevice(Device);
}

static void destroy (GtkWidget *window, gpointer data) {
	ShutdownOpenAL();
	gtk_main_quit ();
}

void explore(GDir *dir, char* folder) {
	const gchar *file;
	FILE *liste;

	liste = fopen("list.txt", "a");
	while((file = g_dir_read_name(dir))) {
		if(g_str_has_suffix(file, "flac") || g_str_has_suffix(file, "mp3")) {
			g_fprintf(liste, "%s\n", g_build_path("/", folder, file, NULL));
		}
		else {
		//	printf("%s\n", g_build_path("/", folder, file, NULL));
			explore(g_dir_open(g_build_path("/", folder, file, NULL), 0, NULL), g_build_path("/", folder, file, NULL));
		}
	}
	fclose(liste);
}

static gboolean endless(gpointer label) {
	float resnum = 0;
	
	FILE *file;
	char line[1000];
	int i;
	int count;
	int rand;
	char *pline;
	
	pline = line;

	file = fopen("list.txt", "r");
	
	for(; fgets(line, 1000, file) != NULL; ++count)
		;

	while(resnum != 1) {
		rewind(file);
		pline = line;
		for(i = 0; i < g_random_int_range(0, count); ++i)
			fgets(line, 1000, file);

		for(;*pline != '\n';++pline)
			;
		*pline = '\0';
		resnum = analyze(line);
	}

	gtk_label_set_text((GtkLabel*)label, line);
	status = 0;
	play(NULL, label);
}
/* When a folder is selected, use that as the new location of the other chooser. */
static void folder_changed (GtkFileChooser *chooser1, GtkLabel *label) {
	float resnum = 0;
	gchar *folder = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (chooser1));
	GDir *dir = g_dir_open (folder, 0, NULL);
	explore(dir, folder);

	FILE *file;
	char line[1000];
	int i;
	int count;
	int rand;
	char *pline;
	
	pline = line;

	file = fopen("list.txt", "r");
	
	for(; fgets(line, 1000, file) != NULL; ++count)
		;

	while(resnum != 1) {
		rewind(file);
		pline = line;
		for(i = 0; i < g_random_int_range(0, count); ++i)
			fgets(line, 1000, file);

		for(;*pline != '\n';++pline)
			;
		*pline = '\0';
	
		resnum = analyze(line);
		gtk_label_set_text(label, line);
	}
}
/* When a file is selected, display the full path in the GtkLabel widget. */

static void file_changed (GtkFileChooser *chooser2, GtkLabel *label) {
	int resnum = 0;
	gchar *filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (chooser2));

	resnum = analyze(filename);
	if(resnum == 2) {
  		gtk_label_set_text (label, "Can't Conclude");

	}
	else if(resnum == 0) {
  		gtk_label_set_text (label, "Much loud");

	}
	else if(resnum == 1) {
  		gtk_label_set_text (label, "Such calm");

	}
}

int play(GtkWidget *button, GtkLabel *label) {
	ALenum format;
	ALuint buffer;
	alGetSourcei(source, AL_SOURCE_STATE, &status);

	if(status == 0 || status == AL_STOPPED) {
		g_timer_start(elapsed);
		alGenBuffers(1, &buffer);
		alGenSources(1, &source);

		format = AL_FORMAT_STEREO16;
		alBufferData(buffer, format, current_sample_array, nSamples * sizeof(ALint), sample_rate);

		alSourcei(source, AL_BUFFER, buffer);
		alGetSourcei(source, AL_SOURCE_STATE, &status);
		alSourcePlay(source); // finally playing the track!
		g_source_remove(tag);
		tag = g_timeout_add_seconds(duration, endless, label);
		return 0;
	}

	if(status == AL_PLAYING) {
		g_timer_stop(elapsed);
		g_source_remove(tag);
		alSourcePause(source);
		return 0;
	} 
	else {
		g_timer_continue(elapsed);
		alSourcePlay(source);
		tag = g_timeout_add_seconds(duration - (int)g_timer_elapsed(elapsed, NULL), endless, label);
		return 0;
	}
}

int main(int argc, char **argv) {
	InitOpenAL();
	source =0;
	elapsed = g_timer_new();
	GtkWidget *window, *button, *chooser1, *chooser2, *vbox, *label;
	GtkFileFilter *filter1, *filter2;

	gtk_init(&argc, &argv);
	
//	g_remove("list.txt");
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(window), "lelele player");
	label = gtk_label_new ("");
	gtk_container_set_border_width(GTK_CONTAINER(window), 25);
	gtk_widget_set_size_request(window, 500, 200);

	button = gtk_button_new_with_mnemonic("_Play/Pause");
	g_signal_connect (G_OBJECT (window), "destroy", G_CALLBACK (destroy), NULL);
  	g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(play), (gpointer)label);

  /* Create two buttons, one to select a folder and one to select a file. */

	chooser1 = gtk_file_chooser_button_new ("Chooser a Folder", GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER);
	chooser2 = gtk_file_chooser_button_new ("Chooser a Folder", GTK_FILE_CHOOSER_ACTION_OPEN);

  /* Monitor when the selected folder or file are changed. */
	g_signal_connect (G_OBJECT (chooser2), "selection_changed", G_CALLBACK (file_changed), (gpointer) label);
	g_signal_connect (G_OBJECT (chooser1), "selection_changed", G_CALLBACK(folder_changed), (gpointer)label);
  /* Set both file chooser buttons to the location of the user's home directory. */
	gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (chooser2), g_get_home_dir());

	gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (chooser1), g_get_home_dir());
  /* Provide a filter to show all files and one to shown only 3 types of images. */
	filter1 = gtk_file_filter_new ();
	filter2 = gtk_file_filter_new ();
	gtk_file_filter_set_name (filter1, "Audio Files");
	gtk_file_filter_set_name (filter2, "All Files");
	gtk_file_filter_add_pattern (filter1, "*.mp3");
	gtk_file_filter_add_pattern (filter1, "*.flac");
	gtk_file_filter_add_pattern (filter1, "*.aac");
	gtk_file_filter_add_pattern (filter2, "*");

  /* Add the both filters to the file chooser button that selects files. */
	gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (chooser2), filter1);
	gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (chooser2), filter2);

	vbox = gtk_vbox_new (FALSE, 5);

 	gtk_box_pack_start_defaults (GTK_BOX (vbox), chooser1);
	gtk_box_pack_start_defaults (GTK_BOX (vbox), chooser2);
	gtk_box_pack_start_defaults (GTK_BOX (vbox), button);
	gtk_box_pack_start_defaults (GTK_BOX (vbox), label);

	gtk_container_add (GTK_CONTAINER (window), vbox);
	
	gtk_widget_show_all(window);
	gtk_main();
	free(current_sample_array);
	return 0;
}
