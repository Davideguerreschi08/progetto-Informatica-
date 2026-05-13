#ifndef MAPPA_H
#define MAPPA_H

#include "tipi.h"

Stanza *costruisci_mappa(void);
void distruggi_mappa(Stanza **stanze, int n);
void stampa_mappa(Stanza **stanze, int n, Stanza *corrente, Eroe *eroe);
int e_calpestabile(int r, int c);
int porta_chiusa_in(int r, int c);
void apri_porta_in(int r, int c);
int stanza_id_per_posizione(int r, int c);
int mostro_in_posizione(int r, int c, Stanza **out);
int oggetto_in_posizione(int r, int c, Stanza **out);

extern Stanza *tutte_stanze[MAX_STANZE];
extern int     num_stanze;

#endif