#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/select.h>

#define MAX_CLIENTS 100

typedef struct {
    int fd;
    int authenticated;
} TelnetClient;

int check_login(char *user, char *pass) {
    FILE *f = fopen("users.txt", "r");
    if (!f) return 0;
    char f_user[50], f_pass[50];
    while (fscanf(f, "%s %s", f_user, f_pass) != EOF) {
        if (strcmp(user, f_user) == 0 && strcmp(pass, f_pass) == 0) {
            fclose(f);
            return 1;
        }
    }
    fclose(f);
    return 0;
}

int main() {
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(8080);

    bind(listener, (struct sockaddr *)&addr, sizeof(addr));
    listen(listener, 5);

    TelnetClient clients[MAX_CLIENTS];
    int nClients = 0;
    fd_set fdread;

    printf("Telnet Server is running on port 8080...\n");

    while (1) {
        FD_ZERO(&fdread);
        FD_SET(listener, &fdread);
        int max_fd = listener;

        for (int i = 0; i < nClients; i++) {
            FD_SET(clients[i].fd, &fdread);
            if (clients[i].fd > max_fd) max_fd = clients[i].fd;
        }

        select(max_fd + 1, &fdread, NULL, NULL, NULL);

        if (FD_ISSET(listener, &fdread)) {
            int client = accept(listener, NULL, NULL);
            clients[nClients].fd = client;
            clients[nClients].authenticated = 0;
            nClients++;
            send(client, "Login (user pass): ", 19, 0);
        }

        for (int i = 0; i < nClients; i++) {
            if (FD_ISSET(clients[i].fd, &fdread)) {
                char buf[256];
                int ret = recv(clients[i].fd, buf, sizeof(buf) - 1, 0);
                if (ret <= 0) {
                    close(clients[i].fd);
                    clients[i] = clients[nClients - 1];
                    nClients--; i--; continue;
                }
                buf[ret] = 0;
                
                if (buf[ret-1] == '\n') buf[ret-1] = 0;

                if (!clients[i].authenticated) {
                    char user[50], pass[50];
                    if (sscanf(buf, "%s %s", user, pass) == 2 && check_login(user, pass)) {
                        clients[i].authenticated = 1;
                        send(clients[i].fd, "Login Success. Enter command: ", 30, 0);
                    } else {
                        send(clients[i].fd, "Login Failed. Try again: ", 25, 0);
                    }
                } else {
                    // Thuc thi lenh
                    char cmd[300], response[2048];
                    sprintf(cmd, "%s > out.txt", buf); 
                    system(cmd);

                    FILE *f = fopen("out.txt", "r");
                    int n = fread(response, 1, sizeof(response), f);
                    send(clients[i].fd, response, n, 0);
                    fclose(f);
                }
            }
        }
    }
    return 0;
}