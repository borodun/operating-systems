#include <stdio.h>
#include <stdlib.h>
#include <libgen.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>


int main(int argc, char *argv[]) {
    struct stat fileStat;
    char features[10];
    
    for (int i = 1; i < argc; i++) {
        if (lstat(argv[i], &fileStat) < 0) {
            perror("Error occurred with lstat");
            continue;
        }

        if (S_ISREG(fileStat.st_mode)) {
            features[0] = '-';
        } else if (S_ISDIR(fileStat.st_mode)) {
            features[0] = 'd';
        } else {
            features[0] = '?';
        }
        
        features[1] = (fileStat.st_mode & S_IRUSR) ? 'r' : '-';
        features[2] = (fileStat.st_mode & S_IWUSR) ? 'w' : '-';
        features[3] = (fileStat.st_mode & S_IXUSR) ? 'x' : '-';
        
        features[4] = (fileStat.st_mode & S_IRGRP) ? 'r' : '-';
        features[5] = (fileStat.st_mode & S_IWGRP) ? 'w' : '-';
        features[6] = (fileStat.st_mode & S_IXGRP) ? 'x' : '-';
        
        features[7] = (fileStat.st_mode & S_IROTH) ? 'r' : '-';
        features[8] = (fileStat.st_mode & S_IWOTH) ? 'w' : '-';
        features[9] = (fileStat.st_mode & S_IXOTH) ? 'x' : '-';
        
        printf("%s", features);

        printf(" %lu", fileStat.st_nlink);

        struct passwd *passwd = getpwuid(fileStat.st_uid);
        if (passwd != NULL) {
            printf(" %u", passwd->pw_uid);
        } else {
            printf(" %d", fileStat.st_uid);
        }

        struct group *grp = getgrgid(fileStat.st_gid);
        if (grp != NULL) {
            printf(" %s", grp->gr_name);
        } else {
            printf(" %d", fileStat.st_gid);
        }

        if (S_ISREG(fileStat.st_mode)) {
            printf(" %4lu", fileStat.st_size);
        }

        char dateOfEdit[FILENAME_MAX] = {0};
        strftime(dateOfEdit, FILENAME_MAX, "%b %e %H:%M", localtime(&(fileStat.st_ctime)));
        printf(" %s", dateOfEdit);

        printf(" %s\n", basename(argv[i]));
    }
    return 0;
}