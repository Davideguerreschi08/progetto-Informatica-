#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/mappa.h"

#define MAPPA_RIGHE_T   26
#define MAPPA_COLONNE_T 62
#define MAX_ELEMENTI_MAPPA 32

static unsigned char porta_aperta[MAX_ELEMENTI_MAPPA];
static Oggetto *oggetti_mappa[MAX_ELEMENTI_MAPPA];
static Mostro *mostri_mappa[MAX_ELEMENTI_MAPPA];

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

typedef struct {
    int  r, c;
    char simbolo;
    int  tipo; /* 0 = oggetto, 1 = mostro, 2 = porta */
} ElementoMappa;

#define EL_OGGETTO 0
#define EL_MOSTRO  1
#define EL_PORTA   2

static const ElementoMappa ELEMENTI[] = {
    { 5, 50, 'P', EL_OGGETTO },
    { 6, 17, 'M', EL_MOSTRO  },
    {10, 26, 'K', EL_OGGETTO },
    { 2, 11, 'F', EL_OGGETTO },
    {10, 24, 'M', EL_MOSTRO  },
    { 2, 27, 'M', EL_MOSTRO  },
    {10,  3, 'B', EL_OGGETTO },
    {14, 17, 'M', EL_MOSTRO  },
    {14, 54, 'M', EL_MOSTRO  },
    {12, 58, 'M', EL_MOSTRO  },
    {12, 61, '[', EL_PORTA   },
    {-1, -1, '\0', -1 }
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

static int indice_elemento(int r, int c)
{
    for (int i = 0; ELEMENTI[i].r != -1; i++) {
        if (ELEMENTI[i].r == r && ELEMENTI[i].c == c)
            return i;
    }
    return -1;
}

static char simbolo_oggetto(const Oggetto *o)
{
    if (!o) return '\0';
    switch (o->tipo) {
        case POZIONE:        return 'P';
        case POZIONE_VELENO: return 'V';
        case CHIAVE:         return 'K';
        case BOMBA:          return 'B';
        case AMULETO_FORZA:  return 'F';
        case AMULETO_DIFESA: return 'D';
        default:             return 'o';
    }
}

static void inizializza_elementi_mappa(void)
{
    memset(porta_aperta, 0, sizeof porta_aperta);
    memset(oggetti_mappa, 0, sizeof oggetti_mappa);
    memset(mostri_mappa,  0, sizeof mostri_mappa);

    oggetti_mappa[0] = nuovo_oggetto("Pozione di cura",      POZIONE,        20);
    oggetti_mappa[2] = nuovo_oggetto("Chiave arrugginita",   CHIAVE,          0);
    oggetti_mappa[3] = nuovo_oggetto("Amuleto della forza", AMULETO_FORZA, 1);
    oggetti_mappa[6] = nuovo_oggetto("Bomba",                BOMBA,          30);

    mostri_mappa[1] = nuovo_mostro("Goblin",        GOBLIN,    18,  6, 2,  20,  10);
    mostri_mappa[4] = nuovo_mostro("Scheletro",     SCHELETRO, 25,  8, 3,  30,  15);
    mostri_mappa[5] = nuovo_mostro("Mago nero",     MAGO,      32, 10, 4,  40,  20);
    mostri_mappa[7] = nuovo_mostro("Demone minore", DEMONE,    40, 12, 5,  50,  25);
    mostri_mappa[8] = nuovo_mostro("Drago nero occhi rossi", BOSS, 80, 14, 6, 150, 100);
}

void costruisci_mappa(void)
{
    inizializza_elementi_mappa();
}

static void libera_oggetto(Oggetto *o)
{
    while (o) {
        Oggetto *next = o->next;
        free(o);
        o = next;
    }
}

void distruggi_mappa(void)
{
    for (int i = 0; i < MAX_ELEMENTI_MAPPA; i++) {
        if (oggetti_mappa[i]) {
            libera_oggetto(oggetti_mappa[i]);
            oggetti_mappa[i] = NULL;
        }
        if (mostri_mappa[i]) {
            free(mostri_mappa[i]);
            mostri_mappa[i] = NULL;
        }
    }
}

int salva_stato_mappa(FILE *file)
{
    if (!file) return -1;
    for (int i = 0; ELEMENTI[i].r != -1; i++) {
        unsigned char porta = porta_aperta[i] ? 1 : 0;
        fwrite(&porta, sizeof(porta), 1, file);

        if (ELEMENTI[i].tipo == EL_MOSTRO) {
            unsigned char vivo = (mostri_mappa[i] && mostri_mappa[i]->vivo) ? 1 : 0;
            fwrite(&vivo, sizeof(vivo), 1, file);
        }

        if (ELEMENTI[i].tipo == EL_OGGETTO) {
            unsigned char has = oggetti_mappa[i] ? 1 : 0;
            fwrite(&has, sizeof(has), 1, file);
            if (has) {
                Oggetto *o = oggetti_mappa[i];
                fwrite(o->nome, sizeof(o->nome), 1, file);
                int tipo = (int)o->tipo;
                fwrite(&tipo, sizeof(tipo), 1, file);
                fwrite(&o->valore, sizeof(o->valore), 1, file);
            }
        }
    }
    return 0;
}

int carica_stato_mappa(FILE *file)
{
    if (!file) return -1;

    for (int i = 0; ELEMENTI[i].r != -1; i++) {
        unsigned char porta;
        if (fread(&porta, sizeof(porta), 1, file) != 1) return -1;
        porta_aperta[i] = porta ? 1 : 0;

        if (ELEMENTI[i].tipo == EL_MOSTRO) {
            unsigned char vivo;
            if (fread(&vivo, sizeof(vivo), 1, file) != 1) return -1;
            if (mostri_mappa[i]) {
                mostri_mappa[i]->vivo = vivo ? 1 : 0;
                if (!mostri_mappa[i]->vivo) mostri_mappa[i]->hp = 0;
            }
        }

        if (ELEMENTI[i].tipo == EL_OGGETTO) {
            unsigned char has;
            if (fread(&has, sizeof(has), 1, file) != 1) return -1;
            if (!has) {
                libera_oggetto(oggetti_mappa[i]);
                oggetti_mappa[i] = NULL;
            } else {
                char nome[MAX_NOME];
                int tipo;
                int valore;
                if (fread(&nome, sizeof(nome), 1, file) != 1) return -1;
                if (fread(&tipo, sizeof(tipo), 1, file) != 1) return -1;
                if (fread(&valore, sizeof(valore), 1, file) != 1) return -1;
                libera_oggetto(oggetti_mappa[i]);
                oggetti_mappa[i] = nuovo_oggetto(nome, (TipoOggetto)tipo, valore);
            }
        }
    }
    return 0;
}

static int indice_porta_a(int r, int c)
{
    for (int i = 0; ELEMENTI[i].r != -1; i++) {
        if (ELEMENTI[i].tipo != EL_PORTA) continue;
        if (ELEMENTI[i].r == r && ELEMENTI[i].c == c) return i;
    }
    return -1;
}

int e_calpestabile(int r, int c)
{
    if (r < 0 || r >= MAPPA_RIGHE_T) return 0;
    if (c < 0 || c >= MAPPA_COLONNE_T) return 0;
    int i = indice_porta_a(r, c);
    if (i >= 0) return porta_aperta[i] ? 1 : 0;
    return MAPPA_TERMINALE[r][c] == ' ';
}

int porta_chiusa_in(int r, int c)
{
    int i = indice_porta_a(r, c);
    return (i >= 0 && !porta_aperta[i]);
}

void apri_porta_in(int r, int c)
{
    int i = indice_porta_a(r, c);
    if (i >= 0) porta_aperta[i] = 1;
}

Mostro *mostro_in_posizione(int r, int c)
{
    int i = indice_elemento(r, c);
    if (i < 0 || ELEMENTI[i].tipo != EL_MOSTRO) return NULL;
    Mostro *m = mostri_mappa[i];
    return (m && m->vivo) ? m : NULL;
}

Oggetto *oggetto_in_posizione(int r, int c)
{
    int i = indice_elemento(r, c);
    if (i < 0) return NULL;
    return oggetti_mappa[i];
}

Oggetto *rimuovi_oggetto_da_posizione(int r, int c)
{
    int i = indice_elemento(r, c);
    if (i < 0) return NULL;
    Oggetto *o = oggetti_mappa[i];
    oggetti_mappa[i] = NULL;
    return o;
}

void aggiungi_oggetto_in_posizione(int r, int c, Oggetto *oggetto)
{
    if (!oggetto) return;
    int i = indice_elemento(r, c);
    if (i < 0) {
        free(oggetto);
        return;
    }
    libera_oggetto(oggetti_mappa[i]);
    oggetti_mappa[i] = oggetto;
}

static void stampa_comandi(void)
{
    printf("\n");
    printf("╔════════════════════════════════════════════════════════╗\n");
    printf("║                      COMANDI DISPONIBILI               ║\n");
    printf("╠════════════════════════════════════════════════════════╣\n");
    printf("║ Movimento: W=Nord  S=Sud  A=Ovest  D=Est               ║\n");
    printf("║ Comandi:   usa, inventario, salva, carica              ║\n");
    printf("╚════════════════════════════════════════════════════════╝\n");
}

void stampa_mappa(Eroe *eroe)
{
    stampa_comandi();

    char buf[MAPPA_RIGHE_T][MAPPA_COLONNE_T + 1];
    for (int r = 0; r < MAPPA_RIGHE_T; r++)
        strcpy(buf[r], MAPPA_TERMINALE[r]);

    for (int i = 0; ELEMENTI[i].r != -1; i++) {
        int r = ELEMENTI[i].r;
        int c = ELEMENTI[i].c;

        if (ELEMENTI[i].tipo == EL_PORTA) {
            if (!porta_aperta[i]) buf[r][c] = ELEMENTI[i].simbolo;
            continue;
        }

        if (ELEMENTI[i].tipo == EL_MOSTRO) {
            Mostro *m = mostri_mappa[i];
            if (m && m->vivo) {
                buf[r][c] = 'M';
                continue;
            }
        }

        char s = simbolo_oggetto(oggetti_mappa[i]);
        if (s != '\0') buf[r][c] = s;
    }

    if (eroe) {
        int r = eroe->pos_riga;
        int c = eroe->pos_col;
        if (r >= 0 && r < MAPPA_RIGHE_T && c >= 0 && c < MAPPA_COLONNE_T)
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
                case 6:  printf("  | Difesa:  %-5d  |" , eroe->difesa); break;
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
                case 17: printf("  F = Amuleto forza"); break;
                case 18: printf("  D = Amuleto difesa"); break;
                case 19: printf("  [ = Porta"); break;
                default: break;
            }
        }
        putchar('\n');
    }
    printf("\n");
}
