#ifndef MAPPA_H
#define MAPPA_H

#include <stdio.h>
#include "tipi.h"

void costruisci_mappa(void);
void distruggi_mappa(void);
void stampa_mappa(Eroe *eroe);
int e_calpestabile(int r, int c);
int porta_chiusa_in(int r, int c);
void apri_porta_in(int r, int c);
Mostro *mostro_in_posizione(int r, int c);
Oggetto *oggetto_in_posizione(int r, int c);
Oggetto *rimuovi_oggetto_da_posizione(int r, int c);
void aggiungi_oggetto_in_posizione(int r, int c, Oggetto *oggetto);
int salva_stato_mappa(FILE *file);
int carica_stato_mappa(FILE *file);

#endif