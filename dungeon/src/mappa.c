#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/mappa.h"

Stanza *tutte_stanze[MAX_STANZE];
int num_stanze = 0;

static Oggetto *nuovo_oggetto(const char *nome, TipoOggetto tipo, int valore)
{
    Oggetto *oggetto = malloc(sizeof(Oggetto));
    if (!oggetto) return NULL;

    strncpy(oggetto->nome, nome, MAX_NOME);
    oggetto->nome[MAX_NOME - 1] = '\0';
    oggetto->tipo = tipo;
    oggetto->valore = valore;
    oggetto->next = NULL;
    return oggetto;
}

static Mostro *nuovo_mostro(const char *nome, TipoMostro tipo, int hp, int attacco,
                            int difesa, int xp, int oro)
{
    Mostro *mostro = malloc(sizeof(Mostro));
    if (!mostro) return NULL;

    strncpy(mostro->nome, nome, MAX_NOME);
    mostro->nome[MAX_NOME - 1] = '\0';
    mostro->tipo = tipo;
    mostro->hp = hp;
    mostro->hp_max = hp;
    mostro->attacco = attacco;
    mostro->difesa = difesa;
    mostro->xp_ricompensa = xp;
    mostro->oro_ricompensa = oro;
    mostro->vivo = 1;
    return mostro;
}

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

static void aggiungi_oggetto(Stanza *stanza, Oggetto *oggetto)
{
    if (!stanza || !oggetto) return;
    oggetto->next = stanza->oggetti;
    stanza->oggetti = oggetto;
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

    Stanza *s0 = nuova_stanza(0, "Sala Iniziale", "Un ingresso illuminato da torce consumate.");
    Stanza *s1 = nuova_stanza(1, "Corridoio Est", "Un corridoio stretto e umido con pareti di pietra.");
    Stanza *s2 = nuova_stanza(2, "Sala dei Tesori", "Borse vuote e scaffali rotti punteggiano questa stanza.");
    Stanza *s3 = nuova_stanza(3, "Sala del Mostro", "Un'ombra si muove nell'oscurità.");
    Stanza *s4 = nuova_stanza(4, "Antica Biblioteca", "Munumenti di libri e pergamene polverose.");
    Stanza *s5 = nuova_stanza(5, "Pozzo della Trappola", "Attenzione: pare che il pavimento nasconda un meccanismo.");
    Stanza *s6 = nuova_stanza(6, "Sala delle Pozioni", "Flaconi colorati e vapori strani riempiono l'aria.");
    Stanza *s7 = nuova_stanza(7, "Sala del Boss", "L'aria è carica di minaccia. Qui vive il boss del dungeon.");

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

    aggiungi_oggetto(s0, nuovo_oggetto("Pozione di cura", POZIONE, 20));
    aggiungi_oggetto(s1, nuovo_oggetto("Chiave arrugginita", CHIAVE, 0));
    aggiungi_oggetto(s2, nuovo_oggetto("Amuleto del coraggio", AMULETO, 0));
    aggiungi_oggetto(s4, nuovo_oggetto("Bomba", BOMBA, 30));
    aggiungi_oggetto(s6, nuovo_oggetto("Pozione velenosa", POZIONE_VELENO, 15));

    s1->mostro = nuovo_mostro("Goblin", GOBLIN, 18, 6, 2, 20, 10);
    s3->mostro = nuovo_mostro("Scheletro", SCHELETRO, 25, 8, 3, 30, 15);
    s4->mostro = nuovo_mostro("Mago oscuro", MAGO, 32, 10, 4, 40, 20);
    s5->mostro = nuovo_mostro("Demone minore", DEMONE, 40, 12, 5, 50, 25);
    s7->mostro = nuovo_mostro("Drago nero", BOSS, 80, 14, 6, 150, 100);

    return s0;
}

static void libera_oggetti(Oggetto *oggetto)
{
    while (oggetto) {
        Oggetto *next = oggetto->next;
        free(oggetto);
        oggetto = next;
    }
}

void distruggi_mappa(Stanza **stanze, int n)
{
    if (!stanze || n <= 0) return;

    for (int i = 0; i < n && i < MAX_STANZE; i++) {
        if (stanze[i]) {
            libera_oggetti(stanze[i]->oggetti);
            free(stanze[i]->mostro);
            free(stanze[i]);
            stanze[i] = NULL;
        }
    }

    num_stanze = 0;
}

void stampa_mappa(Stanza **stanze, int n, Stanza *corrente)
{
    (void)stanze;
    (void)n;

    enum { MAPPA_RIGHE = 24, MAPPA_COLONNE = 62 };

    // Mappatura stanze → coordinate sulla mappa (riga, colonna)
    typedef struct {
        int stanza_id;
        int riga;
        int colonna;
    } PosizioneMappa;

    static const PosizioneMappa mappa_stanze[] = {
        {0, 2, 1},      // Sala Iniziale (S)
        {1, 2, 11},     // Corridoio Est (O)
        {2, 2, 17},     // Sala dei Tesori (&)
        {3, 2, 27},     // Sala del Mostro (M)
        {4, 10, 1},     // Antica Biblioteca (B)
        {5, 10, 17},    // Pozzo della Trappola (C)
        {6, 14, 17},    // Sala delle Pozioni (P)
        {7, 21, 45},    // Sala del Boss (M)
    };

    static const char MAPPA_BASE[MAPPA_RIGHE][MAPPA_COLONNE + 1] = {
        "I------------------------------------------------------------I",
        "I#####################################                       I",
        "IS/        O               M         #                       I",
        "I###############   ###############   #                       I",
        "I              #   #             #   ####################    I",
        "I              #   #             #                      #    I",
        "I              #   #             ####################   #    I",
        "I              #   #                                #   #    I",
        "I              #   #                                #   #    I",
        "I###############   ##########                       #   #    I",
        "I# B  T     T          M  C #                       #   #    I",
        "I###############   ##########                       #   #####I",
        "I              #   #                                #        I",
        "I              #   #                                #   #####I",
        "I              #   #             ##########         # M #    I",
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

    // Copia la mappa di base in una mappa modificabile
    char mappa_dinamica[MAPPA_RIGHE][MAPPA_COLONNE + 1];
    for (int r = 0; r < MAPPA_RIGHE; r++) {
        strcpy(mappa_dinamica[r], MAPPA_BASE[r]);
    }

    // Posiziona il giocatore (&) sulla mappa se la stanza è valida
    if (corrente) {
        for (int i = 0; i < (int)(sizeof(mappa_stanze) / sizeof(mappa_stanze[0])); i++) {
            if (mappa_stanze[i].stanza_id == corrente->id) {
                int r = mappa_stanze[i].riga;
                int c = mappa_stanze[i].colonna;
                if (r >= 0 && r < MAPPA_RIGHE && c >= 0 && c < MAPPA_COLONNE) {
                    mappa_dinamica[r][c] = '&';
                }
                break;
            }
        }
    }

    printf("\n=== MAPPA DEL DUNGEON ===\n");
    for (int r = 0; r < MAPPA_RIGHE; r++) {
        for (int c = 0; c < MAPPA_COLONNE; c++) {
            putchar(mappa_dinamica[r][c]);
        }
        putchar('\n');
    }

    printf("\nStanza attuale: %s (ID: %d)\n\n", corrente ? corrente->nome : "Sconosciuta", corrente ? corrente->id : -1);
    puts("Legenda:");
    puts("  # = Muro");
    puts("  S = Spawn/Inizio");
    puts("  & = Giocatore (stanza corrente)");
    puts("  M = Mostro");
    puts("  O = Oggetto");
    puts("  B = Baule");
    puts("  T = Trappola");
    puts("  P = Pozione");
    puts("  C = Chiave");
    puts("");
}
