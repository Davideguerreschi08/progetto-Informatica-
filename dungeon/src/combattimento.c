#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../include/combattimento.h"
#include "../include/eroe.h"
#include "../include/mappa.h"

static void stampa_barra_hp(int hp, int hp_max, int larghezza)
{
    if (hp_max <= 0) hp_max = 1;
    int pieni = (hp * larghezza) / hp_max;
    if (pieni < 0) pieni = 0;
    if (pieni > larghezza) pieni = larghezza;
    printf("[");
    for (int i = 0; i < larghezza; i++)
        printf(i < pieni ? "#" : " ");
    printf("] %d/%d", hp, hp_max);
}

static void stampa_schermata(Eroe *eroe, Mostro *mostro)
{
    printf("\n");
    printf("  +--------------------------------------------+\n");
    printf("  |  %-20s                    |\n", mostro->nome);
    printf("  |  HP: ");
    stampa_barra_hp(mostro->hp, mostro->hp_max, 20);
    printf("\n");
    printf("  +--------------------------------------------+\n");
    printf("  |  %-20s                    |\n", eroe->nome);
    printf("  |  HP: ");
    stampa_barra_hp(eroe->hp, eroe->hp_max, 20);
    printf("\n");
    printf("  +--------------------------------------------+\n");
    printf("\n");
    printf("  Cosa vuoi fare?\n");
    printf("  [1] Attacca\n");
    printf("  [2] Usa oggetto\n");
    printf("  [3] Fuggi\n");
    printf("  > ");
}

static int turno_mostro(Eroe *eroe, Mostro *mostro)
{
    int danno = (rand() % mostro->attacco) + 1 - eroe->difesa;
    if (danno < 1) danno = 1;
    eroe->hp -= danno;
    printf("  %s contrattacca e infligge %d danni!\n", mostro->nome, danno);
    return danno;
}

static int usa_oggetto_in_combat(Eroe *eroe, Mostro *mostro)
{
    if (eroe->inventario.top < 0) {
        printf("  Inventario vuoto!\n");
        return 0;
    }

    // Mostra inventario con indici
    printf("  === SCEGLI OGGETTO DA USARE IN COMBATTIMENTO ===\n");
    for (int i = 0; i <= eroe->inventario.top; i++) {
        printf("  [%d] %s (valore: %d)\n", i, 
               eroe->inventario.oggetti[i]->nome, 
               eroe->inventario.oggetti[i]->valore);
    }
    printf("  ================================================\n");

    // Chiedi scelta
    printf("  Scegli l'oggetto da usare (0-%d): ", eroe->inventario.top);
    int scelta;
    if (scanf("%d", &scelta) != 1) {
        printf("  Input non valido.\n");
        // Pulisci input buffer
        while (getchar() != '\n');
        return 0;
    }

    if (scelta < 0 || scelta > eroe->inventario.top) {
        printf("  Scelta non valida.\n");
        return 0;
    }

    // Rimuovi l'oggetto scelto dalla pila
    Oggetto* ogg = eroe->inventario.oggetti[scelta];
    
    // Sposta tutti gli elementi successivi indietro di una posizione
    for (int i = scelta; i < eroe->inventario.top; i++) {
        eroe->inventario.oggetti[i] = eroe->inventario.oggetti[i + 1];
    }
    eroe->inventario.top--;

    printf("  Hai usato: %s\n", ogg->nome);
    switch (ogg->tipo) {
        case POZIONE:
            eroe->hp += ogg->valore;
            if (eroe->hp > eroe->hp_max) eroe->hp = eroe->hp_max;
            printf("  Recuperi %d HP! HP: %d/%d\n", ogg->valore, eroe->hp, eroe->hp_max);
            free(ogg);
            break;
        case BOMBA: {
            int danno = ogg->valore;
            mostro->hp -= danno;
            printf("  BOOM! La bomba infligge %d danni a %s!\n", danno, mostro->nome);
            free(ogg);
            break;
        }
        case POZIONE_VELENO: {
            int danno = ogg->valore;
            mostro->hp -= danno;
            printf("  Il veleno infligge %d danni a %s!\n", danno, mostro->nome);
            free(ogg);
            break;
        }
        case AMULETO_FORZA:
            eroe->bonus_danno += 6 * ogg->valore;
            printf("  Amuleto della forza livello %d! Danno aumentato di %d!\n", ogg->valore, 6 * ogg->valore);
            free(ogg);
            break;
        case AMULETO_DIFESA:
            eroe->difesa += 4 * ogg->valore;
            printf("  Amuleto della difesa livello %d! Difesa aumentata di %d!\n", ogg->valore, 4 * ogg->valore);
            free(ogg);
            break;
        default:
            printf("  Non puoi usare questo oggetto in combattimento!\n");
            push(eroe, ogg);
            return 0;
    }
    return 1;
}

static void mostro_sconfitto(Eroe *eroe, Mostro *mostro)
{
    mostro->vivo = 0;
    if (mostro->hp < 0) mostro->hp = 0;
    printf("\n  *** %s e' stato sconfitto! ***\n", mostro->nome);
    printf("  Guadagni %d XP e %d oro!\n", mostro->xp_ricompensa, mostro->oro_ricompensa);
    aggiungiXP(eroe, mostro->xp_ricompensa);
    eroe->oro += mostro->oro_ricompensa;

    if (mostro->tipo == BOSS) {
        Oggetto *pozione = malloc(sizeof(Oggetto));
        if (pozione) {
            strncpy(pozione->nome, "Pozione di cura", MAX_NOME);
            pozione->nome[MAX_NOME - 1] = '\0';
            pozione->tipo = POZIONE;
            pozione->valore = 20;
            pozione->next = NULL;
            aggiungi_oggetto_in_posizione(eroe->pos_riga, eroe->pos_col, pozione);
            printf("  Il boss lascia una Pozione di cura!\n");
        }
    }
}

static void eroe_sconfitto(Eroe *eroe)
{
    if (eroe->hp < 0) eroe->hp = 0;
    printf("\n  *** %s e' caduto in battaglia. Game over. ***\n", eroe->nome);
}

void inizia_combattimento(Eroe *eroe, Mostro *mostro)
{
    if (!eroe || !mostro || !mostro->vivo) {
        printf("Non c'e' nessun avversario valido qui.\n");
        return;
    }

    eroe->bonus_danno = 0;
    printf("\n  *** Un %s ti blocca la strada! ***\n", mostro->nome);
    char input[16];

    while (eroe->hp > 0 && mostro->hp > 0) {

        stampa_schermata(eroe, mostro);

        if (!fgets(input, sizeof(input), stdin)) break;
        int scelta = input[0] - '0';
        int turno_usato = 1;

        switch (scelta) {

            case 1: {
                int danno = (rand() % 1000) + 1 + eroe->bonus_danno - mostro->difesa;
                if (danno < 1) danno = 1;
                mostro->hp -= danno;
                printf("  %s attacca e infligge %d danni a %s!\n",
                       eroe->nome, danno, mostro->nome);
                sleep(1);
                break;
            }

            case 2:
                turno_usato = usa_oggetto_in_combat(eroe, mostro);
                break;

            case 3: {
                int fuga = rand() % 2;
                if (fuga) {
                    printf("  Sei fuggito dal combattimento!\n");
                    return;
                } else {
                    printf("  Non riesci a fuggire!\n");
                }
                break;
            }

            default:
                printf("  Scelta non valida. Scegli 1, 2 o 3.\n");
                turno_usato = 0;
                break;

        }  // <-- questa } mancava: era il bug principale

        if (turno_usato && mostro->hp > 0 && eroe->hp > 0) {
            printf("  ...\n");
            sleep(1);
            turno_mostro(eroe, mostro);
            sleep(1);
        }
    }

    if (mostro->hp <= 0) mostro_sconfitto(eroe, mostro);
    if (eroe->hp <= 0)   eroe_sconfitto(eroe);
}