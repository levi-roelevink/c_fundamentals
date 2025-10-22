#include <libc.h>

const int MAX_REQUEST_BYTES = 100;

int main() {
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

    printf("Listening on localhost:8080");

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

        close(req_socket_fd);
    }

    return 0;
}
