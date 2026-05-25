#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <dirent.h>
#include <sys/stat.h>

#define PORT 8080
#define STORAGE_DIR "./files" // Thư mục chứa file trên server

void signal_handler(int sig) {
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

void process_client(int client) {
    char buf[256];
    struct dirent *entry;
    DIR *dp = opendir(STORAGE_DIR);
    
    if (dp == NULL) {
        char *msg = "ERROR No files to download\r\n";
        send(client, msg, strlen(msg), 0);
        close(client);
        exit(1);
    }

    int count = 0;
    char list[2048] = "";
    while ((entry = readdir(dp))) {
        if (entry->d_type == DT_REG) { // Chỉ lấy file thông thường
            count++;
            strcat(list, entry->d_name);
            strcat(list, "\r\n");
        }
    }
    
    if (count == 0) {
        char *msg = "ERROR No files to download\r\n";
        send(client, msg, strlen(msg), 0);
        closedir(dp);
        close(client);
        exit(0);
    }

    sprintf(buf, "OK %d\r\n", count);
    send(client, buf, strlen(buf), 0);
    send(client, list, strlen(list), 0);
    send(client, "\r\n", 2, 0);
    closedir(dp);

    while (1) {
        int len = recv(client, buf, sizeof(buf) - 1, 0);
        if (len <= 0) break;
        buf[len] = 0;
        
        // Xóa ký tự xuống dòng nếu có từ client
        if (buf[strlen(buf)-1] == '\n') buf[strlen(buf)-1] = 0;
        if (buf[strlen(buf)-1] == '\r') buf[strlen(buf)-1] = 0;

        char filepath[512];
        sprintf(filepath, "%s/%s", STORAGE_DIR, buf);
        
        FILE *f = fopen(filepath, "rb");
        if (f == NULL) {
            char *error_msg = "File not found. Please try again: \r\n";
            send(client, error_msg, strlen(error_msg), 0);
        } else {
            // Lấy kích thước file
            struct stat st;
            stat(filepath, &st);
            long filesize = st.st_size;

            sprintf(buf, "OK %ld\r\n", filesize);
            send(client, buf, strlen(buf), 0);

            // Gửi nội dung file
            char file_buf[1024];
            int bytes_read;
            while ((bytes_read = fread(file_buf, 1, sizeof(file_buf), f)) > 0) {
                send(client, file_buf, bytes_read, 0);
            }
            
            fclose(f);
            printf("Sent file %s to client.\n", filepath);
            break; // Đóng kết nối sau khi gửi xong theo yêu cầu
        }
    }
    close(client);
    exit(0);
}

int main() {
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(PORT);
    
    bind(listener, (struct sockaddr *)&addr, sizeof(addr));
    listen(listener, 5);
    
    signal(SIGCHLD, signal_handler);
    printf("File Server logic running on port %d...\n", PORT);

    while (1) {
        int client = accept(listener, NULL, NULL);
        if (fork() == 0) {
            close(listener);
            process_client(client);
        }
        close(client);
    }
    return 0;
}