#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>

void *handle_time(void *arg) {
    int sk = *(int *)arg;
    free(arg);
    char buf[256];
    while (1) {
        int len = recv(sk, buf, sizeof(buf) - 1, 0);
        if (len <= 0) break;
        buf[len] = 0;
        char cmd[32], fmt[32];
        int n = sscanf(buf, "%s %s", cmd, fmt);
        if (strcmp(cmd, "GET_TIME") == 0) {
            time_t now = time(NULL);
            struct tm *t = localtime(&now);
            char res[64];
            if (n < 2) strcpy(res, "Missing format\n");
            else if (strcmp(fmt, "dd/mm/yyyy") == 0) strftime(res, 64, "%d/%m/%Y\n", t);
            else if (strcmp(fmt, "dd/mm/yy") == 0) strftime(res, 64, "%d/%m/%y\n", t);
            else if (strcmp(fmt, "mm/dd/yyyy") == 0) strftime(res, 64, "%m/%d/%Y\n", t);
            else if (strcmp(fmt, "mm/dd/yy") == 0) strftime(res, 64, "%m/%d/%y\n", t);
            else strcpy(res, "Invalid format\n");
            send(sk, res, strlen(res), 0);
        }
    }
    close(sk);
    return NULL;
}

int main() {
    int listener = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr = {AF_INET, htons(9002), htonl(INADDR_ANY)};
    bind(listener, (struct sockaddr *)&addr, sizeof(addr));
    listen(listener, 10);
    while (1) {
        int *sk = malloc(sizeof(int));
        *sk = accept(listener, NULL, NULL);
        pthread_t tid;
        pthread_create(&tid, NULL, handle_time, sk);
        pthread_detach(tid);
    }
    return 0;
}