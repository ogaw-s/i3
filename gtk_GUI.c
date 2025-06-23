#include <gtk/gtk.h>
#include <string.h>
#include <stdio.h>
#define CHAT_BUF 2048

extern int muted_mine;
extern int muted_partner;
extern char message_mine[CHAT_BUF];
extern char message_partner[CHAT_BUF];
extern int send_flag;
extern int recv_flag;


typedef struct pair{
    GtkWidget *textview;
    GtkTextBuffer *buf;
} Pair;

void change_mute(GtkWidget *button, gpointer data) {
    GtkWidget *label = (GtkWidget *)data;

    if (muted_mine == 1) {
        gtk_button_set_label(GTK_BUTTON(button), "保留");
        gtk_label_set_text(GTK_LABEL(label), "通話中");
        muted_mine = 0;
        sprintf(message_mine, "/m_OFF");
        send_flag = 1;
    } else {
        gtk_button_set_label(GTK_BUTTON(button), "解除");
        gtk_label_set_text(GTK_LABEL(label), "ミュート中");
        muted_mine = 1;
        sprintf(message_mine, "/m_ON");
        send_flag = 1;
    } 

}

void entry_activate(GtkWidget *entry, gpointer data){
    Pair *pair = (Pair *)data;
    GtkWidget *text_view = pair->textview;

    const gchar *text = gtk_entry_get_text(entry);
    strcpy(message_mine, text);
    send_flag = 1;

    GtkTextBuffer *buffer = pair->buf;

    // バッファの末尾を取得
    GtkTextIter start_iter, end_iter;
    gtk_text_buffer_get_start_iter(buffer, &start_iter);
    gtk_text_buffer_get_end_iter(buffer, &end_iter);

    char S[CHAT_BUF];

    // 追記するテキスト
    sprintf(S, "[自分] %s\n\n", text);
    
    gtk_text_buffer_insert(buffer, &end_iter, S, -1);

    gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW(text_view), &end_iter, 0.0, FALSE, 0, 0);

    gint text_length = gtk_text_buffer_get_char_count(buffer);

    if (text_length > 500) {
        gtk_text_buffer_get_start_iter(buffer, &start_iter);
        gtk_text_buffer_get_iter_at_offset(buffer, &end_iter, text_length - 500);
        gtk_text_buffer_delete(buffer, &start_iter, &end_iter);
    }

    gtk_entry_set_text(entry, ""); 
}

gboolean check_recv(gpointer data) {
    if (recv_flag) {
        Pair *pair = (Pair *)data;
        GtkWidget *text_view = pair->textview;
        GtkTextBuffer *buffer = pair->buf;
        GtkTextIter start_iter, end_iter;
        gtk_text_buffer_get_start_iter(buffer, &start_iter);
        gtk_text_buffer_get_end_iter(buffer, &end_iter);

        char dest[100];
        strncpy(dest, message_partner, recv_flag);
        dest[recv_flag] = '\0';
        
        char S[CHAT_BUF];
        sprintf(S, "[相手] %s\n\n", dest);
        
        gtk_text_buffer_insert(buffer, &end_iter, S, -1);

        gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW(text_view), &end_iter, 0.0, FALSE, 0, 0);

        gint text_length = gtk_text_buffer_get_char_count(buffer);

        if (text_length > 500) {
            gtk_text_buffer_get_start_iter(buffer, &start_iter);
            gtk_text_buffer_get_iter_at_offset(buffer, &end_iter, text_length - 500);
            gtk_text_buffer_delete(buffer, &start_iter, &end_iter);
        }

        recv_flag = 0;
    }

    return TRUE;  // TRUE を返すと次も呼ばれる（ループ）
}

gboolean check_muted_partner(gpointer data){
    GtkWidget *label = (GtkWidget *)data;

    if (muted_partner == 0) {
        gtk_label_set_text(GTK_LABEL(label), "通話中");
    } else {
        gtk_label_set_text(GTK_LABEL(label), "ミュート中");
    } 
    return TRUE;
}

void *display_GUI() {
    gtk_init(NULL, NULL);

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(window), 500, 500);

    g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    GtkWidget *box1 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    GtkWidget *box_23 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    GtkWidget *box4 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

    gtk_widget_set_size_request(GTK_WIDGET(box), 500, 500);
    gtk_widget_set_size_request(GTK_WIDGET(box1), 500, 50);
    gtk_widget_set_size_request(GTK_WIDGET(box_23), 500, 400);
    gtk_widget_set_size_request(GTK_WIDGET(box4), 500, 50);

    // 設定:button1
    GtkWidget *button1 = gtk_button_new_with_label("保留");

    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(provider, "button1 { background-color: #f0f0f0; }", -1, NULL);

    GtkStyleContext *context = gtk_widget_get_style_context(button1);
    gtk_style_context_add_provider(context, GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_USER);
    
    g_object_unref(provider);

    gtk_widget_set_size_request(GTK_WIDGET(button1), 100, 30);
    gtk_widget_set_margin_top(button1, 10);
    gtk_widget_set_margin_bottom(button1, 10);
    gtk_widget_set_margin_start(button1, 10);
    gtk_widget_set_margin_end(button1, 5);

    // 設定:entry
    GtkWidget *entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry), "ここにメッセージを入力...");

    gtk_widget_set_size_request(GTK_WIDGET(entry), 370, 30);
    gtk_widget_set_margin_top(entry, 10);
    gtk_widget_set_margin_bottom(entry, 10);
    gtk_widget_set_margin_start(entry, 5);
    gtk_widget_set_margin_end(entry, 10);

    // 設定:text_view1
    GtkWidget *text_view1 = gtk_text_view_new();
    GtkTextBuffer *buffer1 = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view1));
    gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view1), FALSE);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(text_view1), FALSE);

    Pair pair1 = {
        .textview = text_view1,
        .buf = buffer1,
    };

    GtkCssProvider *provider1 = gtk_css_provider_new();
    gtk_css_provider_load_from_data(provider1, "textview { background-color: #f0f0f0; }", -1, NULL);

    GtkStyleContext *context1 = gtk_widget_get_style_context(text_view1);
    gtk_style_context_add_provider(context1, GTK_STYLE_PROVIDER(provider1), GTK_STYLE_PROVIDER_PRIORITY_USER);
    
    g_object_unref(provider1);

    GtkWidget *scrolled_window1 = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(scrolled_window1), text_view1);
    gtk_widget_set_size_request(scrolled_window1, 230, 380);

    gtk_widget_set_margin_top(scrolled_window1, 10);
    gtk_widget_set_margin_bottom(scrolled_window1, 10);
    gtk_widget_set_margin_start(scrolled_window1, 10);
    gtk_widget_set_margin_end(scrolled_window1, 10);


    // 設定:text_view2
    GtkWidget *text_view2 = gtk_text_view_new();
    GtkTextBuffer *buffer2 = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view2));
    gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view2), FALSE);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(text_view2), FALSE);

    Pair pair2 = {
        .textview = text_view2,
        .buf = buffer2,
    };

    GtkCssProvider *provider2 = gtk_css_provider_new();
    gtk_css_provider_load_from_data(provider2, "textview { background-color: #f0f0f0; }", -1, NULL);

    GtkStyleContext *context2 = gtk_widget_get_style_context(text_view2);
    gtk_style_context_add_provider(context2, GTK_STYLE_PROVIDER(provider2), GTK_STYLE_PROVIDER_PRIORITY_USER);
    
    g_object_unref(provider2);

    GtkWidget *scrolled_window2 = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(scrolled_window2), text_view2);
    gtk_widget_set_size_request(scrolled_window2, 230, 380);

    gtk_widget_set_margin_top(scrolled_window2, 10);
    gtk_widget_set_margin_bottom(scrolled_window2, 10);
    gtk_widget_set_margin_start(scrolled_window2, 10);
    gtk_widget_set_margin_end(scrolled_window2, 10);

    // 設定:label1
    GtkWidget *label1 = gtk_label_new("通話中");
    GtkCssProvider *prov1 = gtk_css_provider_new();
    gtk_css_provider_load_from_data(prov1, "label1 { background-color: #f0f0f0; }", -1, NULL);

    GtkStyleContext *cont1 = gtk_widget_get_style_context(label1);
    gtk_style_context_add_provider(cont1, GTK_STYLE_PROVIDER(prov1), GTK_STYLE_PROVIDER_PRIORITY_USER);
    
    g_object_unref(prov1);

    gtk_widget_set_size_request(label1, 230, 30);

    gtk_widget_set_margin_top(label1, 10);
    gtk_widget_set_margin_bottom(label1, 10);
    gtk_widget_set_margin_start(label1, 10);
    gtk_widget_set_margin_end(label1, 10);

    // 設定:label2
    GtkWidget *label2 = gtk_label_new("通話中");
    GtkCssProvider *prov2 = gtk_css_provider_new();
    gtk_css_provider_load_from_data(prov2, "label2 { background-color: #f0f0f0;}", -1, NULL);

    GtkStyleContext *cont2 = gtk_widget_get_style_context(label2);
    gtk_style_context_add_provider(cont2, GTK_STYLE_PROVIDER(prov2), GTK_STYLE_PROVIDER_PRIORITY_USER);
    
    g_object_unref(prov2);
    
    gtk_widget_set_size_request(label2, 230, 30);

    gtk_widget_set_margin_top(label2, 10);
    gtk_widget_set_margin_bottom(label2, 10);
    gtk_widget_set_margin_start(label2, 10);
    gtk_widget_set_margin_end(label2, 10);

    // 配置: box1
    gtk_box_pack_start(GTK_BOX(box1), button1, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box1), entry, FALSE, FALSE, 0);
    
    gtk_widget_set_halign(button1, GTK_ALIGN_START);

    // 配置: box_23
    gtk_box_pack_start(GTK_BOX(box_23), scrolled_window1, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box_23), scrolled_window2, FALSE, FALSE, 0);

    gtk_widget_set_halign(scrolled_window1, GTK_ALIGN_START);
    gtk_widget_set_halign(scrolled_window2, GTK_ALIGN_END);

    // 配置: box4
    gtk_box_pack_start(GTK_BOX(box4), label1, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box4), label2, FALSE, FALSE, 0);

    // 配置: box
    gtk_container_add(GTK_CONTAINER(box), box4);
    gtk_container_add(GTK_CONTAINER(box), box_23);
    gtk_container_add(GTK_CONTAINER(box), box1);

    gtk_widget_set_valign(box1, GTK_ALIGN_END);
    gtk_widget_set_valign(box_23, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(box4, GTK_ALIGN_START);

    gtk_container_add(GTK_CONTAINER(window), box);

    g_signal_connect(G_OBJECT(button1), "clicked", G_CALLBACK(change_mute), label1);
    g_signal_connect(entry, "activate", G_CALLBACK(entry_activate), &pair1);

    // 時間処理
    g_timeout_add(300, check_recv, &pair2); // 300msごと
    g_timeout_add(300, check_muted_partner, label2);

    gtk_widget_show_all(window);

    gtk_main();

    return NULL;
}