#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

void *telnet_thread(void *arg) {
    int client = *(int *)arg;
    free(arg);
    char buf[256], user[64], pass[64];
    
    // Đăng nhập: client gửi "username password"
    send(client, "Nhap username password: ", 24, 0);
    int len = recv(client, buf, sizeof(buf) - 1, 0);
    if (len <= 0) { close(client); return NULL; }
    buf[len] = 0;
    sscanf(buf, "%s %s", user, pass);

    FILE *f = fopen("users.txt", "r");
    char f_user[64], f_pass[64];
    int auth = 0;
    if (f) {
        while (fscanf(f, "%s %s", f_user, f_pass) != EOF) {
            if (strcmp(user, f_user) == 0 && strcmp(pass, f_pass) == 0) {
                auth = 1; break;
            }
        }
        fclose(f);
    }

    if (!auth) {
        send(client, "Login Failed\n", 13, 0);
        close(client); return NULL;
    }
    send(client, "Login Success. Enter command:\n", 30, 0);

    while (1) {
        len = recv(client, buf, sizeof(buf) - 1, 0);
        if (len <= 0) break;
        buf[len] = 0;
        buf[strcspn(buf, "\r\n")] = 0;

        char cmd[512];
        sprintf(cmd, "%s > out.txt 2>&1", buf); // Chạy lệnh và lưu kết quả
        system(cmd);

        FILE *res = fopen("out.txt", "r");
        if (res) {
            while (fgets(buf, sizeof(buf), res)) send(client, buf, strlen(buf), 0);
            fclose(res);
        }
        send(client, "\nDone.\n", 7, 0);
    }
    close(client);
    return NULL;
}

int main() {
    int listener = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(9001);
    bind(listener, (struct sockaddr *)&addr, sizeof(addr));
    listen(listener, 10);

    while (1) {
        int *c = malloc(sizeof(int));
        *c = accept(listener, NULL, NULL);
        pthread_t tid;
        pthread_create(&tid, NULL, telnet_thread, c);
        pthread_detach(tid);
    }
    return 0;
}