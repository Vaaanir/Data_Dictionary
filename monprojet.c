#include <stdio.h>
// #include <stdlib.h>
#include "table.h"

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
                //node()
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
                            //faire exec set à node
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
