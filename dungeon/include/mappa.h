#ifndef MAPPA_H
#define MAPPA_H

#include "tipi.h"

// Crea una singola stanza
Stanza* crea_stanza(int id, const char* desc);

// Aggiunge un oggetto alla lista della stanza
void aggiungi_oggetto(Stanza* s, Oggetto* obj);

// Inizializza l'intera mappa del dungeon
Stanza* inizializza_mappa(void);

// Stampa la mappa in ASCII art
void stampa_mappa(Stanza* stanza_corrente);

// Libera la memoria della mappa
void libera_mappa(Stanza* ingresso);

#endif
