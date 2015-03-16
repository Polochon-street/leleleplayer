#include <stdio.h>
#include "gui.h"

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

static gboolean endless(gpointer play_argument) {
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
		free(current_sample_array);
		rewind(file);
		pline = line;
		for(i = 0; i < g_random_int_range(0, count); ++i)
			fgets(line, 1000, file);

		for(;*pline != '\n';++pline)
			;
		*pline = '\0';
		resnum = analyze(line);
	}

	gtk_label_set_text((GtkLabel*)(((struct play_arguments*)play_argument)->label), line);
	status = 0;
	play(NULL, play_argument);
}

/* When a folder is selected, use that as the new location of the other chooser. */
static void folder_changed (GtkFileChooser *chooser1, struct changed_arguments *changed_argument) {
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
		free(current_sample_array);
		int rand = g_random_int_range(0, count);
		rewind(file);
		pline = line;
		for(i = 0; i < rand; ++i)
			fgets(line, 1000, file);

		for(;*pline != '\n';++pline)
			;
		*pline = '\0';
	
		resnum = analyze(line);
		gtk_label_set_text((GtkLabel*)changed_argument->label, line);
	}
}
/* When a file is selected, display the full path in the GtkLabel widget. */

static void file_changed (GtkFileChooser *chooser2, struct changed_arguments *changed_argument) {
	int resnum = 0;
	gchar *filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (chooser2));

	resnum = analyze(filename);
	gtk_adjustment_configure(changed_argument->adjust, 0, 0, duration, 1, 1, 1);
	gtk_adjustment_changed(changed_argument->adjust);

	if(resnum == 2) {
  		gtk_label_set_text ((GtkLabel*)changed_argument->label, "Can't Conclude");

	}
	else if(resnum == 0) {
  		gtk_label_set_text ((GtkLabel*)changed_argument->label, "Much loud");

	}
	else if(resnum == 1) {
  		gtk_label_set_text ((GtkLabel*)changed_argument->label, "Such calm");

	}
}

static timer_progressbar(gpointer play_argument) {
	gtk_adjustment_set_value (((struct play_arguments*)play_argument)->adjust, g_timer_elapsed(((struct play_arguments*)play_argument)->elapsed, NULL));
	gtk_adjustment_changed(((struct play_arguments*)play_argument)->adjust);
	g_source_remove(((struct play_arguments*)play_argument)->bartag);
	((struct play_arguments*)play_argument)->bartag = g_timeout_add_seconds(1, timer_progressbar, play_argument);
}

int play(GtkWidget *button, struct play_arguments *play_argument) {
	ALenum format;
	ALuint buffer;
	alGetSourcei(source, AL_SOURCE_STATE, &status);

	if(status == 0 || status == AL_STOPPED) {
		alDeleteBuffers(1, &buffer);
		alSourcei(source, AL_BUFFER, 0);
		alDeleteSources(1, &source);
		g_timer_start(play_argument->elapsed);
		alGenBuffers(1, &buffer);
		alGenSources(1, &source);

		format = AL_FORMAT_STEREO16;
		alBufferData(buffer, format, current_sample_array, nSamples * sizeof(ALint), sample_rate);

		alSourcei(source, AL_BUFFER, buffer);
		alGetSourcei(source, AL_SOURCE_STATE, &status);
		alSourcePlay(source);
		g_source_remove(play_argument->tag);
		if(play_argument->endless_check || play_argument->continue_count-- > 0)
			play_argument->tag = g_timeout_add_seconds(duration, endless, play_argument);
		play_argument->bartag = g_timeout_add_seconds(1, timer_progressbar, play_argument);
		return 0;
	}

	if(status == AL_PLAYING) {
		if(play_argument->endless_check) {
			g_timer_stop(play_argument->elapsed);
			g_source_remove(play_argument->tag);
		}
		alSourcePause(source);
		return 0;
	} 
	else {
		alSourcePlay(source);
		if(play_argument->endless_check) {
			g_timer_continue(play_argument->elapsed);
			play_argument->tag = g_timeout_add_seconds(duration - (int)g_timer_elapsed(play_argument->elapsed, NULL), endless, play_argument->label);
		}
		return 0;
	}
}



static void check_toggled (GtkToggleButton *check, int *endless_check) {
	*endless_check = !(*endless_check);
}

static void continue_check_toggled(GtkToggleButton *continue_check, GtkWidget *spin_int) {
	if(gtk_toggle_button_get_active(continue_check))
		gtk_widget_set_sensitive(spin_int, TRUE);
	else
		gtk_widget_set_sensitive(spin_int, FALSE);
}

static void continue_spin_count(GtkSpinButton *spin_int, int *continue_count) {
	*continue_count = gtk_spin_button_get_value(spin_int);
}

int main(int argc, char **argv) {

	struct play_arguments play_argument;
	struct play_arguments *pplay_argument = &play_argument;
	struct changed_arguments changed_argument;
	struct changed_arguments *pchanged_argument = &changed_argument;

	pplay_argument->label = pchanged_argument->label;
	pchanged_argument->elapsed = pplay_argument->elapsed;
	pchanged_argument->bartag = pplay_argument->bartag;	

	InitOpenAL();
	source = 0;
	pplay_argument->endless_check = 0;
	pchanged_argument->elapsed = pplay_argument->elapsed = g_timer_new();
	GtkWidget *window, *button, *chooser1, *chooser2, *vbox, *check, *continue_check, *button1, *button2, *spin_int, *progressbar;
	GtkFileFilter *filter1, *filter2;

	gtk_init(&argc, &argv);
	
	g_remove("list.txt");
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	
	pplay_argument->adjust = pchanged_argument->adjust = (GtkAdjustment*)gtk_adjustment_new(0, 0, 100, 1, 1, 1);

	#if GTK3
	progressbar = gtk_scale_new(GTK_ORIENTATION_HORIZONTAL, adjust);
	#endif
	progressbar = gtk_hscale_new(pchanged_argument->adjust);
	gtk_window_set_title(GTK_WINDOW(window), "lelele player");
	pchanged_argument->label = pplay_argument->label = gtk_label_new ("");
	gtk_container_set_border_width(GTK_CONTAINER(window), 25);
	gtk_widget_set_size_request(window, 500, 200);

	spin_int = gtk_spin_button_new_with_range(1, 10000, 1);
	button = gtk_button_new_with_mnemonic("_Play/Pause");
	button1 = gtk_button_new_with_mnemonic("test1");
	button2 = gtk_button_new_with_mnemonic("test2");
	check = gtk_check_button_new_with_label ("Endless mode.");
	continue_check = gtk_check_button_new_with_label("Play this number of songs.");


	g_signal_connect (G_OBJECT (window), "destroy", G_CALLBACK (destroy), NULL);
  	g_signal_connect (G_OBJECT(button), "clicked", G_CALLBACK(play), pplay_argument);
	g_signal_connect (G_OBJECT(check), "toggled", G_CALLBACK(check_toggled), &(pplay_argument->endless_check));
	g_signal_connect(G_OBJECT(continue_check), "toggled", G_CALLBACK(continue_check_toggled), spin_int);
	g_signal_connect(G_OBJECT(spin_int), "value-changed", G_CALLBACK(continue_spin_count), &(pplay_argument->continue_count));


  /* Create two buttons, one to select a folder and one to select a file. */

	chooser1 = gtk_file_chooser_button_new ("Chooser a Folder", GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER);
	chooser2 = gtk_file_chooser_button_new ("Chooser a Folder", GTK_FILE_CHOOSER_ACTION_OPEN);

  /* Monitor when the selected folder or file are changed. */
	g_signal_connect (G_OBJECT (chooser2), "selection_changed", G_CALLBACK (file_changed), pchanged_argument);
	g_signal_connect (G_OBJECT (chooser1), "selection_changed", G_CALLBACK(folder_changed), pchanged_argument);
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


	gtk_widget_set_sensitive(spin_int, FALSE);
	vbox = gtk_vbox_new (GTK_ORIENTATION_VERTICAL, 5);
	gtk_box_pack_start_defaults (GTK_BOX (vbox), chooser1);
	gtk_box_pack_start_defaults (GTK_BOX (vbox), chooser2);
	gtk_box_pack_start_defaults (GTK_BOX (vbox), button);
	gtk_box_pack_start_defaults (GTK_BOX (vbox), pchanged_argument->label);
	gtk_box_pack_start_defaults (GTK_BOX (vbox), check);
	gtk_box_pack_start_defaults (GTK_BOX(vbox), progressbar);
	gtk_box_pack_start_defaults (GTK_BOX(vbox), continue_check);
	gtk_box_pack_start_defaults (GTK_BOX(vbox), spin_int);



#ifdef GTK3
	vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 5);
 	gtk_box_pack_start (GTK_BOX (vbox), chooser1, TRUE, TRUE, 3);
	gtk_box_pack_start (GTK_BOX (vbox), chooser2, TRUE, TRUE, 3);
	gtk_box_pack_start (GTK_BOX (vbox), button, TRUE, TRUE, 3);
	gtk_box_pack_start (GTK_BOX (vbox), pchanged_argument->label, TRUE, TRUE, 3);
	gtk_box_pack_start (GTK_BOX (vbox), check, TRUE, TRUE, 3);
	gtk_box_pack_start (GTK_BOX(vbox), progressbar, TRUE, TRUE, 3);
#endif


	gtk_container_add (GTK_CONTAINER (window), vbox);
	
	gtk_widget_show_all(window);

	gtk_main();
	return 0;
}
