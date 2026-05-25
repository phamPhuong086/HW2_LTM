#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

void *recv_thread(void *arg) {
    int sk = *(int *)arg;
    char buf[1024];
    while (1) {
        int len = recv(sk, buf, sizeof(buf) - 1, 0);
        if (len <= 0) {
            printf("\nMat ket noi den Server.\n");
            exit(0);
        }
        buf[len] = 0;
        printf("\n%s\n> ", buf);
        fflush(stdout);
    }
}

int main() {
    int client = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr = {AF_INET, htons(9000), inet_addr("127.0.0.1")};
    
    if (connect(client, (struct sockaddr *)&addr, sizeof(addr))) return 1;

    pthread_t tid;
    pthread_create(&tid, NULL, recv_thread, &client);

    char buf[256];
    while (1) {
        fgets(buf, sizeof(buf), stdin);
        send(client, buf, strlen(buf), 0);
    }
    return 0;
}