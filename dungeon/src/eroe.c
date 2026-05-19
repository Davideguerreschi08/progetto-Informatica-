// eroe.c — gestione inventario (pila), livelli, uso oggetti.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/eroe.h"

// Soglie di XP per passare al livello successivo.
// livEXP[i] = XP necessari per raggiungere il livello i+1.
const int livEXP[NUM_LIVELLI] = {0, 100, 200, 300, 400};

// ─── CREAZIONE EROE ───────────────────────────────────────────────────────────

// Alloca e inizializza un nuovo eroe con valori di partenza fissi.
// La posizione di spawn (2,5) corrisponde alla cella iniziale sulla Mappa 1.
Eroe *crea_eroe(const char *nome){
    Eroe *e = malloc(sizeof(Eroe));
    if (!e) return NULL;

    strncpy(e->nome, nome, MAX_NOME - 1);//strncpy per non sforare
    e->nome[MAX_NOME - 1] = '\0'; 

    e->hp          = 100;
    e->hp_max      = 100;
    e->attacco     = 10;
    e->difesa      = 5;
    e->bonus_danno = 0;
    e->xp          = 0;
    e->livello     = 1;
    e->oro         = 0;

    e->inventario.top = -1;  // pila vuota

    e->pos_riga = 2;   // spawn sulla mappa
    e->pos_col  = 5;

    return e;
}

// ─── INVENTARIO (PILA) ────────────────────────────────────────────────────────

// Aggiunge un oggetto in cima alla pila.
// Se la pila è piena, stampa un avviso e non aggiunge.
void push(Eroe *e, Oggetto *ogg){
    if (e->inventario.top >= MAX_INVENTARIO - 1) {
        printf("Inventario pieno! Non puoi raccogliere altri oggetti.\n");
        return;
    }
    e->inventario.top++;
    e->inventario.oggetti[e->inventario.top] = ogg;
    printf("Hai raccolto: %s\n", ogg->nome);
}

// Rimuove e restituisce l'oggetto in cima alla pila.
// Restituisce NULL se la pila è vuota.
Oggetto *pop(Eroe *e){
    if (e->inventario.top < 0) {
        printf("Inventario vuoto!\n");
        return NULL;
    }
    Oggetto *ogg = e->inventario.oggetti[e->inventario.top];
    e->inventario.top--;
    return ogg;
}

// Stampa tutti gli oggetti in inventario con il loro indice.
void mostraInventario(Eroe *e){
    printf("\n=== INVENTARIO ===\n");
    if (e->inventario.top < 0) {
        printf("Inventario vuoto.\n");
        printf("==================\n\n");
        return;
    }
    for (int i = 0; i <= e->inventario.top; i++) {
        printf("[%d] %s (valore: %d)\n",
               i,
               e->inventario.oggetti[i]->nome,
               e->inventario.oggetti[i]->valore);
    }
    printf("==================\n\n");
}

// ─── USO OGGETTI ──────────────────────────────────────────────────────────────

// Chiede all'utente quale oggetto usare fuori dal combattimento.
// Alcuni oggetti (amuleti, bombe, veleni) sono riservati al combattimento
// e vengono rimessi in inventario se usati fuori.
void usaOggetto(Eroe *e){
    if (e->inventario.top < 0) {
        printf("Inventario vuoto!\n");
        return;
    }

    // Mostra gli oggetti disponibili
    printf("\n=== SCEGLI OGGETTO DA USARE ===\n");
    for (int i = 0; i <= e->inventario.top; i++) {
        printf("[%d] %s (valore: %d)\n",
               i,
               e->inventario.oggetti[i]->nome,
               e->inventario.oggetti[i]->valore);
    }
    printf("================================\n");

    printf("Scegli l'oggetto (0-%d): ", e->inventario.top);

    char buf[16];
    if (!fgets(buf, sizeof(buf), stdin)) {
        printf("Input non valido.\n");
        return;
    }
    int scelta;
    if (sscanf(buf, "%d", &scelta) != 1 ||scelta < 0 || scelta > e->inventario.top) {
        printf("Scelta non valida.\n");
        return;
    }

    // Rimuove l'oggetto scelto shiftando il resto verso il basso
    Oggetto *ogg = e->inventario.oggetti[scelta];
    for (int i = scelta; i < e->inventario.top; i++)
        e->inventario.oggetti[i] = e->inventario.oggetti[i + 1];
    e->inventario.top--;

    printf("Hai usato: %s\n", ogg->nome);

    switch (ogg->tipo) {

        case POZIONE:
            // Cura l'eroe; non può superare hp_max
            e->hp += ogg->valore;
            if (e->hp > e->hp_max) e->hp = e->hp_max;
            printf("Recuperi %d HP. HP: %d/%d\n", ogg->valore, e->hp, e->hp_max);
            free(ogg);
            break;

        case AMULETO_FORZA:// Riservati al combattimento: effetto attivo solo durante lo scontro
            printf("Gli amuleti si usano solo durante un combattimento!\n");
            push(e, ogg);
            break;
        case AMULETO_DIFESA:
            // Riservati al combattimento: effetto attivo solo durante lo scontro
            printf("Gli amuleti si usano solo durante un combattimento!\n");
            push(e, ogg);
            break;

        case BOMBA: 
            // Riservate al combattimento: effetto attivo solo durante lo scontro
            printf("Le bombe si usano solo durante un combattimento!\n");
            push(e, ogg);
            break;
        case POZIONE_VELENO:
            // Riservati al combattimento
            printf("Puoi usarlo solo durante un combattimento!\n");
            push(e, ogg);
            break;

        case CHIAVE:
            // La chiave si usa avvicinandosi a una porta bloccata e premendo D/W/A/S
            printf("E' una chiave: avvicinati a una porta bloccata.\n");
            push(e, ogg);
            break;

        default:
            printf("Non sai come usare questo oggetto.\n");
            push(e, ogg);
            break;
    }
}

// ─── PROGRESSIONE ─────────────────────────────────────────────────────────────

// Aggiunge XP all'eroe e controlla se è avvenuto un level-up.
// A ogni livello: +5 HP max, +3 bonus danno, HP ripristinati al massimo.
void aggiungiXP(Eroe *e, int xp){
    e->xp += xp;
    printf("Hai guadagnato %d XP!\n", xp);

    // Controlla tutti i livelli possibili (può salire più di uno)
    while (e->livello < NUM_LIVELLI - 1 && e->xp >= livEXP[e->livello]) {
        e->livello++;
        e->hp_max  += 5;
        e->bonus_danno += 3;
        e->hp = e->hp_max;  // ripristino HP al level-up
        printf("** Hai raggiunto il livello %d! **\n", e->livello);
        printf("   HP max +5 = %d  |  Danno bonus +3 = %d\n",e->hp_max, e->bonus_danno);
    }
}