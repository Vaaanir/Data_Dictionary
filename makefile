CC = gcc
OBJS = table.o testTable.o
EXEC = testTable
OBJ0 = projet_0.o
EXEC0 = projet_0
OBJ1 = projet1.o
EXEC1 = projet1
OBJ2 = monprojet.o table.o
EXEC2 = monprojet
OBJ3 = projet_jul.o table.o
EXEC3 = projet_jul
all: $(EXEC) $(EXEC0) $(EXEC1) $(EXEC2) $(EXEC3)
$(EXEC): $(OBJS)
		$(CC) $(OBJS) -o $(EXEC)
$(EXEC0): $(OBJ0)
		$(CC) $(OBJ0) -o $(EXEC0)
$(EXEC1): $(OBJ1)
		$(CC) $(OBJ1) -o $(EXEC1)
$(EXEC2): $(OBJ2)
		$(CC) $(OBJ2) -o $(EXEC2)
$(EXEC3): $(OBJ3)
		$(CC) $(OBJ3) -o $(EXEC3)
clean :
		@echo "On efface les fichiers objets" 
		rm $(OBJS) $(OBJ0) $(OBJ1) $(OBJ2) $(OBJ3)