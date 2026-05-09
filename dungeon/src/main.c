#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "../include/tipi.h"
#include "../include/eroe.h"
#include "../include/mappa.h"
#include "../include/combattimento.h"
#include "../include/salvataggio.h"

static TipoComando parse_comando(const char *input, char *argomento);
static void esegui_comando(TipoComando cmd, const char *argomento,
                           Eroe *eroe, bool *partita_vinta);
static void prendi_oggetto(Eroe *eroe, const char *nome);

int main(void)
{
    // Inizializza il generatore di numeri casuali una volta sola all'avvio.
    // Senza questa riga rand() darebbe sempre la stessa sequenza.
    srand((unsigned int)time(NULL));

    Eroe *eroe = crea_eroe("Avventuriero");
    Stanza *iniziale = costruisci_mappa();
    eroe->stanza_corrente = iniziale;
    if (iniziale) iniziale->visitata = true;

    bool partita_vinta = false;
    char input[MAX_INPUT];
    char argomento[MAX_NOME];

    printf("\n╔════════════════════════════════════════════════════════╗\n");
    printf("║       BENVENUTO NEL DUNGEON - AVVENTURA INIZIA         ║\n");
    printf("╚════════════════════════════════════════════════════════╝\n\n");
    printf("Movimento: W=Nord  S=Sud  A=Ovest  D=Est\n");
    printf("Comandi:   guarda, prendi <oggetto>, usa, attacca,\n");
    printf("           inventario, salva, carica\n\n");

    stampa_mappa(tutte_stanze, num_stanze, eroe->stanza_corrente, eroe);

    while (eroe->hp > 0 && !partita_vinta) {
        printf("> ");
        if (!fgets(input, MAX_INPUT, stdin))
            break;

        TipoComando cmd = parse_comando(input, argomento);
        esegui_comando(cmd, argomento, eroe, &partita_vinta);
        stampa_mappa(tutte_stanze, num_stanze, eroe->stanza_corrente, eroe);
    }

    if (partita_vinta)
        printf("\n*** HAI VINTO! Il dungeon e' liberato! ***\n");
    else
        printf("\nPartita terminata.\n");

    distruggi_mappa(tutte_stanze, num_stanze);
    free(eroe);
    return 0;
}

static TipoComando parse_comando(const char *input, char *argomento)
{
    char comando[32] = {0};
    argomento[0] = '\0';
    sscanf(input, "%31s %63[^\n]", comando, argomento);

    // WASD → direzione testuale, poi trattati come CMD_VAI
    if (strlen(comando) == 1) {
        char c = tolower(comando[0]);
        if (c == 'w') { strcpy(argomento, "nord");  return CMD_VAI; }
        if (c == 's') { strcpy(argomento, "sud");   return CMD_VAI; }
        if (c == 'a') { strcpy(argomento, "ovest"); return CMD_VAI; }
        if (c == 'd') { strcpy(argomento, "est");   return CMD_VAI; }
    }

    if (strcmp(comando, "vai")        == 0) return CMD_VAI;
    if (strcmp(comando, "guarda")     == 0) return CMD_GUARDA;
    if (strcmp(comando, "prendi")     == 0) return CMD_PRENDI;
    if (strcmp(comando, "usa")        == 0) return CMD_USA;
    if (strcmp(comando, "attacca")    == 0) return CMD_ATTACCA;
    if (strcmp(comando, "inventario") == 0) return CMD_INVENTARIO;
    if (strcmp(comando, "salva")      == 0) return CMD_SALVA;
    if (strcmp(comando, "carica")     == 0) return CMD_CARICA;
    if (strcmp(comando, "mappa")      == 0) return CMD_MAPPA;
    return CMD_SCONOSCIUTO;
}

static void prendi_oggetto(Eroe *eroe, const char *nome)
{
    Stanza *s = eroe->stanza_corrente;
    if (!s || !nome || nome[0] == '\0') {
        printf("Specifica un oggetto da prendere.\n");
        return;
    }
    Oggetto **ptr = &s->oggetti;
    while (*ptr) {
        if (strcmp((*ptr)->nome, nome) == 0) {
            Oggetto *ogg = *ptr;
            *ptr = ogg->next;
            ogg->next = NULL;
            push(eroe, ogg);
            return;
        }
        ptr = &(*ptr)->next;
    }
    printf("Non c'e' nessun oggetto chiamato '%s' qui.\n", nome);
}

static void esegui_comando(TipoComando cmd, const char *argomento,
                           Eroe *eroe, bool *partita_vinta)
{
    switch (cmd) {

    case CMD_VAI: {
        // Calcola la cella di destinazione in base alla direzione
        int dr = 0, dc = 0;
        if      (strcmp(argomento, "nord")  == 0) dr = -1;
        else if (strcmp(argomento, "sud")   == 0) dr = +1;
        else if (strcmp(argomento, "est")   == 0) dc = +1;
        else if (strcmp(argomento, "ovest") == 0) dc = -1;
        else { printf("Direzione non valida.\n"); break; }

        int nr = eroe->pos_riga + dr;
        int nc = eroe->pos_col  + dc;

        // CONTROLLO CHE HAI DESCRITTO:
        // se la cella di destinazione e' uno spazio → puoi andare
        // se e' '#' o 'I' → muro, non puoi andare
        if (e_calpestabile(nr, nc)) {
            eroe->pos_riga = nr;
            eroe->pos_col  = nc;

            // Controlla se il giocatore e' entrato in una nuova stanza
            int id = stanza_id_per_posizione(nr, nc);
            if (id >= 0 && id < num_stanze) {
                Stanza *nuova = tutte_stanze[id];
                if (nuova && nuova != eroe->stanza_corrente) {
                    // Cambia stanza e stampa il nome
                    cambiaStanza(&eroe->stanza_corrente, nuova);
                    nuova->visitata = true;
                }
            }
        } else {
            printf("Muro! Non puoi andare in quella direzione.\n");
        }
        break;
    }

    case CMD_GUARDA:
        if (eroe->stanza_corrente) {
            printf("\n--- %s ---\n", eroe->stanza_corrente->nome);
            printf("%s\n", eroe->stanza_corrente->descrizione);
            // Mostra oggetti nella stanza
            if (eroe->stanza_corrente->oggetti) {
                printf("Oggetti presenti: ");
                for (Oggetto *o = eroe->stanza_corrente->oggetti; o; o = o->next)
                    printf("[%s] ", o->nome);
                printf("\n");
            }
            // Mostra mostro se vivo
            if (eroe->stanza_corrente->mostro && eroe->stanza_corrente->mostro->vivo)
                printf("Attenzione! Qui c'e' un %s (HP: %d).\n",
                       eroe->stanza_corrente->mostro->nome,
                       eroe->stanza_corrente->mostro->hp);
        }
        break;

    case CMD_PRENDI:
        prendi_oggetto(eroe, argomento);
        break;

    case CMD_USA:
        usaOggetto(eroe);
        break;

    case CMD_ATTACCA:
        if (eroe->stanza_corrente && eroe->stanza_corrente->mostro
            && eroe->stanza_corrente->mostro->vivo) {
            inizia_combattimento(eroe, eroe->stanza_corrente->mostro);
            if (eroe->stanza_corrente->mostro->tipo == BOSS &&
                !eroe->stanza_corrente->mostro->vivo)
                *partita_vinta = true;
        } else {
            printf("Non c'e' nessun nemico da attaccare qui.\n");
        }
        break;

    case CMD_INVENTARIO:
        mostraInventario(eroe);
        break;

    case CMD_MAPPA:
        printf("La mappa e' gia' visibile!\n");
        break;

    case CMD_SALVA: {
        const char *file = argomento[0] ? argomento : "partita.sav";
        if (salva_partita(eroe, eroe->stanza_corrente, file) == 0)
            printf("Partita salvata in '%s'.\n", file);
        else
            printf("Errore durante il salvataggio.\n");
        break;
    }

    case CMD_CARICA: {
        const char *file = argomento[0] ? argomento : "partita.sav";
        if (carica_partita(eroe, &eroe->stanza_corrente, file) == 0)
            printf("Partita caricata da '%s'.\n", file);
        else
            printf("Errore durante il caricamento.\n");
        break;
    }

    default:
        printf("Comando non riconosciuto.\n");
        printf("Usa: W/A/S/D, guarda, prendi, usa, attacca, inventario, salva, carica\n");
        break;
    }
}