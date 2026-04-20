// movimento, inventario (pila), livelli
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "eroe.h"

#define NUM_LIVELLI 5 //livelli di tutto il gioco
const int livEXP[NUM_LIVELLI]={0, 50, 120, 250, 500};

//PUSH (stack)
void push(Eroe* e,  Oggetto ogg){
    nodo* nuovoNodo = malloc(sizeof(nodo)); 
    nuovoNodo->oggetto=ogg;
    nuovoNodo->next = e->inventario;
    e->inventario= nuovoNodo;

}

//pop
Oggetto pop(Eroe* e){
    Oggetto vuoto={"Vuoto", 0};

    if(e->inventario==NULL){
        return vuoto;
    }
    nodo* temp=e->inventario;
    Oggetto ogg=temp->oggetto;
    free(temp);
    return ogg;

}
//stamoa del contenuto dell' inventario

void mostraInventario(Eroe* e){
    nodo* current= e->inventario;

    printf("Inventario:\n");
    while(current!=NULL){
        printf("- %s (%d)\n", current->oggetto.nome, current->oggetto.valore);
        current=current->next;
    }
}

// Spostamento tra stanze (movimento)
void cambiaStanza(int* stanzaCorrente, int nuovaStanza){
    *stanzaCorrente= nuovaStanza;
    printf("%d* stanza\n", *stanzaCorrente);
}

// Aumento di livello e XP

void aggiungiXP(Eroe* e, int xp){
    e->esp+= xp; 
    printf("Complimenti hai guadagnato %d XP!\n", xp);

    while(e->livello< NUM_LIVELLI-1 && e->esp>=livEXP[e->livello+1]){
        
        e->livello++;
        e->hp_max+=10;
        e->attacco+=20;
        e->hp=e->hp_max;

        printf("NUOVO LIVELLO! Ora sei a livello %d\n", e->livello);
    }

}

//USA OGGETTO
void usaOggetto(Eroe* e){

    Oggetto ogg=pop(e);

    if(strcmp(ogg.nome, "Nessuno")==0){
        printf("invetario vuoto!\n");
        return;
    }
    printf("hai usato %s\n");
    e->hp+=ogg.valore;
    if(e->hp> e->hp_max){
        e->hp=e->hp_max;
    }
    printf("HP attuali: %d\n", e->hp);
}