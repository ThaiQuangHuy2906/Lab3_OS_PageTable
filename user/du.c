#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

int du(char *path, int a_flag, int s_flag) {
    char buf[512], *p;
    int fd;
    struct dirent de;
    struct stat st;
    int total_size = 0;

    if ((fd = open(path, 0)) < 0) {
        fprintf(2, "du: cannot open %s\n", path);
        return 0;
    }

    if (fstat(fd, &st) < 0) {
        fprintf(2, "du: cannot stat %s\n", path);
        close(fd);
        return 0;
    }

    if (st.type == T_FILE) {
        total_size = st.size;
        if(a_flag && !s_flag)
            printf("%d\t%s\n", total_size, path);
    }
    else if (st.type == T_DIR) {
        total_size = 0;

        if (strlen(path) + 1 + DIRSIZ + 1 > sizeof buf) {
            fprintf(2, "du: path too long\n");
            close(fd);
            return 0;
        }
        strcpy(buf, path);
        p = buf+strlen(buf);
        *p++ = '/';

        while (read(fd, &de, sizeof(de)) == sizeof(de)) {
            if (de.inum == 0 || strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0)
                continue;

            memmove(p, de.name, DIRSIZ);
            p[DIRSIZ] = '\0';
            total_size += du(buf, a_flag, s_flag);
        }

        if (!s_flag)
            printf("%d\t%s\n", total_size, path);
    }
    close(fd);
    return total_size;
}

int main(int argc, char *argv[]) {
    int a_flag = 0;
    int s_flag = 0;
    char *path = ".";

    for (int i = 1; i < argc; ++i) {
        if(strcmp(argv[i], "-a") == 0)
            a_flag = 1;
        else if(strcmp(argv[i], "-s") == 0)
            s_flag = 1;
        else if (argv[i][0] != '-')
            path = argv[i];
    }

    int final_size = du(path, a_flag, s_flag);
    if(s_flag)
        printf("%d\t%s\n", final_size, path);

    exit(0);
}