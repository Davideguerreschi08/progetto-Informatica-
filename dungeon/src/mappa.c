#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/mappa.h"

#define MAPPA_RIGHE_T   26
#define MAPPA_COLONNE_T 62
#define MAX_ELEMENTI_MAPPA 32

static unsigned char porta_aperta[MAX_ELEMENTI_MAPPA];

Stanza *tutte_stanze[MAX_STANZE];
int num_stanze = 0;

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
    "I              #   #                                #         ",
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
    s->visitata     = false;
    s->bloccata     = false;
    s->nascosta     = false;
    s->bloccata_est = -1;
    s->oggetti      = NULL;
    s->mostro       = NULL;
    s->nord = s->sud = s->est = s->ovest = NULL;
    return s;
}

static void aggiungi_oggetto(Stanza *stanza, Oggetto *oggetto)
{
    if (!stanza || !oggetto) return;
    oggetto->next   = stanza->oggetti;
    stanza->oggetti = oggetto;
}

static void collega_stanze(Stanza *da, Stanza *a, const char *dir)
{
    if (!da || !a || !dir) return;
    if      (strcmp(dir, "nord")  == 0) { da->nord  = a; a->sud   = da; }
    else if (strcmp(dir, "sud")   == 0) { da->sud   = a; a->nord  = da; }
    else if (strcmp(dir, "est")   == 0) { da->est   = a; a->ovest = da; }
    else if (strcmp(dir, "ovest") == 0) { da->ovest = a; a->est   = da; }
}

Stanza *costruisci_mappa(void)
{
    memset(porta_aperta, 0, sizeof porta_aperta);
    num_stanze = 0;
    Stanza *s0 = nuova_stanza(0, "Sala Iniziale",        "Un ingresso illuminato da torce consumate.");
    Stanza *s1 = nuova_stanza(1, "Corridoio Est",        "Un corridoio stretto e umido.");
    Stanza *s2 = nuova_stanza(2, "Sala dei Tesori",      "Borse vuote e scaffali rotti.");
    Stanza *s3 = nuova_stanza(3, "Sala del Mostro",      "Un'ombra si muove nell'oscurita'.");
    Stanza *s4 = nuova_stanza(4, "Antica Biblioteca",    "Mucchi di libri e pergamene polverose.");
    Stanza *s5 = nuova_stanza(5, "Pozzo della Trappola", "Il pavimento nasconde un meccanismo.");
    Stanza *s6 = nuova_stanza(6, "Sala delle Pozioni",   "Flaconi colorati e vapori strani.");
    Stanza *s7 = nuova_stanza(7, "Sala del Boss",        "L'aria e' carica di minaccia.");

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

    aggiungi_oggetto(s0, nuovo_oggetto("Pozione di cura",      POZIONE,        20));
    aggiungi_oggetto(s1, nuovo_oggetto("Chiave arrugginita",   CHIAVE,          0));
    aggiungi_oggetto(s2, nuovo_oggetto("Amuleto della forza", AMULETO_FORZA, 1));
    aggiungi_oggetto(s4, nuovo_oggetto("Bomba",                BOMBA,          30));
    aggiungi_oggetto(s6, nuovo_oggetto("Pozione velenosa",     POZIONE_VELENO, 15));

    s1->mostro = nuovo_mostro("Goblin",        GOBLIN,    18,  6, 2,  20,  10);
    s3->mostro = nuovo_mostro("Scheletro",     SCHELETRO, 25,  8, 3,  30,  15);
    s4->mostro = nuovo_mostro("Mago nero",   MAGO,      32, 10, 4,  40,  20);
    s5->mostro = nuovo_mostro("Demone minore", DEMONE,    40, 12, 5,  50,  25);
    s7->mostro = nuovo_mostro("Drago nero ochhi rossi",    BOSS,      80, 14, 6, 150, 100);

    return s0;
}

static void libera_oggetti(Oggetto *o)
{
    while (o) { Oggetto *n = o->next; free(o); o = n; }
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

static int indice_porta_a(int r, int c);

int e_calpestabile(int r, int c)
{
    if (r < 0 || r >= MAPPA_RIGHE_T) return 0;
    if (c < 0 || c >= MAPPA_COLONNE_T) return 0;
    int i = indice_porta_a(r, c);
    if (i >= 0) return porta_aperta[i] ? 1 : 0;
    return MAPPA_TERMINALE[r][c] == ' ';
}

static const int ZONE[][5] = {
    {0,  1,  2,  1, 35},
    {4,  1,  8, 38, 60},
    {1,  3,  9, 16, 18},
    {5,  4,  8,  1, 14},
    {2, 10, 11,  2, 27},
    {3,  7, 13, 20, 51},
    {6, 13, 21, 53, 60},
    {7, 21, 23,  4, 55},
    {-1, 0,  0,  0,  0}
};

int stanza_id_per_posizione(int r, int c)
{
    for (int i = 0; ZONE[i][0] != -1; i++) {
        if (r >= ZONE[i][1] && r <= ZONE[i][2] &&
            c >= ZONE[i][3] && c <= ZONE[i][4])
            return ZONE[i][0];
    }
    return -1;
}

typedef struct {
    int  r, c;
    char simbolo;
    int  stanza_id;
    int  tipo; /* 0 = oggetto (simbolo dalla lista stanza), 1 = mostro, 2 = porta */
} ElementoMappa;

#define EL_OGGETTO 0
#define EL_MOSTRO  1
#define EL_PORTA   2

static const ElementoMappa ELEMENTI[] = {
    { 5, 50, 'P',  0, EL_OGGETTO },
    { 6, 17, 'M',  1, EL_MOSTRO },
    {10, 26, 'K',  1, EL_OGGETTO },
    {2,  11, 'O',  2, EL_OGGETTO },
    {10, 24, 'M',  3, EL_MOSTRO },
    { 2, 27, 'M',  4, EL_MOSTRO },
    { 5, 42, 'T',  4, EL_OGGETTO },
    { 14, 17, 'M',  5, EL_MOSTRO },
    {14, 54, 'M',  6, EL_MOSTRO },
    {23, 25, 'M',  7, EL_MOSTRO },
    {12, 61, '[',  0, EL_PORTA },
    {-1, -1, '\0', -1, -1}
};

static int indice_porta_a(int r, int c)
{
    for (int i = 0; ELEMENTI[i].r != -1; i++) {
        if (ELEMENTI[i].tipo != EL_PORTA) continue;
        if (ELEMENTI[i].r == r && ELEMENTI[i].c == c) return i;
    }
    return -1;
}

int porta_chiusa_in(int r, int c)
{
    int i = indice_porta_a(r, c);
    if (i < 0 || i >= MAX_ELEMENTI_MAPPA) return 0;
    return porta_aperta[i] ? 0 : 1;
}

void apri_porta_in(int r, int c)
{
    int i = indice_porta_a(r, c);
    if (i >= 0 && i < MAX_ELEMENTI_MAPPA) porta_aperta[i] = 1;
}

static char simbolo_oggetto_stanza(int stanza_id)
{
    if (stanza_id < 0 || stanza_id >= num_stanze) return '\0';
    Oggetto *o = tutte_stanze[stanza_id]->oggetti;
    if (!o) return '\0';
    switch (o->tipo) {
        case POZIONE:        return 'P';
        case POZIONE_VELENO: return 'V';
        case CHIAVE:         return 'K';
        case BOMBA:          return 'B';
        case ARMA:           return 'A';
        case ARMATURA:       return 'R';
        case AMULETO_FORZA:  return 'F';
        case AMULETO_DIFESA: return 'D';
        default:             return 'o';
    }
}

int mostro_in_posizione(int r, int c, Stanza **out)
{
    for (int i = 0; ELEMENTI[i].r != -1; i++) {
        if (ELEMENTI[i].tipo != 1) continue;
        if (ELEMENTI[i].r != r || ELEMENTI[i].c != c) continue;
        int sid = ELEMENTI[i].stanza_id;
        if (sid < 0 || sid >= num_stanze) continue;
        Stanza *s = tutte_stanze[sid];
        if (!s || !s->mostro || !s->mostro->vivo) continue;
        if (out) *out = s;
        return 1;
    }
    return 0;
}

int oggetto_in_posizione(int r, int c, Stanza **out)
{
    for (int i = 0; ELEMENTI[i].r != -1; i++) {
        if (ELEMENTI[i].tipo != 0) continue;
        if (ELEMENTI[i].r != r || ELEMENTI[i].c != c) continue;
        int sid = ELEMENTI[i].stanza_id;
        if (sid < 0 || sid >= num_stanze) continue;
        Stanza *s = tutte_stanze[sid];
        if (!s || !s->oggetti) continue;
        if (out) *out = s;
        return 1;
    }
    return 0;
}

static void stampa_comandi(void)
{
    printf("\n");
    printf("╔════════════════════════════════════════════════════════╗\n");
    printf("║                      COMANDI DISPONIBILI                ║\n");
    printf("╠════════════════════════════════════════════════════════╣\n");
    printf("║ Movimento: W=Nord  S=Sud  A=Ovest  D=Est               ║\n");
    printf("║ Comandi:   guarda, usa, inventario, salva, carica      ║\n");
    printf("╚════════════════════════════════════════════════════════╝\n");
}

void stampa_mappa(Stanza **stanze, int n, Stanza *corrente, Eroe *eroe)
{
    stampa_comandi();
    (void)corrente;

    char buf[MAPPA_RIGHE_T][MAPPA_COLONNE_T + 1];
    for (int r = 0; r < MAPPA_RIGHE_T; r++)
        strcpy(buf[r], MAPPA_TERMINALE[r]);

    for (int i = 0; ELEMENTI[i].r != -1; i++) {
        int  r   = ELEMENTI[i].r;
        int  c   = ELEMENTI[i].c;
        int  sid = ELEMENTI[i].stanza_id;
        int  t   = ELEMENTI[i].tipo;

        if (t == EL_PORTA) {
            if (i < MAX_ELEMENTI_MAPPA && !porta_aperta[i])
                buf[r][c] = ELEMENTI[i].simbolo;
            continue;
        }

        if (sid < 0 || sid >= n || !stanze[sid]) continue;

        if (t == EL_MOSTRO) {
            if (stanze[sid]->mostro && stanze[sid]->mostro->vivo)
                buf[r][c] = 'M';
        } else if (t == EL_OGGETTO) {
            char s = simbolo_oggetto_stanza(sid);
            if (s != '\0')
                buf[r][c] = s;
        }
    }

    if (eroe) {
        int r = eroe->pos_riga;
        int c = eroe->pos_col;
        if (r > 0 && r < MAPPA_RIGHE_T && c > 0 && c < MAPPA_COLONNE_T)
            buf[r][c] = '&';
    }

    printf("\n");
    printf("%-62s  STATO EROE\n", "=== MAPPA DEL DUNGEON ===");
    printf("%-62s  +------------------+\n", "");

    for (int r = 0; r < MAPPA_RIGHE_T; r++) {
        printf("%s", buf[r]);
        if (eroe) {
            switch (r) {
                case 1:  printf("  | Nome: %-12s |", eroe->nome); break;
                case 2:  printf("  | HP:  %3d/%3d    |", eroe->hp, eroe->hp_max); break;
                case 3:  printf("  | Livello: %-5d  |", eroe->livello); break;
                case 4:  printf("  | XP:  %-8d  |", eroe->xp); break;
                case 5:  printf("  | Att. agg.: %-5d |" , eroe->bonus_danno); break;
                case 6:  printf("  | Difesa:  %-5d  |" , eroe->hp_max); break;
                case 7:  printf("  | Oro: %-8d  |", eroe->oro); break;
                case 8:  printf("  | Inv: %2d/%2d item |",
                                eroe->inventario.top + 1, MAX_INVENTARIO); break;
                case 9:  printf("  +------------------+"); break;
                case 10: printf("  LEGENDA:"); break;
                case 11: printf("  & = Tu"); break;
                case 12: printf("  M = Mostro"); break;
                case 13: printf("  P = Pozione"); break;
                case 14: printf("  V = Veleno"); break;
                case 15: printf("  K = Chiave"); break;
                case 16: printf("  B = Bomba"); break;
                case 17: printf("  A = Arma"); break;
                case 18: printf("  R = Armatura"); break;
                case 19: printf("  F = Amuleto forza"); break;
                case 20: printf("  D = Amuleto difesa"); break;
                case 21: printf("  [ = Porta"); break;
                default: break;
            }
        }
        putchar('\n');
    }
    printf("\n");
}