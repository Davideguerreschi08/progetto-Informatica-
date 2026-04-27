#include <stdio.h>
#include "combattimento.h"
#include "eroe.h"

void combatti_turno(Eroe* eroe, Mostro* mostro)
{
    if (!eroe || !mostro || !mostro->vivo) {
        return;
    }

    int danno_eroe = eroe->attacco - mostro->difesa;
    if (danno_eroe < 1) danno_eroe = 1;
    mostro->hp -= danno_eroe;
    printf("%s infligge %d danni a %s.\n", eroe->nome, danno_eroe, mostro->nome);

    if (mostro->hp > 0) {
        int danno_mostro = mostro->attacco - eroe->difesa;
        if (danno_mostro < 1) danno_mostro = 1;
        eroe->hp -= danno_mostro;
        printf("%s contrattacca per %d danni.\n", mostro->nome, danno_mostro);
    }
}

void inizia_combattimento(Eroe* eroe, Mostro* mostro)
{
    if (!eroe || !mostro || !mostro->vivo) {
        printf("Non c'è nessun avversario valido da combattere qui.\n");
        return;
    }

    printf("Inizia il combattimento contro %s!\n", mostro->nome);
    while (eroe->hp > 0 && mostro->hp > 0) {
        combatti_turno(eroe, mostro);
    }

    if (mostro->hp <= 0) {
        mostro_sconfitto(eroe, mostro);
    }
    if (eroe->hp <= 0) {
        eroe_sconfitto(eroe);
    }
}

void mostro_sconfitto(Eroe* eroe, Mostro* mostro)
{
    if (!eroe || !mostro) return;
    mostro->vivo = 0;
    if (mostro->hp < 0) mostro->hp = 0;
    printf("%s è stato sconfitto!\n", mostro->nome);
    aggiungiXP(eroe, mostro->xp_ricompensa);
    eroe->oro += mostro->oro_ricompensa;
}

void eroe_sconfitto(Eroe* eroe)
{
    if (!eroe) return;
    if (eroe->hp < 0) eroe->hp = 0;
    printf("%s è stato sconfitto. La partita è finita.\n", eroe->nome);
}
