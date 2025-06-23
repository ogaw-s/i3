# コンパイラ
CC = gcc

# GTK関連のフラグをpkg-configで取得
# --cflags はヘッダーファイルのパス (-I) を提供
GTK_CFLAGS = $(shell pkg-config --cflags gtk+-3.0)
# --libs はライブラリのパス (-L) とライブラリ名 (-l) を提供
GTK_LIBS = $(shell pkg-config --libs gtk+-3.0)

# Cコンパイラフラグ
# -Wall: 全ての警告を有効にする
# -pthread: POSIXスレッドをサポート
# $(GTK_CFLAGS): GTKのヘッダーパス
CFLAGS = -Wall -pthread $(GTK_CFLAGS)

# リンカフラグ
# -pthread: POSIXスレッドライブラリをリンク
# -lsox: SoXライブラリをリンク
# $(GTK_LIBS): GTKのライブラリ
LDFLAGS = -pthread -lsox $(GTK_LIBS)

# ビルドディレクトリ
BIN_DIR = bin

# ソースファイル
SRCS = main.c audio_stream.c tcp_stream.c chat_stream.c audio_effects.c gtk_GUI.c
# オブジェクトファイル (BIN_DIRの下に生成)
OBJS = $(addprefix $(BIN_DIR)/, $(SRCS:.c=.o))
# 最終実行ファイル
TARGET = $(BIN_DIR)/voicechat

# デフォルトターゲット
all: $(TARGET)

# 実行ファイルのビルドルール
$(TARGET): $(OBJS)
	@mkdir -p $(BIN_DIR) # binディレクトリが存在しない場合作成
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) # CFLAGSもリンク時に必要になることがあるので含める

# オブジェクトファイルのビルドルール
$(BIN_DIR)/%.o: %.c
	@mkdir -p $(BIN_DIR) # binディレクトリが存在しない場合作成
	$(CC) $(CFLAGS) -c $< -o $@

# クリーンルール
clean:
	rm -rf $(BIN_DIR)

.PHONY: all clean