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
    int node = 0;
    int n = atoi(argv[1]);
    printf("father's pid -> %d\n",getpid());
    int tubeM[2];   // main pipe
    int tubes[n][2];    // other pipes
    while (node < n)
    {
        switch (fork())
        {
        case -1:
            perror("fork");
            exit(-1);

        case 0:
            printf("%d\n",getpid());
            for (int i = 0; i < n; i++)
            {
                if(i == node) {
                    close(tubes[i][0]);
                } else if (i == node-1 || (node==0 && i == n-1)) {
                    close(tubes[i][1]);
                } else {
                    close(tubes[i][0]);
                    close(tubes[i][1]);
                }
            }
            close(0);
            if (node == 0)
            {
                dup(tubes[n-1][0]);
                close(tubes[n-1][0]);
            } else {
                dup(tubes[node-1][0]);
                close(tubes[node-1][0]);
            }
            close(1);
            dup(tubes[node][1]);
            close(tubes[node][1]);
            printf("son's pid n°%d -> %d, pid of his father -> %d\n",node,getpid(),getppid());
            exit(0);

        default:
            wait(NULL);
            printf("default's pid (father) -> %d\n",getpid());
        }
        node++;
    }
    for (int i = 0; i < n; i++)
    {
        close(tubes[i][0]);
        close(tubes[i][1]);
    }
    return 0;
}
