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

static void controlla_incontro(Eroe *eroe, bool *partita_vinta)
{
    if (!eroe) return;
    Stanza *s = NULL;
    if (!mostro_in_posizione(eroe->pos_riga, eroe->pos_col, &s)) return;

    printf("\n  *** Sei finito sopra un %s! Lo scontro inizia! ***\n",
           s->mostro->nome);
    eroe->stanza_corrente = s;
    stampa_mappa(tutte_stanze, num_stanze, s, eroe);
    inizia_combattimento(eroe, s->mostro);

    if (s->mostro->tipo == BOSS && !s->mostro->vivo)
        *partita_vinta = true;
}

static void controlla_oggetto(Eroe *eroe)
{
    if (!eroe) return;
    Stanza *s = NULL;
    if (!oggetto_in_posizione(eroe->pos_riga, eroe->pos_col, &s)) return;

    Oggetto *ogg = s->oggetti;
    s->oggetti = ogg->next;
    ogg->next = NULL;

    printf("\n  *** Hai trovato: %s! ***\n", ogg->nome);
    push(eroe, ogg);
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
    printf("Comandi:   guarda, usa, inventario, salva, carica\n\n");

    stampa_mappa(tutte_stanze, num_stanze, eroe->stanza_corrente, eroe);

    while (eroe->hp > 0 && !partita_vinta) {
        printf("> ");
        if (!fgets(input, MAX_INPUT, stdin)) break;

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

static int trova_indice_chiave(Eroe *eroe)
{
    if (!eroe) return -1;
    for (int i = 0; i <= eroe->inventario.top; i++) {
        if (eroe->inventario.oggetti[i]->tipo == CHIAVE)
            return i;
    }
    return -1;
}

static Oggetto *rimuovi_oggetto_inventario(Eroe *eroe, int indice)
{
    if (!eroe || indice < 0 || indice > eroe->inventario.top) return NULL;
    Oggetto *ogg = eroe->inventario.oggetti[indice];
    for (int i = indice; i < eroe->inventario.top; i++)
        eroe->inventario.oggetti[i] = eroe->inventario.oggetti[i + 1];
    eroe->inventario.top--;
    return ogg;
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

            int id = stanza_id_per_posizione(nr, nc);
            if (id >= 0 && id < num_stanze) {
                Stanza *nuova = tutte_stanze[id];
                if (nuova && nuova != eroe->stanza_corrente) {
                    cambiaStanza(&eroe->stanza_corrente, nuova);
                    nuova->visitata = true;
                }
            }

            controlla_oggetto(eroe);
            controlla_incontro(eroe, partita_vinta);

        } else if (porta_chiusa_in(nr, nc)) {
            int idx = trova_indice_chiave(eroe);
            if (idx >= 0) {
                char risposta[8];
                printf("La porta e' bloccata. Vuoi usare una chiave per aprirla? (s/n): ");
                if (fgets(risposta, sizeof risposta, stdin) &&
                    (tolower((unsigned char)risposta[0]) == 's')) {
                    Oggetto *chiave = rimuovi_oggetto_inventario(eroe, idx);
                    if (chiave) free(chiave);
                    apri_porta_in(nr, nc);
                    printf("Hai sbloccato la porta con la chiave.\n");

                    eroe->pos_riga = nr;
                    eroe->pos_col  = nc;
                    int id = stanza_id_per_posizione(nr, nc);
                    if (id >= 0 && id < num_stanze) {
                        Stanza *nuova = tutte_stanze[id];
                        if (nuova && nuova != eroe->stanza_corrente) {
                            cambiaStanza(&eroe->stanza_corrente, nuova);
                            nuova->visitata = true;
                        }
                    }
                    controlla_oggetto(eroe);
                    controlla_incontro(eroe, partita_vinta);
                } else {
                    printf("La porta resta bloccata.\n");
                }
            } else {
                printf("La porta e' bloccata. Ti serve una chiave.\n");
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
        printf("Il combattimento parte automaticamente: calpesta il nemico (M) sulla mappa.\n");
        break;

    case CMD_INVENTARIO:
        mostraInventario(eroe);
        break;

    case CMD_MAPPA:
        printf("La mappa e' gia' visibile!\n");
        break;

    case CMD_SALVA: {
        const char *file = argomento[0] ? argomento : "partita.sav";
        if (salva_partita(eroe, eroe->stanza_corrente, file) == 0) {
            // Messaggio stampato da salva_partita
        } else {
            printf("Errore durante il salvataggio.\n");
        }
        break;
    }

    case CMD_CARICA: {
        const char *file = argomento[0] ? argomento : "partita.sav";
        if (carica_partita(eroe, &eroe->stanza_corrente, file) == 0) {
            // Messaggio stampato da carica_partita
        } else {
            printf("Errore durante il caricamento.\n");
        }
        break;
    }

    default:
        printf("Comando non riconosciuto.\n");
        printf("Usa: W/A/S/D, guarda, usa, inventario, salva, carica\n");
        break;
    }
}