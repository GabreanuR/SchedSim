#ifndef INPUT_H
#define INPUT_H

#include "process_defs.h"

// Returneaza 0 (Succes) sau -1 (Eroare)
int read_input(const char* filename, ProcessQueue* queue);

#endif