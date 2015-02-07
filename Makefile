CC=gcc
CFLAGS=-g
LDFLAGS=-lavformat -lavutil -lavcodec -lm -std=c99
EXEC=analyse

all: $(EXEC) 

analyse: amp_sort.o decode.o envelope.o main.o freq_sort.o
	gcc -o analyse amp_sort.o decode.o envelope.o main.o freq_sort.o $(LDFLAGS) $(CFLAGS)
	@rm -Rf *.o

amp_sort.o: amp_sort.c
	@gcc -o amp_sort.o -c amp_sort.c 

decode.o: decode.c
	@gcc -o decode.o -c decode.c 

envelope.o: envelope.c
	@gcc -o envelope.o -c envelope.c 

main.o: main.c
	@gcc -o main.o -c main.c -lavformat 

freq_sort.o: freq_sort.c
	@gcc -o freq_sort.o -c freq_sort.c 

clean:
	rm -Rf *.o

mrproper: clean
	rm -Rf $(EXEC)
