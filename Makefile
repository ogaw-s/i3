CC = gcc
CFLAGS = -Wall -pthread `pkg-config --cflags gtk+-3.0`
LDFLAGS = -lsox `pkg-config --libs gtk+-3.0`

BIN_DIR = bin

SRCS = main.c audio_stream.c tcp_stream.c chat_stream.c audio_effects.c gtk_GUI.c
OBJS = $(SRCS:%.c=$(BIN_DIR)/%.o)
TARGET = $(BIN_DIR)/voicechat

all: $(TARGET)

$(TARGET): $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(CC) -o $@ $^ $(LDFLAGS)

$(BIN_DIR)/%.o: %.c
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BIN_DIR)

.PHONY: all clean
