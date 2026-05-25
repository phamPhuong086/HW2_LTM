#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

// Cấu trúc để lưu cặp client đã ghép
typedef struct {
    int client1;
    int client2;
} client_pair_t;

int waiting_client = -1; // Biến lưu client đang chờ ghép cặp
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; 

void *chat_thread(void *params) {
    client_pair_t pair = *(client_pair_t *)params;
    free(params); 

    int client_from = pair.client1;
    int client_to = pair.client2;
    char buf[256];

    while (1) {
        int len = recv(client_from, buf, sizeof(buf), 0);
        if (len <= 0) {
            // Nếu client_from ngắt kết nối, thông báo và ngắt luôn client_to
            printf("Client %d disconnected. Closing partner %d...\n", client_from, client_to);
            break;
        }
        
        // Chuyển tiếp tin nhắn sang client còn lại
        send(client_to, buf, len, 0);
    }

    close(client_from);
    close(client_to);
    return NULL;
}

int main() {
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    
    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(9000);
    
    bind(listener, (struct sockaddr *)&addr, sizeof(addr));
    listen(listener, 10);
    
    printf("Chat Server (Pairing) is listening on port 9000...\n");

    while (1) {
        int client = accept(listener, NULL, NULL);
        printf("New client connected: %d\n", client);

        pthread_mutex_lock(&mutex);
        if (waiting_client == -1) {
            waiting_client = client;
            char *msg = "Waiting for a partner...\n";
            send(client, msg, strlen(msg), 0);
            pthread_mutex_unlock(&mutex);
        } else {
            // Đã có người chờ, thực hiện ghép cặp
            int partner = waiting_client;
            waiting_client = -1; // Reset hàng đợi
            pthread_mutex_unlock(&mutex);

            printf("Pairing client %d and %d\n", partner, client);
            send(partner, "Partner found! You can chat now.\n", 33, 0);
            send(client, "Partner found! You can chat now.\n", 33, 0);

            client_pair_t *p1 = malloc(sizeof(client_pair_t));
            p1->client1 = partner; p1->client2 = client;
            pthread_t tid1;
            pthread_create(&tid1, NULL, chat_thread, p1);
            pthread_detach(tid1);

            client_pair_t *p2 = malloc(sizeof(client_pair_t));
            p2->client1 = client; p2->client2 = partner;
            pthread_t tid2;
            pthread_create(&tid2, NULL, chat_thread, p2);
            pthread_detach(tid2);
        }
    }

    close(listener);
    return 0;
}