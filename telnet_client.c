#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

int main() {
    int client = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr = {AF_INET, htons(9001), inet_addr("127.0.0.1")};
    
    if (connect(client, (struct sockaddr *)&addr, sizeof(addr))) return 1;

    char buf[1024];
    // Nhan yeu cau dang nhap
    int len = recv(client, buf, sizeof(buf)-1, 0);
    buf[len] = 0; printf("%s", buf);

    // Gui "user pass"
    fgets(buf, sizeof(buf), stdin);
    send(client, buf, strlen(buf), 0);

    // Luong nhan ket qua lenh
    while (1) {
        len = recv(client, buf, sizeof(buf)-1, 0);
        if (len <= 0) break;
        buf[len] = 0;
        printf("%s", buf);
        
        if (strstr(buf, ">") || strstr(buf, "Success")) {
            fgets(buf, sizeof(buf), stdin);
            send(client, buf, strlen(buf), 0);
        }
    }
    close(client);
    return 0;
}