// gtk_GUI.c

#include <gtk/gtk.h>
#include <string.h>
#include <stdio.h>

extern int muted;
extern ssize_t write(int fd, const void *buf, size_t count);
extern int sock2;

GtkWidget *chat_entry;
GtkWidget *chat_output_view;

void change_mute(GtkWidget *button, gpointer data) {
    int *flag_muted = (int *)data;

    *flag_muted = !(*flag_muted); // ミュート状態をトグル

    // GUIボタンのラベルを更新
    if (*flag_muted == 1) {
        gtk_button_set_label(GTK_BUTTON(button), "保留中");
    } else {
        gtk_button_set_label(GTK_BUTTON(button), "保留ボタン");
    }

    // 相手に状態変化をチャットメッセージとして送信
    char buf[2048 + 6]; // CHAT_BUF + "CHAT:" + '\n' + '\0'
    snprintf(buf, sizeof(buf), "CHAT:[SYSTEM]%s\n", // [SYSTEM] プレフィックスを追加
             *flag_muted ? "相手がミュートしました" : "相手がミュートを解除しました");

    ssize_t sent = write(sock2, buf, strlen(buf));
    if (sent < 0) {
        perror("change_mute: write");
    }

    // 自分のGUIにも状態変化を表示
    display_received_chat(g_strdup_printf("[システム] %s\n", *flag_muted ? "ミュートしました" : "ミュートを解除しました"));
}

void send_chat_gui(GtkWidget *button, gpointer data) {
    const char *msg_text = gtk_entry_get_text(GTK_ENTRY(chat_entry));
    char buf[2048 + 6];

    // GUIからの送信では/mコマンドはボタンで処理されるため、ここでの特殊処理は不要
    // if (strcmp(msg_text, "/m") == 0) { ... } のブロックは削除またはコメントアウト

    snprintf(buf, sizeof(buf), "CHAT:%s\n", msg_text);

    ssize_t sent = write(sock2, buf, strlen(buf));
    if (sent < 0) {
        perror("send_chat_gui: write");
    }

    gtk_entry_set_text(GTK_ENTRY(chat_entry), "");

    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(chat_output_view));
    GtkTextIter end_iter;
    gtk_text_buffer_get_end_iter(buffer, &end_iter);
    gtk_text_buffer_insert(buffer, &end_iter, "[自分] ", -1);
    gtk_text_buffer_insert(buffer, &end_iter, msg_text, -1);
    gtk_text_buffer_insert(buffer, &end_iter, "\n", -1);

    GtkTextMark *mark = gtk_text_buffer_get_insert(buffer);
    gtk_text_view_scroll_to_mark(GTK_TEXT_VIEW(chat_output_view), mark, 0.0, FALSE, 0.0, 0.0);
}


gboolean append_text_to_chat_view_idle(gpointer user_data) {
    char *message = (char *)user_data;
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(chat_output_view));
    GtkTextIter end_iter;
    gtk_text_buffer_get_end_iter(buffer, &end_iter);
    gtk_text_buffer_insert(buffer, &end_iter, message, -1);
    gtk_text_buffer_insert(buffer, &end_iter, "\n", -1); // display_received_chatに渡す文字列に改行がない場合を考慮

    GtkTextMark *mark = gtk_text_buffer_get_insert(buffer);
    gtk_text_view_scroll_to_mark(GTK_TEXT_VIEW(chat_output_view), mark, 0.0, FALSE, 0.0, 0.0);

    g_free(message);
    return FALSE;
}

void display_received_chat(const char *message) {
    g_idle_add(append_text_to_chat_view_idle, g_strdup(message));
}


void *display_GUI() {
    gtk_init(NULL, NULL);

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Voice Chat GUI");
    gtk_container_set_border_width(GTK_CONTAINER(window), 10);
    gtk_window_set_default_size(GTK_WINDOW(window), 600, 400);

    g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *main_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(window), main_vbox);

    GtkWidget *mute_button = gtk_button_new_with_label("保留ボタン");
    gtk_box_pack_start(GTK_BOX(main_vbox), mute_button, FALSE, FALSE, 0);
    g_signal_connect(G_OBJECT(mute_button), "clicked", G_CALLBACK(change_mute), &muted);

    chat_output_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(chat_output_view), FALSE);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(chat_output_view), FALSE);

    GtkWidget *scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(scrolled_window), chat_output_view);
    gtk_box_pack_start(GTK_BOX(main_vbox), scrolled_window, TRUE, TRUE, 0);

    GtkWidget *chat_input_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start(GTK_BOX(main_vbox), chat_input_hbox, FALSE, FALSE, 0);

    chat_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(chat_entry), "メッセージを入力...");
    gtk_box_pack_start(GTK_BOX(chat_input_hbox), chat_entry, TRUE, TRUE, 0);

    GtkWidget *send_button = gtk_button_new_with_label("送信");
    gtk_box_pack_start(GTK_BOX(chat_input_hbox), send_button, FALSE, FALSE, 0);

    g_signal_connect(G_OBJECT(send_button), "clicked", G_CALLBACK(send_chat_gui), NULL);
    g_signal_connect(G_OBJECT(chat_entry), "activate", G_CALLBACK(send_chat_gui), NULL);

    gtk_widget_show_all(window);

    gtk_main();

    return NULL;
}