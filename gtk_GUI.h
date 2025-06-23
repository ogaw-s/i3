#ifndef GTK_GUI_H
#define GTK_GUI_H

#include <gtk/gtk.h>

extern int muted; // main.c または別の場所で定義されていることを前提

void change_mute(GtkWidget *button, gpointer data);

void *display_GUI();

// 受信したチャットメッセージをGUIに表示する関数の宣言
void display_received_chat(const char *message);

#endif // GTK_GUI_H