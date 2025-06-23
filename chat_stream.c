#include "chat_stream.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sox.h> // このヘッダーがchat_stream.cで本当に必要か確認
#include <unistd.h>
#include <sys/socket.h>

// GUI更新関数を呼び出すためにgtk_GUI.hをインクルード
#include "gtk_GUI.h" // ★★★★★ この行を追加 ★★★★★

#define CHAT_BUF 2048

extern int sock2;
extern int muted;
// extern sox_format_t *in, *out; // chat_stream.c でこれらが必要か確認

void *send_chat(void *arg) {
    // ... (この関数は変更なし) ...
    char msg[CHAT_BUF];

    while (fgets(msg, sizeof(msg), stdin)) {
        msg[strcspn(msg, "\n")] = '\0'; //改行をとる

        char buf[CHAT_BUF + 6];
        if (strcmp(msg, "/m") == 0) {
            muted = !muted;
            snprintf(buf, sizeof(buf), "CHAT:%s\n",
                     muted ? "相手がミュートしました"
                           : "相手がミュートを解除しました");
        }else if (strcmp(msg, "/yeah") == 0){
            // send_audio_file 関数は audio_effects.h にあります
            // 必要なら audio_effects.h をインクルードし、send_audio_file を宣言
            // #include "audio_effects.h"
            // send_audio_file("audiofiles/yeah.wav");
        } else {
            snprintf(buf, sizeof(buf), "CHAT:%s\n", msg);
        }
        ssize_t sent = write(sock2, buf, strlen(buf));

        if (sent < 0) {
            perror("send_chat: write");
            break;
        }
        // GUIへの表示（自分が送信したメッセージ）
        // GUIスレッドから直接呼び出される場合や、メインスレッドへのキューイングが必要な場合がある
        // send_chat_gui 関数が既にdisplay_GUIで処理しているため、ここでの表示は不要かもしれない
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

        if (n >= 5 && strncmp(buf, "CHAT:", 5) == 0) {
            // GUIにチャットメッセージを表示
            // メッセージをコピーして、g_idle_add に渡す
            char *msg_to_display = g_strdup_printf("[CHAT] %s", buf + 5);
            display_received_chat(msg_to_display); // ★★★★★ この行を追加 ★★★★★

            // 元のstderrへの書き込みは不要になるか、デバッグ用に残すか
            // fwrite("[CHAT] ", 1, 7, stderr);
            // fwrite(buf + 5, 1, n - 5, stderr);
            continue;
        }

        // CHAT: プレフィックスがない場合は、そのままstdoutに出力（またはGUIに表示）
        // ここもGUIに表示するならdisplay_received_chatを使う
        char *raw_msg_to_display = g_strdup(buf);
        display_received_chat(raw_msg_to_display); // ★★★★★ この行を追加 ★★★★★

        // 旧来のstdoutへの書き込み
        // if (fwrite(buf, 1, n, stdout) < n) {
        //     perror("recv_chat: fwrite");
        //     break;
        // }
        // fflush(stdout);
    }

    if (n < 0) {
        perror("recv_chat: read");
    }
    return NULL;
}