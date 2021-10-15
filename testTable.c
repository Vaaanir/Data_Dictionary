#include <stdio.h>
#include <stdlib.h>
#include "table.h"
int main(int argc, char const *argv[])
{
    PTable_entry ptete = (PTable_entry) malloc(sizeof(Table_entry));
    //PTable_entry ptete;
    ptete = NULL;
    store(&ptete,9,"bizarre");
    store(&ptete,7,"je trouve");
    store(&ptete,8,"ça");
    store(&ptete,10,"ahah");
    store(&ptete,6,"mais");
    fprintf(stdout,"On affiche la liste entière : \n");
    display(ptete);
    fprintf(stdout,"On cherche une valeur existante : \n");
    fprintf(stdout,"Le resultat est : %s\n",lookup(ptete,7));
    fprintf(stdout,"On cherche une valeur non existante : \n");
    fprintf(stdout,"Le resultat est : %s\n",lookup(ptete,12));
    free(ptete);
    return 0;
}
