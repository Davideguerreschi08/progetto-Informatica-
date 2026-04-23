// movimento, inventario (pila), livelli
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "eroe.h"

#define NUM_LIVELLI 5 //livelli di tutto il gioco
const int livEXP[NUM_LIVELLI] = {0, 50, 120, 250, 500};

// Crea un nuovo eroe
Eroe* crea_eroe(const char* nome) {
    Eroe* e = (Eroe*)malloc(sizeof(Eroe));
    strcpy(e->nome, nome);
    e->hp = 100;
    e->hp_max = 100;
    e->attacco = 10;
    e->difesa = 5;
    e->xp = 0;
    e->livello = 1;
    e->oro = 0;
    e->inventario.top = -1;
    e->stanza_corrente = NULL;
    return e;
}

// PUSH (stack) - aggiunge oggetto in cima alla pila
void push(Eroe* e, Oggetto* obj) {
    if (e->inventario.top >= MAX_INVENTARIO - 1) {
        printf("Inventario pieno!\n");
        return;
    }
    e->inventario.top++;
    e->inventario.oggetti[e->inventario.top] = obj;
    printf("Hai raccolto: %s\n", obj->nome);
}

// POP - toglie oggetto dalla cima della pila
Oggetto* pop(Eroe* e) {
    if (e->inventario.top < 0) {
        printf("Inventario vuoto!\n");
        return NULL;
    }
    Oggetto* obj = e->inventario.oggetti[e->inventario.top];
    e->inventario.top--;
    return obj;
}

// Stampa il contenuto dell'inventario
void mostraInventario(Eroe* e) {
    printf("\n=== INVENTARIO ===\n");
    if (e->inventario.top < 0) {
        printf("Inventario vuoto.\n");
        return;
    }
    for (int i = 0; i <= e->inventario.top; i++) {
        printf("[%d] %s (valore: %d)\n", i, 
               e->inventario.oggetti[i]->nome, 
               e->inventario.oggetti[i]->valore);
    }
    printf("==================\n\n");
}

// Spostamento tra stanze (movimento)
void cambiaStanza(Stanza** stanzaCorrente, Stanza* nuovaStanza) {
    if (nuovaStanza == NULL) {
        printf("Non puoi andare in quella direzione!\n");
        return;
    }
    *stanzaCorrente = nuovaStanza;
    printf("Sei entrato in: %s\n", nuovaStanza->nome);
}

// Aumento di livello e XP
void aggiungiXP(Eroe* e, int xp) {
    e->xp += xp;
    printf("Hai guadagnato %d XP!\n", xp);

    while (e->livello < NUM_LIVELLI - 1 && e->xp >= livEXP[e->livello + 1]) {
        e->livello++;
        e->hp_max += 10;
        e->attacco += 2;
        e->hp = e->hp_max;
        printf("** NUOVO LIVELLO! Ora sei a livello %d **\n", e->livello);
    }
}

// Usa un oggetto dall'inventario
void usaOggetto(Eroe* e) {
    Oggetto* obj = pop(e);
    if (obj == NULL) {
        return;
    }
    printf("Hai usato: %s\n", obj->nome);
    e->hp += obj->valore;
    if (e->hp > e->hp_max) {
        e->hp = e->hp_max;
    }
    printf("HP attuali: %d/%d\n", e->hp, e->hp_max);
}

// Stampa lo stato dell'eroe
void stampa_stato(Eroe* e) {
    printf("\n=== STATO EROE ===\n");
    printf("Nome: %s\n", e->nome);
    printf("HP: %d/%d\n", e->hp, e->hp_max);
    printf("Livello: %d | XP: %d\n", e->livello, e->xp);
    printf("Attacco: %d | Difesa: %d\n", e->attacco, e->difesa);
    printf("Oro: %d\n", e->oro);
    printf("Stanza: %s\n", e->stanza_corrente ? e->stanza_corrente->nome : "Nessuna");
    printf("==================\n\n");
}