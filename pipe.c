#include <unistd.h>
#include "pipe.h"
//TURC Julien
//ZAETTA Lucas
void init_pipes(int taille, int tubeM[2], int** tubes) {
    pipe(tubeM);
    for (int i = 0; i < taille; i++)
    {
        pipe(tubes[i]);
    }
}

void close_pipes(int ind, int taille, int** tubes, int* tubeM) {
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
}  