#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>

typedef struct {
    int socket;
    char name[64];
} client_t;

client_t *clients[100];
int num_clients = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void *client_thread(void *arg) {
    int client_sk = *(int *)arg;
    free(arg);
    char buf[256], name[64];
    
    // Giao thức đăng nhập: client_id: name
    while (1) {
        char *msg = "Hay nhap ten theo cu phap (client_id: ten_cua_ban): ";
        send(client_sk, msg, strlen(msg), 0);
        int len = recv(client_sk, buf, sizeof(buf) - 1, 0);
        if (len <= 0) { close(client_sk); return NULL; }
        buf[len] = 0;

        if (strncmp(buf, "client_id: ", 11) == 0) {
            strcpy(name, buf + 11);
            name[strcspn(name, "\r\n")] = 0; // Xử lý xóa ký tự xuống dòng
            break;
        }
    }

    pthread_mutex_lock(&mutex);
    client_t *new_client = malloc(sizeof(client_t));
    new_client->socket = client_sk;
    strcpy(new_client->name, name);
    clients[num_clients++] = new_client;
    pthread_mutex_unlock(&mutex);

    // Chuyển tiếp tin nhắn kèm thời gian
    while (1) {
        int len = recv(client_sk, buf, sizeof(buf) - 1, 0);
        if (len <= 0) break;
        buf[len] = 0;

        time_t now = time(NULL);
        struct tm *t = localtime(&now);
        char timestamp[32], out_msg[512];
        strftime(timestamp, sizeof(timestamp), "%Y/%m/%d %I:%M:%S%p", t);
        sprintf(out_msg, "%s %s: %s", timestamp, name, buf);

        pthread_mutex_lock(&mutex);
        for (int i = 0; i < num_clients; i++) {
            if (clients[i]->socket != client_sk) 
                send(clients[i]->socket, out_msg, strlen(out_msg), 0);
        }
        pthread_mutex_unlock(&mutex);
    }
    
    close(client_sk);
    return NULL;
}

int main() {
    int listener = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(9000);
    bind(listener, (struct sockaddr *)&addr, sizeof(addr));
    listen(listener, 10);

    while (1) {
        int *client_sk = malloc(sizeof(int));
        *client_sk = accept(listener, NULL, NULL);
        pthread_t tid;
        pthread_create(&tid, NULL, client_thread, client_sk);
        pthread_detach(tid);
    }
    return 0;
}