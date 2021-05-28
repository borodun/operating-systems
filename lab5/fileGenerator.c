#include <stdlib.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s fileName lineCount\n", argv[0]);
    }

    FILE *out = fopen(argv[1], "w");

    for (int i = 0; i < atoi(argv[2]); ++i) {
        fprintf(out, "This is line number %d\n", i+1);
    }

    fclose(out);
    return 0;
}