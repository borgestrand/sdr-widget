#all:
#ifeq ($(CC),clang)
#	clang -std=gnu99 -g -O3 -Wall -o ../../bin/softrock -lpthread -lportaudio -lpulse-simple -lusb *.c
#else
#	gcc -std=gnu99 -g -O3 -Wall -o ../../bin/softrock -lpthread -lportaudio -lpulse-simple -lusb *.c
#endif



bin_PROGRAMS = softrock

softrock_CFLAGS = -std=gnu99 -g -O3 -Wall

softrock_SOURCES = client.c        client.h       \
                   config_ops.c    config_ops.h   \
		   listener.c      listener.h     \
		   operations.c    operations.h   \
                   receiver.c      receiver.h     \
       jackio.c		jackio.h						\
                   server.c                       \
		   softrock.c      softrock.h     \
		   softrockio.c    softrockio.h   \
		   util.c          util.h         \
		   messages.h 	
		   
