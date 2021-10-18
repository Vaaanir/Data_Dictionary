#include <stdio.h>
#include <stdlib.h>
#include "table.h"

int **createTable(int nbLin){
    int nbCol = 2;
	int **table = (int **)malloc(sizeof(int*)*nbLin);
	int *table2 = (int *)malloc(sizeof(int)*nbCol*nbLin);
	for(int i = 0 ; i < nbLin ; i++){
		table[i] = &table2[i*nbCol];
	}
	return table;
}
 
void freeTable(int **table){
	free(table[0]);
	free(table);
}

void init_pipes() {

}

void controller(int taille) {
    int ent;
    int cle;
    int n;
    int node = 0;
    char valeur[128];
    while (node < taille)
    {
        switch (fork(n))
            {
            case -1:
                perror("fork");
                    exit(-1);
                break;
            
            case 0:
                //node(cmd,)
                break;

            default:
                do
                {
                    fprintf(stdout,"Merci de saisir la commande (0 = exit, 1 = set, 2 = lookup, 3 = dump) : ");
                    fscanf(stdin,"%d",&ent);
                    switch (ent)
                        {
                        case 0:
                            //exit (envoi du signal d'arret à chaque node)
                            break;

                        case 1:
                            fprintf(stdout,"Saisir la cle (decimal number) : ");
                            fscanf(stdin,"%d",&cle);
                            fprintf(stdout,"Saisir la valeur (chaine de caracteres, max 128 chars) : ");
                            fscanf(stdin,"%s",&valeur);
                            //faire exec set à node 0 qui transmettra au bon node
                            break;

                        case 2:
                            fprintf(stdout,"Saisir la cle (decimal number) : ");
                            fscanf(stdin,"%d",&cle);
                            //faire exec lookup à node
                            break;

                        case 3:
                            //dump (debug)
                            break;
                        
                        default:
                            break;
                        }
                } while (ent!=0);
                break;
            }
            node++;
    }
    
    
    
}

//void node()

int main(int argc, char const *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr,"usage : ./monprojet int\n");
        exit(-1);
    }
    int n = atoi(argv[1]);
    controller(n);
    
    return 0;
}
