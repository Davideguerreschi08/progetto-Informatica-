#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/tipi.h"
#include "../include/eroe.h"
#include "../include/mappa.h"

typedef struct {
    char        nome[MAX_NOME];
    int         hp;
    int         hp_max;
    int         attacco;
    int         difesa;
    int         xp;
    int         livello;
    int         oro;
    int         stanza_id;
} Salvataggio;

typedef struct {
    char        nome[MAX_NOME];
    TipoOggetto tipo;
    int         valore;
} OggettoSalvato;

int salva_partita(Eroe* eroe, Stanza* stanza_corrente, const char* filename)
{
    if (!eroe || !filename || filename[0] == '\0') return -1;

    FILE *file = fopen(filename, "wb");
    if (!file) { perror("fopen"); return -1; }

    Salvataggio s;
    strncpy(s.nome, eroe->nome, MAX_NOME);
    s.hp       = eroe->hp;
    s.hp_max   = eroe->hp_max;
    s.attacco  = eroe->attacco;
    s.difesa   = eroe->difesa;
    s.xp       = eroe->xp;
    s.livello  = eroe->livello;
    s.oro      = eroe->oro;
    s.stanza_id = stanza_corrente ? stanza_corrente->id : -1;
    fwrite(&s, sizeof(s), 1, file);

    int count = eroe->inventario.top + 1;
    fwrite(&count, sizeof(int), 1, file);
    for (int i = 0; i < count; i++) {
        OggettoSalvato os;
        strncpy(os.nome, eroe->inventario.oggetti[i]->nome, MAX_NOME);
        os.tipo   = eroe->inventario.oggetti[i]->tipo;
        os.valore = eroe->inventario.oggetti[i]->valore;
        fwrite(&os, sizeof(os), 1, file);
    }

    fclose(file);
    return 0;
}

int carica_partita(Eroe* eroe, Stanza** stanza_corrente, const char* filename)
{
    if (!eroe || !stanza_corrente || !filename || filename[0] == '\0') return -1;

    FILE *file = fopen(filename, "rb");
    if (!file) { perror("fopen"); return -1; }

    Salvataggio s;
    if (fread(&s, sizeof(s), 1, file) != 1) { fclose(file); return -1; }

    strncpy(eroe->nome, s.nome, MAX_NOME - 1);
    eroe->hp      = s.hp;
    eroe->hp_max  = s.hp_max;
    eroe->attacco = s.attacco;
    eroe->difesa  = s.difesa;
    eroe->xp      = s.xp;
    eroe->livello = s.livello;
    eroe->oro     = s.oro;

    if (s.stanza_id >= 0 && s.stanza_id < num_stanze)
        *stanza_corrente = tutte_stanze[s.stanza_id];
    else
        *stanza_corrente = NULL;

    eroe->inventario.top = -1;

    int count = 0;
    if (fread(&count, sizeof(int), 1, file) == 1) {
        for (int i = 0; i < count; i++) {
            OggettoSalvato os;
            if (fread(&os, sizeof(os), 1, file) != 1) break;

            Oggetto *ogg = malloc(sizeof(Oggetto));
            strncpy(ogg->nome, os.nome, MAX_NOME);
            ogg->tipo   = os.tipo;
            ogg->valore = os.valore;
            ogg->next   = NULL;
            push(eroe, ogg);
        }
    }

    fclose(file);
    return 0;
}
