

// eroe.h — funzioni per gestire l'eroe: inventario, livelli, oggetti.
#ifndef EROE_H
#define EROE_H
 
#include "tipi.h"
 
// Numero di livelli disponibili e soglie XP (definiti in eroe.c)
#define NUM_LIVELLI 5
extern const int livEXP[];
 
// Inventario (pila LIFO)
void    push(Eroe *e, Oggetto *ogg);  // aggiunge in cima
Oggetto *pop(Eroe *e);               // rimuove dalla cima
void    mostraInventario(Eroe *e);   // stampa il contenuto
 
// Progressione
void aggiungiXP(Eroe *e, int xp);    // aggiunge XP e controlla level-up
 
// Uso oggetti fuori dal combattimento
void usaOggetto(Eroe *e);
 
// Creazione
Eroe *crea_eroe(const char *nome);
 
#endif