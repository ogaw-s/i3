#include <gtk/gtk.h>

extern int muted;

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

void *display_GUI() {
    gtk_init(NULL, NULL);

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);
    
    g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);

    GtkWidget *button1 = gtk_button_new_with_label("保留ボタン");
    
    // GtkWidget *button2 = gtk_button_new_with_label("ボタン2");
    // GtkWidget *button3 = gtk_button_new_with_label("ボタン3");

    gtk_box_pack_start(GTK_BOX(box), button1, TRUE, TRUE, 0);
    // gtk_box_pack_start(GTK_BOX(box), button2, TRUE, TRUE, 0);
    // gtk_box_pack_start(GTK_BOX(box), button3, TRUE, TRUE, 0);

    g_signal_connect(G_OBJECT(button1), "clicked", G_CALLBACK(change_mute), &muted);

    gtk_container_add(GTK_CONTAINER(window), box);

    gtk_widget_show_all(window);

    gtk_main();

    return NULL;
}