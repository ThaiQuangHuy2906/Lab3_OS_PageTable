#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

char* fmtname(char *path) {
    static char buf[DIRSIZ + 1];
    char *p;

    for (p = path + strlen(path); p >= path && *p != '/'; --p)
        ;
    p++;

    if (strlen(p) >= DIRSIZ)
        return p;
    memmove(buf, p, strlen(p));
    buf[strlen(p)] = '\0';
    return buf;
}

void tree(char *path, int level, int max_depth, int only_dir) {
    char buf[512], *p;
    int fd;
    struct dirent de;
    struct stat st;

    if (max_depth >= 0 && level > max_depth)
        return;

    if ((fd = open(path, 0)) < 0) {
        fprintf(2, "tree: cannot open %s\n", path);
        return;
    }

    if (fstat(fd, &st) < 0) {
        fprintf(2, "tree: cannot stat %s\n", path);
        close(fd);
        return;
    }

    if (st.type == T_FILE) {
        if (only_dir) {
            close(fd);
            return;
        }
        for (int i = 0; i < level; ++i)
            printf("  ");

        printf("%s\n", fmtname(path));
    }
    else if (st.type == T_DIR) {
        for (int i = 0; i < level; ++i)
            printf("  ");

        printf("%s\n", fmtname(path));

        if (strlen(path) + 1 + DIRSIZ + 1 > sizeof buf) {
            fprintf(2, "tree: path too long\n");
            close(fd);
            return;
        }
        strcpy(buf, path);
        p = buf+strlen(buf);
        *p++ = '/';

        while(read(fd, &de, sizeof(de)) == sizeof(de)) {
            if(de.inum == 0 || strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0)
                continue;
            memmove(p, de.name, DIRSIZ);
            p[DIRSIZ] = '\0';
            tree(buf, level + 1, max_depth, only_dir);
        }
    }
    close(fd);
}

int main(int argc, char *argv[]) {
    int max_depth = -1;
    int only_dir = 0;
    char *path = ".";

    for (int i = 1; i < argc; ++i) {
        if(strcmp(argv[i], "-L") == 0 && i + 1 < argc)
            max_depth = atoi(argv[++i]);
        else if(strcmp(argv[i], "-d") == 0)
            only_dir = 1;
        else if (argv[i][0] != '-')
            path = argv[i];
    }

    tree(path, 0, max_depth, only_dir);
    exit(0);
}