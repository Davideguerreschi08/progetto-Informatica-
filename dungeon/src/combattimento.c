// combattimento.c — sistema di combattimento a turni.
// L'eroe e il mostro si alternano finché uno dei due raggiunge 0 HP.
// Gli amuleti possono essere usati durante il combattimento per
// ottenere bonus temporanei (bonus_danno viene resettato all'inizio di ogni scontro).
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../include/combattimento.h"
#include "../include/eroe.h"
#include "../include/mappa.h"

// Flag che segnala se il boss è stato sconfitto (letto da main.c)
static int boss_vinto = 0;

// ─── STAMPA BARRA HP ──────────────────────────────────────────────────────────

// Disegna una barra stile [###   ] N/MAX di larghezza fissa.
static void stampa_barra_hp(int hp, int hp_max, int larghezza){
    if (hp_max <= 0) hp_max = 1;

    // Quante celle sono "piene"
    int pieni = (hp * larghezza) / hp_max;
    if (pieni < 0) pieni = 0;
    if (pieni > larghezza) pieni = larghezza;

    printf("[");
    for (int i = 0; i < larghezza; i++) {
        if (i < pieni)
            printf("#");
        else
            printf(" ");
    }
    printf("] %d/%d", hp, hp_max);
}

// ─── SCHERMATA COMBATTIMENTO ──────────────────────────────────────────────────

// Mostra lo stato corrente del combattimento: HP di entrambi i contendenti
// e le opzioni disponibili.
static void stampa_schermata(Eroe *eroe, Mostro *mostro){
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
    fflush(stdout);
}

// ─── TURNO DEL MOSTRO ─────────────────────────────────────────────────────────

// Il mostro attacca l'eroe. Il danno è casuale, ridotto dalla difesa dell'eroe.
// Il danno minimo garantito è 1.
static void turno_mostro(Eroe *eroe, Mostro *mostro){
    int danno = (rand() % mostro->attacco) + 1 + 7 - eroe->difesa;
    if (danno < 1) danno = 1;

    eroe->hp -= danno;
    printf("  %s contrattacca e infligge %d danni!\n", mostro->nome, danno);
}

// ─── USO OGGETTO IN COMBATTIMENTO ─────────────────────────────────────────────

// Permette all'eroe di usare un oggetto dall'inventario durante il suo turno.
// Restituisce 1 se un turno è stato consumato, 0 altrimenti.
static int usa_oggetto_in_combat(Eroe *eroe, Mostro *mostro){
    if (eroe->inventario.top < 0) {
        printf("  Inventario vuoto!\n");
        return 0;
    }

    // Mostra gli oggetti disponibili
    printf("  === SCEGLI OGGETTO ===\n");
    for (int i = 0; i <= eroe->inventario.top; i++) {
        printf("  [%d] %s (valore: %d)\n",
               i,
               eroe->inventario.oggetti[i]->nome,
               eroe->inventario.oggetti[i]->valore);
    }
    printf("  =====================\n");
    printf("  Scegli (0-%d): ", eroe->inventario.top);
    fflush(stdout);

    // Legge la scelta
    char scelta_buf[16];
    if (!fgets(scelta_buf, sizeof(scelta_buf), stdin)) {
        printf("  Input non valido.\n");
        return 0;
    }
    int scelta;
    if (sscanf(scelta_buf, "%d", &scelta) != 1 ||
        scelta < 0 || scelta > eroe->inventario.top) {
        printf("  Scelta non valida.\n");
        return 0;
    }

    // Rimuove l'oggetto scelto dalla pila shiftando il resto
    Oggetto *ogg = eroe->inventario.oggetti[scelta];
    for (int i = scelta; i < eroe->inventario.top; i++)
        eroe->inventario.oggetti[i] = eroe->inventario.oggetti[i + 1];
    eroe->inventario.top--;

    printf("  Hai usato: %s\n", ogg->nome);

    switch (ogg->tipo) {

        case POZIONE:
            // Cura l'eroe, senza superare hp_max
            eroe->hp += ogg->valore;
            if (eroe->hp > eroe->hp_max) eroe->hp = eroe->hp_max;
            printf("  Recuperi %d HP! HP: %d/%d\n", ogg->valore, eroe->hp, eroe->hp_max);
            free(ogg);
            break;

        case BOMBA:
            // Danno fisso al mostro
            mostro->hp -= ogg->valore;
            printf("  BOOM! La bomba infligge %d danni a %s!\n", ogg->valore, mostro->nome);
            free(ogg);
            break;

        case POZIONE_VELENO: {
            // Danno casuale tra 20 e 40
            int danno = 20 + (rand() % 21);
            mostro->hp -= danno;
            printf("  Il veleno infligge %d danni a %s!\n", danno, mostro->nome);
            free(ogg);
            break;
        }

        case AMULETO_FORZA:
            // Aumenta il bonus danno per il resto di questo combattimento
            eroe->bonus_danno += 6 * ogg->valore;
            printf("  Amuleto della forza (lv %d): danno +%d!\n",
                   ogg->valore, 6 * ogg->valore);
            free(ogg);
            break;

        case AMULETO_DIFESA:
            // Aumenta la difesa per il resto di questo combattimento
            eroe->difesa += 4 * ogg->valore;
            printf("  Amuleto della difesa (lv %d): difesa +%d!\n",
                   ogg->valore, 4 * ogg->valore);
            free(ogg);
            break;

        default:
            // Oggetto non utilizzabile in combattimento: rimesso in inventario
            printf("  Non puoi usare questo oggetto in combattimento!\n");
            push(eroe, ogg);
            return 0;
    }
    return 1;
}

// ─── FINE COMBATTIMENTO ───────────────────────────────────────────────────────

// Chiamata quando il mostro raggiunge 0 HP.
static void mostro_sconfitto(Eroe *eroe, Mostro *mostro){
    mostro->vivo = 0;
    if (mostro->hp < 0) mostro->hp = 0;

    printf("\n  *** %s e' stato sconfitto! ***\n", mostro->nome);
    printf("  Guadagni %d XP e %d oro!\n",
           mostro->xp_ricompensa, mostro->oro_ricompensa);

    aggiungiXP(eroe, mostro->xp_ricompensa);
    eroe->oro += mostro->oro_ricompensa;

    // Se era il boss, imposta il flag di vittoria
    if (mostro->tipo == BOSS)
        boss_vinto = 1;
}

// Chiamata quando l'eroe raggiunge 0 HP.
static void eroe_sconfitto(Eroe *eroe){
    if (eroe->hp < 0) eroe->hp = 0;
    printf("\n  *** %s e' caduto in battaglia. Game over (gioca meglio!). ***\n", eroe->nome);
}


// Restituisce 1 se il boss è stato sconfitto dall'inizio della sessione.
int boss_sconfitto(void){
    return boss_vinto;
}

// Avvia e gestisce un combattimento a turni tra eroe e mostro.
// Il combattimento termina quando uno dei due raggiunge 0 HP o l'eroe fugge.
void inizia_combattimento(Eroe *eroe, Mostro *mostro){
    if (!eroe || !mostro || !mostro->vivo) {
        printf("Non c'e' nessun avversario valido qui.\n");
        return;
    }

    // Resetta il bonus danno temporaneo all'inizio di ogni scontro
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
                // Attacco: danno casuale (1-10) + attacco + bonus - difesa mostro
                int danno = (rand() % 10) + 1 + eroe->attacco + eroe->bonus_danno- mostro->difesa;
                if (danno < 1) danno = 1;
                mostro->hp -= danno;
                printf("  %s attacca e infligge %d danni a %s!\n",eroe->nome, danno, mostro->nome);
                sleep(1);
                break;
            }

            case 2:
                // Usa un oggetto (consuma il turno solo se l'oggetto è stato usato)
                turno_usato = usa_oggetto_in_combat(eroe, mostro);
                break;

            case 3: {
                // Fuga: 50% di successo
                int dado = rand() % 2;
                if (dado == 1) {
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
        }

        // Il mostro contrattacca solo se l'eroe ha consumato il suo turno
        // e il combattimento non è ancora finito
        if (turno_usato && mostro->hp > 0 && eroe->hp > 0) {
            printf("  ...\n");
            sleep(1);
            turno_mostro(eroe, mostro);
            sleep(1);
        }
    }

    // Determina l'esito
    if (mostro->hp <= 0) mostro_sconfitto(eroe, mostro);
    if (eroe->hp   <= 0) eroe_sconfitto(eroe);
}