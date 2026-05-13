#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>//讀寫函示
#include <arpa/inet.h>//將 IP 地址在字串和二進制格式之間轉換。
#include <sys/select.h>//提供 I/O 多路複用
#include <sys/time.h>//計時
#include <math.h>
#include "cJSON.c"//用來處理json格式檔案
#define PORT 8080
#define TIMEOUT 600 // 10 分鐘 (秒)

#define MaxChar 256 //only support 256 characters  

typedef struct Node {
    char symbol;           // sumbole
    int freq;              // frequency of the Node 
    struct Node *left;     // left sub-tree
    struct Node *right;    // right sub-tree
} Node;

//create a new node
Node *createNode(char symbol, int freq) {
    Node * node =(Node*)malloc (sizeof(Node));
    node->symbol = symbol;
    node->freq = freq;
    node->left = node->right = NULL; 
    return node;
}

// compare the order  
int compare(const void* a, const void* b) {
    return (*(Node**)a)->freq - (*(Node**)b)->freq;
} 

// calculate the frequence of each characters in the string  
void calculateFrequency(const char *str, int freq[]) {
    // 初始化 freq 陣列  
    memset(freq, 0, sizeof(int) * 256);
    int i;
    // 確保字串有效  
    if (str == NULL || strlen(str) == 0) {
        printf("Error: Input string is empty or NULL.\n");
        return;
    }

    // 計算每個字元的頻率  
    for (i = 0; i < strlen(str); i++) {
        freq[(unsigned char)str[i]]++;
    }
    //int length = strlen(str); correct check : 1 
    //printf("there are %d letters in this string",length) ;
    // 打印結果供檢查  
    /*for (i = 0; i < 256; i++) {
        if (freq[i] > 0) {
            printf("Character '%c' appears %d times\n", i, freq[i]);
        }
    }*/
    // correction check:1 
    }

//build the huffman tree
Node *buildHuffmanTree(int freq[]){
 // heap is a pointer array which point to the Node 
    Node *heap[MaxChar]={NULL};
    int i , heapsize = 0;
 // initiate the huffman tree 
    for (i=0; i<MaxChar; i++){
        if (freq[i]>0){ 
  // only let the frequence >0 to be put in the tree
        heap[heapsize++]=createNode((char)i,freq[i]);
  // printf("the frequence of %c is %d\n",heap[heapsize-1]->symbol,heap[heapsize-1]->freq) ;
  //correction check: 1 
        }
    }
    while (heapsize >1){
        qsort(heap, heapsize, sizeof(Node*),compare);
        Node *left = heap[0];
        Node *right = heap[1];
        // combine left and right into the new node  
        Node *merged =createNode ('\0', left->freq + right->freq );
        merged->left = left;
        merged->right = right; 
        // update the heap array 
        heap [0] = merged;
        heap [1] = heap[--heapsize];
        }
    return heap[0];
    }
// base on the character that huffman tree has conserved 
// generate the HuffmanCodes
void generateHuffmanCodes(Node* root, char* code, int depth, char codes[MaxChar][MaxChar]) {
    if (!root) return;
    // record the code if it is the leaf node
    if (!root->left && !root->right) {
        code[depth] = '\0';
        strcpy(codes[(unsigned char)root->symbol], code);
        return;
    }
    // using recursion to work the left,right sub tree 
    code[depth] = '0';
    generateHuffmanCodes(root->left, code, depth + 1, codes);
 // this is the right sub tree
    code[depth] = '1';
    generateHuffmanCodes(root->right, code, depth + 1, codes);
}
void calculate_huffman_lengths(Node *root, int depth, double *avg_length, int total_chars) {
    if (!root->left && !root->right) {
        *avg_length += (double)root->freq / total_chars * depth;
        return;
    }
    if (root->left) calculate_huffman_lengths(root->left, depth + 1, avg_length, total_chars);
    if (root->right) calculate_huffman_lengths(root->right, depth + 1, avg_length, total_chars);
}
// 字符種類數 
int check_character_types(const char *password) {
    int has_lower = 0, has_upper = 0, has_digit = 0, has_special = 0,i;
    for (i = 0; password[i] != '\0'; i++) {
        if (islower(password[i])) has_lower = 1;
        else if (isupper(password[i])) has_upper = 1;
        else if (isdigit(password[i])) has_digit = 1;
        else has_special = 1;
    }
    return has_lower + has_upper + has_digit + has_special;
}
//calculate the entropy
double calculate_entropy(char *string, int *freq) {
    int total_chars = strlen(string),i;
    double entropy ;
    for (i=0; i<MaxChar; i++) {
        if(freq[i]>0) {
            double p = (double)freq[i] / total_chars;
            entropy -= p * log2(p); 
        }
    }
    return entropy; 
}
//calculate the total entropy
double calculate_total_entropy(double entropy, char *string) {
    int total_char = strlen(string);
    double total_entropy;
    total_entropy = entropy*total_char;
    return total_entropy;
}

// 判斷密碼強度 
const char* evaluate_password(const char *password, double entropy, double avg_length, int char_types) {
    int length = strlen(password);
    if (length < 8 || avg_length - entropy > 2.0) {
        return "weak,建議修改";
    } else if (entropy >= 20 && entropy < 40 && length >= 8) {
        return "medium,強度適中,請定期修改密碼";
    } else if (entropy >= 50 && length >= 12 && char_types >= 3) {
        return "strong,別忘掉喔";
    }
    return "strong";
}

cJSON *read_json_file(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        return cJSON_CreateObject(); // 若檔案不存在，則返回一個新的空 JSON 物件
    }

    fseek(file, 0, SEEK_END);
    long filesize = ftell(file);
    rewind(file);

    char *data = malloc(filesize + 1);
    fread(data, 1, filesize, file);
    data[filesize] = '\0';
    fclose(file);

    cJSON *json = cJSON_Parse(data);
    free(data);

    return json;
}

void write_json_file(const char *filename, cJSON *json) {
    char *data = cJSON_Print(json);
    FILE *file = fopen(filename, "w");
    if (file != NULL) {
        fwrite(data, 1, strlen(data), file);
        fclose(file);
    }
    free(data);
}

// #include "cJSON.h"

void set(const char *filename, int client_socket, const char *service_name, const char *account, const char *password) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Failed to open file");
        return;
    }

    // 讀取檔案內容
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);
    char *file_content = (char *)malloc(length + 1);
    if (!file_content) {
        perror("Failed to allocate memory");
        fclose(file);
        return;
    }
    fread(file_content, 1, length, file);
    file_content[length] = '\0';
    fclose(file);

    // 解析 JSON
    cJSON *root = cJSON_Parse(file_content);
    free(file_content); // 釋放檔案內容記憶體
    if (!root) {
        printf("Error parsing JSON: %s\n", cJSON_GetErrorPtr());
        return;
    }

    // 查找或新增服務名稱的物件
    cJSON *service = cJSON_GetObjectItemCaseSensitive(root, service_name);
    if (!service) {
        service = cJSON_CreateObject();
        cJSON_AddItemToObject(root, service_name, service);
    }

    // 檢查帳號是否已存在
    cJSON *existing_account = cJSON_GetObjectItemCaseSensitive(service, account);
    if (existing_account) {
        printf("account existed\n");
        cJSON_Delete(root);
        send(client_socket,"\naccount existed. Please try again\n", strlen("\naccount existed. Please try again\n"),0);
        return;
    }

    // 新增帳號和密碼
    cJSON_AddStringToObject(service, account, password);

    // 將修改後的 JSON 寫回檔案
    file = fopen(filename, "w");
    if (!file) {
        perror("Failed to open file for writing");
        cJSON_Delete(root);
        return;
    }

    char *updated_content = cJSON_Print(root);
    if (updated_content) {
        fwrite(updated_content, 1, strlen(updated_content), file);
        free(updated_content); // 釋放序列化的 JSON 字串記憶體
    }

    fclose(file);
    cJSON_Delete(root);
    printf("Set operation completed successfully.\n");
    send(client_socket,"\nAdded successfully.\n", strlen("\nAdded successfully.\n"),0);
}

void checking_ID(const char *service_name, const char *account_name, cJSON *root, int client_socket){

    cJSON *service = cJSON_GetObjectItemCaseSensitive(root, service_name);
        if (service != NULL && cJSON_IsObject(service)) {
            // printf("%s:\n", service_name);  // 打印服務名稱
            // send(client_socket, service_name, sizeof(service_name),0);
            // 遍歷該服務的帳號和密碼
            cJSON *account = NULL;
            char password[256]={0};
            printf("checking2\n");
            cJSON_ArrayForEach(account, service) {
                if (cJSON_IsString(account) && strcmp((account->string),account_name) == 0) {
                    snprintf(password, sizeof(password), "Password:%s",cJSON_GetStringValue(account) );
                    send(client_socket,password, strlen(password),0);
                }
            }
            if (strlen(password)==0){
                send(client_socket, "Account not found.", strlen("Account not found."), 0);
            }
        } else{
            send(client_socket,"Service not found", strlen("Service not found"),0);
        }
}

void process_request(const char *service_name, cJSON *root, int client_socket) {

        // 查找該服務名稱在 JSON 中
        cJSON *service = cJSON_GetObjectItemCaseSensitive(root, service_name);
        if (service != NULL && cJSON_IsObject(service)) {
            // int freq [256] = {0};
            // char codes [256][256] ={0};
            // char input[1024];
            char response[1000] = {0};
            snprintf(response, 1000, "%s:\n", service_name); // 準備回應字串
            // printf("%s:\n", service_name);  // 打印服務名稱
            // send(client_socket, service_name, sizeof(service_name),0);
            // 遍歷該服務的帳號和密碼
            cJSON *account = NULL;
            cJSON_ArrayForEach(account, service) {
                if (cJSON_IsString(account)) {
                    char temp[256];
                    snprintf(temp, sizeof(temp), "  - account: %s, password: %s\n", account->string, cJSON_GetStringValue(account));
                    strncat(response, temp, 1000 - strlen(response) - 1);
                    const char *password = cJSON_GetStringValue(account);

                    printf("  - account: %s, password: %s\n", account->string, password);
                    // printf("%s\n", input);
                    // printf("%s\n",cJSON_GetStringValue(account));
                    // strncpy(input, cJSON_GetStringValue(account), sizeof(input)-1);
                    // printf("%s\n", input);
                    // input[strcspn(input, "\n")]='\0';
                    // printf("%s\n", input);
                }
            }
            
            // input the string
            // printf("please type the word:") ;
            // fgets(cJSON_GetStringValue(account),sizeof(cJSON_GetStringValue(account)),stdin);
            
            // calculate the frequence of input
            // calculateFrequency(input,freq);
            // // build the huffman Tree
            // Node *root = buildHuffmanTree(freq);
            // // encode the huffman codes
            // char code[MaxChar]={0};
            // generateHuffmanCodes(root, code, 0, codes);
            // double entropy = calculate_entropy (input, freq);
            // double total_entropy = calculate_total_entropy(entropy,input);
            // // calculate huffman lengths\n
            // double avg_length = 0.0;
            // int length = strlen(input);
            // calculate_huffman_lengths(root, 0, &avg_length, length);
            // int char_types = check_character_types(input);
            // const char* strength = evaluate_password(input, entropy, avg_length, char_types);

            // char temp2[256];
            // snprintf(temp2, sizeof(temp2), "\n密碼熵為： %.2f 位元\n",entropy );
            // strncat(response, temp2, 1000 - strlen(response) - 1);
            // snprintf(temp2, sizeof(temp2), "總密碼熵為： %.2f 位元\n",total_entropy );
            // strncat(response, temp2, 1000 - strlen(response) - 1);
            // snprintf(temp2, sizeof(temp2), "密碼強度建議： %s \n", strength);
            // strncat(response, temp2, 1000 - strlen(response) - 1);
            strncat(response, "END\n", 1000 - strlen(response) - 1);
            printf("%s", response);
            send(client_socket, response, strlen(response), 0);
        }
        else {
            const char *not_found = "ERROR, Service not found.\n";
            printf("Service %s not found.\n", service_name);  // 如果服務名稱不存在
            send(client_socket, not_found, strlen(not_found), 0);
        }
    }
void process_request_precise(const char *service_name, const char *account_name, cJSON *root, int client_socket) {

        // 查找該服務名稱在 JSON 中
        cJSON *service = cJSON_GetObjectItemCaseSensitive(root, service_name);
        if (service != NULL && cJSON_IsObject(service)) {
            int freq [256] = {0};
            char codes [256][256] ={0};
            char input[1024]={0};
            char response[1000] = {0};
            snprintf(response, 1000, "%s:\n", service_name); // 準備回應字串
            // printf("%s:\n", service_name);  // 打印服務名稱
            // send(client_socket, service_name, sizeof(service_name),0);
            // 遍歷該服務的帳號和密碼
            cJSON *account = NULL;
            cJSON_ArrayForEach(account, service) {
                if (cJSON_IsString(account) && strcmp((account->string),account_name) == 0) {
                    char temp[256];
                    snprintf(temp, sizeof(temp), "  - account: %s, password: %s", account->string, cJSON_GetStringValue(account));
                    strncat(response, temp, 1000 - strlen(response) - 1);
                    const char *password = cJSON_GetStringValue(account);

                    printf("  - account: %s, password: %s\n", account->string, password);
                    // printf("%s\n", input);
                    // printf("%s\n",cJSON_GetStringValue(account));
                    strncpy(input, cJSON_GetStringValue(account), sizeof(input)-1);
                    // printf("%s\n", input);
                    input[strcspn(input, "\n")]='\0';
                    printf("%s\n", input);
                }
            }
            
            // input the string
            // printf("please type the word:") ;
            // fgets(cJSON_GetStringValue(account),sizeof(cJSON_GetStringValue(account)),stdin);
            if(strlen(input) != 0){
                calculateFrequency(input,freq);
                // build the huffman Tree
                Node *root = buildHuffmanTree(freq);
                // encode the huffman codes
                char code[MaxChar]={0};
                generateHuffmanCodes(root, code, 0, codes);
                double entropy = calculate_entropy (input, freq);
                double total_entropy = calculate_total_entropy(entropy,input);
                // calculate huffman lengths\n
                double avg_length = 0.0;
                int length = strlen(input);
                calculate_huffman_lengths(root, 0, &avg_length, length);
                int char_types = check_character_types(input);
                const char* strength = evaluate_password(input, total_entropy, avg_length, char_types);

                char temp2[256];
                snprintf(temp2, sizeof(temp2), "\n密碼熵為： %.2f 位元\n",entropy );
                strncat(response, temp2, 1000 - strlen(response) - 1);
                snprintf(temp2, sizeof(temp2), "總密碼熵為： %.2f 位元\n",total_entropy );
                strncat(response, temp2, 1000 - strlen(response) - 1);
                snprintf(temp2, sizeof(temp2), "密碼強度建議： %s \n", strength);
                strncat(response, temp2, 1000 - strlen(response) - 1);
                strncat(response, "END\n", 1000 - strlen(response) - 1);
                printf("%s", response);
                send(client_socket, response, strlen(response), 0);
            }
            else{
                const char *account_not_found = "ERROR, account not found.\n";
                printf("account %s not found.\n", account_name);  // 如果服務名稱不存在
                send(client_socket, account_not_found, strlen(account_not_found), 0);
            }
            // calculate the frequence of input
            
        }
        else {
            const char *not_found = "ERROR, Service not found.";
            printf("Service %s not found.\n", service_name);  // 如果服務名稱不存在
            send(client_socket, not_found, strlen(not_found), 0);
        }
    }

// 讀取 (get) 指定 key 的資料
void get(const char *filename, const char *key) {
    cJSON *json = read_json_file(filename);

    cJSON *entry = cJSON_GetObjectItem(json, key);
    if (entry) {
        char *value = cJSON_Print(entry);
        printf("Key '%s': %s\n", key, value);
        free(value);
    } else {
        printf("Key '%s' not found\n", key);
    }

    cJSON_Delete(json);
}

// 更新 (update) 指定 key 的資料
void up(const char *filename, int client_socket, const char *service_name, const char *account, const char *password) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Failed to open file");
        return;
    }

    // 讀取檔案內容
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);
    char *file_content = (char *)malloc(length + 1);
    if (!file_content) {
        perror("Failed to allocate memory");
        fclose(file);
        return;
    }
    fread(file_content, 1, length, file);
    file_content[length] = '\0';
    fclose(file);

    // 解析 JSON
    cJSON *root = cJSON_Parse(file_content);
    free(file_content); // 釋放檔案內容記憶體
    if (!root) {
        printf("Error parsing JSON: %s\n", cJSON_GetErrorPtr());
        return;
    }

    // 查找服務名稱物件
    cJSON *service = cJSON_GetObjectItemCaseSensitive(root, service_name);
    if (!service) {
        printf("service not found\n");
        cJSON_Delete(root);
        send(client_socket,"\nservice not found. Please try again\n", strlen("\nservice not found. Please try again\n"),0);
        return;
    }

    // 查找帳號是否存在
    cJSON *existing_account = cJSON_GetObjectItemCaseSensitive(service, account);
    if (!existing_account) {
        printf("account not found\n");
        cJSON_Delete(root);
        send(client_socket,"\naccount not found. Please try again\n", strlen("\naccount not found. Please try again\n"),0);
        return;
    }

    // 更新密碼
    cJSON_SetValuestring(existing_account, password);

    // 將修改後的 JSON 寫回檔案
    file = fopen(filename, "w");
    if (!file) {
        perror("Failed to open file for writing");
        cJSON_Delete(root);
        return;
    }

    char *updated_content = cJSON_Print(root);
    if (updated_content) {
        fwrite(updated_content, 1, strlen(updated_content), file);
        free(updated_content); // 釋放序列化的 JSON 字串記憶體
    }

    fclose(file);
    cJSON_Delete(root);
    printf("Update operation completed successfully.\n");
    send(client_socket,"\nUpdated successfully.\n", strlen("\nUpdated successfully.\n"),0);
}

void del2(const char *file_name, int client_socket, const char *service_name, const char *account) {
    // 開啟 JSON 檔案
    FILE *file = fopen(file_name, "r");
    if (file == NULL) {
        send(client_socket, "Error opening file\n", strlen("Error opening file\n"), 0);
        return;
    }

    // 讀取 JSON 檔案內容
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *buffer = (char *)malloc(file_size + 1);
    if (buffer == NULL) {
        send(client_socket, "Memory allocation failed\n", strlen("Memory allocation failed\n"), 0);
        fclose(file);
        return;
    }

    fread(buffer, 1, file_size, file);
    buffer[file_size] = '\0';
    fclose(file);

    // 解析 JSON
    cJSON *root = cJSON_Parse(buffer);
    free(buffer);
    if (root == NULL) {
        send(client_socket, "Error parsing JSON\n", strlen("Error parsing JSON\n"), 0);
        return;
    }

    // 尋找指定服務名稱
    cJSON *service = cJSON_GetObjectItemCaseSensitive(root, service_name);
    if (service == NULL || !cJSON_IsObject(service)) {
        send(client_socket, "Service not found\n", strlen("Service not found\n"), 0);
        cJSON_Delete(root);
        return;
    }

    // 檢查是否有該帳號
    cJSON *entry = cJSON_GetObjectItemCaseSensitive(service, account);
    if (entry == NULL) {
        send(client_socket, "Account not found\n", strlen("Account not found\n"), 0);
        cJSON_Delete(root);
        return;
    }

    // 刪除帳號
    cJSON_DeleteItemFromObject(service, account);
    send(client_socket, "-Deleted successfully.\n", strlen("-Deleted successfully.\n"), 0);

    // 如果服務名稱底下已沒有帳密，刪除該服務
    if (cJSON_GetArraySize(service) == 0) {
        cJSON_DeleteItemFromObject(root, service_name);
        send(client_socket, "-Service deleted as it became empty.\n", strlen("-Service deleted as it became empty.\n"), 0);
    }

    // 寫回 JSON 檔案
    char *updated_json = cJSON_Print(root);
    if (updated_json != NULL) {
        file = fopen(file_name, "w");
        if (file != NULL) {
            fwrite(updated_json, 1, strlen(updated_json), file);
            fclose(file);
        }
        free(updated_json);
    } else {
        send(client_socket, "Failed to update file\n", strlen("Failed to update file\n"), 0);
    }

    cJSON_Delete(root);
}

// 刪除 (del) 指定 key 的資料
void del(const char *filename, int client_socket, const char *service_name) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Failed to open file");
        send(client_socket, "Failed to open file\n", strlen("Failed to open file\n"), 0);
        return;
    }

    // 讀取檔案內容
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);
    char *file_content = (char *)malloc(length + 1);
    if (!file_content) {
        perror("Failed to allocate memory");
        fclose(file);
        send(client_socket, "Memory allocation error\n", strlen("Memory allocation error\n"), 0);
        return;
    }
    fread(file_content, 1, length, file);
    file_content[length] = '\0';
    fclose(file);

    // 解析 JSON
    cJSON *root = cJSON_Parse(file_content);
    free(file_content); // 釋放檔案內容記憶體
    if (!root) {
        printf("Error parsing JSON: %s\n", cJSON_GetErrorPtr());
        send(client_socket, "Error parsing JSON\n", strlen("Error parsing JSON\n"), 0);
        return;
    }

    // 檢查是否存在該服務名稱
    cJSON *service = cJSON_GetObjectItemCaseSensitive(root, service_name);
    if (!service) {
        printf("Error, Service '%s' not found.\n", service_name);
        send(client_socket, "Error, Service not found.\n", strlen("Error, Service not found\n"), 0);
        cJSON_Delete(root);
        return;
    }

    // 刪除該服務名稱
    cJSON_DeleteItemFromObject(root, service_name);
    printf("Service '%s' deleted successfully\n", service_name);
    send(client_socket, "Service deleted successfully\n", strlen("Service deleted successfully\n"), 0);

    // 將修改後的 JSON 寫回檔案
    file = fopen(filename, "w");
    if (!file) {
        perror("Failed to open file for writing");
        send(client_socket, "Failed to write to file\n", strlen("Failed to write to file\n"), 0);
        cJSON_Delete(root);
        return;
    }

    char *updated_content = cJSON_Print(root);
    if (updated_content) {
        fwrite(updated_content, 1, strlen(updated_content), file);
        free(updated_content); // 釋放序列化的 JSON 字串記憶體
    }

    fclose(file);
    cJSON_Delete(root);
}


int main(){
    int server_fd, client_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    fd_set readfds;
    struct timeval timeout;
    char buffer[1024] = {0};
    char *welcome_msg = "Connection established. Type 'exit' to disconnect.\n";

    // 建立伺服端 socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    // 設置 socket 選項
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    // 綁定地址和埠
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // 開始監聽
    if (listen(server_fd, 3) < 0) {
        perror("Listen");
        exit(EXIT_FAILURE);
    }
    printf("Server is listening on port %d...\n", PORT);

    // 接受客戶端連線
    if ((client_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
        perror("Accept");
        exit(EXIT_FAILURE);
    }
    printf("Client connected.\n");

    // 傳送歡迎訊息
    // send(client_socket, welcome_msg, strlen(welcome_msg), 0);


    // 保持連線直到接收到 'exit' 或超時
    while (1) {
        // 設置 select 的參數
        FD_ZERO(&readfds);
        FD_SET(client_socket, &readfds);

        timeout.tv_sec = TIMEOUT; // 閒置時間 (秒)
        timeout.tv_usec = 0;

        int activity = select(client_socket + 1, &readfds, NULL, NULL, &timeout);

        if (activity == 0) { // 超時
            printf("Client idle for %d seconds. Disconnecting...\n", TIMEOUT);
            break;
        } else if (activity < 0) { // 錯誤
            perror("select error");
            break;
        }

        if (FD_ISSET(client_socket, &readfds)) {
            memset(buffer, 0, sizeof(buffer));
            int bytes_read = read(client_socket, buffer, sizeof(buffer));
            if (bytes_read <= 0) {
                printf("Client disconnected.\n");
                break;
            }
        
            printf("Received: %s\n", buffer);

            char *first_part_cmd, *second_part_service_name, *third_part_account, *fourth_part_password;

            first_part_cmd = strtok(buffer, " ");
            second_part_service_name = strtok(NULL, " ");
            third_part_account = strtok(NULL, " ");
            fourth_part_password = strtok(NULL, " ");

            // 檢查是否收到 'exit'
            if (strncmp(first_part_cmd, "exit", 4) == 0) {
                printf("Client requested to exit. Closing connection...\n");
                break;
            }

            else if (strncmp(first_part_cmd, "ls", 2) == 0) {
                const char *file_name = "test.json";
                FILE *file = fopen(file_name, "r");

                // 檢查檔案是否成功開啟
                if (file == NULL) {
                    printf("Error opening file %s\n", file_name);
                    return 1;
                }

                // 取得檔案大小
                fseek(file, 0, SEEK_END);
                long file_size = ftell(file);
                fseek(file, 0, SEEK_SET);

                // 讀取檔案內容到字串緩衝區
                char *account_buffer = (char *)malloc(file_size + 1);
                if (account_buffer == NULL) {
                    printf("Memory allocation failed\n");
                    fclose(file);
                    return 1;
                }

                fread(account_buffer, 1, file_size, file);
                account_buffer[file_size] = '\0'; // 確保字串結尾

                // 關閉檔案
                fclose(file);

                // 解析 JSON 字串
                cJSON *root = cJSON_Parse(account_buffer);
                if (root == NULL) {
                    printf("Error parsing JSON\n");
                    free(account_buffer);
                    return 1;
                }
                

                // 假設 JSON 是物件類型，列出所有鍵
                printf("Services in JSON file:\n");
                cJSON *service = NULL;
                char temp3[256];
                char response[1000] = {0};
                cJSON_ArrayForEach(service, root) {
                if (cJSON_IsObject(service)) {
                    printf("- %s\n", service->string);
                    snprintf(temp3, sizeof(temp3), " - %s\n",service->string);
                    strncat(response, temp3, 1000 - strlen(response) - 1);
                }
                }
                send(client_socket, response, strlen(response),0);

                // 釋放 JSON 資源
                cJSON_Delete(root);

                // 釋放讀取的緩衝區
                free(account_buffer);
            }
            else if (strncmp(first_part_cmd, "get", 3) == 0){
                const char *file_name = "test.json";
                FILE *file = fopen(file_name, "r");

                // 檢查檔案是否成功開啟
                if (file == NULL) {
                    printf("Error opening file %s\n", file_name);
                    return 1;
                }
                // 取得檔案大小
                fseek(file, 0, SEEK_END);
                long file_size = ftell(file);
                fseek(file, 0, SEEK_SET);
                // 讀取檔案內容到字串緩衝區
                char *account_buffer = (char *)malloc(file_size + 1);
                if (account_buffer == NULL) {
                    printf("Memory allocation failed\n");
                    fclose(file);
                    return 1;
                }

                fread(account_buffer, 1, file_size, file);
                account_buffer[file_size] = '\0'; // 確保字串結尾

                // 關閉檔案
                fclose(file);

                // 解析 JSON 字串
                cJSON *root = cJSON_Parse(account_buffer);
                if (root == NULL) {
                    printf("Error parsing JSON\n");
                    free(account_buffer);
                    return 1;
                }

                if(second_part_service_name == NULL && third_part_account == NULL){
                    printf("Service and account name is missing.\n");
                    send(client_socket,"Wrong input. Please try again.\nEND\n", strlen("Wrong input. Please try again.\nEND\n"),0);
                } else if(third_part_account == NULL && second_part_service_name != NULL){
                    printf("process_request.\n");
                    process_request(second_part_service_name, root, client_socket);
                } else if(third_part_account != NULL && second_part_service_name != NULL){
                    printf("process_request_precise.\n");
                    process_request_precise(second_part_service_name, third_part_account, root, client_socket);
                }

                // 釋放 JSON 資源
                cJSON_Delete(root);
                // 釋放讀取的緩衝區
                free(account_buffer);
            }
            else if (strncmp(first_part_cmd, "del", 3) == 0){
                if(second_part_service_name==NULL){
                    printf("Wrong input. Please try again.\n");
                    send(client_socket,"Wrong input. Please try again.", strlen("Wrong input. Please try again."),0);
                }
                else {
                const char *file_name = "test.json";
                del2(file_name, client_socket, second_part_service_name, third_part_account);
                }
                // delete function(second_part_service_name) 尋找並刪除
            }
            else if (strncmp(first_part_cmd, "up", 2) == 0){
                if(second_part_service_name==NULL||third_part_account==NULL||fourth_part_password==NULL){
                    printf("Wrong input. Please try again.\n");
                    send(client_socket,"Wrong input. Please try again.", strlen("Wrong input. Please try again."),0);
                }
                else {
                //new function(second_part_service_name, third_part_account, fourth_part_password ) 增加新的到最後
                const char *file_name = "test.json";
                up(file_name, client_socket, second_part_service_name, third_part_account, fourth_part_password);
                }
                //update function(second_part_service_name, third_part_account) 尋找並更改帳號密碼
            }
            else if (strncmp(first_part_cmd, "check2", 6) == 0){
                if(second_part_service_name==NULL||third_part_account==NULL){
                    printf("Wrong input. Please try again.\n");
                    send(client_socket,"Wrong input. Please try again.", strlen("Wrong input. Please try again."),0);
                } else {
                    const char *file_name = "test.json";
                    FILE *file = fopen(file_name, "r");

                    // 檢查檔案是否成功開啟
                    if (file == NULL) {
                        printf("Error opening file %s\n", file_name);
                        return 1;
                    }
                    // 取得檔案大小
                    fseek(file, 0, SEEK_END);
                    long file_size = ftell(file);
                    fseek(file, 0, SEEK_SET);
                    // 讀取檔案內容到字串緩衝區
                    char *account_buffer = (char *)malloc(file_size + 1);
                    if (account_buffer == NULL) {
                        printf("Memory allocation failed\n");
                        fclose(file);
                        return 1;
                    }

                    fread(account_buffer, 1, file_size, file);
                    account_buffer[file_size] = '\0'; // 確保字串結尾

                    // 關閉檔案
                    fclose(file);

                    // 解析 JSON 字串
                    cJSON *root = cJSON_Parse(account_buffer);
                    if (root == NULL) {
                        printf("Error parsing JSON\n");
                        free(account_buffer);
                        return 1;
                    }
                    printf("checking\n");
                    checking_ID(second_part_service_name, third_part_account, root, client_socket);

                    // 釋放 JSON 資源
                    cJSON_Delete(root);

                    // 釋放讀取的緩衝區
                    free(account_buffer);
                }  
            }
            else if (strncmp(first_part_cmd, "check", 5) == 0){
                if(second_part_service_name==NULL||third_part_account==NULL||fourth_part_password==NULL){
                    printf("Wrong input. Please try again.\n");
                    send(client_socket,"Wrong input. Please try again.", strlen("Wrong input. Please try again."),0);
                } else {
                    const char *file_name = "test.json";
                    FILE *file = fopen(file_name, "r");

                    // 檢查檔案是否成功開啟
                    if (file == NULL) {
                        printf("Error opening file %s\n", file_name);
                        return 1;
                    }
                    // 取得檔案大小
                    fseek(file, 0, SEEK_END);
                    long file_size = ftell(file);
                    fseek(file, 0, SEEK_SET);
                    // 讀取檔案內容到字串緩衝區
                    char *account_buffer = (char *)malloc(file_size + 1);
                    if (account_buffer == NULL) {
                        printf("Memory allocation failed\n");
                        fclose(file);
                        return 1;
                    }

                    fread(account_buffer, 1, file_size, file);
                    account_buffer[file_size] = '\0'; // 確保字串結尾

                    // 關閉檔案
                    fclose(file);

                    // 解析 JSON 字串
                    cJSON *root = cJSON_Parse(account_buffer);
                    if (root == NULL) {
                        printf("Error parsing JSON\n");
                        free(account_buffer);
                        return 1;
                    }

                    checking_ID(second_part_service_name, third_part_account, root, client_socket);

                    // 釋放 JSON 資源
                    cJSON_Delete(root);

                    // 釋放讀取的緩衝區
                    free(account_buffer);
                }  
            }
            
            else if (strncmp(first_part_cmd, "new", 3) == 0){
                if(second_part_service_name==NULL||third_part_account==NULL||fourth_part_password==NULL){
                    printf("Wrong input. Please try again.\n");
                    send(client_socket,"Wrong input. Please try again.", strlen("Wrong input. Please try again."),0);
                }
                else {
                //new function(second_part_service_name, third_part_account, fourth_part_password ) 增加新的到最後
                const char *file_name = "test.json";
                set(file_name, client_socket, second_part_service_name, third_part_account, fourth_part_password);
                }
            }
        }
    }

    // 關閉連線
    close(client_socket);
    close(server_fd);
    printf("Server shut down.\n");

    return 0;
}