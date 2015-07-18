CC=gcc -O3 -Wall
MCC=x86_64-w64-mingw32-gcc -O3 -Wall
CFLAGS=
LDFLAGS=`pkg-config --cflags --libs gstreamer-1.0` `pkg-config --cflags gtk+-3.0` `pkg-config --libs gtk+-3.0` -lavformat -lavutil -lavcodec -lm -std=c99
MLDFLAGS=`x86_64-w64-mingw32-pkg-config --cflags --libs gstreamer-1.0` `x86_64-w64-mingw32-pkg-config --cflags gtk+-3.0` `x86_64-w64-mingw32-pkg-config --libs gtk+-3.0` -lavformat -lavutil -lavcodec -lm -std=c99 # -mwindows

EXEC=analyze

all: $(EXEC) 

analyze: amp_sort.o decode.o envelope.o freq_sort.o gui_dummy.o analyze.o
	$(CC) -o analyze analyze.o gui_dummy.o amp_sort.o decode.o envelope.o freq_sort.o $(LDFLAGS) 
	@rm -Rf *.o

player: amp_sort.o decode.o envelope.o freq_sort.o analyze.o gui.o 
	$(CC) -o player analyze.o gui.o amp_sort.o decode.o envelope.o freq_sort.o $(LDFLAGS) 
	@rm -Rf *.o

player_exe: amp_sort_exe.o decode_exe.o envelope_exe.o freq_sort_exe.o gui_exe.o analyze_exe.o
	$(MCC) -o player.exe analyze_exe.o gui_exe.o amp_sort_exe.o decode_exe.o envelope_exe.o freq_sort_exe.o $(MLDFLAGS) 
	@rm -Rf *.o

analyze_exe.o: analyze.c
	@$(MCC) -o analyze_exe.o -c analyze.c `x86_64-w64-mingw32-pkg-config --cflags gstreamer-1.0`

amp_sort_exe.o: amp_sort.c
	@$(MCC) -o amp_sort_exe.o -c amp_sort.c `x86_64-w64-mingw32-pkg-config --cflags gstreamer-1.0`

decode_exe.o: decode.c
	@$(MCC) -o decode_exe.o -c decode.c `x86_64-w64-mingw32-pkg-config --cflags gstreamer-1.0`

envelope_exe.o: envelope.c
	@$(MCC) -o envelope_exe.o -c envelope.c `x86_64-w64-mingw32-pkg-config --cflags gstreamer-1.0`

gui_exe.o: gui.c
	@$(MCC) -o gui_exe.o -c gui.c `x86_64-w64-mingw32-pkg-config --cflags gtk+-3.0` `x86_64-w64-mingw32-pkg-config --cflags gstreamer-1.0`

freq_sort_exe.o: freq_sort.c
	@$(MCC) -o freq_sort_exe.o -c freq_sort.c `x86_64-w64-mingw32-pkg-config --cflags gstreamer-1.0`

analyze.o: analyze.c
	@$(CC) -o analyze.o -c analyze.c `pkg-config --cflags --libs gstreamer-1.0` 

amp_sort.o: amp_sort.c
	@$(CC) -o amp_sort.o -c amp_sort.c `pkg-config --cflags --libs gstreamer-1.0`

decode.o: decode.c
	@$(CC) -o decode.o -c decode.c `pkg-config --cflags --libs gstreamer-1.0`

envelope.o: envelope.c
	@$(CC) -o envelope.o -c envelope.c `pkg-config --cflags --libs gstreamer-1.0`

gui_dummy.o: gui_dummy.c
	@$(CC) -o gui_dummy.o -c gui_dummy.c `pkg-config --cflags --libs gstreamer-1.0`

gui.o: gui.c
	@$(CC) -o gui.o -c gui.c `pkg-config --cflags --libs gstreamer-1.0` `pkg-config --cflags gtk+-3.0`

freq_sort.o: freq_sort.c
	@$(CC) -o freq_sort.o -c freq_sort.c  `pkg-config --cflags --libs gstreamer-1.0`


clean:
	rm -Rf *.o

mrproper: clean
	rm -Rf $(EXEC)
