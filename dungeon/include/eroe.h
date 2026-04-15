#ifndef EROE_H
#define EROE_H

#define MAX_INV 10
#define MAX_NOME 50

typedef struct Oggetto{
    char nome[MAX_NOME];
    int valore;
}Oggetto;

typedef struct nodo{
    Oggetto oggetto;
    struct nodo* next;
}nodo;

typedef struct Eroe{
    char nome[MAX_NOME];
    int hp;
    int hp_max;
    int attacco;
    int livello;
    int esp;
    nodo* inventario;
}Eroe;

//funzioni per inventario
void push(Eroe* e, Oggetto itm);
Oggetto pop(Eroe* e);
void mostraInventario(Eroe* e);

//movimento del personaggio
void cambiaStanza(int* stanzaCorrente, int nuovaStanza);

//incremento esp e livelli
void aggiungiXP(Eroe* e, int xp);

//itmes (oggetti)
void usaOgetto(Eroe* e);

#endif;