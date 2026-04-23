#ifndef SALVATAGGIO_H
#define SALVATAGGIO_H

#include "tipi.h"

// Salva lo stato della partita su file
int salva_partita(Eroe* eroe, Stanza* stanza_corrente, const char* filename);

// Carica lo stato della partita da file
int carica_partita(Eroe* eroe, Stanza** stanza_corrente, const char* filename);

// Stampa il menu del salvataggio
void menu_salvataggio(void);

#endif
