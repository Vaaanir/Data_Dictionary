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
 
void freeTable(int **table){//vide la table
	free(table[0]);
	free(table);
}

void init_pipes(int taille, int tubeM[2], int** tubes) {
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
    if (ind == 0)//position permet de simplifier les lectures
    {
        position = taille;
    }
    int act;
    while (y!=-1){//permet de faire continuer à tourner le processus fil
        act = 0;
        read(tubes[position-1][0], &y, sizeof(int)); //on lit la valeur qu'on veut
        //On peut mettre un switch ici 
        if (y==SET)   // La commande est SET
        {
            int key;    
            read(tubes[position-1][0], &key, sizeof(int));  //on recup la clé
            char valeur[128];
            read(tubes[position-1][0], valeur, sizeof(char)*128);   //on recup la valeur
            fprintf(stdout,"clé -> %d taille -> %d ind -> %d\n",key,taille,ind);
            int mod = key%taille;
            if (mod < 0) {
                mod += taille;
            }
            if (mod==ind) {  // on est dans noeud qui gère le set
                store(&ptete,key,valeur);
                fprintf(stdout,"clé : %d \t valeur : %s \n",key,lookup(ptete,key)); // test pour voir si on as bien la valeur
                act = 1;
                write(tubeM[1],&act,sizeof(int)); // il faut ensuite signaler au controller qu'on a terminé
            } else {
                // on doit write les 3 infos dans le prochain tube
                write(tubes[ind][1], &y, sizeof(int));
                write(tubes[ind][1], &key, sizeof(int));
                write(tubes[ind][1], &valeur, sizeof(char)*128);
            }
        }
        else if (y==LOOKUP){ //commande lookup
            int key;
            read(tubes[position-1][0], &key, sizeof(int));  //on recup la clé
            int mod = key%taille;
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
        }
        else if(y==DUMP){
            int rec=3; //variable bateau permettant de donner le go au fils suivant (et donc permettre de ne pas mélanger les affichages) 
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
            printf("j'exécute avant de sortir\n");
            x=-1;
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
            //fprintf(stdout,"Saisir la valeur (chaine de caracteres, max 128 chars) : ");
            //fscanf(stdin,"%s",valeur);
            //on demande à l'utilisateur de saisir sa chaine de char
            printf("Saisir la valeur (chaine de caracteres, max 128 chars) : \n");
            scanf("%c",&temp);
            //fscanf(stdin,"%s",valeur);
            fgets(valeur, 128, stdin);
            //faire exec set à node 0 qui transmettra au bon node
            //ecrit dans tube(n-1) 
            x = 1;
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
            x =2;
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
            x=3;
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
            printf("ahah je me suis perdu\n");
            break;
        }
    } while (ent!=0);
}



int main(int argc, char const *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr,"usage : ./monprojet int\n");
        exit(-1);
    }
    int n = atoi(argv[1]);
    if (n>=2)
    {
        controller(n);
    } else {
        fprintf(stderr,"Usage: ./monprojet int (>1)\n");
        return -1;
    }
    return 0;
}