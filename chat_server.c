#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/select.h>
#include <time.h>

#define MAX_CLIENTS 100

typedef struct {
    int fd;
    char name[50];
    int registered; // 0: chua dang nhap, 1: da dang nhap
} ClientInfo;

int main() {
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(8080);

    bind(listener, (struct sockaddr *)&addr, sizeof(addr));
    listen(listener, 5);

    ClientInfo clients[MAX_CLIENTS];
    int nClients = 0;
    fd_set fdread;

    printf("Chat Server is running on port 8080...\n");

    while (1) {
        FD_ZERO(&fdread);
        FD_SET(listener, &fdread);
        int max_fd = listener;

        for (int i = 0; i < nClients; i++) {
            FD_SET(clients[i].fd, &fdread);
            if (clients[i].fd > max_fd) max_fd = clients[i].fd;
        }

        select(max_fd + 1, &fdread, NULL, NULL, NULL);

        // Chap nhan ket noi moi
        if (FD_ISSET(listener, &fdread)) {
            int client = accept(listener, NULL, NULL);
            if (nClients < MAX_CLIENTS) {
                clients[nClients].fd = client;
                clients[nClients].registered = 0;
                nClients++;
                send(client, "Hay nhap 'client_id: client_name' de bat dau:\n", 46, 0);
            } else {
                close(client);
            }
        }

        // Xu ly tin nhan tu clients
        for (int i = 0; i < nClients; i++) {
            if (FD_ISSET(clients[i].fd, &fdread)) {
                char buf[256];
                int ret = recv(clients[i].fd, buf, sizeof(buf) - 1, 0);
                if (ret <= 0) {
                    close(clients[i].fd);
                    clients[i] = clients[nClients - 1];
                    nClients--;
                    i--;
                    continue;
                }
                buf[ret] = 0;

                if (clients[i].registered == 0) {
                    // Kiem tra cu phap: client_id: client_name
                    char id[50], name[50];
                    if (sscanf(buf, "%[^:]: %s", id, name) == 2) {
                        strcpy(clients[i].name, id);
                        clients[i].registered = 1;
                        send(clients[i].fd, "Dang nhap thanh cong!\n", 22, 0);
                    } else {
                        send(clients[i].fd, "Sai cu phap! Nhap lai 'client_id: client_name':\n", 48, 0);
                    }
                } else {
                    // Da dang nhap -> Gui cho cac client khac
                    time_t rawtime;
                    struct tm *timeinfo;
                    char time_str[30], out_msg[512];
                    
                    time(&rawtime);
                    timeinfo = localtime(&rawtime);
                    strftime(time_str, sizeof(time_str), "%Y/%m/%d %I:%M:%S%p", timeinfo);
                    
                    sprintf(out_msg, "%s %s: %s", time_str, clients[i].name, buf);
                    
                    for (int j = 0; j < nClients; j++) {
                        if (clients[j].fd != clients[i].fd && clients[j].registered) {
                            send(clients[j].fd, out_msg, strlen(out_msg), 0);
                        }
                    }
                }
            }
        }
    }
    return 0;
}