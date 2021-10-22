#include <stdlib.h>
#include "tableau.h"
//TURC Julien
//ZAETTA Lucas
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