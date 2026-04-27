#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/tipi.h"
#include "../include/eroe.h"
#include "../include/mappa.h"
#include "../include/combattimento.h"
#include "../include/salvataggio.h"

static TipoComando parse_comando(const char *input, char *argomento);
static void esegui_comando(TipoComando cmd, const char *argomento,
                           Eroe *eroe, bool *partita_vinta);
static void prendi_oggetto(Eroe *eroe, const char *nome);
static Stanza *destinazione_da_direzione(Stanza *stanza, const char *direzione);

int main(void)
{
    Eroe *eroe = crea_eroe("Avventuriero");
    Stanza *iniziale = costruisci_mappa();
    eroe->stanza_corrente = iniziale;
    if (iniziale) iniziale->visitata = true;

    bool partita_vinta = false;
    char input[MAX_INPUT];
    char argomento[MAX_NOME];

    while (eroe->hp > 0 && !partita_vinta) {
        stampa_stato(eroe);
        printf("> ");
        if (!fgets(input, MAX_INPUT, stdin))
            break;

        TipoComando cmd = parse_comando(input, argomento);
        esegui_comando(cmd, argomento, eroe, &partita_vinta);
    }

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

    if (strcmp(comando, "vai") == 0) return CMD_VAI;
    if (strcmp(comando, "guarda") == 0) return CMD_GUARDA;
    if (strcmp(comando, "prendi") == 0) return CMD_PRENDI;
    if (strcmp(comando, "usa") == 0) return CMD_USA;
    if (strcmp(comando, "attacca") == 0) return CMD_ATTACCA;
    if (strcmp(comando, "inventario") == 0) return CMD_INVENTARIO;
    if (strcmp(comando, "salva") == 0) return CMD_SALVA;
    if (strcmp(comando, "carica") == 0) return CMD_CARICA;
    if (strcmp(comando, "mappa") == 0) return CMD_MAPPA;
    return CMD_SCONOSCIUTO;
}

static Stanza *destinazione_da_direzione(Stanza *stanza, const char *direzione)
{
    if (!stanza || direzione[0] == '\0')
        return NULL;

    if (strcmp(direzione, "nord") == 0) return stanza->nord;
    if (strcmp(direzione, "sud") == 0) return stanza->sud;
    if (strcmp(direzione, "est") == 0) return stanza->est;
    if (strcmp(direzione, "ovest") == 0) return stanza->ovest;
    return NULL;
}

static void prendi_oggetto(Eroe *eroe, const char *nome)
{
    Stanza *s = eroe->stanza_corrente;
    if (!s || !nome || nome[0] == '\0') {
        printf("Specificare un oggetto da prendere.\n");
        return;
    }

    Oggetto **ptr = &s->oggetti;
    while (*ptr) {
        if (strcmp((*ptr)->nome, nome) == 0) {
            Oggetto *oggetto = *ptr;
            *ptr = oggetto->next;
            oggetto->next = NULL;
            push(eroe, oggetto);
            return;
        }
        ptr = &(*ptr)->next;
    }

    printf("Non c'è nessun oggetto chiamato '%s' qui.\n", nome);
}

static void esegui_comando(TipoComando cmd, const char *argomento,
                           Eroe *eroe, bool *partita_vinta)
{
    switch (cmd) {
    case CMD_VAI: {
        Stanza *destinazione = destinazione_da_direzione(eroe->stanza_corrente, argomento);
        if (destinazione) {
            cambiaStanza(&eroe->stanza_corrente, destinazione);
            destinazione->visitata = true;
        } else {
            printf("Non puoi andare in quella direzione. Usa nord/sud/est/ovest.\n");
        }
        break;
    }
    case CMD_GUARDA:
        if (eroe->stanza_corrente) {
            printf("%s\n%s\n", eroe->stanza_corrente->nome,
                   eroe->stanza_corrente->descrizione);
            if (eroe->stanza_corrente->mostro && eroe->stanza_corrente->mostro->vivo)
                printf("Qui c'è un %s.\n", eroe->stanza_corrente->mostro->nome);
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
                !eroe->stanza_corrente->mostro->vivo) {
                *partita_vinta = true;
            }
        } else {
            printf("Non c'è nessun nemico da attaccare qui.\n");
        }
        break;
    case CMD_INVENTARIO:
        mostraInventario(eroe);
        break;
    case CMD_MAPPA:
        stampa_mappa(tutte_stanze, num_stanze, eroe->stanza_corrente);
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
        printf("Comando non riconosciuto. Usa: vai, guarda, prendi, usa, attacca, inventario, mappa, salva, carica.\n");
        break;
    }
}
