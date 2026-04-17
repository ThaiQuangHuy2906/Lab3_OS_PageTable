#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

void sieve(int left_pipe[2]) __attribute__((noreturn));

void sieve(int left_pipe[2]) {
    close(left_pipe[1]);
    int prime;

    if (read(left_pipe[0], &prime, sizeof(prime)) == 0) {
        exit(0);
    }
    printf("prime %d\n", prime);

    int right_pipe[2];
    pipe(right_pipe);
    int pid = fork();
    if (pid == 0) {
        close(left_pipe[0]);
        sieve(right_pipe);
    }
    else {
        close(right_pipe[0]);
        int num;
        while (read(left_pipe[0], &num, sizeof(num)) > 0) {
            if (num % prime != 0) {
                write(right_pipe[1], &num, sizeof(num));
            }
        }
        close(left_pipe[0]);
        close(right_pipe[1]);
        wait((int*)0);
        exit(0);
    }
}

int main() {
    int initial_pipe[2];
    pipe(initial_pipe);
    int pid = fork();
    if (pid == 0) {
        sieve(initial_pipe);
    }
    else {
        close(initial_pipe[0]);
        for (int i = 2; i <= 280; i++) {
            write(initial_pipe[1], &i, sizeof(i));
        }
        close(initial_pipe[1]);
        wait((int*)0);
        exit(0);
    }
    return 0;
}