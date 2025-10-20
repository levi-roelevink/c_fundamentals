#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char* DEFAULT_FILE = "index.html";

// Returns the memory address of the actual path
// Input: "GET /blog HTTP/1.1..."
// Goal: "blog/index.html"
char *to_path(char *req) {
    char *start, *end;
    const int length = strlen(req);

    for (start = req; start[0] != ' '; start++) {
        if (!start[0]) { // End of string / NULL-terminated ('0' or '\0')
            return NULL;
        }
    }
    start++;

    for (end = start; end[0] != ' '; end++) {
        if (!end[0]) { // End of string / NULL-terminated
            return NULL;
        }
    }

    if (end[-1] != '/') {
        // In case the URL included '/' at the end
        end[0] = '/';
        end++;
    }

    // Not enough space to copy in "index.html"
    if (end + strlen(DEFAULT_FILE) > req + length) {
        return NULL;
    }

    // + 1 for size_t to include NULL-terminator
    memcpy(end, DEFAULT_FILE, strlen(DEFAULT_FILE) + 1);

    // Skip leading '/'
    return start + 1;
}

int main() {
    // This declaration will result in a variable on the stack, so inside the main function's memory
    // Declaring like "char *req" would result in it being read-only and on the heap
    char req1[] = "GET /blog HTTP/1.1\nHost: example.com";
    printf("Should be \"blog/index.html\": \"%s\"\n", to_path(req1));

    char req2[] = "GET /blog/ HTTP/1.1\nHost: example.com";
    printf("Should be \"blog/index.html\": \"%s\"\n", to_path(req2));

    char req3[] = "GET / HTTP/1.1\nHost: example.com";
    printf("Should be \"index.html\": \"%s\"\n", to_path(req3));

    char req4[] = "GET /blog ";
    printf("Should be \"(null)\": \"%s\"\n", to_path(req4));

    return 0;
}