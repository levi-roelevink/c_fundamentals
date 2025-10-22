#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>
#include <sys/fcntl.h>
#include <sys/stat.h>

const char *DEFAULT_FILE = "index.html";

// Returns the memory address of the actual path
// Input: "GET /blog HTTP/1.1..."
// Goal: "blog/index.html"
char *to_path(char *req) {
    char *start, *end;
    const int length = strlen(req);

    for (start = req; start[0] != ' '; start++) {
        if (!start[0]) {
            // End of string / NULL-terminated ('0' or '\0')
            return NULL;
        }
    }
    start++;

    for (end = start; end[0] != ' '; end++) {
        if (!end[0]) {
            // End of string / NULL-terminated
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

char *print_file(const char *path) {
    int fd = open(path, O_RDONLY);
    if (fd == -1) {
        printf("Error opening file: \"%s\", error code: %d\n", path, errno);
    }

    struct stat metadata;

    // This will write 144 bytes of metadata from fd into the metadata object
    // The '&' gives the address of the first of those 144 bytes
    // Otherwise the 144 bytes would've been copied to pass as argument
    if (fstat(fd, &metadata) == -1) {
        printf("Error retrieving information about file: \"%s\", error code: %d\n", path, errno);
    }

    // 👉 Change this to `char *` and malloc(). (malloc comes from <stdlib.h>)
    //    Hint 1: Don't forget to handle the case where malloc returns NULL!
    //    Hint 2: Don't forget to `free(buf)` later, to prevent memory leaks.
    // char buf[metadata.st_size + 1];
    char *buf = malloc(metadata.st_size + 1);
    // TODO: this wil crash if the file size is too large, split the reading up into chunks

    if (buf == NULL) {
        printf("Error allocating memory on the heap, error code: %d\n", errno);
        exit(0);
    }

    ssize_t bytes_read = read(fd, buf, metadata.st_size);
    if (bytes_read == -1) {
        printf("Error reading bytes from file descriptor: \"%s\", error code: %d\n", path, errno);
    }
    buf[bytes_read] = '\0';

    close(fd);
    return buf;

    // 👉 Go back and add error handling for all the cases above where errors could happen.
    //    (You can just printf that an error happened.) Some relevant docs:
    //
    //    https://www.man7.org/linux/man-pages/man2/open.2.html
    //    https://www.man7.org/linux/man-pages/man2/stat.2.html
    //    https://www.man7.org/linux/man-pages/man2/read.2.html
    //    https://www.man7.org/linux/man-pages/man3/malloc.3.html
}

int main() {
    char req1[] = "GET / HTTP/1.1\nHost: example.com";
    // char *buf = print_file(to_path(req1));
char *buf = print_file("ppooop");
    printf("File contents: %s\n", buf);
    free(buf);

    char req2[] = "GET /blog HTTP/1.1\nHost: example.com";
    buf = print_file(to_path(req2));
    printf("File contents: %s\n", buf);
    free(buf);

    return 0;
}

// This declaration will result in a variable on the stack, so inside the main function's memory
// Declaring like "char *req" would result in it being read-only and on the heap
// char req1[] = "GET /blog HTTP/1.1\nHost: example.com";
// printf("Should be \"blog/index.html\": \"%s\"\n", to_path(req1));
//
// char req2[] = "GET /blog/ HTTP/1.1\nHost: example.com";
// printf("Should be \"blog/index.html\": \"%s\"\n", to_path(req2));
//
// char req3[] = "GET / HTTP/1.1\nHost: example.com";
// printf("Should be \"index.html\": \"%s\"\n", to_path(req3));
//
// char req4[] = "GET /blog ";
// printf("Should be \"(null)\": \"%s\"\n", to_path(req4));
