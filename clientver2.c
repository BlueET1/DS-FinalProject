#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[1024] = {0};

    // 建立客戶端 socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        return -1;
    }

    // 設置伺服端地址
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // 轉換伺服端 IP 地址
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        perror("Invalid address/Address not supported");
        return -1;
    }

    // 嘗試連線
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection failed");
        return -1;
    }
    printf("Connected to server.\n");

    // 與伺服端互動
    while (1) {
        printf("Enter commend: ");
        fgets(buffer, sizeof(buffer), stdin);
        // printf("%s", buffer);

        buffer[strcspn(buffer, "\n")] = 0; // 移除換行符
        

        // 檢查是否輸入 'exit'
        if (strcmp(buffer, "exit") == 0) {
            printf("Disconnecting from server...\n");
            break;
        }

        else if (strcmp(buffer, "ls") == 0){
            send(sock, buffer, strlen(buffer), 0);
            memset(buffer, 0, sizeof(buffer));
            int bytes_read = read(sock, buffer, sizeof(buffer) - 1);
            while(1){
                if (bytes_read > 0) {
                    buffer[bytes_read] = '\0'; // 確保字串以 '\0' 結尾
                    break;
                }
            }
            printf("\nServices are: \n%s\n", buffer);
        }
        else if (strncmp(buffer, "get", 3) == 0){
            // 發送請求到伺服端
            // printf("進cmd處理\n");
            send(sock, buffer, strlen(buffer), 0);
            memset(buffer, 0, sizeof(buffer));
            // 接收伺服器的回應
            int total_bytes_read = 0;
            int bytes_read = 0;
            // printf("進迴圈之前\n");

            while ((bytes_read = read(sock, buffer + total_bytes_read, sizeof(buffer) - total_bytes_read - 1)) > 0) {
                // printf("進入迴圈，已讀取 %d 字節\n", bytes_read);
                total_bytes_read += bytes_read;

                // 如果接收到 END\n，表示數據接收完成
                if (strstr(buffer, "END") != NULL) {
                    // printf("找到END\n");
                    break;
                }
                else if(strstr(buffer, "ERROR") != NULL){
                    break;
                }

            }

            // 輸出處理後的結果
            // printf("判斷式之前\n");
            if (total_bytes_read > 0) {
                buffer[total_bytes_read] = '\0'; // 確保字串結尾
                // printf("判斷式之後，已讀取 %d 字節\n", total_bytes_read);

                // 去除 END 部分
                char *end_marker = strstr(buffer, "END\n");
                if (end_marker != NULL) {
                    *end_marker = '\0'; // 用空字元終止字串，去除 END
                }

                // 打印處理後的伺服器回應
                printf("\n%s\n", buffer);
            } else {
                printf("Failed to receive data from server.\n");
            }
        }
        else if (strncmp(buffer, "new", 3) == 0){
            send(sock, buffer, strlen(buffer), 0);
            memset(buffer, 0, sizeof(buffer));
            int bytes_read = read(sock, buffer, sizeof(buffer) - 1);
            while(1){
                if (bytes_read > 0) {
                    buffer[bytes_read] = '\0'; // 確保字串以 '\0' 結尾
                    break;
                }
            }
            printf("\n%s\n\n", buffer);
        }
        else if(strncmp(buffer, "up", 2) == 0){
            //check ID
            char temp[1024] = {0};
            strcpy(temp, buffer);
            const char *prefix = "check"; // 要插入的字串
            size_t prefix_len = strlen(prefix);
            size_t buffer_len = strlen(buffer);

            // 確保總長度不超過 buffer 的大小
            if (prefix_len + buffer_len < sizeof(buffer)) {
                // 移動原始字串，為 prefix 騰出空間
                memmove(buffer + prefix_len, buffer, buffer_len + 1); // +1 用於包含 '\0'
                // 複製 prefix 到 buffer 的頭部
                memcpy(buffer, prefix, prefix_len);
            } 
            send(sock, buffer, strlen(buffer), 0);
            memset(buffer, 0, sizeof(buffer));
            int bytes_read = read(sock, buffer, sizeof(buffer) - 1);
            while(1){
                if (bytes_read > 0) {
                    buffer[bytes_read] = '\0'; // 確保字串以 '\0' 結尾
                    break;
                }
            }
            printf("\n%s\n\n",buffer);
            if(strncmp(buffer, "Password:",9) == 0){
                memmove(buffer, buffer + strlen("Password:"), strlen(buffer) - strlen("Password:") + 1);
                char userinput[100]={0};
                printf("Please type in original password: ");
                fgets(userinput, sizeof(userinput), stdin);
                size_t len = strlen(userinput);
                if (len > 0 && userinput[len - 1] == '\n') {
                    userinput[len - 1] = '\0';
                }
                if (strcmp(buffer,userinput) == 0) {
                printf("\n - identity checked");
                send(sock, temp, strlen(temp), 0);
                memset(buffer, 0, sizeof(buffer));
                int bytes_read = read(sock, buffer, sizeof(buffer) - 1);
                while(1){
                    if (bytes_read > 0) {
                        buffer[bytes_read] = '\0'; // 確保字串以 '\0' 結尾
                        break;
                    }
                }
                printf("\n%s\n", buffer);
                } else{
                printf("\nWrong Password, Please try again\n");
                }
            } else{
                printf("//pass idenity check\n\n");
            }
            
        }
        else if(strncmp(buffer, "del", 3) == 0){
            //check ID
            char temp[1024] = {0};
            strcpy(temp, buffer);
            const char *prefix = "check2"; // 要插入的字串
            size_t prefix_len = strlen(prefix);
            size_t buffer_len = strlen(buffer);

            // 確保總長度不超過 buffer 的大小
            if (prefix_len + buffer_len < sizeof(buffer)) {
                // 移動原始字串，為 prefix 騰出空間
                memmove(buffer + prefix_len, buffer, buffer_len + 1); // +1 用於包含 '\0'
                // 複製 prefix 到 buffer 的頭部
                memcpy(buffer, prefix, prefix_len);
            } 
            send(sock, buffer, strlen(buffer), 0);
            memset(buffer, 0, sizeof(buffer));
            int bytes_read = read(sock, buffer, sizeof(buffer) - 1);
            while(1){
                if (bytes_read > 0) {
                    buffer[bytes_read] = '\0'; // 確保字串以 '\0' 結尾
                    break;
                }
            }
            printf("\n%s\n\n",buffer);
            if(strncmp(buffer, "Password:",9) == 0){
                memmove(buffer, buffer + strlen("Password:"), strlen(buffer) - strlen("Password:") + 1);
                char userinput[100]={0};
                printf("Please type in original password: ");
                fgets(userinput, sizeof(userinput), stdin);
                size_t len = strlen(userinput);
                if (len > 0 && userinput[len - 1] == '\n') {
                    userinput[len - 1] = '\0';
                }
                if (strcmp(buffer,userinput) == 0) {
                    printf("\n - identity checked\n");
                    send(sock, temp, strlen(temp), 0);
                    memset(buffer, 0, sizeof(buffer));
                    int bytes_read = read(sock, buffer, sizeof(buffer) - 1);
                    while(1){
                        if (bytes_read > 0) {
                            buffer[bytes_read] = '\0'; // 確保字串以 '\0' 結尾
                            break;
                        }
                    }
                    printf("\n%s\n", buffer);
                } else {
                    printf("\nWrong Password, Please try again\n\n");
                }
            } else{
                printf("//pass idenity check\n\n");
            }
        }
        else{
            printf("invalid input. Please try again.\n\n");
        }
    }
    // 關閉 socket
    close(sock);
    return 0;
}