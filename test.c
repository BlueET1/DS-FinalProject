#include <stdio.h>
#include <stdlib.h>
#include "cJSON.h"  // 確保 cJSON 標頭檔被包含

int main() {
    const char *json_string = "{\"Netflix\": \"123456\"}";
    cJSON *json = cJSON_Parse(json_string);

    if (json == NULL) {
        printf("Failed to parse JSON\n");
        return 1;
    }

    cJSON *password = cJSON_GetObjectItemCaseSensitive(json, "Netflix");
    if (cJSON_IsString(password) && (password->valuestring != NULL)) {
        printf("Password for Netflix: %s\n", password->valuestring);
    }

    cJSON_Delete(json);
    return 0;
}
