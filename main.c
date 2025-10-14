#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Returns the memory address of the actual path
// Input: "GET /blog HTTP/1.1..."
// Goal: "blog/index.html"
char *to_path(const char *req) {
    const int length = strlen(req);

    int startIndex = 0;
    int endIndex = 0;

    for (int i = 0; i < length; i++) {
        if (req[i] == '/') {
            startIndex = i + 1;
            for (int j = startIndex; j < length; j++) {
                if (req[j] == ' ') {
                    endIndex = j - 1;
                    break;
                }
            }
            break;
        }
    }

    const char *suffix = "/index.html";
    const int pathLength = endIndex - startIndex + 1;
    const int totalLength = pathLength + strlen(suffix);
    char *result = malloc(totalLength);

    // src: index in memory of first byte we want to copy
    strncpy(result, req + startIndex, pathLength);
    strcat(result, suffix);
    return result; // Caller must free()
}

int main() {
    char *req = "GET /blog HTTP/1.1...";
    char *path = to_path(req);

    if (path) {
        printf("Path: %s\n", path);
        // Frees the memory allocated on the heap by malloc in the to_path function
        free(path);
    }

    return 0;
}