#include "tcp_stream.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

// ソケットを作る
int setup_socket(int is_server, const char *ip, int port) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket");
        return -1;
    }

    if (is_server) {
        //サーバー側でソケットをつくる
        struct sockaddr_in serv_addr = {0};
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        serv_addr.sin_port = htons(port);

        bind(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
        listen(sock, 1);
        printf("Waiting for connection on port %d...\n", port);

        struct sockaddr_in client_addr;
        socklen_t len = sizeof(client_addr);
        int client_sock = accept(sock, (struct sockaddr *)&client_addr, &len);
        if (client_sock < 0) {
            perror("accept");
            close(sock);
            return -1;
        }
        close(sock);
        return client_sock;
    } else {
        //クライアント側としてソケットを作る
        struct sockaddr_in server_addr = {0};
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port);
        inet_pton(AF_INET, ip, &server_addr.sin_addr);

        if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
            perror("connect");
            close(sock);
            return -1;
        }
        return sock;
    }
}
