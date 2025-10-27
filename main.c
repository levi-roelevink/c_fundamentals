#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <libc.h>

const char *DEFAULT_FILE = "index.html";
const int MAX_REQUEST_BYTES = 32768;

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
        return NULL;
    }

    struct stat metadata;

    // This will write 144 bytes of metadata from fd into the metadata object
    // The '&' gives the address of the first of those 144 bytes
    // Otherwise the 144 bytes would've been copied to pass as argument
    if (fstat(fd, &metadata) == -1) {
        printf("Error retrieving information about file: \"%s\", error code: %d\n", path, errno);
        close(fd);
        return NULL;
    }

    // 👉 Change this to `char *` and malloc(). (malloc comes from <stdlib.h>)
    //    Hint 1: Don't forget to handle the case where malloc returns NULL!
    //    Hint 2: Don't forget to `free(buf)` later, to prevent memory leaks.
    // char buf[metadata.st_size + 1];
    char *buf = malloc(metadata.st_size + 1);
    // TODO: this wil crash if the file size is too large, split the reading up into chunks

    if (buf == NULL) {
        printf("Error allocating memory on the heap, error code: %d\n", errno);
        close(fd);
        return NULL;
    }

    ssize_t bytes_read = read(fd, buf, metadata.st_size);
    if (bytes_read == -1) {
        printf("Error reading bytes from file descriptor: \"%s\", error code: %d\n", path, errno);
        close(fd);
        free(buf);
        return NULL;
    }

    buf[bytes_read] = '\0';

    close(fd);
    return buf;
}

void socket_listen() {
    // Open a socket
    // Listen for connections
    // Read from a socket
    // Write to a socket

    // AF_INET specifies IPv4, SOCK_STREAM specifies TCP, 0 is the protocol number
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);

    int opt = 1;
    setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in address; // IPv4
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);

    // Bind the info going to the port 8080 to the socket
    bind(socket_fd, (struct sockaddr *) &address, sizeof(address));

    // '4' is the number of pending connections that can stack up before we start refusing new ones
    listen(socket_fd, 4);

    printf("Listening on localhost:8080\n");

    // TODO: Read request from a socket

    // Request string, we want the bytes from the network in here
    // Then we're going to pass this address off to the to_path function
    char req[MAX_REQUEST_BYTES + 1];
    int addrlen = sizeof(address);

    // Wait for requests to come in
    while (1) {
        int req_socket_fd = accept(socket_fd, (struct sockaddr *) &address, (socklen_t *) &addrlen);

        // Get data from req_socket_fd
        ssize_t bytes_read = read(req_socket_fd, req, MAX_REQUEST_BYTES);
        char *path = to_path(req);
        if (path == NULL) {
            printf("HTTP/1.1 400 Bad Request\n\n");
        } else {
            printf("Path from to_path: \"%s\"\n", path);
        }
        // int fd = open(path, O_RDONLY); // Read file contents

        char *response = print_file(path);
        if (response != NULL) {
            printf("Response is not NULL\n");
            // char *content_type = "Content-Type: text/plain\n\n";
            char *valid_response = "HTTP/1.1 200 OK\n\n";
            // Write the requested page to the request socket
            write(req_socket_fd, valid_response, strlen(valid_response));
            write(req_socket_fd, "Hello, World!\n", 14);

            write(1, response, strlen(response));
            free(response);
        }

        // struct stat metadata;
        // fstat(fd, &metadata);
        // char contents[metadata.st_size + 1];
        // read(fd, contents, metadata.st_size + 1);
        // write(1, contents, metadata.st_size + 1);
        // To make it a real HTTP-server, just write the response to the socket instead of out


        // close(fd);
        close(req_socket_fd);
    }
}

int main() {
    socket_listen();
    // char req1[] = "GET / HTTP/1.1\nHost: example.com";
    // char *buf = print_file(to_path(req1));
    // printf("File contents: %s\n", buf);
    // free(buf);
    //
    // char req2[] = "GET /blog HTTP/1.1\nHost: example.com";
    // buf = print_file(to_path(req2));
    // printf("File contents: %s\n", buf);
    // free(buf);

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
