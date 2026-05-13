#ifndef EROE_H
#define EROE_H

#include "tipi.h"

#define NUM_LIVELLI 5
extern const int livEXP[];

// Funzioni per gestire l'inventario (pila)
void push(Eroe* e, Oggetto* obj);
Oggetto* pop(Eroe* e);
void mostraInventario(Eroe* e);

// Funzioni per movimento del personaggio
void cambiaStanza(Stanza** stanzaCorrente, Stanza* nuovaStanza);

// Funzioni per incremento exp e livelli
void aggiungiXP(Eroe* e, int xp);

// Funzioni per uso oggetti
void usaOggetto(Eroe* e);

// Funzione per creare un nuovo eroe
Eroe* crea_eroe(const char* nome);

// Funzione per stampare lo stato dell'eroe
void stampa_stato(Eroe* e);

#endif