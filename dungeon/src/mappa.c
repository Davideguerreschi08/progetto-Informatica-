#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/mappa.h"

#define MAPPA_RIGHE_T   26
#define MAPPA_COLONNE_T 62
#define MAX_ELEMENTI_MAPPA 64



typedef struct {
    int  r, c;
    char simbolo;
    int  tipo; /* 0 = oggetto, 1 = mostro, 2 = porta */
} ElementoMappa;

#define EL_OGGETTO 0
#define EL_MOSTRO  1
#define EL_PORTA   2

static unsigned char porta_aperta[MAX_ELEMENTI_MAPPA];
static Oggetto *oggetti_mappa[MAX_ELEMENTI_MAPPA];
static Mostro *mostri_mappa[MAX_ELEMENTI_MAPPA];
static char mappa_corrente[MAPPA_RIGHE_T][MAPPA_COLONNE_T + 1];
static int stanza_corrente = 1;
static ElementoMappa elementi[MAX_ELEMENTI_MAPPA];
static int elementi_count = 0;
static int spawn_r = 2, spawn_c = 5;

// MAPPA_TERMINALE constant removed

static void riempi_mappa_vuota(void)
{
    for (int r = 0; r < MAPPA_RIGHE_T; r++) {
        for (int c = 0; c < MAPPA_COLONNE_T; c++) {
            if (r == 0 || r == MAPPA_RIGHE_T - 1)
                mappa_corrente[r][c] = (c == 0 || c == MAPPA_COLONNE_T - 1) ? 'I' : '-';
            else if (c == 0 || c == MAPPA_COLONNE_T - 1)
                mappa_corrente[r][c] = 'I';
            else
                mappa_corrente[r][c] = ' ';
        }
        mappa_corrente[r][MAPPA_COLONNE_T] = '\0';
    }
    spawn_r = 2;
    spawn_c = 5;
}

static void reset_elementi(void)
{
    elementi_count = 0;
    memset(porta_aperta, 0, sizeof porta_aperta);
    memset(oggetti_mappa, 0, sizeof oggetti_mappa);
    memset(mostri_mappa,  0, sizeof mostri_mappa);
}

static int aggiungi_elemento(int r, int c, char simbolo, int tipo,
                             Oggetto *oggetto, Mostro *mostro)
{
    if (elementi_count >= MAX_ELEMENTI_MAPPA) return -1;
    elementi[elementi_count].r       = r;
    elementi[elementi_count].c       = c;
    elementi[elementi_count].simbolo = simbolo;
    elementi[elementi_count].tipo    = tipo;
    oggetti_mappa[elementi_count]    = oggetto;
    mostri_mappa[elementi_count]     = mostro;
    porta_aperta[elementi_count]     = 0;
    return elementi_count++;
}

static Oggetto *nuovo_oggetto(const char *nome, TipoOggetto tipo, int valore);
static Mostro *nuovo_mostro(const char *nome, TipoMostro tipo,
                            int hp, int attacco, int difesa, int xp, int oro);
static char simbolo_oggetto(const Oggetto *o);
static void libera_oggetto(Oggetto *o);

static int carica_mappa_da_testo(int stanza)
{
    const char *path = NULL;
    if (stanza == 1) {
        path = "stanze/Mappa1 prototipo finale.txt";
    } else if (stanza == 2) {
        path = "stanze/Mappa2 prototipo finale(con BOSS).txt";
    } else {
        return -1;
    }

    FILE *file = fopen(path, "r");
    if (!file) return -1;

    char line[128];
    for (int r = 0; r < MAPPA_RIGHE_T; r++) {
        if (fgets(line, sizeof(line), file)) {
            int len = strcspn(line, "\r\n");
            for (int c = 0; c < MAPPA_COLONNE_T; c++) {
                if (c < len)
                    mappa_corrente[r][c] = line[c];
                else
                    mappa_corrente[r][c] = ' ';
            }
        } else {
            for (int c = 0; c < MAPPA_COLONNE_T; c++)
                mappa_corrente[r][c] = ' ';
        }
        mappa_corrente[r][MAPPA_COLONNE_T] = '\0';
    }

    fclose(file);
    return 0;
}

static void copia_mappa_default(void)
{
    /* Proviamo a caricare la mappa 1 da file; se fallisce, usiamo una mappa vuota bordata */
    if (carica_mappa_da_testo(1) == 0) {
        /* spawn impostato dal file tramite 'S' se presente */
        return;
    }
    riempi_mappa_vuota();
}

static int indice_elemento(int r, int c)
{
    for (int i = 0; i < elementi_count; i++) {
        if (elementi[i].r == r && elementi[i].c == c)
            return i;
    }
    return -1;
}

static int indice_porta_a(int r, int c)
{
    for (int i = 0; i < elementi_count; i++) {
        if (elementi[i].tipo != EL_PORTA) continue;
        if (elementi[i].r == r && elementi[i].c == c)
            return i;
    }
    return -1;
}

static void carica_elementi_mappa1(void)
{
    aggiungi_elemento(5, 50, 'P', EL_OGGETTO,
                      nuovo_oggetto("Pozione di cura", POZIONE, 20),
                      NULL);
    aggiungi_elemento(6, 17, 'M', EL_MOSTRO,
                      NULL,
                      nuovo_mostro("Goblin", GOBLIN, 18, 6, 2, 20, 10));
    aggiungi_elemento(10, 26, 'K', EL_OGGETTO,
                      nuovo_oggetto("Chiave arrugginita", CHIAVE, 0),
                      NULL);
    aggiungi_elemento(2, 11, 'F', EL_OGGETTO,
                      nuovo_oggetto("Amuleto della forza", AMULETO_FORZA, 1),
                      NULL);
    aggiungi_elemento(10, 24, 'M', EL_MOSTRO,
                      NULL,
                      nuovo_mostro("Scheletro", SCHELETRO, 25, 8, 3, 30, 15));
    aggiungi_elemento(2, 27, 'M', EL_MOSTRO,
                      NULL,
                      nuovo_mostro("Mago nero", MAGO, 32, 10, 4, 40, 20));
    aggiungi_elemento(10, 3, 'B', EL_OGGETTO,
                      nuovo_oggetto("Bomba", BOMBA, 30),
                      NULL);
    aggiungi_elemento(14, 17, 'M', EL_MOSTRO,
                      NULL,
                      nuovo_mostro("Demone minore", DEMONE, 40, 12, 5, 50, 25));
    aggiungi_elemento(14, 54, 'M', EL_MOSTRO,
                      NULL,
                      nuovo_mostro("Drago nero occhi rossi", BOSS, 80, 14, 6, 150, 100));
    aggiungi_elemento(12, 58, 'M', EL_MOSTRO,
                      NULL,
                      nuovo_mostro("Drago scheletro", DRAGO_SCHELETRO, 50, 11, 5, 100, 80));
    aggiungi_elemento(12, 61, '[', EL_PORTA, NULL, NULL);
    aggiungi_elemento(12, 60, 'P', EL_OGGETTO,
                      nuovo_oggetto("Pozione di cura", POZIONE, 20),
                      NULL);
}

static void carica_elementi_mappa2(void)
{
    if (carica_mappa_da_testo(2) != 0) {
        copia_mappa_default();
        return;
    }

    spawn_r = -1;
    spawn_c = -1;
    int scheletro_speciale = 0;

    for (int r = 0; r < MAPPA_RIGHE_T; r++) {
        for (int c = 0; c < MAPPA_COLONNE_T; c++) {
            char ch = mappa_corrente[r][c];
            if (ch == 'M') {
                if (!scheletro_speciale) {
                    aggiungi_elemento(r, c, 'M', EL_MOSTRO,
                                      NULL,
                                      nuovo_mostro("Drago scheletro", DRAGO_SCHELETRO, 60, 12, 6, 120, 80));
                    scheletro_speciale = 1;
                } else {
                    aggiungi_elemento(r, c, 'M', EL_MOSTRO,
                                      NULL,
                                      nuovo_mostro("Scheletro errante", SCHELETRO, 22, 7, 3, 25, 12));
                }
                mappa_corrente[r][c] = ' ';
            } else if (ch == '@') {
                aggiungi_elemento(r, c, 'M', EL_MOSTRO,
                                  NULL,
                                  nuovo_mostro("Cavaliere dimenticato", BOSS, 160, 16, 10, 300, 180));
                mappa_corrente[r][c] = ' ';
            } else if (ch == 'P') {
                aggiungi_elemento(r, c, 'P', EL_OGGETTO,
                                  nuovo_oggetto("Pozione", POZIONE, 20),
                                  NULL);
                mappa_corrente[r][c] = ' ';
            } else if (ch == 'B') {
                aggiungi_elemento(r, c, 'B', EL_OGGETTO,
                                  nuovo_oggetto("Bomba", BOMBA, 30),
                                  NULL);
                mappa_corrente[r][c] = ' ';
            } else if (ch == 'V') {
                aggiungi_elemento(r, c, 'V', EL_OGGETTO,
                                  nuovo_oggetto("Pozione veleno", POZIONE_VELENO, 10),
                                  NULL);
                mappa_corrente[r][c] = ' ';
            } else if (ch == 'K' || ch == 'C') {
                aggiungi_elemento(r, c, 'K', EL_OGGETTO,
                                  nuovo_oggetto("Chiave", CHIAVE, 0),
                                  NULL);
                mappa_corrente[r][c] = ' ';
            } else if (ch == 'F') {
                aggiungi_elemento(r, c, 'F', EL_OGGETTO,
                                  nuovo_oggetto("Amuleto forza", AMULETO_FORZA, 1),
                                  NULL);
                mappa_corrente[r][c] = ' ';
            } else if (ch == 'D') {
                aggiungi_elemento(r, c, 'D', EL_OGGETTO,
                                  nuovo_oggetto("Amuleto difesa", AMULETO_DIFESA, 1),
                                  NULL);
                mappa_corrente[r][c] = ' ';
            } else if (ch == '[') {
                aggiungi_elemento(r, c, '[', EL_PORTA, NULL, NULL);
                mappa_corrente[r][c] = ' ';
            } else if (ch == 'S') {
                spawn_r = r;
                spawn_c = c;
                mappa_corrente[r][c] = ' ';
            }
        }
    }

    if (spawn_r < 0 || spawn_c < 0) {
        spawn_r = 12;
        spawn_c = 1;
    }
}

static void inizializza_elementi_mappa(void)
{
    reset_elementi();
    if (stanza_corrente == 1) {
        copia_mappa_default();
        carica_elementi_mappa1();
        spawn_r = 2;
        spawn_c = 5;
    } else if (stanza_corrente == 2) {
        carica_elementi_mappa2();
    } else {
        copia_mappa_default();
    }
}

void costruisci_mappa(void)
{
    inizializza_elementi_mappa();
}

int get_stanza_corrente(void)
{
    return stanza_corrente;
}

void set_stanza_corrente(int stanza)
{
    if (stanza > 0) stanza_corrente = stanza;
}

void cambia_stanza(int nuova, Eroe *eroe)
{
    if (nuova <= 0 || nuova == stanza_corrente) return;
    stanza_corrente = nuova;
    distruggi_mappa();
    inizializza_elementi_mappa();
    if (eroe) {
        if (spawn_r >= 0 && spawn_c >= 0) {
            eroe->pos_riga = spawn_r;
            eroe->pos_col  = spawn_c;
        } else {
            eroe->pos_riga = 12;
            eroe->pos_col = 1;
        }
    }
}

int entra_porta(Eroe *eroe)
{
    if (!eroe) return 0;
    int r = eroe->pos_riga;
    int c = eroe->pos_col;
    int i = indice_porta_a(r, c);
    if (i < 0) return 0;
    if (!porta_aperta[i]) return 0;
    if (stanza_corrente == 1) {
        printf("\nHai varcato la porta e sei stato teletrasportato nella stanza finale!\n");
        cambia_stanza(2, eroe);
        return 1;
    }
    return 0;
}

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
        libera_oggetto(oggetti_mappa[i]);
        oggetti_mappa[i] = NULL;
        if (mostri_mappa[i]) {
            free(mostri_mappa[i]);
            mostri_mappa[i] = NULL;
        }
    }
    reset_elementi();
}

int salva_stato_mappa(FILE *file)
{
    if (!file) return -1;
    for (int i = 0; i < elementi_count; i++) {
        unsigned char porta = porta_aperta[i] ? 1 : 0;
        fwrite(&porta, sizeof(porta), 1, file);

        if (elementi[i].tipo == EL_MOSTRO) {
            unsigned char vivo = (mostri_mappa[i] && mostri_mappa[i]->vivo) ? 1 : 0;
            fwrite(&vivo, sizeof(vivo), 1, file);
        }

        if (elementi[i].tipo == EL_OGGETTO) {
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
    for (int i = 0; i < elementi_count; i++) {
        unsigned char porta;
        if (fread(&porta, sizeof(porta), 1, file) != 1) return -1;
        porta_aperta[i] = porta ? 1 : 0;

        if (elementi[i].tipo == EL_MOSTRO) {
            unsigned char vivo;
            if (fread(&vivo, sizeof(vivo), 1, file) != 1) return -1;
            if (mostri_mappa[i]) {
                mostri_mappa[i]->vivo = vivo ? 1 : 0;
                if (!mostri_mappa[i]->vivo) mostri_mappa[i]->hp = 0;
            }
        }

        if (elementi[i].tipo == EL_OGGETTO) {
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

int e_calpestabile(int r, int c)
{
    if (r < 0 || r >= MAPPA_RIGHE_T) return 0;
    if (c < 0 || c >= MAPPA_COLONNE_T) return 0;
    int i = indice_porta_a(r, c);
    if (i >= 0) return porta_aperta[i] ? 1 : 0;
    return mappa_corrente[r][c] == ' ';
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
    if (i < 0 || elementi[i].tipo != EL_MOSTRO) return NULL;
    Mostro *m = mostri_mappa[i];
    return (m && m->vivo) ? m : NULL;
}

Oggetto *oggetto_in_posizione(int r, int c)
{
    int i = indice_elemento(r, c);
    if (i < 0 || elementi[i].tipo != EL_OGGETTO) return NULL;
    return oggetti_mappa[i];
}

Oggetto *rimuovi_oggetto_da_posizione(int r, int c)
{
    int i = indice_elemento(r, c);
    if (i < 0 || elementi[i].tipo != EL_OGGETTO) return NULL;
    Oggetto *o = oggetti_mappa[i];
    oggetti_mappa[i] = NULL;
    return o;
}

void aggiungi_oggetto_in_posizione(int r, int c, Oggetto *oggetto)
{
    if (!oggetto) return;
    int i = indice_elemento(r, c);
    if (i < 0 || elementi[i].tipo != EL_OGGETTO) {
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
        strcpy(buf[r], mappa_corrente[r]);

    for (int i = 0; i < elementi_count; i++) {
        int r = elementi[i].r;
        int c = elementi[i].c;

        if (elementi[i].tipo == EL_PORTA) {
            if (!porta_aperta[i]) buf[r][c] = elementi[i].simbolo;
            continue;
        }

        if (elementi[i].tipo == EL_MOSTRO) {
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
                case 5:  printf("  | Att. agg.: %-5d |", eroe->bonus_danno); break;
                case 6:  printf("  | Difesa:  %-5d  |", eroe->difesa); break;
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
