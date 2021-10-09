#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char const *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr,"usage : ./projet_O int\n");
        exit(-1);
    }
    int i = 0;
    printf("father's pid -> %d\n",getpid());
    while (i < atoi(argv[1]))
    {
        switch (fork())
        {
        case -1:
            perror("fork");
            exit(-1);

        case 0:
            printf("son's pid n°%d -> %d, pid of his father -> %d\n",i,getpid(),getppid());
            exit(0);

        default:
            wait(NULL);
            printf("default's pid (father) -> %d\n",getpid());
        }
        i++;
    }
    return 0;
}
