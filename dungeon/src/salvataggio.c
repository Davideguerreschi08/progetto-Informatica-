// salvataggio.c — serializzazione e deserializzazione dello stato di gioco.
// Formato del file binario:
//   1. Struct Salvataggio (dati dell'eroe + stanza corrente)
//   2. int count (numero oggetti in inventario)
//   3. count * OggettoSalvato (oggetti dell'inventario)
//   4. Stato della mappa (mostri vivi/morti, oggetti presenti, porte aperte)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/tipi.h"
#include "../include/eroe.h"
#include "../include/mappa.h"

// ─── STRUCT AUSILIARIE ────────────────────────────────────────────────────────

// Dati dell'eroe che vengono serializzati su file.
typedef struct {
    char nome[MAX_NOME];
    int  hp;
    int  hp_max;
    int  attacco;
    int  difesa;
    int  bonus_danno;
    int  xp;
    int  livello;
    int  oro;
    int  pos_riga;
    int  pos_col;
    int  stanza;      // numero della stanza corrente
} Salvataggio;

// Dati di un singolo oggetto dell'inventario serializzati su file.
typedef struct {
    char        nome[MAX_NOME];
    TipoOggetto tipo;
    int         valore;
} OggettoSalvato;

// ─── HELPER ───────────────────────────────────────────────────────────────────

// Svuota e libera tutto l'inventario dell'eroe prima di sovrascriverlo.
static void libera_inventario(Eroe *eroe){
    if (!eroe) return;
    while (eroe->inventario.top >= 0) {
        Oggetto *ogg = pop(eroe);
        free(ogg);
    }
}

// ─── SALVATAGGIO ──────────────────────────────────────────────────────────────

// Salva lo stato completo della partita nel file "save/<filename>".
// Restituisce 0 in caso di successo, -1 in caso di errore.
int salva_partita(Eroe *eroe, const char *filename){
    if (!eroe || !filename || filename[0] == '\0') return -1;

    // Costruisce il percorso completo
    char filepath[512];
    snprintf(filepath, sizeof(filepath), "save/%s", filename);

    FILE *file = fopen(filepath, "wb");
    if (!file) {
        perror("fopen (salvataggio)");
        return -1;
    }

    // 1. Dati dell'eroe
    Salvataggio s;
    strncpy(s.nome, eroe->nome, MAX_NOME - 1);
    s.nome[MAX_NOME - 1] = '\0';
    s.hp          = eroe->hp;
    s.hp_max      = eroe->hp_max;
    s.attacco     = eroe->attacco;
    s.difesa      = eroe->difesa;
    s.bonus_danno = eroe->bonus_danno;
    s.xp          = eroe->xp;
    s.livello     = eroe->livello;
    s.oro         = eroe->oro;
    s.pos_riga    = eroe->pos_riga;
    s.pos_col     = eroe->pos_col;
    s.stanza      = get_stanza_corrente();
    fwrite(&s, sizeof(s), 1, file);

    // 2. Inventario: prima il conteggio, poi gli oggetti uno a uno
    int count = eroe->inventario.top + 1;
    fwrite(&count, sizeof(count), 1, file);
    for (int i = 0; i < count; i++) {
        OggettoSalvato os;
        strncpy(os.nome, eroe->inventario.oggetti[i]->nome, MAX_NOME - 1);
        os.nome[MAX_NOME - 1] = '\0';
        os.tipo   = eroe->inventario.oggetti[i]->tipo;
        os.valore = eroe->inventario.oggetti[i]->valore;
        fwrite(&os, sizeof(os), 1, file);
    }

    // 3. Stato della mappa (delega a mappa.c)
    if (salva_stato_mappa(file) != 0) {
        fclose(file);
        return -1;
    }

    fclose(file);
    printf("Partita salvata in '%s'.\n", filepath);
    return 0;
}

// ─── CARICAMENTO ──────────────────────────────────────────────────────────────

// Carica lo stato della partita dal file "save/<filename>".
// Restituisce 0 in caso di successo, -1 in caso di errore.
int carica_partita(Eroe *eroe, const char *filename){
    if (!eroe || !filename || filename[0] == '\0') return -1;

    char filepath[512];
    snprintf(filepath, sizeof(filepath), "save/%s", filename);

    FILE *file = fopen(filepath, "rb");
    if (!file) {
        perror("fopen (caricamento)");
        return -1;
    }

    // 1. Legge i dati dell'eroe
    Salvataggio s;
    if (fread(&s, sizeof(s), 1, file) != 1) {
        fclose(file);
        return -1;
    }

    strncpy(eroe->nome, s.nome, MAX_NOME - 1);
    eroe->nome[MAX_NOME - 1] = '\0';
    eroe->hp          = s.hp;
    eroe->hp_max      = s.hp_max;
    eroe->attacco     = s.attacco;
    eroe->difesa      = s.difesa;
    eroe->bonus_danno = s.bonus_danno;
    eroe->xp          = s.xp;
    eroe->livello     = s.livello;
    eroe->oro         = s.oro;
    eroe->pos_riga    = s.pos_riga;
    eroe->pos_col     = s.pos_col;

    // Ripristina la stanza corrente prima di ricostruire la mappa
    set_stanza_corrente(s.stanza);

    // 2. Inventario: svuota quello corrente e ricarica da file
    libera_inventario(eroe);
    eroe->inventario.top = -1;

    int count = 0;
    if (fread(&count, sizeof(count), 1, file) == 1) {
        for (int i = 0; i < count; i++) {
            OggettoSalvato os;
            if (fread(&os, sizeof(os), 1, file) != 1) {
                fclose(file);
                return -1;
            }
            Oggetto *ogg = malloc(sizeof(Oggetto));
            if (!ogg) {
                fclose(file);
                return -1;
            }
            strncpy(ogg->nome, os.nome, MAX_NOME - 1);
            ogg->nome[MAX_NOME - 1] = '\0';
            ogg->tipo   = os.tipo;
            ogg->valore = os.valore;
            ogg->next   = NULL;
            push(eroe, ogg);
        }
    }

    // 3. Ricostruisce la mappa della stanza corrente, poi applica lo stato salvato
    distruggi_mappa();
    costruisci_mappa();

    if (carica_stato_mappa(file) != 0) {
        fclose(file);
        return -1;
    }

    fclose(file);
    printf("Partita caricata da '%s'.\n", filepath);
    return 0;
}