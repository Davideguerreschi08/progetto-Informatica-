#ifndef SALVATAGGIO_H
#define SALVATAGGIO_H

#include "tipi.h"

// Salva lo stato della partita su file
int salva_partita(Eroe* eroe, const char* filename);

// Carica lo stato della partita da file
int carica_partita(Eroe* eroe, const char* filename);

#endif