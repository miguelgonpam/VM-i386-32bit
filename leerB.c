#include <stdio.h>

int main() {
    FILE *fp;
    int c, i, max;

    fp = fopen("tarea", "rb");
    if (fp == NULL) {
        fprintf(stderr, "cannot open input file\n");
        return 1;
    }
    while ( (c = getc(fp)) != EOF ) {
        printf("%02x ", c);
    }

    fclose(fp);
    return 0;
}
