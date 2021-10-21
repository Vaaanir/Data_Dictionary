#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include "table.h"
#define EXIT 0
#define SET 1
#define LOOKUP 2
#define DUMP 3

int **createTable(int nbLin){ //crée le tableau permettant de faire passer les commandes dans les pipes
    int nbCol = 2;
	int **table = (int **)malloc(sizeof(int*)*nbLin);
	int *table2 = (int *)malloc(sizeof(int)*nbCol*nbLin);
	for(int i = 0 ; i < nbLin ; i++){
		table[i] = &table2[i*nbCol];
	}
	return table;
}
 
void freeTable(int **table){//vide la table ?
	free(table[0]);
	free(table);
}

void init_pipes(int taille, int tubeM[2], int** tubes) {//ne marche pas
    tubes = createTable(taille);   // other pipes
    pipe(tubeM);
    for (int i = 0; i < taille; i++)
    {
        pipe(tubes[i]);
    }
}

void node(int** tubes,int * tubeM ,int taille, int ind) {
    //faire la fermeture de tout ce qui est inutile
    close(tubeM[0]);
    for (int i = 0; i < taille; i++)
    {
        if(i == ind) { //on close la lecture de son propre tube
            close(tubes[i][0]);
        } else if (i == ind-1 || (ind==0 && i == taille-1)) { //on ferme l'écriture du tube d'avant
            close(tubes[i][1]);
        } else {
            close(tubes[i][0]);//on ferme les tubes dont on n'as pas besoin
            close(tubes[i][1]);
        }
    }

    int position = ind;

    PTable_entry ptete = (PTable_entry) malloc(sizeof(Table_entry));
    ptete = NULL; 

    int y=0; //on initialise la var pour entrer dans le while
    if (ind == 0)
    {
        position = taille;
    }
    while (y!=-1){//permet de faire continuer à tourner le processus fil
        read(tubes[position-1][0], &y, sizeof(int)); //on lit la valeur qu'on veux
        if (y==1)   // La commande est SET
        {
            int key;
            read(tubes[position-1][0], &key, sizeof(int));  //on recup la clé
            char valeur[128];
            read(tubes[position-1][0], valeur, sizeof(char)*128);   //on recup la valeur
            if (key%taille==ind) {  // on est dans noeud qui gère le set
                store(&ptete,key,valeur);
                fprintf(stdout,"clé : %d \t valeur : %s \n",key,lookup(ptete,key)); // test pour voir si on as bien la valeur
                // il faut ensuite signaler au controller qu'on a terminé
                //
                //write(tubeM[0][0])
            } else {
                // on doit write les 3 infos dans le prochain tube
                write(tubes[ind][1], &y, sizeof(int));
                write(tubes[ind][1], &key, sizeof(int));
                write(tubes[ind][1], &valeur, sizeof(char)*128);
            }
        }
        else if (y==2){ //commande lookup
            int key;
            read(tubes[position-1][0], &key, sizeof(int));  //on recup la clé
            if (key%taille==ind){
                char valeur[128];
                int act=0;
                if (lookup(ptete,key)!=NULL){
                    act=1;
                    write(tubeM[1], &act, sizeof(int));
                    write(tubeM[1], lookup(ptete,key), sizeof(char)*128);
                }
                else {
                    write(tubeM[1], &act, sizeof(int));
                }
            }
            else{ //il passe à son voisin la recherche
                write(tubes[ind][1], &y, sizeof(int));
                write(tubes[ind][1], &key, sizeof(int));

            }
        }
    }

    //fermer tous les pipes pour éviter que les processus fils ne deviennent zombie
    
    
}

void controller(int taille) {
    int ent;
    int cle;
    PTable_entry ptete = (PTable_entry) malloc(sizeof(Table_entry));
    ptete = NULL; 
    int nodec = 0;
    char valeur[128];

    int tubeM[2];   // main pipe
    int **tubes = createTable(taille);   // other pipes 
    pipe(tubeM);
    for (int i = 0; i < taille; i++)
    {
        pipe(tubes[i]);
    }
    // init_pipes(taille, tubeM, tubes);
    
    while (nodec < taille)
    {
        switch (fork())
            {
            case -1:
                perror("fork");
                    exit(-1);
                break;
            
            case 0:
                printf("%d pid -> %d\n",nodec,getpid());
                node(tubes, tubeM, taille, nodec);
                exit(0);
                break;

            default://faire if pour vérifier si c'est un processus fils
                break;
            }
            nodec++;
    }
    do {
        fprintf(stdout,"Merci de saisir la commande (0 = exit, 1 = set, 2 = lookup, 3 = dump) : ");
        fscanf(stdin,"%d",&ent);
        int x;
        switch (ent) {
            case EXIT:
            //exit (envoi du signal d'arret à chaque node)
            printf("j'exécute avant de sortir\n");
            x=-1;
            write(tubes[taille-1][1],&x,sizeof(int));
            exit(0);
            break;
        case SET:
            fprintf(stdout,"Saisir la cle (decimal number) : ");
            fscanf(stdin,"%d",&cle);
            fprintf(stdout,"Saisir la valeur (chaine de caracteres, max 128 chars) : ");
            fscanf(stdin,"%s",valeur);
            //faire exec set à node 0 qui transmettra au bon node
            //ecrit dans tube(n-1) 
            x = 1;
            write(tubes[taille-1][1], &x, sizeof(int));
            write(tubes[taille-1][1], &cle, sizeof(int));
            write(tubes[taille-1][1], &valeur, sizeof(char)*128);
            break;

        case LOOKUP:
            fprintf(stdout,"Saisir la cle (decimal number) : ");
            fscanf(stdin,"%d",&cle);
            //faire exec lookup à node
            //ecrit dans tube(n-1) la clé
            x =2;
            char valeur[128];
            write(tubes[taille-1][1], &x, sizeof(int));
            write(tubes[taille-1][1], &cle, sizeof(int));
            read(tubeM[0],&x,sizeof(int));
            if (x==1){
                read(tubeM[0],valeur,sizeof(char)*128);
                fprintf(stdout,"Valeur trouvée = %s \n",valeur);
            }
            else{
                fprintf(stdout,"Valeur non trouvée \n");
            }
            break;

        case DUMP:
            //dump (debug)
            break;
                        
        default:
            break;
                        }
    } while (ent!=0);
// int y;
// read(tubes[taille-1][0], &y, sizeof(int));
// int key;
// read(tubes[taille-1][0], &key, sizeof(int));
// char string[128];
// read(tubes[taille-1][0], string, sizeof(char)*128);
// printf("Cmd -> %d Clé -> %d Valeur -> %s\n", y, key, string);
    
    
}



int main(int argc, char const *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr,"usage : ./monprojet int\n");
        exit(-1);
    }
    int n = atoi(argv[1]);
    controller(n);
    //essayer do while merci de rentrer... et appeler controller dedans
    return 0;
}