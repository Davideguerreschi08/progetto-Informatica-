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

// Controlla se il giocatore si trova ESATTAMENTE sulla cella di un mostro vivo.
// mostro_in_posizione scorre la tabella ELEMENTI in mappa.c:
// se trova un mostro di tipo 1 con (r,c) uguale alla pos del giocatore,
// restituisce 1 e scrive il puntatore alla stanza in *s.
static void controlla_incontro(Eroe *eroe, bool *partita_vinta)
{
    if (!eroe) return;

    Stanza *s = NULL;
    if (!mostro_in_posizione(eroe->pos_riga, eroe->pos_col, &s)) return;

    // Il giocatore e' esattamente sulla cella 'M' → combattimento automatico
    printf("\n  *** Sei finito sopra un %s! Lo scontro inizia! ***\n",
           s->mostro->nome);

    // Aggiorna la stanza corrente nel caso non fosse gia' quella giusta
    eroe->stanza_corrente = s;

    // Stampa la mappa una volta prima che inizi il combattimento
    stampa_mappa(tutte_stanze, num_stanze, s, eroe);

    inizia_combattimento(eroe, s->mostro);

    // Se era il boss e l'abbiamo sconfitto → partita vinta
    if (s->mostro->tipo == BOSS && !s->mostro->vivo)
        *partita_vinta = true;
}

int main(void)
{
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
        if (!fgets(input, MAX_INPUT, stdin)) break;

        TipoComando cmd = parse_comando(input, argomento);
        esegui_comando(cmd, argomento, eroe, &partita_vinta);

        // Stampa la mappa aggiornata dopo ogni comando.
        // Se il mostro e' morto, 'M' e' gia' sparito perche'
        // stampa_mappa controlla mostro->vivo prima di disegnarlo.
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
        int dr = 0, dc = 0;
        if      (strcmp(argomento, "nord")  == 0) dr = -1;
        else if (strcmp(argomento, "sud")   == 0) dr = +1;
        else if (strcmp(argomento, "est")   == 0) dc = +1;
        else if (strcmp(argomento, "ovest") == 0) dc = -1;
        else { printf("Direzione non valida.\n"); break; }

        int nr = eroe->pos_riga + dr;
        int nc = eroe->pos_col  + dc;

        if (e_calpestabile(nr, nc)) {
            eroe->pos_riga = nr;
            eroe->pos_col  = nc;

            // Aggiorna la stanza logica se siamo entrati in una nuova zona
            int id = stanza_id_per_posizione(nr, nc);
            if (id >= 0 && id < num_stanze) {
                Stanza *nuova = tutte_stanze[id];
                if (nuova && nuova != eroe->stanza_corrente) {
                    cambiaStanza(&eroe->stanza_corrente, nuova);
                    nuova->visitata = true;
                }
            }

            // Controlla se il giocatore e' finito esattamente sulla cella 'M'.
            // Se si', il combattimento parte automaticamente.
            controlla_incontro(eroe, partita_vinta);

        } else {
            printf("Muro! Non puoi andare in quella direzione.\n");
        }
        break;
    }

    case CMD_GUARDA:
        if (eroe->stanza_corrente) {
            printf("\n--- %s ---\n", eroe->stanza_corrente->nome);
            printf("%s\n", eroe->stanza_corrente->descrizione);
            if (eroe->stanza_corrente->oggetti) {
                printf("Oggetti presenti: ");
                for (Oggetto *o = eroe->stanza_corrente->oggetti; o; o = o->next)
                    printf("[%s] ", o->nome);
                printf("\n");
            }
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
        printf("Il combattimento parte automaticamente quando calpesti un nemico (M).\n");
        printf("Muoviti sopra di lui e lo scontro inizia da solo.\n");
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