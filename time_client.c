#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

int main() {
    int client = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr = {AF_INET, htons(9002), inet_addr("127.0.0.1")};
    
    if (connect(client, (struct sockaddr *)&addr, sizeof(addr))) return 1;

    printf("Nhap lenh (VD: GET_TIME dd/mm/yyyy): \n");
    char buf[256];
    while (1) {
        printf("> ");
        fgets(buf, sizeof(buf), stdin);
        send(client, buf, strlen(buf), 0);

        int len = recv(client, buf, sizeof(buf) - 1, 0);
        if (len <= 0) break;
        buf[len] = 0;
        printf("Server tra ve: %s", buf);
    }
    close(client);
    return 0;
}