#include <unistd.h>
#include <stdio.h>
#include <string.h>

// Returns the memory address of the actual path
// char *to_path(char *req) {
//     // Input: "GET /blog HTTP/1.1..."
//     // Goal: "blog/index.html"
//     char *start = req;
// }

int main() {
    // char *req = "GET /blog HTTP/1.1...";
    // char *path = to_path(req);
    //
    // printf("Path: %s", path);

    char header[50];
    strcpy(header, "HTTP 200 OK");

    char *pointer = header;

    strcpy(header, "HTTP 404 NOT OK");

    return 0;
}