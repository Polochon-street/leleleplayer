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

void explore(GDir *dir, char *folder, FILE *list) {
	gchar file[1000];
	
	while((g_stpcpy(file, g_dir_read_name(dir)))) {
		if( g_file_test(g_build_path("/", folder, file, NULL), G_FILE_TEST_IS_REGULAR) && ( g_str_has_suffix(file, "flac") || g_str_has_suffix(file, "mp3") ) ) {
			g_fprintf(list, "%s\n", g_build_path("/", folder, file, NULL));
		}
		else {
			explore(g_dir_open(g_build_path("/", folder, file, NULL), 0, NULL), g_build_path("/", folder, file, NULL), list);
		}
	}
}

static gboolean endless(gpointer argument) {
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
		int rand = g_random_int_range(0, count);
		rewind(file);
		pline = line;
		for(i = 0; i < rand; ++i)
			fgets(line, 1000, file);

		for(;*pline != '\n';++pline)
			;
		*pline = '\0';
		resnum = analyze(line);
	}

	gtk_label_set_text((GtkLabel*)(((struct arguments*)argument)->label), line);
	g_timer_reset(((struct arguments*)argument)->elapsed);
	((struct arguments*)argument)->offset = 0;
	gtk_adjustment_configure(((struct arguments*)argument)->adjust, 0, 0, duration, 1, 1, 1);
	gtk_adjustment_changed(((struct arguments*)argument)->adjust);

	if(((struct arguments*)argument)->continue_count-- > 0 || ((struct arguments*)argument)->endless_check) {
		play(NULL, argument);
	}
}

/* When a folder is selected, use that as the new location of the other chooser. */
static void folder_changed (GtkFileChooser *chooser1, struct arguments *argument) {
	gchar *folder = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (chooser1));
	GDir *dir = g_dir_open (folder, 0, NULL);
	FILE *list;
	list = fopen("list.txt", "w+");
	explore(dir, folder, list);
	float resnum = 0;
	char line[1000];
	int i;
	int count, tmpcount;
	int rand;
	char *pline;
	
	rewind(list);
	for(; fgets(line, 1000, list) != NULL; ++count)
		;
	
	tmpcount = count;
	
	while(resnum != 1 && tmpcount-- > 0) {
		free(current_sample_array);
		int rand = g_random_int_range(0, count);
		rewind(list);
		
		for(i = 0; i < rand; ++i)
			fgets(line, 1000, list);
		line[strcspn(line, "\n")] = '\0';
		
		resnum = analyze(line);
	}

	if (tmpcount > 0) {
	gtk_label_set_text((GtkLabel*)(((struct arguments*)argument)->label), line);
	gtk_adjustment_configure(((struct arguments*)argument)->adjust, 0, 0, duration, 1, 1, 1);
	((struct arguments*)argument)->offset = 0;
	gtk_adjustment_changed(((struct arguments*)argument)->adjust);
	}
	else
		gtk_label_set_text((GtkLabel*)(((struct arguments*)argument)->label), "No calm songs found!");


}
/* When a file is selected, display the full path in the GtkLabel widget. */

static void next (GtkButton *next_button, struct arguments *argument) {
	if (argument->continue_count < 0) {
		argument->continue_count = 1;
		alSourceStop(source);
	}
	else {
		argument->continue_count++; 
		alSourceStop(source);
	}
	endless(argument);
}

static void file_changed (GtkFileChooser *chooser2, struct arguments *argument) {
	int resnum = 0;
	gchar *filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (chooser2));

	resnum = analyze(filename);
	gtk_adjustment_configure(argument->adjust, 0, 0, duration, 1, 1, 1);
	gtk_adjustment_changed(argument->adjust);

	if(resnum == 2) {
  		gtk_label_set_text ((GtkLabel*)argument->label, "Can't Conclude");

	}
	else if(resnum == 0) {
  		gtk_label_set_text ((GtkLabel*)argument->label, "Much loud");

	}
	else if(resnum == 1) {
  		gtk_label_set_text ((GtkLabel*)argument->label, "Such calm");

	}
}

static timer_progressbar(gpointer argument) {
	alGetSourcei(source, AL_SOURCE_STATE, &status);
	if(status == AL_PLAYING) {
		gtk_adjustment_set_value (((struct arguments*)argument)->adjust, g_timer_elapsed(((struct arguments*)argument)->elapsed, NULL) + ((struct arguments*)argument)->offset);
		gtk_adjustment_changed(((struct arguments*)argument)->adjust);
		g_source_remove(((struct arguments*)argument)->bartag);
		((struct arguments*)argument)->bartag = g_timeout_add_seconds(1, timer_progressbar, argument);
	}
}

int play(GtkWidget *button, struct arguments *argument) {

	ALenum format;
	ALuint buffer;
	alGetSourcei(source, AL_SOURCE_STATE, &status);

	if(argument->first == 1)
		InitOpenAL();

	if(status == 0 || status == AL_STOPPED) {
		argument->first = 0;
		alDeleteBuffers(1, &buffer);
		alSourcei(source, AL_BUFFER, 0);
		alDeleteSources(1, &source);
		g_timer_start(argument->elapsed);
		alGenBuffers(1, &buffer);
		alGenSources(1, &source);

		format = AL_FORMAT_STEREO16;
		alBufferData(buffer, format, current_sample_array, nSamples * sizeof(ALint), sample_rate);

		alSourcei(source, AL_BUFFER, buffer);
		alSourcePlay(source);
		g_source_remove(argument->tag);
		if(argument->endless_check || argument->continue_count > 0) {
			argument->tag = g_timeout_add_seconds(duration, endless, argument);
		}
		argument->bartag = g_timeout_add_seconds(1, timer_progressbar, argument);
		return 0;
	}

	if(status == AL_PLAYING) {
		if(argument->endless_check) {
			g_timer_stop(argument->elapsed);
			g_source_remove(argument->tag);
		}
		alSourcePause(source);
		return 0;
	} 
	else{
		alSourcePlay(source);
		if(argument->endless_check) {
			g_timer_continue(argument->elapsed);
			argument->tag = g_timeout_add_seconds(duration - (int)g_timer_elapsed(argument->elapsed, NULL), endless, argument->label);
		}
		return 0;
	}
}

static void check_toggled (GtkToggleButton *check, struct arguments *argument) {
	if(status == AL_PLAYING && argument->endless_check) {
		g_source_remove(argument->tag);
		argument->tag = g_timeout_add_seconds(duration - (int)g_timer_elapsed(argument->elapsed, NULL), endless, argument);
	}
	argument->endless_check = !(argument->endless_check);
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

static void slider_changed(GtkRange *progressbar, struct arguments *argument) {
	if(fabs(gtk_adjustment_get_value(argument->adjust) - g_timer_elapsed(argument->elapsed, NULL) - argument->offset) > 0.005f) {
		argument->offset = gtk_adjustment_get_value(argument->adjust);
		alSourcei(source, AL_BUFFER, 0);
		alDeleteSources(1, &source);
		ALenum format;
		ALuint buffer;
		alSourcei(source, AL_BUFFER, 0);
		g_timer_start(argument->elapsed);
		alGenBuffers(1, &buffer);
		alGenSources(1, &source);
		format = AL_FORMAT_STEREO16;
		alBufferData(buffer, format, current_sample_array + channels * (int) ((gdouble) (nb_bytes_per_sample * sample_rate) * (gdouble) gtk_adjustment_get_value(argument->adjust)), (nSamples - (int) ( (gdouble) gtk_adjustment_get_value(argument->adjust) * (gdouble) sample_rate )) * sizeof(ALint), sample_rate);
		alSourcei(source, AL_BUFFER, buffer);
		alGetSourcei(source, AL_SOURCE_STATE, &status);
		alSourcePlay(source);
		g_source_remove(argument->tag);
	
		if(argument->endless_check || argument->continue_count > 0)
			argument->tag = g_timeout_add_seconds(duration - (int)gtk_adjustment_get_value(argument->adjust), endless, argument);
		argument->bartag = g_timeout_add_seconds(1, timer_progressbar, argument);
	}
}

int main(int argc, char **argv) {

	struct arguments argument;
	struct arguments *pargument = &argument;
	
	//InitOpenAL(); // potential memory leak
	source = 0;
	pargument->first = 1;
	pargument->endless_check = 0;	
	pargument->continue_count = -1;
	pargument->offset = 0;
	pargument->elapsed = g_timer_new();
	GtkWidget *window, *button, *chooser1, *chooser2, *vbox, *check, *continue_check, *next_button, *button2, *spin_int, *progressbar;
	GtkFileFilter *filter1, *filter2;

	gtk_init(&argc, &argv);
	
	//g_remove("list.txt");
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	
	pargument->adjust = (GtkAdjustment*)gtk_adjustment_new(0, 0, 100, 1, 1, 1);

	#if GTK3
	progressbar = gtk_scale_new(GTK_ORIENTATION_HORIZONTAL, adjust);
	#endif
	progressbar = gtk_hscale_new(pargument->adjust);
	gtk_scale_set_draw_value ((GtkScale*)progressbar, FALSE);
	gtk_window_set_title(GTK_WINDOW(window), "lelele player");
	pargument->label = pargument->label = gtk_label_new ("");
	gtk_container_set_border_width(GTK_CONTAINER(window), 25);
	gtk_widget_set_size_request(window, 500, 500);

	spin_int = gtk_spin_button_new_with_range(1, 10000, 1);
	button = gtk_button_new_with_mnemonic("_Play/Pause");
	next_button = gtk_button_new_with_mnemonic("Next");
	button2 = gtk_button_new_with_mnemonic("test2");
	check = gtk_check_button_new_with_label ("Endless mode.");
	continue_check = gtk_check_button_new_with_label("Play this number of songs.");


	g_signal_connect (G_OBJECT (window), "destroy", G_CALLBACK (destroy), NULL);
  	g_signal_connect (G_OBJECT(button), "clicked", G_CALLBACK(play), pargument);
	g_signal_connect (G_OBJECT(next_button), "clicked", G_CALLBACK(next), pargument);
	g_signal_connect (G_OBJECT(check), "toggled", G_CALLBACK(check_toggled), pargument);
	g_signal_connect(G_OBJECT(continue_check), "toggled", G_CALLBACK(continue_check_toggled), spin_int);
	g_signal_connect(G_OBJECT(spin_int), "value-changed", G_CALLBACK(continue_spin_count), &(pargument->continue_count));
	g_signal_connect(G_OBJECT(progressbar), "value-changed", G_CALLBACK(slider_changed), pargument);


  /* Create two buttons, one to select a folder and one to select a file. */

	chooser1 = gtk_file_chooser_button_new ("Chooser a Folder", GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER);
	chooser2 = gtk_file_chooser_button_new ("Chooser a Folder", GTK_FILE_CHOOSER_ACTION_OPEN);

  /* Monitor when the selected folder or file are changed. */
	g_signal_connect (G_OBJECT (chooser2), "selection_changed", G_CALLBACK (file_changed), pargument);
	g_signal_connect (G_OBJECT (chooser1), "selection_changed", G_CALLBACK(folder_changed), pargument);

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
	gtk_box_pack_start_defaults (GTK_BOX (vbox), pargument->label);
	gtk_box_pack_start_defaults (GTK_BOX (vbox), check);
	gtk_box_pack_start_defaults (GTK_BOX(vbox), progressbar);
	gtk_box_pack_start_defaults (GTK_BOX(vbox), continue_check);
	gtk_box_pack_start_defaults (GTK_BOX(vbox), spin_int);
	gtk_box_pack_start_defaults (GTK_BOX(vbox), next_button);


#ifdef GTK3
	vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 5);
 	gtk_box_pack_start (GTK_BOX (vbox), chooser1, TRUE, TRUE, 3);
	gtk_box_pack_start (GTK_BOX (vbox), chooser2, TRUE, TRUE, 3);
	gtk_box_pack_start (GTK_BOX (vbox), button, TRUE, TRUE, 3);
	gtk_box_pack_start (GTK_BOX (vbox), pargument->label, TRUE, TRUE, 3);
	gtk_box_pack_start (GTK_BOX (vbox), check, TRUE, TRUE, 3);
	gtk_box_pack_start (GTK_BOX(vbox), progressbar, TRUE, TRUE, 3);
	gtk_box_pack_start (GTK_BOX(vbox), continue_check, TRUE, TRUE, 3);
	gtk_box_pack_start (GTK_BOX(vbox), spin_int, TRUE, TRUE, 3);
	gtk_box_pack_start (GTK_BOX(vbox), next_button, TRUE, TRUE, 3);

#endif


	gtk_container_add (GTK_CONTAINER (window), vbox);
	
	gtk_widget_show_all(window);

	gtk_main();
	return 0;
}
