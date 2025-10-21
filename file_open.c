#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdlib.h>

int main() {
    char *path = "example.txt";
    int fd = open(path, O_RDONLY);

    if (fd == -1) {
        // Handle errors
    }

    struct stat metadata;
    if (fstat(fd, &metadata) == 1) { // This will write 144 bytes of metadata from fd into the metadata object
        // The '&' gives the address of the first of those 144 bytes
        // Otherwise the 144 bytes would've been copied to pass as argument
        // Handle errors
    }

    // Get the size of the file on disk
    ssize_t size = metadata.st_size;
    // Make a buffer of that many bytes
    char buffer[size];
    // Read that many bytes into the buffer
    ssize_t length = read(fd, buffer, size);

    if (length == -1) {
    // Handle errors
    } else {
        printf("Successfully read file: %s\n", path);
        printf("File contents: \"%s\"\n", buffer);
    }

    // fwrite()

    close(fd);

    return 0;
}