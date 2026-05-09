#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/mappa.h"

// Dimensioni della mappa - devono corrispondere a MAPPA_TERMINALE
#define MAPPA_RIGHE_T   26
#define MAPPA_COLONNE_T 62

Stanza *tutte_stanze[MAX_STANZE];
int num_stanze = 0;

// La mappa e' a livello di file (non dentro stampa_mappa) cosi'
// e_calpestabile puo' accedervi direttamente.
// "static" significa visibile solo in questo file .c
static const char MAPPA_TERMINALE[MAPPA_RIGHE_T][MAPPA_COLONNE_T + 1] = {
    "I------------------------------------------------------------I",
    "I#####################################                       I",
    "I                                    #                       I",
    "I###############   ###############   #                       I",
    "I              #   #             #   ####################    I",
    "I              #   #             #                      #    I",
    "I              #   #             ####################   #    I",
    "I              #   #                                #   #    I",
    "I              #   #                                #   #    I",
    "I###############   ##########                       #   #    I",
    "I#                          #                       #   #    I",
    "I###############   ##########                       #   #####I",
    "I              #   #                                #        I",
    "I              #   #                                #   #####I",
    "I              #   #             ##########         #   #    I",
    "I              #   #             #        #         #   #    I",
    "I              #   #             #   ######         #   #    I",
    "I              #   #             #   #              #   #    I",
    "I              #   #             #   #              #   #    I",
    "I              #   #             #   #              #   #    I",
    "I   ############   #             #   #              #   #    I",
    "I   #              #             #   #              #   #    I",
    "I   #   ##########################   ################   #    I",
    "I   #                                                   #    I",
    "I   #####################################################    I",
    "I------------------------------------------------------------I"
};

// ─── FUNZIONI DI SUPPORTO INTERNE ─────────────────────────────────────────────

static Oggetto *nuovo_oggetto(const char *nome, TipoOggetto tipo, int valore)
{
    Oggetto *o = malloc(sizeof(Oggetto));
    if (!o) return NULL;
    strncpy(o->nome, nome, MAX_NOME);
    o->nome[MAX_NOME - 1] = '\0';
    o->tipo   = tipo;
    o->valore = valore;
    o->next   = NULL;
    return o;
}

static Mostro *nuovo_mostro(const char *nome, TipoMostro tipo,
                            int hp, int attacco, int difesa, int xp, int oro)
{
    Mostro *m = malloc(sizeof(Mostro));
    if (!m) return NULL;
    strncpy(m->nome, nome, MAX_NOME);
    m->nome[MAX_NOME - 1] = '\0';
    m->tipo           = tipo;
    m->hp             = hp;
    m->hp_max         = hp;
    m->attacco        = attacco;
    m->difesa         = difesa;
    m->xp_ricompensa  = xp;
    m->oro_ricompensa = oro;
    m->vivo           = 1;
    return m;
}

static Stanza *nuova_stanza(int id, const char *nome, const char *descrizione)
{
    Stanza *s = malloc(sizeof(Stanza));
    if (!s) return NULL;
    s->id = id;
    strncpy(s->nome,        nome,        MAX_NOME);
    strncpy(s->descrizione, descrizione, MAX_DESCRIZIONE);
    s->nome[MAX_NOME - 1]               = '\0';
    s->descrizione[MAX_DESCRIZIONE - 1] = '\0';
    s->visitata    = false;
    s->bloccata    = false;
    s->nascosta    = false;
    s->bloccata_est = -1;
    s->oggetti     = NULL;
    s->mostro      = NULL;
    s->nord = s->sud = s->est = s->ovest = NULL;
    return s;
}

static void aggiungi_oggetto(Stanza *stanza, Oggetto *oggetto)
{
    if (!stanza || !oggetto) return;
    oggetto->next    = stanza->oggetti;
    stanza->oggetti  = oggetto;
}

static void collega_stanze(Stanza *da, Stanza *a, const char *dir)
{
    if (!da || !a || !dir) return;
    if      (strcmp(dir, "nord")  == 0) { da->nord  = a; a->sud   = da; }
    else if (strcmp(dir, "sud")   == 0) { da->sud   = a; a->nord  = da; }
    else if (strcmp(dir, "est")   == 0) { da->est   = a; a->ovest = da; }
    else if (strcmp(dir, "ovest") == 0) { da->ovest = a; a->est   = da; }
}

// ─── COSTRUZIONE MAPPA ────────────────────────────────────────────────────────

Stanza *costruisci_mappa(void)
{
    num_stanze = 0;

    Stanza *s0 = nuova_stanza(0, "Sala Iniziale",
                              "Un ingresso illuminato da torce consumate.");
    Stanza *s1 = nuova_stanza(1, "Corridoio Est",
                              "Un corridoio stretto e umido con pareti di pietra.");
    Stanza *s2 = nuova_stanza(2, "Sala dei Tesori",
                              "Borse vuote e scaffali rotti punteggiano questa stanza.");
    Stanza *s3 = nuova_stanza(3, "Sala del Mostro",
                              "Un'ombra si muove nell'oscurita'.");
    Stanza *s4 = nuova_stanza(4, "Antica Biblioteca",
                              "Mucchi di libri e pergamene polverose.");
    Stanza *s5 = nuova_stanza(5, "Pozzo della Trappola",
                              "Attenzione: il pavimento nasconde un meccanismo.");
    Stanza *s6 = nuova_stanza(6, "Sala delle Pozioni",
                              "Flaconi colorati e vapori strani riempiono l'aria.");
    Stanza *s7 = nuova_stanza(7, "Sala del Boss",
                              "L'aria e' carica di minaccia. Qui vive il boss.");

    if (!s0||!s1||!s2||!s3||!s4||!s5||!s6||!s7) {
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

    aggiungi_oggetto(s0, nuovo_oggetto("Pozione di cura",    POZIONE,       20));
    aggiungi_oggetto(s1, nuovo_oggetto("Chiave arrugginita", CHIAVE,         0));
    aggiungi_oggetto(s2, nuovo_oggetto("Amuleto del coraggio", AMULETO,      0));
    aggiungi_oggetto(s4, nuovo_oggetto("Bomba",              BOMBA,         30));
    aggiungi_oggetto(s6, nuovo_oggetto("Pozione velenosa",   POZIONE_VELENO,15));

    s1->mostro = nuovo_mostro("Goblin",       GOBLIN,    18,  6, 2,  20, 10);
    s3->mostro = nuovo_mostro("Scheletro",    SCHELETRO, 25,  8, 3,  30, 15);
    s4->mostro = nuovo_mostro("Mago oscuro",  MAGO,      32, 10, 4,  40, 20);
    s5->mostro = nuovo_mostro("Demone minore",DEMONE,    40, 12, 5,  50, 25);
    s7->mostro = nuovo_mostro("Drago nero",   BOSS,      80, 14, 6, 150,100);

    return s0;
}

// ─── LIBERAZIONE MEMORIA ──────────────────────────────────────────────────────

static void libera_oggetti(Oggetto *o)
{
    while (o) {
        Oggetto *next = o->next;
        free(o);
        o = next;
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

// ─── MOVIMENTO: CONTROLLO CELLA ───────────────────────────────────────────────

// Restituisce 1 se la cella (r,c) e' uno spazio ' ' → calpestabile.
// Questa e' la logica che hai descritto: controlla il carattere nella matrice.
int e_calpestabile(int r, int c)
{
    if (r < 0 || r >= MAPPA_RIGHE_T) return 0;
    if (c < 0 || c >= MAPPA_COLONNE_T) return 0;
    return MAPPA_TERMINALE[r][c] == ' ';
}

// Tabella delle zone: ogni riga associa un rettangolo visivo a una stanza.
// Formato: {id_stanza, r_min, r_max, c_min, c_max}
// NOTA: se il giocatore esce dai bordi della zona (corridoi di transizione)
// rimane nell'ultima stanza visitata finche' non entra in una nuova zona.
// Potresti dover aggiustare questi valori guardando dove '&' si trova
// mentre cammini nella mappa.
static const int ZONE[][5] = {
    {0,  1,  2,  1, 35},   // s0: Sala Iniziale (area top-left, spawn)
    {4,  1,  8, 38, 60},   // s4: Antica Biblioteca (area top-right)
    {1,  3,  9, 16, 18},   // s1: Corridoio Est (corridoio verticale)
    {5,  4,  8,  1, 14},   // s5: Pozzo Trappola (stanza sinistra righe 4-8)
    {2, 10, 11,  2, 27},   // s2: Sala dei Tesori (sotto corridoio)
    {3,  7, 13, 20, 51},   // s3: Sala del Mostro (area centrale destra)
    {6, 13, 21, 53, 60},   // s6: Sala delle Pozioni (lato destro)
    {7, 21, 23,  4, 55},   // s7: Sala del Boss (area grande in basso)
    {-1, 0,  0,  0,  0}    // sentinel: fine array
};

// Restituisce l'id della stanza corrispondente a (r,c), o -1 se fuori da tutte.
int stanza_id_per_posizione(int r, int c)
{
    for (int i = 0; ZONE[i][0] != -1; i++) {
        if (r >= ZONE[i][1] && r <= ZONE[i][2] &&
            c >= ZONE[i][3] && c <= ZONE[i][4]) {
            return ZONE[i][0];
        }
    }
    return -1;
}

// ─── STAMPA MAPPA ─────────────────────────────────────────────────────────────

// Posizione visiva del centro di ogni stanza per mostrare 'M' (mostro vivo).
// Indice = id stanza. Aggiusta questi valori se 'M' appare fuori dalla stanza.
static const int CENTRO_STANZA[8][2] = {
    { 2,  5},  // s0: Sala Iniziale
    { 6, 17},  // s1: Corridoio Est
    {10, 14},  // s2: Sala dei Tesori
    {10, 35},  // s3: Sala del Mostro
    { 2, 48},  // s4: Antica Biblioteca
    { 6,  7},  // s5: Pozzo della Trappola
    {16, 56},  // s6: Sala delle Pozioni
    {23, 30},  // s7: Sala del Boss
};

void stampa_mappa(Stanza **stanze, int n, Stanza *corrente, Eroe *eroe)
{
    (void)corrente;  // non usata direttamente, usiamo eroe->stanza_corrente

    // Crea un buffer modificabile copiando la mappa statica.
    // Su questo buffer sovrascriviamo '&' (giocatore) e 'M' (mostri).
    char buf[MAPPA_RIGHE_T][MAPPA_COLONNE_T + 1];
    for (int r = 0; r < MAPPA_RIGHE_T; r++) {
        strcpy(buf[r], MAPPA_TERMINALE[r]);
    }

    // Sovrascrive 'M' al centro di ogni stanza che ha un mostro vivo.
    // I mostri non appaiono sulla mappa visiva finche' non ci entri
    // (sarebbe troppo facile), ma in questo prototipo li mostriamo sempre.
    for (int i = 0; i < n && i < 8; i++) {
        if (stanze[i] && stanze[i]->mostro && stanze[i]->mostro->vivo) {
            int r = CENTRO_STANZA[i][0];
            int c = CENTRO_STANZA[i][1];
            if (r > 0 && r < MAPPA_RIGHE_T && c > 0 && c < MAPPA_COLONNE_T) {
                buf[r][c] = 'M';
            }
        }
    }

    // Sovrascrive '&' sulla posizione attuale del giocatore.
    // Questo e' il simbolo che si muove quando premi WASD.
    if (eroe) {
        int r = eroe->pos_riga;
        int c = eroe->pos_col;
        if (r > 0 && r < MAPPA_RIGHE_T && c > 0 && c < MAPPA_COLONNE_T) {
            buf[r][c] = '&';
        }
    }

    // Stampa il buffer riga per riga con la sidebar di stato a destra
    printf("\n");
    printf("%-64s STATO EROE\n", "=== MAPPA DEL DUNGEON ===");
    printf("%-64s \u2551\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2557\n", "");

    for (int r = 0; r < MAPPA_RIGHE_T; r++) {
        printf("%s", buf[r]);  // stampa la riga con & e M

        if (eroe) {
            switch (r) {
                case 1:  printf(" \u2551 Nome: %-18s", eroe->nome); break;
                case 2:  printf(" \u2551 HP:  %3d/%3d      ", eroe->hp, eroe->hp_max); break;
                case 3:  printf(" \u2551 Livello: %d        ", eroe->livello); break;
                case 4:  printf(" \u2551 XP:  %3d          ", eroe->xp); break;
                case 5:  printf(" \u2551 Attacco: %2d       ", eroe->attacco); break;
                case 6:  printf(" \u2551 Difesa:  %2d       ", eroe->difesa); break;
                case 7:  printf(" \u2551 Oro: %3d           ", eroe->oro); break;
                case 8:  printf(" \u2551 Inv: %2d/%2d       ",
                                eroe->inventario.top + 1, MAX_INVENTARIO); break;
                case 9:  printf(" \u2551\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u2550\u255d"); break;
                case 10: printf(" \u2551 LEGENDA:       "); break;
                case 11: printf(" \u2551 # = Muro      "); break;
                case 12: printf(" \u2551 & = Giocatore "); break;
                case 13: printf(" \u2551 M = Mostro    "); break;
                case 14: printf(" \u2551 (W=nord       "); break;
                case 15: printf(" \u2551  S=sud        "); break;
                case 16: printf(" \u2551  A=ovest      "); break;
                case 17: printf(" \u2551  D=est)       "); break;
                default: printf(" \u2551"); break;
            }
        }
        putchar('\n');
    }
    printf("\n");
}