#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE] = {0};

    // 建立客戶端 socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        return -1;
    }

    // 定義伺服端地址與埠
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // 轉換 IP 地址
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        perror("Invalid address or Address not supported");
        return -1;
    }

    // 嘗試連接伺服端
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection failed");
        return -1;
    }

    // 接收伺服端訊息
    read(sock, buffer, BUFFER_SIZE);
    printf("Message from server: %s\n", buffer);

    // 關閉 socket
    close(sock);

    return 0;
}
