#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/select.h>

int main() {
    int client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (client == -1) {
        perror("socket() failed");
        return 1;
    }
    
    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(8080); 
    
    if (connect(client, (struct sockaddr *)&addr, sizeof(addr))) {
        perror("connect() failed");
        close(client);
        return 1;
    }
    
    printf("Connected to server...\n");
    
    fd_set fdread;
    char buf[256];

    while (1) {
        FD_ZERO(&fdread);
        FD_SET(STDIN_FILENO, &fdread);
        FD_SET(client, &fdread);

        int ret = select(client + 1, &fdread, NULL, NULL, NULL);
        
        if (ret < 0) {
            perror("select() failed");
            break;
        }

        if (FD_ISSET(STDIN_FILENO, &fdread)) {
            if (fgets(buf, sizeof(buf), stdin) != NULL) {
                // Xóa ký tự xuống dòng
                buf[strcspn(buf, "\r\n")] = 0;
                send(client, buf, strlen(buf), 0);
            }
        }

        if (FD_ISSET(client, &fdread)) {
            ret = recv(client, buf, sizeof(buf) - 1, 0);
            if (ret <= 0) {
                printf("Server disconnected.\n");
                break;
            }
            buf[ret] = 0;
            printf("Received: %s\n", buf);
        }
    }

    close(client);
    return 0;
}