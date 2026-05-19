// main.c — ciclo principale del gioco, parser dei comandi, gestione del movimento.
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

// ─── PROTOTIPI STATICI ────────────────────────────────────────────────────────
static TipoComando parse_comando(const char *input, char *argomento);
static void esegui_comando(TipoComando cmd, const char *argomento,Eroe *eroe, bool *partita_vinta);
static int  trova_indice_chiave(Eroe *eroe);
static Oggetto *rimuovi_oggetto_inventario(Eroe *eroe, int indice);

// ─── INCONTRI ─────────────────────────────────────────────────────────────────

// Controlla se nella posizione corrente c'è un mostro vivo.
// Se sì, avvia il combattimento e controlla se è stato sconfitto il boss.
static void controlla_incontro(Eroe *eroe, bool *partita_vinta){
    if (!eroe) return;

    Mostro *m = mostro_in_posizione(eroe->pos_riga, eroe->pos_col);
    if (!m) return;  // nessun mostro qui

    printf("\n  *** Sei finito sopra un %s! Lo scontro inizia! ***\n", m->nome);
    stampa_mappa(eroe);
    inizia_combattimento(eroe, m);

    // Il flag boss_sconfitto viene impostato dentro combattimento.c
    if (boss_sconfitto())
        *partita_vinta = true;
}

// Controlla se nella posizione corrente c'è un oggetto.
// Se sì, lo rimuove dalla mappa e lo aggiunge all'inventario.
static void controlla_oggetto(Eroe *eroe){
    if (!eroe) return;

    Oggetto *ogg = rimuovi_oggetto_da_posizione(eroe->pos_riga, eroe->pos_col);
    if (!ogg) return;  // nessun oggetto qui

    printf("\n  *** Hai trovato: %s! ***\n", ogg->nome);
    push(eroe, ogg);
}

// ─── MAIN ─────────────────────────────────────────────────────────────────────

int main(){
    srand((unsigned int)time(NULL));

    // Chiede il nome all'utente prima di iniziare
    char nome_eroe[MAX_NOME];
    printf("\n╔════════════════════════════════════════════════════════╗\n");
    printf("║       BENVENUTO NEL DUNGEON - AVVENTURA INIZIA         ║\n");
    printf("╚════════════════════════════════════════════════════════╝\n\n");
    printf("Come si chiama il tuo personaggio? ");
    fflush(stdout);

    if (!fgets(nome_eroe, sizeof(nome_eroe), stdin)) {
        strcpy(nome_eroe, "Avventuriero");
    } else {
        // Rimuove il newline finale
        nome_eroe[strcspn(nome_eroe, "\n")] = '\0';
        // Se l'utente ha premuto invio senza scrivere, usa il nome di default
        if (nome_eroe[0] == '\0')
            strcpy(nome_eroe, "Avventuriero");
    }

    Eroe *eroe = crea_eroe(nome_eroe);
    if (!eroe) {
        fprintf(stderr, "Errore: impossibile creare l'eroe.\n");
        return 1;
    }

    costruisci_mappa();

    bool partita_vinta = false;
    char input[MAX_INPUT];
    char argomento[MAX_NOME];

    printf("\nBenvenuto, %s!\n", eroe->nome);
    printf("Movimento: W=Nord  S=Sud  A=Ovest  D=Est\n");
    printf("Comandi:   usa, inventario, salva [file], carica [file]\n\n");

    stampa_mappa(eroe);

    // Ciclo principale: continua finché l'eroe è vivo e non ha vinto
    while (eroe->hp > 0 && !partita_vinta) {
        printf("> ");
        fflush(stdout);

        if (!fgets(input, MAX_INPUT, stdin)) break;

        TipoComando cmd = parse_comando(input, argomento);
        esegui_comando(cmd, argomento, eroe, &partita_vinta);

        stampa_mappa(eroe);
    }

    if (partita_vinta)
        printf("\n*** HAI VINTO! ti ringraziamo per aver completato la beta, %s! ***\n", eroe->nome);
    else
        printf("\nPartita terminata. Alla prossima, %s!\n", eroe->nome);

    distruggi_mappa();
    free(eroe);
    return 0;
}

// ─── PARSER ───────────────────────────────────────────────────────────────────

// Legge una stringa di input e restituisce il comando corrispondente.
// Eventuali argomenti (es. "nord" in "vai nord") vengono scritti in 'argomento'.
// I tasti W/A/S/D sono scorciatoie per il movimento.
static TipoComando parse_comando(const char *input, char *argomento){
    char comando[32] = {0};
    argomento[0] = '\0';

    // Legge il primo token (il comando) e l'eventuale argomento
    sscanf(input, "%31s %63[^\n]", comando, argomento);

    // Scorciatoie da un solo tasto
    if (strlen(comando) == 1) {
        char c = (char)tolower((unsigned char)comando[0]);
        if (c == 'w') { strcpy(argomento, "nord");  return CMD_VAI; }
        if (c == 's') { strcpy(argomento, "sud");   return CMD_VAI; }
        if (c == 'a') { strcpy(argomento, "ovest"); return CMD_VAI; }
        if (c == 'd') { strcpy(argomento, "est");   return CMD_VAI; }
    }

    // Comandi testuali
    if (strcmp(comando, "vai")        == 0) return CMD_VAI;
    if (strcmp(comando, "usa")        == 0) return CMD_USA;
    if (strcmp(comando, "inventario") == 0) return CMD_INVENTARIO;
    if (strcmp(comando, "salva")      == 0) return CMD_SALVA;
    if (strcmp(comando, "carica")     == 0) return CMD_CARICA;

    return CMD_SCONOSCIUTO;
}

// ─── HELPERS INVENTARIO ───────────────────────────────────────────────────────

// Restituisce l'indice della prima chiave trovata in inventario, o -1 se assente.
static int trova_indice_chiave(Eroe *eroe){
    if (!eroe) return -1;
    for (int i = 0; i <= eroe->inventario.top; i++) {
        if (eroe->inventario.oggetti[i]->tipo == CHIAVE)
            return i;
    }
    return -1;
}

// Rimuove l'oggetto all'indice specificato dall'inventario e lo restituisce.
// Gli oggetti successivi vengono shiftati verso il basso per non lasciare buchi.
static Oggetto *rimuovi_oggetto_inventario(Eroe *eroe, int indice){
    if (!eroe || indice < 0 || indice > eroe->inventario.top) return NULL;

    Oggetto *ogg = eroe->inventario.oggetti[indice];
    for (int i = indice; i < eroe->inventario.top; i++)
        eroe->inventario.oggetti[i] = eroe->inventario.oggetti[i + 1];
    eroe->inventario.top--;

    return ogg;
}

// ─── ESECUZIONE COMANDI ───────────────────────────────────────────────────────

static void esegui_comando(TipoComando cmd, const char *argomento,Eroe *eroe, bool *partita_vinta){
    switch (cmd) {

    // ── MOVIMENTO ──────────────────────────────────────────────────────────────
    case CMD_VAI: {
        // Calcola la nuova posizione in base alla direzione
        int dr = 0, dc = 0;
        if(strcmp(argomento, "nord")  == 0) dr = -1;
        else if (strcmp(argomento, "sud")   == 0) dr = +1;
        else if (strcmp(argomento, "est")   == 0) dc = +1;
        else if (strcmp(argomento, "ovest") == 0) dc = -1;
        else { printf("Direzione non valida. Usa: nord, sud, est, ovest\n"); break; }

        int nr = eroe->pos_riga + dr;
        int nc = eroe->pos_col  + dc;

        if (e_calpestabile(nr, nc)) {
            // Movimento normale
            eroe->pos_riga = nr;
            eroe->pos_col  = nc;

            // Se l'eroe è su una porta aperta che teletrasporta, cambia stanza
            if (entra_porta(eroe)) break;

            // Controlla oggetti e mostri nella nuova posizione
            controlla_oggetto(eroe);
            controlla_incontro(eroe, partita_vinta);

        } else if (porta_chiusa_in(nr, nc)) {
            // La cella successiva ha una porta bloccata: serve la chiave
            int idx = trova_indice_chiave(eroe);

            if (idx >= 0) {
                // L'eroe ha una chiave: chiede conferma
                char risposta[8];
                printf("La porta e' bloccata. Vuoi usare la chiave per aprirla? (s/n): ");
                fflush(stdout);

                if (fgets(risposta, sizeof(risposta), stdin) &&
                    tolower((unsigned char)risposta[0]) == 's') {

                    // Consuma la chiave e apre la porta
                    Oggetto *chiave = rimuovi_oggetto_inventario(eroe, idx);
                    if (chiave) free(chiave);
                    apri_porta_in(nr, nc);
                    printf("Hai sbloccato la porta con la chiave.\n");

                    // Muovi l'eroe dentro la porta
                    eroe->pos_riga = nr;
                    eroe->pos_col  = nc;

                    // Controlla se la porta porta a un'altra stanza
                    if (entra_porta(eroe)) break;

                    controlla_oggetto(eroe);
                    controlla_incontro(eroe, partita_vinta);
                } else {
                    printf("La porta resta bloccata.\n");
                }
            } else {
                printf("La porta e' bloccata. Ti serve una chiave!\n");
            }
        } else {
            printf("Muro! Non puoi andare in quella direzione.\n");
        }
        break;
    }

    // ── USO OGGETTI ────────────────────────────────────────────────────────────
    case CMD_USA:
        usaOggetto(eroe);
        break;

    // ── INVENTARIO ─────────────────────────────────────────────────────────────
    case CMD_INVENTARIO:
        mostraInventario(eroe);
        break;

    // ── SALVATAGGIO ────────────────────────────────────────────────────────────
    case CMD_SALVA: {
        // Usa il nome file dall'argomento, o "partita.sav" come default
        const char *file;
        if (argomento[0] != '\0') file = argomento; else file = "partita.sav";
        if (salva_partita(eroe, file) != 0)
            printf("Errore durante il salvataggio.\n");
        break;
    }

    // ── CARICAMENTO ────────────────────────────────────────────────────────────
    case CMD_CARICA: {
        const char *file;
        if (argomento[0] != '\0') file = argomento; else file = "partita.sav";
        if (carica_partita(eroe, file) != 0)
            printf("Errore durante il caricamento.\n");
        break;
    }

    // ── COMANDO SCONOSCIUTO ────────────────────────────────────────────────────
    default:
        printf("Comando non riconosciuto.\n");
        printf("Comandi validi: W/A/S/D, vai [dir], usa, inventario, salva, carica\n");
        break;
    }
}