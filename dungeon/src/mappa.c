#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/salvataggio.h"
#include "../include/mappa.h"

Stanza *tutte_stanze[MAX_STANZE];
int num_stanze = 0;

static Stanza *nuova_stanza(int id, const char *nome, const char *descrizione)
{
    Stanza *s = malloc(sizeof(Stanza));
    if (!s) return NULL;

    s->id = id;
    strncpy(s->nome, nome, MAX_NOME);
    s->nome[MAX_NOME - 1] = '\0';
    strncpy(s->descrizione, descrizione, MAX_DESCRIZIONE);
    s->descrizione[MAX_DESCRIZIONE - 1] = '\0';

    s->visitata = false;
    s->bloccata = false;
    s->nascosta = false;
    s->bloccata_est = -1;
    s->oggetti = NULL;
    s->mostro = NULL;
    s->nord = NULL;
    s->sud = NULL;
    s->est = NULL;
    s->ovest = NULL;

    return s;
}

static void collega_stanze(Stanza *da, Stanza *a, const char *direzione)
{
    if (!da || !a || !direzione) return;

    if (strcmp(direzione, "nord") == 0) {
        da->nord = a;
        a->sud = da;
    } else if (strcmp(direzione, "sud") == 0) {
        da->sud = a;
        a->nord = da;
    } else if (strcmp(direzione, "est") == 0) {
        da->est = a;
        a->ovest = da;
    } else if (strcmp(direzione, "ovest") == 0) {
        da->ovest = a;
        a->est = da;
    }
}

Stanza *costruisci_mappa(void)
{
    num_stanze = 0;

    Stanza *s0 = nuova_stanza(0, "Sala Iniziale", "Qui inizia la tua avventura nel dungeon.");
    Stanza *s1 = nuova_stanza(1, "Corridoio Est", "Un corridoio stretto e silenzioso.");
    Stanza *s2 = nuova_stanza(2, "Sala dei Tesori", "Una stanza con alcuni oggetti abbandonati.");
    Stanza *s3 = nuova_stanza(3, "Sala del Mostro", "Senti un respiro inquietante qui vicino.");
    Stanza *s4 = nuova_stanza(4, "Antica Biblioteca", "Vecchi scaffali pieni di libri polverosi.");
    Stanza *s5 = nuova_stanza(5, "Pozzo della Trappola", "Attento: il pavimento sembra instabile.");
    Stanza *s6 = nuova_stanza(6, "Sala delle Pozioni", "Boccette colorate luccicano sul tavolo.");
    Stanza *s7 = nuova_stanza(7, "Sala del Boss", "Una porta massiccia conduce al boss finale.");

    if (!s0 || !s1 || !s2 || !s3 || !s4 || !s5 || !s6 || !s7) {
        distruggi_mappa(tutte_stanze, num_stanze);
        return NULL;
    }

    tutte_stanze[num_stanze++] = s0;
    tutte_stanze[num_stanze++] = s1;
    tutte_stanze[num_stanze++] = s2;
    tutte_stanze[num_stanze++] = s3;
    tutte_stanze[num_stanze++] = s4;
    tutte_stanze[num_stanze++] = s5;
    tutte_stanze[num_stanze++] = s6;
    tutte_stanze[num_stanze++] = s7;

    collega_stanze(s0, s1, "est");
    collega_stanze(s1, s2, "sud");
    collega_stanze(s2, s3, "est");
    collega_stanze(s1, s4, "nord");
    collega_stanze(s4, s5, "est");
    collega_stanze(s5, s6, "sud");
    collega_stanze(s6, s7, "est");

    return s0;
}

void distruggi_mappa(Stanza **stanze, int n)
{
    if (!stanze || n <= 0) return;

    for (int i = 0; i < n && i < MAX_STANZE; i++) {
        free(stanze[i]);
        stanze[i] = NULL;
    }

    num_stanze = 0;
}

void stampa_mappa(Stanza **stanze, int n, Stanza *corrente)
{
    (void)stanze;
    (void)n;
    (void)corrente;

    enum { MAPPA_RIGHE = 24, MAPPA_COLONNE = 62 };

    static const char MAPPA[MAPPA_RIGHE][MAPPA_COLONNE + 1] = {
        "I—-----------------------------------------------------------I",
        "I#####################################                       I",
        "IS/&       O               M         #                       I",
        "I###############   ###############   #                       I",
        "I              #   #             #   ####################    I",
        "I              #   #             #        T     P       #    I",
        "I              # M #             ####################   #    I",
        "I              #   #                                #   #    I",
        "I              #   #                                #   #    I",
        "I###############   ##########                       #   #    I",
        "I# B  T     T          M  C #                       # M #    I",
        "I###############   ##########                       #   #####I",
        "I              #   #                                #        I",
        "I              # M #                                #   #####I",
        "I              # P #             ##########         # M #    I",
        "I              #   #             # M M   B#         #   #    I",
        "I              #   #             #  #######         #   #    I",
        "I              #   #             #   #              #   #    I",
        "I   ############   #             #   #              #   #    I",
        "I   #              #             #   #              #   #    I",
        "I   #   ##########################   ################   #    I",
        "I   #     T            M                T          P    #    I",
        "I   #####################################################    I",
        "I------------------------------------------------------------I"
    };

    printf("\n=== MAPPA DEL DUNGEON ===\n");
    for (int r = 0; r < MAPPA_RIGHE; r++) {
        for (int c = 0; c < MAPPA_COLONNE; c++) {
            putchar(MAPPA[r][c]);
        }
        putchar('\n');
    }

    puts("\nLegenda:");
    puts("  # = Muro");
    puts("  S = Spawn/Inizio");
    puts("  & = Giocatore");
    puts("  M = Mostro");
    puts("  O = Oggetto");
    puts("  B = Baule");
    puts("  T = Trappola");
    puts("  P = Pozione");
    puts("  C = Chiave");
    puts("");
}
