#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/salvataggio.h"
#include "../include/mappa.h"

typedef struct {
    char nome[MAX_NOME];
    int hp;
    int hp_max;
    int attacco;
    int difesa;
    int xp;
    int livello;
    int oro;
    int stanza_id;
} Salvataggio;

int salva_partita(Eroe* eroe, Stanza* stanza_corrente, const char* filename)
{
    if (!eroe || !filename || filename[0] == '\0')
        return -1;

    FILE *file = fopen(filename, "wb");
    if (!file) {
        perror("fopen");
        return -1;
    }

    Salvataggio salvataggio;
    strncpy(salvataggio.nome, eroe->nome, MAX_NOME);
    salvataggio.hp = eroe->hp;
    salvataggio.hp_max = eroe->hp_max;
    salvataggio.attacco = eroe->attacco;
    salvataggio.difesa = eroe->difesa;
    salvataggio.xp = eroe->xp;
    salvataggio.livello = eroe->livello;
    salvataggio.oro = eroe->oro;
    salvataggio.stanza_id = stanza_corrente ? stanza_corrente->id : -1;

    if (fwrite(&salvataggio, sizeof(salvataggio), 1, file) != 1) {
        fclose(file);
        return -1;
    }

    fclose(file);
    return 0;
}

int carica_partita(Eroe* eroe, Stanza** stanza_corrente, const char* filename){
    if (!eroe || !stanza_corrente || !filename || filename[0] == '\0')
        return -1;

    FILE *file = fopen(filename, "rb");
    if (!file) {
        perror("fopen");
        return -1;
    }

    Salvataggio salvataggio;
    if (fread(&salvataggio, sizeof(salvataggio), 1, file) != 1) {
        fclose(file);
        return -1;
    }
    fclose(file);

    strncpy(eroe->nome, salvataggio.nome, MAX_NOME - 1);
    eroe->nome[MAX_NOME - 1] = '\0';
    eroe->hp = salvataggio.hp;
    eroe->hp_max = salvataggio.hp_max;
    eroe->attacco = salvataggio.attacco;
    eroe->difesa = salvataggio.difesa;
    eroe->xp = salvataggio.xp;
    eroe->livello = salvataggio.livello;
    eroe->oro = salvataggio.oro;

    if (salvataggio.stanza_id >= 0 && salvataggio.stanza_id < num_stanze){
        *stanza_corrente = tutte_stanze[salvataggio.stanza_id];
    }else{
        *stanza_corrente = NULL;
    }

    return 0;
}

void menu_salvataggio(void){
    printf("Comandi disponibili: salva <file> | carica <file>\n");
}
