#include "chat_stream.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
// #include <sox.h> // Chat StreamではSoXは通常不要なのでコメントアウトまたは削除
#include <unistd.h>
#include <sys/socket.h>

// GUI更新関数を呼び出すためにgtk_GUI.hをインクルード
#include "gtk_GUI.h" // 必要

#define CHAT_BUF 2048

extern int sock2;
extern int muted;

// main.c の send_chat_thread でこの関数が使われるので、
// 標準入力からのチャット送信の役割を持たせる
void *send_chat(void *arg) {
    char msg[CHAT_BUF];

    // 標準入力から読み込み、ネットワークに送信するループ
    while (fgets(msg, sizeof(msg), stdin)) {
        msg[strcspn(msg, "\n")] = '\0'; // 改行をとる

        char buf[CHAT_BUF + 6]; // "CHAT:" + '\n' + '\0' のためのスペース

        // /m コマンドの処理は、GUIからの入力とコマンドラインからの入力の両方で必要なら残す
        // GUIでミュートボタンがあるなら、コマンドラインからの/mは不要な場合も
        if (strcmp(msg, "/m") == 0) {
            muted = !muted; // グローバル変数 muted を切り替える
            snprintf(buf, sizeof(buf), "CHAT:%s\n",
                     muted ? "相手がミュートしました" : "相手がミュートを解除しました");
            // コマンドラインからミュート変更が行われた場合もGUIに通知することを検討
            // display_received_chat(g_strdup_printf("[システム] %s\n", muted ? "ミュートしました" : "ミュートを解除しました"));
        }
        // /yeah コマンドの処理
        // send_audio_file 関数は audio_effects.h にあります
        // 必要なら audio_effects.h をインクルードし、send_audio_file を宣言
        // 例えば、#include "audio_effects.h" をファイルの先頭に追加
        // } else if (strcmp(msg, "/yeah") == 0){
        //     send_audio_file("audiofiles/yeah.wav"); // この関数が利用可能なら
        else {
            snprintf(buf, sizeof(buf), "CHAT:%s\n", msg);
        }

        ssize_t sent = write(sock2, buf, strlen(buf));

        if (sent < 0) {
            perror("send_chat: write");
            break;
        }
        // 標準入力からの送信の場合も、自分のGUIに表示したいなら display_received_chat を使う
        // display_received_chat(g_strdup_printf("[自分(CLI)] %s", msg));
    }
    return NULL;
}


void *recv_chat(void *arg) {
    char buf[CHAT_BUF];
    ssize_t n;
    while ((n = read(sock2, buf, sizeof(buf))) > 0) {
        // 受信したデータをヌル終端する
        if ((size_t)n < sizeof(buf)) {
            buf[n] = '\0';
        } else {
            buf[sizeof(buf) - 1] = '\0'; // バッファの終端を保証
        }

        // 受信メッセージの先頭が "CHAT:" で始まるかを確認
        if (n >= 5 && strncmp(buf, "CHAT:", 5) == 0) {
            // "CHAT:" プレフィックスを除いたメッセージ本体を抽出
            char *msg_content = buf + 5;
            // GUIに表示するメッセージをフォーマット
            // g_strdup_printf は GLib の関数で、文字列を動的にフォーマットして新しいメモリにコピーする
            char *msg_to_display = g_strdup_printf("[相手] %s", msg_content);
            display_received_chat(msg_to_display); // GUI更新関数を呼び出す
            continue; // 次のメッセージへ
        }

        // "CHAT:" プレフィックスがない場合は、そのまま表示（デバッグ用か、未知のプロトコル）
        char *raw_msg_to_display = g_strdup_printf("[未分類] %s", buf);
        display_received_chat(raw_msg_to_display);
    }

    if (n < 0) {
        perror("recv_chat: read");
    }
    return NULL;
}