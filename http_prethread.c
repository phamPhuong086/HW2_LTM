#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

int listener;

void *worker_thread(void *arg) {
    while (1) {
        int client = accept(listener, NULL, NULL);
        if (client < 0) continue;

        char buf[2048];
        // Nhận dữ liệu để làm sạch buffer của socket
        int ret = recv(client, buf, sizeof(buf) - 1, 0);
        if (ret > 0) {
            buf[ret] = 0;
        }

        char *response = 
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html; charset=utf-8\r\n"
            "Content-Length: 52\r\n"
            "Connection: close\r\n"
            "\r\n" 
            "<html><body><h1>Xin chào các bạn từ Port 9000</h1></body></html>";
            
        send(client, response, strlen(response), 0);
        printf("Thread %ld đã xử lý xong 1 yêu cầu\n", (long)pthread_self());
        
        close(client);
    }
    return NULL;
}

int main() {
    listener = socket(AF_INET, SOCK_STREAM, 0);
    if (listener < 0) {
        perror("socket() failed");
        return 1;
    }

    int opt = 1;
    setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(9000);

    if (bind(listener, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind() failed"); 
        return 1;
    }

    if (listen(listener, 20) < 0) {
        perror("listen() failed");
        return 1;
    }

    printf("HTTP Prethreading Server đang chạy tại: http://localhost:9000\n");

    pthread_t tids[8];
    for (int i = 0; i < 8; i++) {
        pthread_create(&tids[i], NULL, worker_thread, NULL);
    }

    // Giữ luồng chính
    for (int i = 0; i < 8; i++) {
        pthread_join(tids[i], NULL);
    }

    close(listener);
    return 0;
}