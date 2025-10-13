#include <stdio.h>
#include <unistd.h>

int main() {
    write(1, "Hello, World!", 13); // 1 means write to stdout, 2 is stderror
    return 0;
}