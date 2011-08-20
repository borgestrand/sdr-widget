all:
ifeq ($(CC),clang)
	clang -std=gnu99 -g -O3 -Wall -o ../../bin/softrock -lpthread -lportaudio -lpulse-simple -lusb *.c
else
	gcc -std=gnu99 -g -O3 -Wall -o ../../bin/softrock -lpthread -lportaudio -lpulse-simple -lusb *.c
endif

