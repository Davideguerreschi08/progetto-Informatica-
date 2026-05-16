#ifndef COMBATTIMENTO_H
#define COMBATTIMENTO_H

#include "tipi.h"

// Inizializza un combattimento
void inizia_combattimento(Eroe* eroe, Mostro* mostro);

// Restituisce 1 se il boss e' stato sconfitto durante il combattimento
int boss_sconfitto(void);

#endif