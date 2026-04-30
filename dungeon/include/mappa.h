#ifndef MAPPA_H
#define MAPPA_H

#include "tipi.h"

/* Costruisce il grafo delle stanze e restituisce il puntatore alla stanza iniziale */
Stanza *costruisci_mappa(void);

/* Libera tutta la memoria del grafo */
void distruggi_mappa(Stanza **stanze, int n);

/* Stampa la rappresentazione ASCII delle stanze visitate */
void stampa_mappa(Stanza **stanze, int n, Stanza *corrente);

/* Array globale di puntatori alle stanze (per iterazione) */
/* Puntatori a tutte le stanze create nella mappa.
   Viene usato per poter scorrere facilmente tutte le stanze. */
extern Stanza *tutte_stanze[MAX_STANZE];
/* Numero effettivo di stanze valide presenti in tutte_stanze. */
extern int     num_stanze;

#endif /* MAPPA_H */