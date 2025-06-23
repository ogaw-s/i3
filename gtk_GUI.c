#include <gtk/gtk.h>
#include <string.h> // for strlen, strcspn
#include <stdio.h>  // for snprintf, perror, fprintf

// main.c または別の場所で定義されていることを前提
extern int muted;

// chat_stream.c からの関数宣言（仮定）
// 実際には chat_stream.h をインクルードするべきですが、ここでは簡略化
extern ssize_t write(int fd, const void *buf, size_t count); // write(2)
extern int sock2; // chat_stream.c で定義されているソケット

// チャット入力欄と送信ボタンへの参照をグローバルに持つか、構造体にまとめる
// ここでは簡略化のためグローバル変数を使用
GtkWidget *chat_entry;
GtkWidget *chat_output_view; // チャット履歴表示用

// ミュートボタンの状態を切り替える関数
void change_mute(GtkWidget *button, gpointer data) {
    int *flag_muted = (int *)data;

    if (*flag_muted == 1) {
        gtk_button_set_label(GTK_BUTTON(button), "保留ボタン");
        *flag_muted = 0;
    } else {
        gtk_button_set_label(GTK_BUTTON(button), "保留中");
        *flag_muted = 1;
    }
}

// チャット送信ボタンがクリックされた時のコールバック関数
void send_chat_gui(GtkWidget *button, gpointer data) {
    const char *msg_text = gtk_entry_get_text(GTK_ENTRY(chat_entry));
    char buf[2048 + 6]; // CHAT_BUF (2048) + "CHAT:" + '\n' + '\0'

    // `/m` コマンドの処理（GUIからの送信でも有効にする場合）
    if (strcmp(msg_text, "/m") == 0) {
        muted = !muted; // グローバル変数 muted を切り替える
        snprintf(buf, sizeof(buf), "CHAT:%s\n",
                 muted ? "相手がミュートしました" : "相手がミュートを解除しました");
    } else {
        snprintf(buf, sizeof(buf), "CHAT:%s\n", msg_text);
    }

    ssize_t sent = write(sock2, buf, strlen(buf));
    if (sent < 0) {
        perror("send_chat_gui: write");
    }

    // 送信後、入力欄をクリア
    gtk_entry_set_text(GTK_ENTRY(chat_entry), "");

    // 送信したメッセージをチャット履歴ビューに追加（自分のメッセージ）
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(chat_output_view));
    GtkTextIter end_iter;
    gtk_text_buffer_get_end_iter(buffer, &end_iter);
    gtk_text_buffer_insert(buffer, &end_iter, "[自分] ", -1);
    gtk_text_buffer_insert(buffer, &end_iter, msg_text, -1); // ★ここが重要★
    gtk_text_buffer_insert(buffer, &end_iter, "\n", -1);

    // テキストビューを最新の行までスクロール
    GtkTextMark *mark = gtk_text_buffer_get_insert(buffer);
    gtk_text_view_scroll_to_mark(GTK_TEXT_VIEW(chat_output_view), mark, 0.0, FALSE, 0.0, 0.0);
}


gboolean append_text_to_chat_view_idle(gpointer user_data) {
    char *message = (char *)user_data;
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(chat_output_view));
    GtkTextIter end_iter;
    gtk_text_buffer_get_end_iter(buffer, &end_iter);
    gtk_text_buffer_insert(buffer, &end_iter, message, -1); // "[CHAT] " は呼び出し元で追加済みと仮定
    gtk_text_buffer_insert(buffer, &end_iter, "\n", -1);

    // テキストビューを最新の行までスクロール
    GtkTextMark *mark = gtk_text_buffer_get_insert(buffer);
    gtk_text_view_scroll_to_mark(GTK_TEXT_VIEW(chat_output_view), mark, 0.0, FALSE, 0.0, 0.0);

    g_free(message); // メッセージのメモリを解放
    return FALSE; // 一度だけ実行し、アイドル処理を削除
}

void display_received_chat(const char *message) {
    // 他のスレッドから呼び出す場合、メインスレッドでGUIを更新するようにキューに入れる
    g_idle_add(append_text_to_chat_view_idle, g_strdup(message)); // メッセージのコピーを渡す
}


void *display_GUI() {
    gtk_init(NULL, NULL);

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Voice Chat GUI"); // ウィンドウタイトル
    gtk_container_set_border_width(GTK_CONTAINER(window), 10); // ウィンドウの余白

    // ウィンドウサイズの設定
    gtk_window_set_default_size(GTK_WINDOW(window), 600, 400); // 例: 幅600px, 高さ400px

    g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // メインの垂直ボックス（UI要素を縦に並べる）
    GtkWidget *main_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(window), main_vbox);

    // --- ミュートボタン ---
    GtkWidget *mute_button = gtk_button_new_with_label("保留ボタン");
    gtk_box_pack_start(GTK_BOX(main_vbox), mute_button, FALSE, FALSE, 0); // 固定サイズで上部に配置
    g_signal_connect(G_OBJECT(mute_button), "clicked", G_CALLBACK(change_mute), &muted);

    // --- チャット履歴表示エリア ---
    chat_output_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(chat_output_view), FALSE); // 編集不可にする
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(chat_output_view), FALSE); // カーソルを非表示に

    GtkWidget *scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(scrolled_window), chat_output_view);
    // 履歴表示エリアはウィンドウのサイズに合わせて拡大できるようにする
    gtk_box_pack_start(GTK_BOX(main_vbox), scrolled_window, TRUE, TRUE, 0);

    // --- チャット入力欄と送信ボタンを含む水平ボックス ---
    GtkWidget *chat_input_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(main_vbox), chat_input_hbox, FALSE, FALSE, 0); // 固定サイズで下部に配置

    chat_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(chat_entry), "メッセージを入力...");
    gtk_box_pack_start(GTK_BOX(chat_input_hbox), chat_entry, TRUE, TRUE, 0); // 入力欄を横いっぱいに

    GtkWidget *send_button = gtk_button_new_with_label("送信");
    gtk_box_pack_start(GTK_BOX(chat_input_hbox), send_button, FALSE, FALSE, 0); // 送信ボタンは固定サイズ

    // 送信ボタンクリック時のコールバック
    g_signal_connect(G_OBJECT(send_button), "clicked", G_CALLBACK(send_chat_gui), NULL);
    // Enterキーが押された時も送信するようにする
    g_signal_connect(G_OBJECT(chat_entry), "activate", G_CALLBACK(send_chat_gui), NULL);


    gtk_widget_show_all(window);

    gtk_main();

    return NULL;
}