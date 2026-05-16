#ifndef MAPPA_H
#define MAPPA_H

#include <stdio.h>
#include "tipi.h"
#include "eroe.h"

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

int get_stanza_corrente(void);
void set_stanza_corrente(int stanza);

/* Cambio stanza: ricostruisce la mappa e posiziona l'eroe sullo spawn */
void cambia_stanza(int nuova, Eroe *eroe);

/* Se l'eroe si trova su una porta aperta che teletrasporta, esegue il cambio.
	Ritorna 1 se è avvenuta una transizione di stanza, 0 altrimenti. */
int entra_porta(Eroe *eroe);

#endif