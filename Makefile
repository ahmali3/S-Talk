s-talk: s-talk.c list.h list.o
	gcc -pthread -o s-talk s-talk.c list.o

clean:
	rm s-talk
