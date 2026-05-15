// movimento, inventario (pila), livelli
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/eroe.h"

#define NUM_LIVELLI 5 // livelli di tutto il gioco
const int livEXP[NUM_LIVELLI] = {0, 100, 200, 300, 400};

// Crea un nuovo eroe
// In crea_eroe, aggiungi la posizione iniziale sulla mappa visiva:
Eroe* crea_eroe(const char* nome) {
    Eroe* e = (Eroe*)malloc(sizeof(Eroe));
    strcpy(e->nome, nome);
    e->hp = 100;
    e->hp_max = 100;
    e->attacco = 10;
    e->difesa = 5;
    e->bonus_danno = 0;
    e->xp = 0;
    e->livello = 1;
    e->oro = 0;
    e->inventario.top = -1;
    e->pos_riga = 2;   // ← NUOVO: posizione di spawn sulla mappa
    e->pos_col  = 5;   // ← NUOVO
    return e;
}

// usaOggetto con switch — ogni tipo fa la cosa giusta
void usaOggetto(Eroe* e) {
    if (e->inventario.top < 0) {
        printf("Inventario vuoto!\n");
        return;
    }

    // Mostra inventario con indici
    printf("\n=== SCEGLI OGGETTO DA USARE ===\n");
    for (int i = 0; i <= e->inventario.top; i++) {
        printf("[%d] %s (valore: %d)\n", i, 
               e->inventario.oggetti[i]->nome, 
               e->inventario.oggetti[i]->valore);
    }
    printf("=================================\n");

    // Chiedi scelta
    printf("Scegli l'oggetto da usare (0-%d): ", e->inventario.top);
    int scelta;
    if (scanf("%d", &scelta) != 1) {
        printf("Input non valido.\n");
        // Pulisci input buffer
        while (getchar() != '\n');
        return;
    }

    if (scelta < 0 || scelta > e->inventario.top) {
        printf("Scelta non valida.\n");
        return;
    }

    // Rimuovi l'oggetto scelto dalla pila
    Oggetto* ogg = e->inventario.oggetti[scelta];
    
    // Sposta tutti gli elementi successivi indietro di una posizione
    for (int i = scelta; i < e->inventario.top; i++) {
        e->inventario.oggetti[i] = e->inventario.oggetti[i + 1];
    }
    e->inventario.top--;

    printf("Hai usato: %s\n", ogg->nome);

    switch (ogg->tipo) {
        case POZIONE:
            // ripristina HP, senza superare il massimo
            e->hp += ogg->valore;
            if (e->hp > e->hp_max) e->hp = e->hp_max;
            printf("Recuperi %d HP. HP: %d/%d\n", ogg->valore, e->hp, e->hp_max);
            free(ogg);  // consumato, libera memoria
            break;

        case AMULETO_FORZA:
        case AMULETO_DIFESA:
            printf("Gli amuleti si usano solo durante un combattimento!\n");
            push(e, ogg);
            break;

        case BOMBA:
        case POZIONE_VELENO:
            // questi si usano solo in combattimento — rimetti in inventario
            printf("Puoi usarlo solo durante un combattimento!\n");
            push(e, ogg);
            break;

        case CHIAVE:
            printf("È una chiave: avvicinati a una porta bloccata.\n");
            push(e, ogg);
            break;

        default:
            printf("Non sai come usare questo oggetto.\n");
            push(e, ogg);
            break;
    }
}

// PUSH (stack) - aggiunge oggetto in cima alla pila
void push(Eroe* e, Oggetto* ogg) {
    if (e->inventario.top >= MAX_INVENTARIO - 1) {
        printf("Inventario pieno!\n");
        return;
    }
    e->inventario.top++;
    e->inventario.oggetti[e->inventario.top] = ogg;
    printf("Hai raccolto: %s\n", ogg->nome);
}

// POP - toglie oggetto dalla cima della pila
Oggetto* pop(Eroe* e) {
    if (e->inventario.top < 0) {
        printf("Inventario vuoto!\n");
        return NULL;
    }
    Oggetto* ogg = e->inventario.oggetti[e->inventario.top];
    e->inventario.top--;
    return ogg;
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

// Aumento di livello e XP
void aggiungiXP(Eroe* e, int xp) {
    e->xp += xp;
    printf("Hai guadagnato %d XP!\n", xp);

    while (e->livello < NUM_LIVELLI - 1 && e->xp >= livEXP[e->livello]) {
        e->livello++;
        e->hp_max += 5;
        e->bonus_danno += 3;
        e->hp = e->hp_max;
        printf("** Hai raggiunto il livello %d! **\n", e->livello);
        printf("** HP massimi +5 = %d, danno aggiuntivo +3 = %d **\n",
               e->hp_max, e->bonus_danno);
    }
}

// Stampa lo stato dell'eroe
void stampa_stato(Eroe* e) {
    printf("\n=== STATO EROE ===\n");
    printf("Nome: %s\n", e->nome);
    printf("HP: %d/%d\n", e->hp, e->hp_max);
    printf("Livello: %d | XP: %d\n", e->livello, e->xp);
    printf("Att. agg.: %d | Difesa: %d\n", e->bonus_danno, e->hp_max);
    printf("Oro: %d\n", e->oro);
    printf("Posizione: (%d, %d)\n", e->pos_riga, e->pos_col);
    printf("==================\n\n");
}