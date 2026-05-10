#ifndef MAPPA_H
#define MAPPA_H

#include "tipi.h"

/* Costruisce il grafo delle stanze e restituisce la stanza iniziale */
Stanza *costruisci_mappa(void);

/* Libera tutta la memoria del grafo */
void distruggi_mappa(Stanza **stanze, int n);

/* Stampa la mappa ASCII con giocatore ('&') e mostri ('M') */
void stampa_mappa(Stanza **stanze, int n, Stanza *corrente, Eroe *eroe);

/* Restituisce 1 se la cella (r,c) e' calpestabile (spazio ' '), 0 altrimenti */
int e_calpestabile(int r, int c);

/* Restituisce l'id della stanza corrispondente a (r,c), o -1 se fuori da tutte */
int stanza_id_per_posizione(int r, int c);

/* Restituisce 1 se nella cella (r,c) c'e' un mostro vivo,
   e scrive il puntatore alla sua stanza in *out. 0 altrimenti. */
int mostro_in_posizione(int r, int c, Stanza **out);

/* Array globale di tutte le stanze e contatore */
extern Stanza *tutte_stanze[MAX_STANZE];
extern int     num_stanze;

#endif /* MAPPA_H */