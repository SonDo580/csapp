#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// cast pointers to (unsigned char *) allow treatment as byte array
typedef unsigned char *byte_pointer;

void show_bytes(byte_pointer start, size_t len)
{
    size_t i;
    for (i = 0; i < len; i++)
        printf("%p\t0x%.2x\n", &start[i], start[i]);
    printf("\n");
}

void show_int(int x)
{
    show_bytes((byte_pointer)&x, sizeof(int));
}

void show_float(float x)
{
    show_bytes((byte_pointer)&x, sizeof(float));
}

void show_pointer(void *x)
{
    show_bytes((byte_pointer)&x, sizeof(void *));
}

int main() {
    int a = 15213;
    printf("int a = %d;\n", a);
    show_int(a);
}