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
	const gchar *dirname;	

	while((dir != NULL) && (dirname = g_dir_read_name(dir)) && (g_stpcpy(file, dirname))) {
		if( g_file_test(g_build_path("/", folder, file, NULL), G_FILE_TEST_IS_REGULAR) && ( g_str_has_suffix(file, "flac") || g_str_has_suffix(file, "mp3") ) ) 
			g_fprintf(list, "%s\n", g_build_path("/", folder, file, NULL));	
		else if(g_file_test(g_build_path("/", folder, file, NULL), G_FILE_TEST_IS_DIR))
			explore(g_dir_open(g_build_path("/", folder, file, NULL), 0, NULL), g_build_path("/", folder, file, NULL), list);
	}
	if (dirname == NULL) {
		g_dir_close(dir);
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

	char artist[strlen(current_song.artist) + 9];
	char title[strlen(current_song.title) + 8];
	char album[strlen(current_song.album) + 8];
	strcpy(artist, "Artist: ");
	strcpy(title, "Title: ");
	strcpy(album, "Album: ");

	gtk_label_set_text((GtkLabel*)(((struct arguments*)argument)->label), "");
	gtk_label_set_text((GtkLabel*)(((struct arguments*)argument)->artist_label), strcat(artist, current_song.artist));
	gtk_label_set_text((GtkLabel*)(((struct arguments*)argument)->title_label), strcat(title, current_song.title));
	gtk_label_set_text((GtkLabel*)(((struct arguments*)argument)->album_label), strcat(album, current_song.album));

	g_timer_reset(((struct arguments*)argument)->elapsed);
	((struct arguments*)argument)->offset = 0;
	gtk_adjustment_configure(((struct arguments*)argument)->adjust, 0, 0, current_song.duration, 1, 1, 1);
	gtk_adjustment_changed(((struct arguments*)argument)->adjust);

	if(--((struct arguments*)argument)->continue_count != 0) {
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
	int count = 0, tmpcount;
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
		char artist[strlen(current_song.artist) + 9];
		char title[strlen(current_song.title) + 8];
		char album[strlen(current_song.album) + 8];
		strcpy(artist, "Artist: ");
		strcpy(title, "Title: ");
		strcpy(album, "Album: ");

		gtk_label_set_text((GtkLabel*)(((struct arguments*)argument)->label), "");
		gtk_label_set_text((GtkLabel*)(((struct arguments*)argument)->artist_label), strcat(artist, current_song.artist));
		gtk_label_set_text((GtkLabel*)(((struct arguments*)argument)->title_label), strcat(title, current_song.title));
		gtk_label_set_text((GtkLabel*)(((struct arguments*)argument)->album_label), strcat(album, current_song.album));

		gtk_adjustment_configure(((struct arguments*)argument)->adjust, 0, 0, current_song.duration, 1, 1, 1);
		((struct arguments*)argument)->offset = 0;
		gtk_adjustment_changed(((struct arguments*)argument)->adjust);
	}
	else
		gtk_label_set_text((GtkLabel*)(((struct arguments*)argument)->label), "No calm songs found!");

}

static void config_folder_changed (GtkFileChooser *chooser1, struct arguments *argument) {
	gchar *folder = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (chooser1));
	GDir *dir = g_dir_open (folder, 0, NULL);
	FILE *list;
	list = fopen("list.txt", "w+");
	float resnum_temp;
	explore(dir, folder, list);
	float resnum = 0;
	ssize_t len = 0;
	char *line = NULL;
	int count = 0;

	fseek(list, 0, SEEK_SET);

	while (fgets(line, 1000, list) != NULL) {
		free(current_sample_array);
		line[strcspn(line, "\n")] = '\0';
		resnum_temp = analyze(line);
		resnum += resnum_temp > 0 ? resnum_temp : 0;
		if(resnum_temp != 0)
			++count;
		printf("%s, %f\n", line, resnum/count);
	}
	
	free(line);
	fclose(list);
	/*tmpcount = count;
	
	while(tmpcount-- > 0) {
		free(current_sample_array);
		int rand = g_random_int_range(0, count);
		rewind(list);
		
		for(i = count; i > tmpcount; --i)
			fgets(line, 1000, list);
		line[strcspn(line, "\n")] = '\0';
		printf("%s, %f\n", line, resnum_temp);
		resnum_temp = analyze(line);
		resnum += resnum_temp;
		if(resnum_temp == 0) {
			--count;
			--tmpcount;
		}
	}*/

	printf("%f\n", resnum/count);
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
	gtk_adjustment_configure(argument->adjust, 0, 0, current_song.duration, 1, 1, 1);
	gtk_adjustment_changed(argument->adjust);

	if(resnum == 2) {
  		gtk_label_set_text ((GtkLabel*)argument->label, "Can't Conclude");
		gtk_label_set_text((GtkLabel*)argument->artist_label, current_song.artist);
		gtk_label_set_text((GtkLabel*)argument->title_label, current_song.title);
		gtk_label_set_text((GtkLabel*)argument->album_label, current_song.album);

	}
	else if(resnum == 0) {
  		gtk_label_set_text ((GtkLabel*)argument->label, "Much loud");
		gtk_label_set_text((GtkLabel*)argument->artist_label, current_song.artist);
		gtk_label_set_text((GtkLabel*)argument->title_label, current_song.title);
		gtk_label_set_text((GtkLabel*)argument->album_label, current_song.album);

	}
	else if(resnum == 1) {
		gtk_label_set_text((GtkLabel*)argument->artist_label, current_song.artist);
		gtk_label_set_text((GtkLabel*)argument->title_label, current_song.title);
		gtk_label_set_text((GtkLabel*)argument->album_label, current_song.album);
  		gtk_label_set_text ((GtkLabel*)argument->label, "Such calm");

	}
	char artist[strlen(current_song.artist) + 9];
	char title[strlen(current_song.title) + 8];
	char album[strlen(current_song.album) + 8];
	strcpy(artist, "Artist: ");
	strcpy(title, "Title: ");
	strcpy(album, "Album: ");

	gtk_label_set_text((GtkLabel*)(((struct arguments*)argument)->artist_label), strcat(artist, current_song.artist));
	gtk_label_set_text((GtkLabel*)(((struct arguments*)argument)->title_label), strcat(title, current_song.title));
	gtk_label_set_text((GtkLabel*)(((struct arguments*)argument)->album_label), strcat(album, current_song.album));

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
		if(argument->endless_check || argument->continue_count != 0) {
			argument->tag = g_timeout_add_seconds(current_song.duration, endless, argument);
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
			argument->tag = g_timeout_add_seconds(current_song.duration - (int)g_timer_elapsed(argument->elapsed, NULL), endless, argument->label);
		}
		return 0;
	}
}

static void check_toggled (GtkToggleButton *check, struct arguments *argument) {
	if(status == AL_PLAYING && argument->endless_check) {
		g_source_remove(argument->tag);
		argument->tag = g_timeout_add_seconds(current_song.duration - (int)g_timer_elapsed(argument->elapsed, NULL), endless, argument);
	}
	argument->endless_check = !(argument->endless_check);
}

static void continue_check_toggled(GtkToggleButton *continue_check, GtkWidget *spin_int) {
	if(gtk_toggle_button_get_active(continue_check))
		gtk_widget_set_sensitive(spin_int, TRUE);
	else
		gtk_widget_set_sensitive(spin_int, FALSE);
}

static void continue_spin_count(GtkSpinButton *spin_int, struct arguments *argument) {
	argument->continue_count = gtk_spin_button_get_value(spin_int);
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
	
		if(argument->continue_count != 0) {
			argument->tag = g_timeout_add_seconds(current_song.duration - (int)gtk_adjustment_get_value(argument->adjust), endless, argument);
		}
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
	pargument->continue_count = 0;
	pargument->offset = 0;
	pargument->elapsed = g_timer_new();
	GtkWidget *window, *hpaned, *pause_button, *bitrate_label, *chooser1, *chooser2, *chooser3, *vboxl, *vboxr, *check,
		 *continue_check, *continue_label, *next_button, *button2, *spin_int, *progressbar, *button_hbox, *continue_hbox;
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
	pargument->label = gtk_label_new ("");
	pargument->title_label = gtk_label_new("");
	pargument->album_label = gtk_label_new("");
	pargument->artist_label = gtk_label_new("");	
	gtk_label_set_line_wrap((GtkLabel*)pargument->title_label, TRUE);
	gtk_label_set_line_wrap((GtkLabel*)pargument->artist_label, TRUE);
	gtk_label_set_line_wrap((GtkLabel*)pargument->album_label, TRUE);
	continue_label = gtk_label_new("Number of songs played\n -1 to go endless");

	gtk_container_set_border_width(GTK_CONTAINER(window), 10);
	gtk_widget_set_size_request(window, 500, 250);
	hpaned = gtk_hpaned_new();

	spin_int = gtk_spin_button_new_with_range(-1, 10000, 1);
	gtk_spin_button_set_value ((GtkSpinButton*)spin_int, 1);
	pause_button = gtk_button_new_with_mnemonic("_Play/Pause");
	next_button = gtk_button_new_with_mnemonic("Next");
	check = gtk_check_button_new_with_label ("Endless mode.");
	//continue_check = gtk_check_button_new_with_label("Play this number of songs.");


	g_signal_connect (G_OBJECT (window), "destroy", G_CALLBACK (destroy), NULL);
  	g_signal_connect (G_OBJECT(pause_button), "clicked", G_CALLBACK(play), pargument);
	g_signal_connect (G_OBJECT(next_button), "clicked", G_CALLBACK(next), pargument);
	g_signal_connect (G_OBJECT(check), "toggled", G_CALLBACK(check_toggled), pargument);
	//g_signal_connect(G_OBJECT(continue_check), "toggled", G_CALLBACK(continue_check_toggled), spin_int);
	g_signal_connect(G_OBJECT(spin_int), "value-changed", G_CALLBACK(continue_spin_count), pargument);
	g_signal_connect(G_OBJECT(progressbar), "value-changed", G_CALLBACK(slider_changed), pargument);


  /* Create two buttons, one to select a folder and one to select a file. */

	chooser1 = gtk_file_chooser_button_new ("Chooser a Folder", GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER);
	chooser2 = gtk_file_chooser_button_new ("Chooser a Folder", GTK_FILE_CHOOSER_ACTION_OPEN);
	chooser3 = gtk_file_chooser_button_new ("Choose a folder for config (may take some time)", GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER);

  /* Monitor when the selected folder or file are changed. */
	g_signal_connect (G_OBJECT (chooser2), "selection_changed", G_CALLBACK (file_changed), pargument);
	g_signal_connect (G_OBJECT (chooser1), "selection_changed", G_CALLBACK(folder_changed), pargument);
	g_signal_connect (G_OBJECT (chooser3), "selection_changed", G_CALLBACK (config_folder_changed), pargument);

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


	//gtk_widget_set_sensitive(spin_int, FALSE);
	vboxl = gtk_vbox_new (TRUE, 5);
	vboxr = gtk_vbox_new (TRUE, 5);
	button_hbox = gtk_hbox_new (TRUE, 5);
	continue_hbox= gtk_hbox_new (TRUE, 5);

	gtk_box_set_homogeneous(GTK_BOX(vboxr), FALSE);
	gtk_box_set_homogeneous(GTK_BOX(continue_hbox), FALSE);

	gtk_box_pack_start_defaults (GTK_BOX (vboxl), chooser1);
	gtk_box_pack_start_defaults (GTK_BOX (vboxl), chooser2);
//	gtk_box_pack_start_defaults (GTK_BOX (vboxl), chooser3);
	//gtk_box_pack_start_defaults (GTK_BOX (vboxl), check);
	
	gtk_box_pack_start_defaults (GTK_BOX (vboxl), continue_hbox);
		gtk_box_pack_start_defaults (GTK_BOX(continue_hbox), continue_label);
		gtk_box_pack_start_defaults (GTK_BOX(continue_hbox), spin_int);
	gtk_box_pack_start_defaults (GTK_BOX (vboxl), button_hbox);
		gtk_box_pack_start_defaults (GTK_BOX (button_hbox), pause_button);
		gtk_box_pack_start_defaults (GTK_BOX(button_hbox), next_button);
	gtk_box_pack_start_defaults (GTK_BOX(vboxl), progressbar);

	gtk_box_pack_start(GTK_BOX (vboxr), pargument->label, FALSE, FALSE, 1);
	gtk_box_pack_start(GTK_BOX (vboxr), pargument->title_label, FALSE, FALSE, 1);
	gtk_box_pack_start(GTK_BOX (vboxr), pargument->album_label, FALSE, FALSE, 1);
	gtk_box_pack_start(GTK_BOX (vboxr), pargument->artist_label, FALSE, FALSE, 1);

#ifdef GTK3
	vboxl = gtk_box_new (GTK_ORIENTATION_VERTICAL, 5);
	vboxr = gtk_vboxr_new (GTK_ORIENTATION_VERTICAL, 5);
 	gtk_box_pack_start (GTK_BOX (vboxl), chooser1, TRUE, TRUE, 3);
	gtk_box_pack_start (GTK_BOX (vboxl), chooser2, TRUE, TRUE, 3);
	gtk_box_pack_start (GTK_BOX (vboxl), pause_button, TRUE, TRUE, 3);
	gtk_box_pack_start (GTK_BOX(vboxl), next_button, TRUE, TRUE, 3);
	gtk_box_pack_start (GTK_BOX (vboxl), pargument->label, TRUE, TRUE, 3);
	gtk_box_pack_start (GTK_BOX (vboxl), check, TRUE, TRUE, 3);
	gtk_box_pack_start (GTK_BOX(vboxl), continue_check, TRUE, TRUE, 3);
	gtk_box_pack_start (GTK_BOX(vboxl), spin_int, TRUE, TRUE, 3);

	gtk_box_pack_start (GTK_BOX(vboxl), progressbar, TRUE, TRUE, 3);
	
	gtk_box_pack_start (GTK_BOX (vboxr), pargument->label, TRUE, TRUE, 3);	
	gtk_box_pack_start_defaults (GTK_BOX (vboxr), pargument->title_label, TRUE, TRUE, 3);
	gtk_box_pack_start_defaults (GTK_BOX (vboxr), pargument->album_label, TRUE, TRUE, 3);
	gtk_box_pack_start_defaults (GTK_BOX (vboxr), pargument->artist_label, TRUE, TRUE, 3);

#endif

	gtk_paned_pack1(GTK_PANED(hpaned), vboxl, FALSE, FALSE);
	gtk_paned_pack2(GTK_PANED(hpaned), vboxr, TRUE, FALSE);
	gtk_container_add (GTK_CONTAINER (window), hpaned);
	
	gtk_widget_show_all(window);

	gtk_main();
	return 0;
}
