#ifndef COMBATTIMENTO_H
#define COMBATTIMENTO_H

#include "tipi.h"

// Simula un turno di combattimento
void combatti_turno(Eroe* eroe, Mostro* mostro);

// Inizializza un combattimento
void inizia_combattimento(Eroe* eroe, Mostro* mostro);

// Gestisce la morte del mostro
void mostro_sconfitto(Eroe* eroe, Mostro* mostro);

// Gestisce la morte dell'eroe
void eroe_sconfitto(Eroe* eroe);

#endif
