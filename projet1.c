#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

int **createTable(int nbLin){
	int **table = (int **)malloc(sizeof(int*)*nbLin);
	int *table2 = (int *)malloc(sizeof(int)*2*nbLin);
	for(int i = 0 ; i < nbLin ; i++){
		table[i] = &table2[i*2];
	}
	return table;
}
 
void freeTable(int **table){
	free(table[0]);
	free(table);
}

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
    int **tubes = createTable(n);   // other pipes
    //int tubes[n][2];
    pipe(tubeM);
    for (int i = 0; i < n; i++)
    {
        pipe(tubes[i]);
    }
    int x = 4;  // entry value
    int y;  // return value
    
    write(tubes[n-1][1], &x, sizeof(int));
    while (node < n)
    {
        switch (fork())
        {
        case -1:
            perror("fork");
            exit(-1);

        case 0:
            close(tubeM[0]);
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
            // close(0);
            if (node == 0)
            {
                // dup(tubes[n-1][0]);
                read(tubes[n-1][0], &y, sizeof(int));
                close(tubes[n-1][0]);
            } else {
                // dup(tubes[node-1][0]);
                read(tubes[node-1][0], &y, sizeof(int));
                close(tubes[node-1][0]);
            }
            if (node == n-1)
            {
                y = y+1;
            } 
            write(tubeM[1], &y, sizeof(int));
            close(tubeM[1]);
            // close(1);
            // dup(tubes[node][1]);
            write(tubes[node][1], &y, sizeof(int));
            close(tubes[node][1]);
            printf("son's pid n°%d -> %d, pid of his father -> %d\n",node,getpid(),getppid());
            exit(0);

        default:
            wait(NULL);
            read(tubeM[0], &y, sizeof(int));
            printf("default's pid (father) -> %d, main pipe number : %d\n",getpid(),y);
        }
        node++;
    }
    for (int i = 0; i < n; i++)
    {
        close(tubes[i][0]);
        close(tubes[i][1]);
    }
    close(tubeM[1]);
    close(tubeM[0]);
    freeTable(tubes);
    return 0;
}
