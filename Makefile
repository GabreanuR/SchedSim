CC = gcc
CFLAGS = -Wall -g
# -Wall: ca sa vedem warning-urile
# -g: permite debugging-ul cu gdb

# Lista de fișiere obiect necesare pentru program
OBJS = main.o generator.o input.o scheduler.o 

# Regula principala: cum construim executabilul 'schedsim'
schedsim: $(OBJS)
	$(CC) $(CFLAGS) -o schedsim $(OBJS)

# Reguli pentru fiecare fisier sursa
main.o: main.c process_defs.h generator.h input.h scheduler.h
	$(CC) $(CFLAGS) -c main.c

generator.o: generator.c generator.h process_defs.h
	$(CC) $(CFLAGS) -c generator.c

input.o: input.c input.h process_defs.h
	$(CC) $(CFLAGS) -c input.c

scheduler.o: scheduler.c scheduler.h process_defs.h
	$(CC) $(CFLAGS) -c scheduler.c

# Curatare (sterge fisierele temporare)
clean:
	rm -f *.o schedsim

