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
const char *ERR_400 = "HTTP/1.1 400 Bad Request\n\n";
const char *ERR_404 = "HTTP/1.1 404 Not Found\n\n";
const char *ERR_413 = "HTTP/1.1 413 Content Too Large\n\n";
const char *ERR_500 = "HTTP/1.1 500 Internal Server Error\n\n";
const int PORT = 8080;

// Returns the memory address of the actual path
// Input: "GET /blog HTTP/1.1..."
// Goal: "blog/index.html"
char *to_path(char *req) {
    printf("Req: %s\n", req);
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
        if (errno == ENOENT) {
            printf("HTTP/1.1 404 Not Found\n\n");
        } else {
            printf("HTTP/1.1 500 Internal Server Error\n\n");
        }
        printf("Error opening file: \"%s\", error code: %d\n", path, errno);
        return NULL;
    }

    struct stat metadata;

    // This will write 144 bytes of metadata from fd into the metadata object
    // The '&' gives the address of the first of those 144 bytes
    // Otherwise the 144 bytes would've been copied to pass as argument
    if (fstat(fd, &metadata) == -1) {
        printf("HTTP/1.1 500 Internal Server Error\n\n");
        printf("Error retrieving information about file: \"%s\", error code: %d\n", path, errno);
        close(fd);
        return NULL;
    }

    char *buf = malloc(metadata.st_size + 1);
    // TODO: this wil crash if the file size is too large, split the reading up into chunks

    if (buf == NULL) {
        printf("HTTP/1.1 500 Internal Server Error\n\n");
        printf("Error allocating memory on the heap, error code: %d\n", errno);
        close(fd);
        return NULL;
    }

    ssize_t bytes_read = read(fd, buf, metadata.st_size);
    if (bytes_read == -1) {
        printf("HTTP/1.1 500 Internal Server Error\n\n");
        printf("Error reading bytes from file descriptor: \"%s\", error code: %d\n", path, errno);
        close(fd);
        free(buf);
        return NULL;
    }

    buf[bytes_read] = '\0';

    close(fd);
    return buf;
}

ssize_t write500(int socket_fd) {
    return write(socket_fd, ERR_500, strlen(ERR_500));
}

int handle_req(int req_socket_fd, char *req) {
    char *path = to_path(req);
    printf("Path from to_path: \"%s\"\n", path);
    if (path == NULL) {
        write(req_socket_fd, ERR_400, strlen(ERR_400));
        return -1;
    }

    int length = strlen(path) - strlen(DEFAULT_FILE);

    char *extension = path;
    for (int i = 0; i < length - 1; i++) {
        // Path contains a file extension
        if (path[i] == '.') {
            extension[length - i - 1] = '\0';
            break;
        }
        extension++;
    }

    if (extension[0] != '.') { // Path does not contain a file extension
        extension = NULL;
    }
    printf("Extension %s\n", extension);

    int fd = open(path, O_RDONLY); // Read file contents
    if (fd == -1) {
        if (errno == ENOENT) {
            write(req_socket_fd, ERR_404, strlen(ERR_404));
        } else {
            write500(req_socket_fd);
        }
        close(fd);
        return -1;
    }

    struct stat metadata;
    if (fstat(fd, &metadata) == -1) {
        write500(req_socket_fd);
        close(fd);
        return -1;
    }

    char *buffer = malloc(metadata.st_size + 1);
    if (buffer == NULL) {
        write500(req_socket_fd);
        close(fd);
        return -1;
    }

    ssize_t bytes_read = read(fd, buffer, metadata.st_size + 1);
    if (bytes_read == -1) {
        write500(req_socket_fd);
        close(fd);
        free(buffer);
        return -1;
    }

    char *http_header = "HTTP/1.1 200 OK\n\n";
    write(req_socket_fd, http_header, strlen(http_header));
    // Write the requested page to the request socket
    write(req_socket_fd, buffer, strlen(buffer));

    int bytes_written = strlen(http_header) + strlen(buffer);

    free(buffer);
    close(fd);
    return bytes_written;
}

void socket_listen() {
    // AF_INET specifies IPv4, SOCK_STREAM specifies TCP, 0 is the protocol number
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1) {
        printf("Opening socket failed.\n");
        return;
    }

    int opt = 1;
    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        printf("Setting socket options failed.\n");
        return;
    }

    struct sockaddr_in address; // IPv4
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind the info going to the port 8080 to the socket
    if (bind(socket_fd, (struct sockaddr *) &address, sizeof(address)) == -1) {
        printf("Binding socket to port %d failed.\n", PORT);
        return;
    }

    // '4' is the number of pending connections that can stack up before we start refusing new ones
    if (listen(socket_fd, 4) == -1) {
        printf("Attempt to start listen on port %d failed.\n", PORT);
        return;
    }

    printf("Listening on localhost:%d\n", PORT);

    // Request string, we want the bytes from the network in here
    // Then we're going to pass this address off to the to_path function
    char req[MAX_REQUEST_BYTES + 1];
    int addrlen = sizeof(address);

    // Wait for requests to come in
    while (1) {
        int req_socket_fd = accept(socket_fd, (struct sockaddr *) &address, (socklen_t *) &addrlen);

        // Get data from req_socket_fd
        ssize_t bytes_read = read(req_socket_fd, req, MAX_REQUEST_BYTES);
        if (bytes_read == -1) {
            write500(req_socket_fd);
        } else if (bytes_read > MAX_REQUEST_BYTES) {
            write(req_socket_fd, ERR_413, strlen(ERR_413));
        } else {
            req[bytes_read] = '\0'; // Null-terminate
            int bytes_written = handle_req(req_socket_fd, req);
            printf("Bytes written: %d\n", bytes_written);
        }

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
