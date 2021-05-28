#include <stdio.h>
#include <string.h>
#include <glob.h>

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: %s regular_expression\n", argv[0]);
        return 0;
    }

    char *template = argv[1];
    printf("template: %s\n", template);

    glob_t result;
    int ret = glob(template, 0, NULL, &result);
    if (ret == GLOB_NOSPACE) {
        printf("Not enough memory\n");
        globfree(&result);
        return -1;
    }
    if (ret == GLOB_ABORTED) {
        printf("Error with read\n");
        globfree(&result);
        return -1;
    }
    if (ret == GLOB_NOMATCH) {
        printf("No matching files template '%s'\n", template);
        globfree(&result);
        return -1;
    }

    printf("Files: \n");
    for (int i = 0; i < result.gl_pathc; i++) {
        printf("\t%s\n", result.gl_pathv[i]);
    }

    globfree(&result);
    return 0;
}