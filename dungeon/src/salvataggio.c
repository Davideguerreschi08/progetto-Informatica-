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
    int         pos_riga;
    int         pos_col;
} Salvataggio;

typedef struct {
    char        nome[MAX_NOME];
    TipoOggetto tipo;
    int         valore;
} OggettoSalvato;

typedef struct {
    int  mostro_vivo;
    int  num_oggetti;
} StatoStanza;

static void salva_stato_stanze(FILE* file, Stanza** stanze, int num)
{
    if (!file || !stanze || num <= 0) return;
    
    for (int i = 0; i < num; i++) {
        if (!stanze[i]) continue;
        
        StatoStanza ss;
        ss.mostro_vivo = (stanze[i]->mostro && stanze[i]->mostro->vivo) ? 1 : 0;
        
        // Conta gli oggetti rimanenti
        int count = 0;
        for (Oggetto *o = stanze[i]->oggetti; o; o = o->next)
            count++;
        ss.num_oggetti = count;
        
        fwrite(&ss, sizeof(StatoStanza), 1, file);
        
        // Salva gli oggetti
        for (Oggetto *o = stanze[i]->oggetti; o; o = o->next) {
            OggettoSalvato os;
            strncpy(os.nome, o->nome, MAX_NOME);
            os.tipo   = o->tipo;
            os.valore = o->valore;
            fwrite(&os, sizeof(OggettoSalvato), 1, file);
        }
    }
}

static void carica_stato_stanze(FILE* file, Stanza** stanze, int num)
{
    if (!file || !stanze || num <= 0) return;
    
    for (int i = 0; i < num; i++) {
        if (!stanze[i]) continue;
        
        StatoStanza ss;
        if (fread(&ss, sizeof(StatoStanza), 1, file) != 1) return;
        
        // Ripristina lo stato del mostro
        if (stanze[i]->mostro) {
            stanze[i]->mostro->vivo = ss.mostro_vivo;
            if (!ss.mostro_vivo) {
                stanze[i]->mostro->hp = 0;
            }
        }
        
        // Ripristina gli oggetti
        // Prima rimuovi tutti gli oggetti
        while (stanze[i]->oggetti) {
            Oggetto *o = stanze[i]->oggetti;
            stanze[i]->oggetti = o->next;
            free(o);
        }
        
        // Poi ricarica quelli salvati
        for (int j = 0; j < ss.num_oggetti; j++) {
            OggettoSalvato os;
            if (fread(&os, sizeof(OggettoSalvato), 1, file) != 1) return;
            
            Oggetto *ogg = malloc(sizeof(Oggetto));
            if (!ogg) return;
            strncpy(ogg->nome, os.nome, MAX_NOME);
            ogg->tipo   = os.tipo;
            ogg->valore = os.valore;
            ogg->next   = stanze[i]->oggetti;
            stanze[i]->oggetti = ogg;
        }
    }
}

int salva_partita(Eroe* eroe, Stanza* stanza_corrente, const char* filename)
{
    if (!eroe || !filename || filename[0] == '\0') return -1;

    // Costruisci il percorso completo con la cartella save/
    char filepath[512];
    snprintf(filepath, sizeof(filepath), "save/%s", filename);

    FILE *file = fopen(filepath, "wb");
    if (!file) { 
        perror("fopen"); 
        return -1; 
    }

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
    s.pos_riga = eroe->pos_riga;
    s.pos_col  = eroe->pos_col;
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

    // Salva lo stato delle stanze (nemici e oggetti)
    salva_stato_stanze(file, tutte_stanze, num_stanze);

    fclose(file);
    printf("✓ Partita salvata in '%s'.\n", filepath);
    return 0;
}

int carica_partita(Eroe* eroe, Stanza** stanza_corrente, const char* filename)
{
    if (!eroe || !stanza_corrente || !filename || filename[0] == '\0') return -1;

    // Costruisci il percorso completo con la cartella save/
    char filepath[512];
    snprintf(filepath, sizeof(filepath), "save/%s", filename);

    FILE *file = fopen(filepath, "rb");
    if (!file) { 
        perror("fopen"); 
        return -1; 
    }

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
    eroe->pos_riga = s.pos_riga;
    eroe->pos_col  = s.pos_col;

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

    // Carica lo stato delle stanze (nemici e oggetti)
    carica_stato_stanze(file, tutte_stanze, num_stanze);

    fclose(file);
    printf("✓ Partita caricata da '%s'.\n", filepath);
    return 0;
}
