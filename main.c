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
        if (!start[0]) { // End of string / NULL-terminated
            return NULL;
        }
    }
    start++;

    for (end = start; end[0] != ' '; end++) {
        if (!end[0]) { // End of string / NULL-terminated
            return NULL;
        }
    }

    printf("End: %c\n", end[0]);
    if (end[-1] != '/') {
        // In case the URL included '/' at the end
        end[0] = '/';
        end++;
    }

    printf("Start: %s\n", start);
    printf("End: %c\n", end[0]);

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
    char req[] = "GET /blog HTTP/1.1...";
    printf("Should be \"blog/index.html\": \"%s\"\n", to_path(req));

    return 0;
}