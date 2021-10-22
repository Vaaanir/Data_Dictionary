#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include "processus.h"
#include "table.h"
#include "pipe.h"
#include "tableau.h"
#define EXIT 0
#define SET 1
#define LOOKUP 2
#define DUMP 3
//TURC Julien
//ZAETTA Lucas
void node(int** tubes,int * tubeM ,int taille, int ind) {
    //faire la fermeture de tout ce qui est inutile
    close_pipes(ind, taille, tubes, tubeM);

    int position = ind;

    PTable_entry ptete = (PTable_entry) malloc(sizeof(Table_entry));
    ptete = NULL; 

    int y=-1; //on initialise la var pour entrer dans le while
    if (ind == 0)//position permet de simplifier les lectures
    {
        position = taille;
    }
    int act;  //variable qui signifie au controller qu'un fils a exécuté la cmd
    int key;  //variable qui stock la clé 
    int mod;  //variable qui stock le modulo de la clé sur le nombre de pipe
    int rec;  //variable permettant de donner le go au fils suivant lors de la cmd bump (et donc permettre de ne pas mélanger les affichages)  
    while (y!=EXIT){//permet de faire continuer à tourner le processus fil
        act = 0;
        read(tubes[position-1][0], &y, sizeof(int)); //on lit la valeur qu'on veut
        //On peut mettre un switch ici 
        switch (y)
        {
        case SET:   
            read(tubes[position-1][0], &key, sizeof(int));  //on recup la clé
            char valeur[128];
            read(tubes[position-1][0], valeur, sizeof(char)*128);   //on recup la valeur
            mod = key%taille;
            if (mod < 0) {
                mod += taille;
            }
            if (mod==ind) {  // on est dans noeud qui gère le set
                store(&ptete,key,valeur);
                act = 1;
                write(tubeM[1],&act,sizeof(int)); // il faut ensuite signaler au controller qu'on a terminé
            } else {
                // on doit write les 3 infos dans le prochain tube
                write(tubes[ind][1], &y, sizeof(int));
                write(tubes[ind][1], &key, sizeof(int));
                write(tubes[ind][1], &valeur, sizeof(char)*128);
            }
            break;

        case LOOKUP:
            read(tubes[position-1][0], &key, sizeof(int));  //on recup la clé
            mod = key%taille;
            if (mod < 0) {
                mod += taille;
            }
            if (mod==ind) {
                char valeur[128];
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
            break;
        
        case DUMP:
            rec=3; 
            act=1;
            if(ind!=0){
                read(tubes[ind-1][0],&rec,sizeof(int));
            }
            printf("process %d :\n",getpid());
            display(ptete);
            if (ind==taille-1){
                write(tubeM[1],&act,sizeof(int)); //permet au controleur de reprendre la mains
            }
            else{
                write(tubes[ind][1],&rec,sizeof(int)); //écriture du go
            }
            break;

        default:
            break;
        }
    }
    //fermer tous les pipes pour éviter que les processus fils ne deviennent zombie
    close(tubes[position-1][0]);  //on ferme le tube qu'on a fini de read
    close(tubes[position][1]);  //on ferme le tube qui a (maybe) était write
    close(tubeM[1]);    //on ferme l'accès au controller
    free(ptete);    //on libère la mémoire de ptete
    exit(0);
}

void controller(int taille) {
    int ent;
    int cle;
    PTable_entry ptete = (PTable_entry) malloc(sizeof(Table_entry));
    ptete = NULL; 
    int nodec = 0;
    char valeur[128];
    char key[128];
    char nb[1];

    int tubeM[2];   // main pipe
    int **tubes = createTable(taille);   // other pipes 
    init_pipes(taille, tubeM, tubes);
    while (nodec < taille)
    {
        switch (fork())
            {
            case -1:
                perror("fork");
                    exit(-1);
                break;
            
            case 0:
                node(tubes, tubeM, taille, nodec);
                exit(0);
                break;

            default:
                break;
            }
            nodec++;
    }
    //close tous les tubes non utilisées par le père
    for (int comp;comp<taille;comp++){
        close(tubes[comp][0]);
    }
    close(tubeM[1]);
    do {
        fprintf(stdout,"Merci de saisir la commande (0 = exit, 1 = set, 2 = lookup, 3 = dump) : ");
        fscanf(stdin,"%s",nb);    //REGARDER SI BIEN INT
        ent = atoi(nb);
        int x;
        switch (ent) {
            case EXIT:
            //exit (envoi du signal d'arret à chaque node)
            x = EXIT;
            for (int i = 0; i < taille; i++)
            {
                if (i==0) {
                    write(tubes[taille-1][1],&x,sizeof(int));                
                } else {
                    write(tubes[i-1][1],&x,sizeof(int));
                }
            }

            while (wait(NULL)!=-1);

            //close tous les tubes
            for (int i = 0;i<taille;i++){
                close(tubes[i][1]);
            }
            close(tubeM[0]);
            exit(0);
            break;
        case SET:
            fprintf(stdout,"Saisir la cle (nombre décimal, 0 par défault) : ");
            fscanf(stdin,"%s",key);    //REGARDER SI BIEN INT
            cle = atoi(key);
            char temp;
            //on demande à l'utilisateur de saisir sa chaine de char
            printf("Saisir la valeur (chaine de caracteres, max 128 chars) : \n");
            scanf("%c",&temp);
            fgets(valeur, 128, stdin);
            //faire exec set à node 0 qui transmettra au bon node
            //ecrit dans tube(n-1) 
            x = SET;
            write(tubes[taille-1][1], &x, sizeof(int));
            write(tubes[taille-1][1], &cle, sizeof(int));
            write(tubes[taille-1][1], &valeur, sizeof(char)*128);
            read(tubeM[0],&x,sizeof(int));
            break;

        case LOOKUP:
            fprintf(stdout,"Saisir la cle (nombre décimal, 0 par défault) : ");
            fscanf(stdin,"%s",key);    //REGARDER SI BIEN INT
            cle = atoi(key); //le atoi met 0 s'il trouve une chaine de caractère, donc 0 est une valeur par défaut
            //faire exec lookup à node
            //ecrit dans tube(n-1) la clé
            x = LOOKUP;
            // char valeur[128];
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
            x = DUMP;
            for (int i = 0; i < taille; i++)
            {
                if (i==0) {
                    write(tubes[taille-1][1],&x,sizeof(int));                
                } else {
                    write(tubes[i-1][1],&x,sizeof(int));
                }
            }
            read(tubeM[0],&x, sizeof(int));
            break;
                        
        default:
            break;
        }
    } while (ent!=0);
}