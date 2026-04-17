#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fcntl.h"

int readline(int fd, char *buf, int max) {
    int i = 0;
    char c;
    int n = 0;
    
    while (i < max - 1) {
        n = read(fd, &c, 1);
        if (n < 1) break;
        if (c == '\n') break;
        buf[i++] = c;
    }
    buf[i] = '\0';
    
    if (n < 1 && i == 0) return 0;
    
    return 1; 
}

int main(int argc, char *argv[]) {
    int q_flag = 0;
    char *f1_name, *f2_name;

    if (argc < 3) {
        printf("usage: diff file1 file2 [-q]\n");
        exit(1);
    }

    if (argc == 4 && strcmp(argv[3], "-q") == 0) {
        q_flag = 1;
        f1_name = argv[1];
        f2_name = argv[2];
    } else if (argc == 4 && strcmp(argv[1], "-q") == 0) {
        q_flag = 1;
        f1_name = argv[2];
        f2_name = argv[3];
    } else {
        f1_name = argv[1];
        f2_name = argv[2];
    }

    int fd1 = open(f1_name, O_RDONLY);
    if (fd1 < 0) {
        printf("diff: cannot open %s\n", f1_name);
        exit(1);
    }

    int fd2 = open(f2_name, O_RDONLY);
    if (fd2 < 0) {
        printf("diff: cannot open %s\n", f2_name);
        close(fd1);
        exit(1);
    }

    char buf1[1024], buf2[1024];
    int line_num = 1;

    while (1) {
        int r1 = readline(fd1, buf1, sizeof(buf1));
        int r2 = readline(fd2, buf2, sizeof(buf2));

        if (r1 == 0 && r2 == 0) break;

        if (r1 == 0 || r2 == 0 || strcmp(buf1, buf2) != 0) {
            
            if (q_flag) {
                printf("diff: files differ\n");
                break;
            }

            if (r1 == 0) {
                printf("%s:%d: <EOF\n", f1_name, line_num);
            } else {
                printf("%s:%d: < %s\n", f1_name, line_num, buf1);
            }

            if (r2 == 0) {
                printf("%s:%d: >EOF\n", f2_name, line_num);
            } else {
                printf("%s:%d: > %s\n", f2_name, line_num, buf2);
            }
        }
        line_num++;
    }

    close(fd1);
    close(fd2);
    exit(0);
}
