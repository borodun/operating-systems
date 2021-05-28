#include <stdio.h>
#include <dirent.h>
#include <string.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s regular_expression\n", argv[0]);
        return -1;
    }

    DIR *dir;
    if ((dir = opendir(".")) == NULL) {
        perror("Error with opening '.'");
        return -1;
    }

    char *template = argv[1];
    size_t templateLength = strlen(template);
    printf("template: %s\n", template);

    for (int i = 0; i < templateLength; i++) {
        if (template[i] == '/') {
            printf("'/' is prohibited\n");
            return -1;
        }
    }

    int matched = 0;
    struct dirent *p;
    printf("Files found:\n");
    while ((p = readdir(dir)) != NULL) {
        size_t fileNameLength = strlen(p->d_name);
        int templateIdx = 0;
        int fileNameIdx;
        int match = 0;

        for (fileNameIdx = 0; (fileNameIdx < fileNameLength) && (templateIdx < templateLength); fileNameIdx++) {
            if (template[templateIdx] == '?') {
                templateIdx++;
            } else if (template[templateIdx] == '*') {
                while (templateIdx < templateLength) {
                    if ('*' != template[templateIdx]) {
                        break;
                    }
                    templateIdx++;
                }

                if (templateLength == templateIdx) {
                    match = 1;
                    break;
                }
                if (template[templateIdx] == '?') {
                    templateIdx++;
                    continue;
                }

                while (fileNameIdx < fileNameLength) {
                    if (template[templateIdx] == p->d_name[fileNameIdx]) {
                        break;
                    }
                    fileNameIdx++;
                }
                templateIdx++;
            } else {
                if (template[templateIdx] != p->d_name[fileNameIdx]) {
                    break;
                }
                templateIdx++;
            }
        }

        if (fileNameLength == fileNameIdx) {
            while (templateIdx < templateLength) {
                if ('*' != template[templateIdx])
                    break;
                templateIdx++;
            }

            if (templateLength == templateIdx) {
                match = 1;
            }
        }

        if (match) {
            printf("\t%s\n", p->d_name);
            matched++;
        }
    }

    if (!matched) {
        printf("No matching files for '%s'\n", template);
    }

    closedir(dir);
    return 0;
}
