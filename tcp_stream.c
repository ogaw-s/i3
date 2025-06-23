#include "tcp_stream.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

int setup_socket(int is_server, const char *ip, int port)
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket");
        return -1;
    }

    if (is_server) {
        /*----- サーバ側 -----*/
        /* ポートをすぐ再利用できるようにする */
        int opt = 1;
        if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
            perror("setsockopt");
            close(sock);
            return -1;
        }

        struct sockaddr_in serv_addr = {0};
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        serv_addr.sin_port = htons(port);

        if (bind(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
            perror("bind");
            close(sock);
            return -1;
        }

        if (listen(sock, 1) < 0) {
            perror("listen");
            close(sock);
            return -1;
        }
        printf("Waiting for connection on port %d...\n", port);

        struct sockaddr_in client_addr;
        socklen_t len = sizeof(client_addr);
        int client_sock = accept(sock, (struct sockaddr *)&client_addr, &len);
        if (client_sock < 0) {
            perror("accept");
            close(sock);
            return -1;
        }

        /* 親ソケットは不要になったので閉じる */
        close(sock);
        return client_sock;

    } else {
        /*----- クライアント側 -----*/
        struct sockaddr_in server_addr = {0};
        server_addr.sin_family = AF_INET;
        server_addr.sin_port   = htons(port);

        if (inet_pton(AF_INET, ip, &server_addr.sin_addr) <= 0) {
            perror("inet_pton");
            close(sock);
            return -1;
        }

        if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
            perror("connect");
            close(sock);
            return -1;
        }
        return sock;
    }
}
