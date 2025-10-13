#include <stdio.h>
#include <unistd.h>

int main() {
    int num = 42;
    printf("The number is: %d\n", num);

    int numTwo = 21;
    printf("The numbers are: %d and %d\n", num, numTwo);

    int sum = num + numTwo;
    printf("The sum is %d\n", sum);

    return 0;
}