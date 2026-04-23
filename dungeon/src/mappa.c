// costruzione grafo, stampa ASCII

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "tipi.h"

// Helper per creare una stanza
Stanza* crea_stanza(int id, const char* desc) {
    Stanza* s = (Stanza*)malloc(sizeof(Stanza));
    s->id = id;
    strcpy(s->descrizione, desc);
    s->visitata = false;
    s->nord = s->sud = s->est = s->ovest = NULL;
    s->oggetti = NULL;
    s->mostro = NULL;
    return s;
}

// Aggiunge un oggetto alla lista della stanza
void aggiungi_oggetto(Stanza* s, const char* nome) {
    Oggetto* obj = (Oggetto*)malloc(sizeof(Oggetto));
    strcpy(obj->nome, nome);
    
    NodoOggetto* nodo = (NodoOggetto*)malloc(sizeof(NodoOggetto));
    nodo->oggetto = obj;
    nodo->next = s->oggetti;
    s->oggetti = nodo;
}

Stanza* inizializza_mappa() {
    // 1. Allocazione
    Stanza* ingresso = crea_stanza(1, "Ingresso Umido");
    Stanza* corridoio = crea_stanza(2, "Corridoio Stretto");
    Stanza* arena = crea_stanza(3, "Arena del Destino");

    // 2. Collegamento Grafo
    ingresso->nord = corridoio;
    corridoio->sud = ingresso;
    corridoio->est = arena;
    arena->ovest = corridoio;

    // 3. Popolamento
    aggiungi_oggetto(ingresso, "Torcia");
    
    Mostro* m = (Mostro*)malloc(sizeof(Mostro));
    strcpy(m->nome, "Goblin");
    m->hp = 50;
    arena->mostro = m;

    return ingresso;
}