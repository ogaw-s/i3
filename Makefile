CC = gcc
CFLAGS = -Wall -pthread -I/usr/include
LDFLAGS = -lsox
OBJS = main.o audio_stream.o tcp_stream.o

all: voicechat

voicechat: $(OBJS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *.o voicechat
