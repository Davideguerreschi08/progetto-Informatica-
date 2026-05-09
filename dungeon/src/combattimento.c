#include <stdio.h>
#include <stdlib.h>   // per rand()
#include "../include/combattimento.h"
#include "../include/eroe.h"

static void combatti_turno(Eroe* eroe, Mostro* mostro)
{
    if (!eroe || !mostro || !mostro->vivo) return;

    // rand() % attacco + 1 → danno variabile tra 1 e attacco
    // poi sottraiamo la difesa del bersaglio (minimo 1)
    int danno_eroe = (rand() % eroe->attacco) + 1 - mostro->difesa;
    if (danno_eroe < 1) danno_eroe = 1;
    mostro->hp -= danno_eroe;
    printf("%s infligge %d danni a %s.\n", eroe->nome, danno_eroe, mostro->nome);

    if (mostro->hp > 0) {
        int danno_mostro = (rand() % mostro->attacco) + 1 - eroe->difesa;
        if (danno_mostro < 1) danno_mostro = 1;
        eroe->hp -= danno_mostro;
        printf("%s contrattacca per %d danni.\n", mostro->nome, danno_mostro);
    }
}

static void mostro_sconfitto(Eroe* eroe, Mostro* mostro)
{
    if (!eroe || !mostro) return;
    mostro->vivo = 0;
    if (mostro->hp < 0) mostro->hp = 0;
    printf("%s e' stato sconfitto!\n", mostro->nome);
    aggiungiXP(eroe, mostro->xp_ricompensa);
    eroe->oro += mostro->oro_ricompensa;
}

static void eroe_sconfitto(Eroe* eroe)
{
    if (!eroe) return;
    if (eroe->hp < 0) eroe->hp = 0;
    printf("%s e' stato sconfitto. La partita e' finita.\n", eroe->nome);
}

void inizia_combattimento(Eroe* eroe, Mostro* mostro)
{
    if (!eroe || !mostro || !mostro->vivo) {
        printf("Non c'e' nessun avversario valido da combattere qui.\n");
        return;
    }

    printf("Inizia il combattimento contro %s!\n", mostro->nome);
    while (eroe->hp > 0 && mostro->hp > 0) {
        combatti_turno(eroe, mostro);
    }

    if (mostro->hp <= 0) mostro_sconfitto(eroe, mostro);
    if (eroe->hp <= 0)   eroe_sconfitto(eroe);
}