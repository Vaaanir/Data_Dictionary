#ifndef PIPE_H 
#define PIPE_H
void init_pipes(int taille, int tubeM[2], int** tubes);
void close_pipes(int ind, int taille, int** tubes, int* tubeM);
#endif