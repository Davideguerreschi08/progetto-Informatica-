// mappa.c — gestione della mappa di gioco.
// La mappa è caricata da file di testo (cartella stanze/).
// Mostri, oggetti e porte sono memorizzati in array paralleli
// indicizzati dalla stessa posizione (r, c) sulla griglia.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/mappa.h"

// Dimensioni della griglia ASCII
#define MAPPA_RIGHE_T    26
#define MAPPA_COLONNE_T  62
// Numero massimo di elementi (mostri + oggetti + porte) per mappa
#define MAX_ELEMENTI_MAPPA 64

// ─── TIPI ELEMENTI ────────────────────────────────────────────────────────────
// Ogni cella della mappa che contiene qualcosa è descritta da un ElementoMappa.
typedef struct {
    int  r, c;    // posizione sulla griglia
    char simbolo; // carattere da mostrare sulla mappa
    int  tipo;    // EL_OGGETTO, EL_MOSTRO o EL_PORTA
} ElementoMappa;

#define EL_OGGETTO 0
#define EL_MOSTRO  1
#define EL_PORTA   2

// ─── DATI INTERNI ─────────────────────────────────────────────────────────────
// Array paralleli: elementi[i], oggetti_mappa[i], mostri_mappa[i] e
// porta_aperta[i] descrivono tutti lo stesso elemento i-esimo.
static ElementoMappa   elementi[MAX_ELEMENTI_MAPPA];
static int             elementi_count = 0;
static Oggetto        *oggetti_mappa[MAX_ELEMENTI_MAPPA];
static Mostro         *mostri_mappa[MAX_ELEMENTI_MAPPA];
static unsigned char   porta_aperta[MAX_ELEMENTI_MAPPA];

// Buffer con la mappa ASCII caricata da file
static char mappa_corrente[MAPPA_RIGHE_T][MAPPA_COLONNE_T + 1];

// Numero della stanza attualmente caricata (1 = Mappa1, 2 = Mappa2 boss, ecc.)
static int stanza_corrente = 1;

// Coordinate di spawn dell'eroe per la stanza corrente
static int spawn_r = 2, spawn_c = 5;

// ─── PROTOTIPI STATICI ────────────────────────────────────────────────────────
static Oggetto *nuovo_oggetto(const char *nome, TipoOggetto tipo, int valore);
static Mostro  *nuovo_mostro(const char *nome, TipoMostro tipo,int hp, int att, int dif, int xp, int oro);
static char     simbolo_oggetto(const Oggetto *o);
static void     libera_oggetto(Oggetto *o);

// ─── HELPERS INTERNI ──────────────────────────────────────────────────────────

// Genera una mappa vuota con solo il bordo (usata come fallback se il file manca).
static void riempi_mappa_vuota(void){
    for (int r = 0; r < MAPPA_RIGHE_T; r++) {
        for (int c = 0; c < MAPPA_COLONNE_T; c++) {
            // Prima e ultima riga → bordo orizzontale
            if (r == 0 || r == MAPPA_RIGHE_T - 1) {
                if (c == 0 || c == MAPPA_COLONNE_T - 1)
                    mappa_corrente[r][c] = 'I';
                else
                    mappa_corrente[r][c] = '-';
            } else {
                // Prima e ultima colonna → bordo verticale
                if (c == 0 || c == MAPPA_COLONNE_T - 1)
                    mappa_corrente[r][c] = 'I';
                else
                    mappa_corrente[r][c] = ' ';
            }
        }
        mappa_corrente[r][MAPPA_COLONNE_T] = '\0';
    }
    spawn_r = 2;
    spawn_c = 5;
}

// Azzera tutti gli array degli elementi e le loro strutture dati.
static void reset_elementi(void){
    elementi_count = 0;
    memset(porta_aperta,  0, sizeof porta_aperta);
    memset(oggetti_mappa, 0, sizeof oggetti_mappa);
    memset(mostri_mappa,  0, sizeof mostri_mappa);
}

// Registra un nuovo elemento nella posizione (r, c).
// Restituisce l'indice assegnato, oppure -1 se l'array è pieno.
static int aggiungi_elemento(int r, int c, char simbolo, int tipo,Oggetto *oggetto, Mostro *mostro){
    if (elementi_count >= MAX_ELEMENTI_MAPPA) return -1;

    elementi[elementi_count].r       = r;
    elementi[elementi_count].c       = c;
    elementi[elementi_count].simbolo = simbolo;
    elementi[elementi_count].tipo    = tipo;
    oggetti_mappa[elementi_count]    = oggetto;
    mostri_mappa[elementi_count]     = mostro;
    porta_aperta[elementi_count]     = 0;  // sempre chiusa all'inizio

    return elementi_count++;
}

// Carica il testo della mappa dal file corrispondente alla stanza richiesta.
// Restituisce 0 in caso di successo, -1 in caso di errore.
static int carica_mappa_da_testo(int stanza){
    const char *path = NULL;

    if (stanza == 1)
        path = "stanze/Mappa1 prototipo finale.txt";
    else if (stanza == 2)
        path = "stanze/Mappa2 prototipo finale(con BOSS).txt";
    else
        return -1;

    FILE *fp = fopen(path, "r");
    if (!fp) return -1;

    char line[128];
    for (int r = 0; r < MAPPA_RIGHE_T; r++) {
        if (fgets(line, sizeof(line), fp)) {
            // Lunghezza della riga senza newline
            int len = (int)strcspn(line, "\r\n");
            for (int c = 0; c < MAPPA_COLONNE_T; c++) {
                if (c < len)
                    mappa_corrente[r][c] = line[c];
                else
                    mappa_corrente[r][c] = ' ';  // riempi con spazi se la riga è corta
            }
        } else {
            // Riga mancante nel file: riempi con spazi
            for (int c = 0; c < MAPPA_COLONNE_T; c++)
                mappa_corrente[r][c] = ' ';
        }
        mappa_corrente[r][MAPPA_COLONNE_T] = '\0';
    }

    fclose(fp);
    return 0;
}

// Cerca un elemento nella posizione (r, c). Restituisce l'indice o -1.
static int indice_elemento(int r, int c){
    for (int i = 0; i < elementi_count; i++) {
        if (elementi[i].r == r && elementi[i].c == c)
            return i;
    }
    return -1;
}

// Cerca specificamente una PORTA nella posizione (r, c). Restituisce l'indice o -1.
static int indice_porta_a(int r, int c){
    for (int i = 0; i < elementi_count; i++) {
        if (elementi[i].tipo != EL_PORTA) continue;
        if (elementi[i].r == r && elementi[i].c == c) return i;
    }
    return -1;
}

// ─── POPOLAMENTO MAPPE ────────────────────────────────────────────────────────

// Carica gli elementi (mostri, oggetti, porte) della Mappa 1.
// Le celle occupate da elementi vengono azzerate nella mappa ASCII
// perché e_calpestabile le gestisce separatamente.
static void carica_elementi_mappa1(void){
    // Oggetti
    aggiungi_elemento( 5, 50, 'P', EL_OGGETTO,nuovo_oggetto("Pozione di cura", POZIONE, 20), NULL);
    aggiungi_elemento(10, 26, 'K', EL_OGGETTO,nuovo_oggetto("Chiave arrugginita", CHIAVE, 0), NULL);
    aggiungi_elemento( 2, 11, 'F', EL_OGGETTO,nuovo_oggetto("Amuleto della forza", AMULETO_FORZA, 1), NULL);
    aggiungi_elemento(10,  3, 'B', EL_OGGETTO,nuovo_oggetto("Bomba", BOMBA, 30), NULL);
    aggiungi_elemento(15, 40, 'P', EL_OGGETTO,nuovo_oggetto("Pozione di cura", POZIONE, 20), NULL);
    aggiungi_elemento(15, 38, 'B', EL_OGGETTO,nuovo_oggetto("Bomba", BOMBA, 30), NULL);
    aggiungi_elemento(22, 11, 'D', EL_OGGETTO,nuovo_oggetto("Amuleto difesa", AMULETO_DIFESA, 1), NULL);
    aggiungi_elemento( 5, 40, 'V', EL_OGGETTO,nuovo_oggetto("Pozione veleno", POZIONE_VELENO, 10), NULL);
    aggiungi_elemento(10,  7, 'V', EL_OGGETTO,nuovo_oggetto("Pozione veleno", POZIONE_VELENO, 10), NULL);

    // Mostri
    aggiungi_elemento( 6, 17, 'M', EL_MOSTRO, NULL,nuovo_mostro("Goblin", GOBLIN, 15, 5, 3, 20, 10));
    aggiungi_elemento(14, 17, 'M', EL_MOSTRO, NULL,nuovo_mostro("Scheletro", SCHELETRO, 20, 7, 4, 30, 13));
    aggiungi_elemento( 2, 27, 'M', EL_MOSTRO, NULL, nuovo_mostro("Mago nero", MAGO, 33, 12, 6, 50, 25));
    aggiungi_elemento(10, 24, 'M', EL_MOSTRO, NULL,nuovo_mostro("Demone minore", DEMONE, 38, 10, 5, 40, 22));
    aggiungi_elemento(22, 35, 'M', EL_MOSTRO, NULL,nuovo_mostro("Drago scheletro", DRAGO_SCHELETRO, 55, 17, 9, 85, 53));
    aggiungi_elemento(12, 58, 'M', EL_MOSTRO, NULL,nuovo_mostro("Drago nero occhi rossi", DRAGO, 80, 19, 11, 130, 90));
    aggiungi_elemento( 9, 54, 'M', EL_MOSTRO, NULL,nuovo_mostro("Scheletro", SCHELETRO, 20, 7, 4, 30, 13));
    aggiungi_elemento(15, 54, 'M', EL_MOSTRO, NULL,nuovo_mostro("Goblin", GOBLIN, 15, 5, 3, 20, 10));

    // Porta (blocca il passaggio finché non aperta con chiave)
    aggiungi_elemento(12, 61, '[', EL_PORTA, NULL, NULL);

    // Azzera le celle occupate da elementi nella mappa ASCII,
    // così e_calpestabile le considera percorribili.
    for (int i = 0; i < elementi_count; i++) {
        int r = elementi[i].r;
        int c = elementi[i].c;
        if (r >= 0 && r < MAPPA_RIGHE_T && c >= 0 && c < MAPPA_COLONNE_T)
            mappa_corrente[r][c] = ' ';
    }
}

// Carica gli elementi della Mappa 2 (stanza del boss).
static void carica_elementi_mappa2(void)
{
    // Se il file della mappa non esiste, usa la mappa vuota come fallback
    if (carica_mappa_da_testo(2) != 0) {
        riempi_mappa_vuota();
        return;
    }

    // Oggetti
    aggiungi_elemento( 6, 26, 'D', EL_OGGETTO,nuovo_oggetto("Amuleto difesa", AMULETO_DIFESA, 1), NULL);
    aggiungi_elemento( 6, 35, 'F', EL_OGGETTO,nuovo_oggetto("Amuleto della forza", AMULETO_FORZA, 1), NULL);
    aggiungi_elemento( 8, 26, 'P', EL_OGGETTO,nuovo_oggetto("Pozione", POZIONE, 20), NULL);
    aggiungi_elemento( 8, 35, 'V', EL_OGGETTO,nuovo_oggetto("Pozione veleno", POZIONE_VELENO, 10), NULL);
    aggiungi_elemento(16, 26, 'P', EL_OGGETTO,nuovo_oggetto("Pozione", POZIONE, 20), NULL);
    aggiungi_elemento(16, 35, 'V', EL_OGGETTO,nuovo_oggetto("Pozione veleno", POZIONE_VELENO,10), NULL);
    aggiungi_elemento(18, 26, 'B', EL_OGGETTO,nuovo_oggetto("Bomba", BOMBA, 30), NULL);
    aggiungi_elemento(18, 35, 'B', EL_OGGETTO,nuovo_oggetto("Bomba", BOMBA, 30), NULL);

    // Mostri
    aggiungi_elemento(11, 26, 'M', EL_MOSTRO, NULL,nuovo_mostro("Scheletro errante", SCHELETRO, 22, 8, 4, 29, 14));
    aggiungi_elemento(11, 35, 'M', EL_MOSTRO, NULL,nuovo_mostro("Scheletro errante", SCHELETRO, 22, 8, 4, 29, 14));
    aggiungi_elemento(12, 40, '@', EL_MOSTRO, NULL,nuovo_mostro("Cavaliere dimenticato", BOSS, 160, 26, 14, 300, 180));
    aggiungi_elemento(13, 26, 'M', EL_MOSTRO, NULL,nuovo_mostro("Scheletro errante", SCHELETRO, 22, 8, 4, 29, 14));
    aggiungi_elemento(13, 35, 'M', EL_MOSTRO, NULL,nuovo_mostro("Scheletro errante", SCHELETRO, 22, 8, 4, 29, 14));

    // Porta di ritorno alla mappa precedente
    aggiungi_elemento(12, 0, '[', EL_PORTA, NULL, NULL);

    // Azzera le celle occupate nella mappa ASCII
    for (int i = 0; i < elementi_count; i++) {
        int r = elementi[i].r;
        int c = elementi[i].c;
        if (r >= 0 && r < MAPPA_RIGHE_T && c >= 0 && c < MAPPA_COLONNE_T)
            mappa_corrente[r][c] = ' ';
    }

    // Spawn nella mappa del boss: lato sinistro della porta
    spawn_r = 12;
    spawn_c =  1;
}

// Carica la mappa e i suoi elementi in base alla stanza corrente.
static void inizializza_elementi_mappa(void){
    reset_elementi();

    if (stanza_corrente == 1) {
        // Carica Mappa 1 da file; se fallisce usa la mappa vuota
        if (carica_mappa_da_testo(1) != 0)
            riempi_mappa_vuota();
        carica_elementi_mappa1();
        spawn_r = 2;
        spawn_c = 5;
    } else if (stanza_corrente == 2) {
        carica_elementi_mappa2();
    } else {
        riempi_mappa_vuota();
    }
}

// ─── API PUBBLICA ─────────────────────────────────────────────────────────────

// Costruisce la mappa della stanza corrente (chiamata all'avvio e dopo cambia_stanza).
void costruisci_mappa(void){
    inizializza_elementi_mappa();
}

// Libera tutta la memoria degli elementi della mappa corrente.
void distruggi_mappa(void){
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

// Restituisce il numero della stanza attualmente caricata.
int get_stanza_corrente(void){
    return stanza_corrente;
}

// Imposta la stanza corrente (usato da carica_partita).
void set_stanza_corrente(int stanza){
    if (stanza > 0)
        stanza_corrente = stanza;
}

// Cambia la stanza: distrugge la corrente, carica la nuova e repositiona l'eroe.
void cambia_stanza(int nuova, Eroe *eroe){
    if (nuova <= 0 || nuova == stanza_corrente) return;

    stanza_corrente = nuova;
    distruggi_mappa();
    inizializza_elementi_mappa();

    // Posiziona l'eroe sullo spawn della nuova stanza
    if (eroe) {
        if (spawn_r >= 0 && spawn_c >= 0) {
            eroe->pos_riga = spawn_r;
            eroe->pos_col  = spawn_c;
        } else {
            eroe->pos_riga = 12;
            eroe->pos_col  =  1;
        }
    }
}

// Controlla se l'eroe è su una porta aperta che porta a un'altra stanza.
// Se sì, esegue il cambio stanza e restituisce 1; altrimenti restituisce 0.
int entra_porta(Eroe *eroe){
    if (!eroe) return 0;

    int r = eroe->pos_riga;
    int c = eroe->pos_col;
    int i = indice_porta_a(r, c);

    if (i < 0) return 0;          // nessuna porta qui
    if (!porta_aperta[i]) return 0; // porta chiusa, non si teletrasporta

    // Transizione tra stanze
    if (stanza_corrente == 1) {
        printf("\nHai varcato la porta: sei entrato nella stanza finale!\n");
        cambia_stanza(2, eroe);
        return 1;
    }

    return 0;
}

// ─── QUERY POSIZIONE ──────────────────────────────────────────────────────────

// Restituisce 1 se la cella (r, c) è percorribile dal giocatore.
// Le celle con mostri/oggetti sono percorribili (il giocatore ci può entrare).
// Le porte chiuse non sono percorribili (bloccano come muri).
int e_calpestabile(int r, int c){
    // Fuori dai limiti della griglia: non calpestabile
    if (r < 0 || r >= MAPPA_RIGHE_T) return 0;
    if (c < 0 || c >= MAPPA_COLONNE_T) return 0;

    // Controlla prima se c'è una porta: se chiusa, blocca; se aperta, passa
    int i = indice_porta_a(r, c);
    if (i >= 0) {
        if (porta_aperta[i]) return 1;
        return 0;
    }

    // Celle con mostri o oggetti: sempre percorribili
    int idx = indice_elemento(r, c);
    if (idx >= 0) {
        if (elementi[idx].tipo == EL_MOSTRO || elementi[idx].tipo == EL_OGGETTO)
            return 1;
    }

    // Altrimenti: calpestabile solo se la cella è uno spazio nella mappa ASCII
    if (mappa_corrente[r][c] == ' ') return 1;
    return 0;
}

// Restituisce 1 se nella posizione (r, c) c'è una porta chiusa.
int porta_chiusa_in(int r, int c){
    int i = indice_porta_a(r, c);
    if (i >= 0 && !porta_aperta[i]) return 1;
    return 0;
}

// Apre la porta nella posizione (r, c).
void apri_porta_in(int r, int c){
    int i = indice_porta_a(r, c);
    if (i >= 0)
        porta_aperta[i] = 1;
}

// Restituisce il puntatore al mostro vivo nella posizione (r, c), o NULL.
Mostro *mostro_in_posizione(int r, int c){
    int i = indice_elemento(r, c);
    if (i < 0 || elementi[i].tipo != EL_MOSTRO) return NULL;

    Mostro *m = mostri_mappa[i];
    if (m && m->vivo) return m;
    return NULL;
}

// Restituisce il puntatore all'oggetto presente in (r, c), o NULL se assente.
Oggetto *oggetto_in_posizione(int r, int c){
    int i = indice_elemento(r, c);
    if (i < 0 || elementi[i].tipo != EL_OGGETTO) return NULL;
    return oggetti_mappa[i];
}

// Rimuove e restituisce l'oggetto in (r, c); la cella diventa vuota.
Oggetto *rimuovi_oggetto_da_posizione(int r, int c){
    int i = indice_elemento(r, c);
    if (i < 0 || elementi[i].tipo != EL_OGGETTO) return NULL;

    Oggetto *o = oggetti_mappa[i];
    oggetti_mappa[i] = NULL;  // cella vuota
    return o;
}

// Aggiunge un oggetto nella posizione (r, c), liberando quello precedente se presente.
void aggiungi_oggetto_in_posizione(int r, int c, Oggetto *oggetto){
    if (!oggetto) return;

    int i = indice_elemento(r, c);
    if (i < 0 || elementi[i].tipo != EL_OGGETTO) {
        free(oggetto);  // posizione non valida: libera l'oggetto
        return;
    }

    libera_oggetto(oggetti_mappa[i]);  // libera l'eventuale oggetto precedente
    oggetti_mappa[i] = oggetto;
}

// ─── SALVATAGGIO / CARICAMENTO STATO MAPPA ────────────────────────────────────

// Serializza su file lo stato corrente di ogni elemento (porte aperte,
// mostri vivi/morti, oggetti presenti/assenti con i loro dati).
int salva_stato_mappa(FILE *file){
    if (!file) return -1;

    for (int i = 0; i < elementi_count; i++) {

        // Stato porta: 1 = aperta, 0 = chiusa
        unsigned char porta;
        if (porta_aperta[i])
            porta = 1;
        else
            porta = 0;
        fwrite(&porta, sizeof(porta), 1, file);

        // Stato mostro: 1 = vivo, 0 = morto
        if (elementi[i].tipo == EL_MOSTRO) {
            unsigned char vivo;
            if (mostri_mappa[i] && mostri_mappa[i]->vivo)
                vivo = 1;
            else
                vivo = 0;
            fwrite(&vivo, sizeof(vivo), 1, file);
        }

        // Stato oggetto: presenza + dati se presente
        if (elementi[i].tipo == EL_OGGETTO) {
            unsigned char has;
            if (oggetti_mappa[i])
                has = 1;
            else
                has = 0;
            fwrite(&has, sizeof(has), 1, file);

            if (has) {
                Oggetto *o = oggetti_mappa[i];
                int tipo = (int)o->tipo;
                fwrite(o->nome,  sizeof(o->nome),  1, file);
                fwrite(&tipo,    sizeof(tipo),      1, file);
                fwrite(&o->valore, sizeof(o->valore), 1, file);
            }
        }
    }
    return 0;
}

// Ripristina lo stato della mappa da file (dopo carica_partita).
int carica_stato_mappa(FILE *file){
    if (!file) return -1;

    for (int i = 0; i < elementi_count; i++) {

        unsigned char porta;
        if (fread(&porta, sizeof(porta), 1, file) != 1) return -1;
        if (porta)
            porta_aperta[i] = 1;
        else
            porta_aperta[i] = 0;

        if (elementi[i].tipo == EL_MOSTRO) {
            unsigned char vivo;
            if (fread(&vivo, sizeof(vivo), 1, file) != 1) return -1;
            if (mostri_mappa[i]) {
                if (vivo)
                    mostri_mappa[i]->vivo = 1;
                else {
                    mostri_mappa[i]->vivo = 0;
                    mostri_mappa[i]->hp   = 0;
                }
            }
        }

        if (elementi[i].tipo == EL_OGGETTO) {
            unsigned char has;
            if (fread(&has, sizeof(has), 1, file) != 1) return -1;

            if (!has) {
                // Oggetto raccolto: libera la memoria e azzera il puntatore
                libera_oggetto(oggetti_mappa[i]);
                oggetti_mappa[i] = NULL;
            } else {
                // Oggetto ancora presente: leggi i dati e ricostruisci
                char nome[MAX_NOME];
                int  tipo, valore;
                if (fread(nome,    sizeof(nome),   1, file) != 1) return -1;
                if (fread(&tipo,   sizeof(tipo),   1, file) != 1) return -1;
                if (fread(&valore, sizeof(valore), 1, file) != 1) return -1;

                libera_oggetto(oggetti_mappa[i]);
                oggetti_mappa[i] = nuovo_oggetto(nome, (TipoOggetto)tipo, valore);
            }
        }
    }
    return 0;
}

// ─── STAMPA MAPPA ─────────────────────────────────────────────────────────────

// Pannello comandi mostrato sopra la mappa ad ogni turno.
static void stampa_comandi(void){
    printf("\n");
    printf("╔════════════════════════════════════════════════════════╗\n");
    printf("║                  COMANDI DISPONIBILI                   ║\n");
    printf("╠════════════════════════════════════════════════════════╣\n");
    printf("║  Movimento: W=Nord  S=Sud  A=Ovest  D=Est              ║\n");
    printf("║  Comandi:   usa, inventario, salva, carica             ║\n");
    printf("╚════════════════════════════════════════════════════════╝\n");
}

// Disegna la mappa con gli elementi sovrapposti e il pannello laterale dello stato.
void stampa_mappa(Eroe *eroe){
    stampa_comandi();

    // Copia della mappa ASCII su cui sovrapponiamo i simboli degli elementi
    char buf[MAPPA_RIGHE_T][MAPPA_COLONNE_T + 1];
    for (int r = 0; r < MAPPA_RIGHE_T; r++)
        strcpy(buf[r], mappa_corrente[r]);

    // Disegna ogni elemento sul buffer
    for (int i = 0; i < elementi_count; i++) {
        int r = elementi[i].r;
        int c = elementi[i].c;

        if (elementi[i].tipo == EL_PORTA) {
            // La porta appare solo se chiusa; se aperta, la cella è libera
            if (!porta_aperta[i])
                buf[r][c] = elementi[i].simbolo;
            continue;
        }

        if (elementi[i].tipo == EL_MOSTRO) {
            Mostro *m = mostri_mappa[i];
            if (m && m->vivo) {
                buf[r][c] = 'M';
                continue;
            }
        }

        // Oggetto: mostra il simbolo solo se ancora presente
        char s = simbolo_oggetto(oggetti_mappa[i]);
        if (s != '\0')
            buf[r][c] = s;
    }

    // Posizione del giocatore (sovrascrive tutto il resto)
    if (eroe) {
        int r = eroe->pos_riga;
        int c = eroe->pos_col;
        if (r >= 0 && r < MAPPA_RIGHE_T && c >= 0 && c < MAPPA_COLONNE_T)
            buf[r][c] = '&';
    }

    // Stampa riga per riga con il pannello laterale affiancato
    printf("\n");
    printf("%-62s  STATO EROE\n", "=== MAPPA DEL DUNGEON ===");
    printf("%-62s  +---------------------+\n", "");

    for (int r = 0; r < MAPPA_RIGHE_T; r++) {
        printf("%s", buf[r]);

        if (eroe) {
            switch (r) {
                case 1:  printf("  | Nome: %-14s|", eroe->nome); break;
                case 2:  printf("  | HP:   %3d/%-3d       |", eroe->hp, eroe->hp_max); break;
                case 3:  printf("  | Livello:  %-5d     |", eroe->livello); break;
                case 4:  printf("  | XP:  %-9d      |", eroe->xp); break;
                case 5:  printf("  | Attacco:  %-5d     |", eroe->attacco + eroe->bonus_danno); break;
                case 6:  printf("  | Difesa:   %-5d     |", eroe->difesa); break;
                case 7:  printf("  | Oro:  %-9d     |", eroe->oro); break;
                case 8:  printf("  | Inv:  %2d/%2d oggetti |",eroe->inventario.top + 1, MAX_INVENTARIO); break;
                case 9:  printf("  +---------------------+"); break;
                case 10: printf("  LEGENDA:"); break;
                case 11: printf("  & = Tu (giocatore)"); break;
                case 12: printf("  M = Mostro"); break;
                case 13: printf("  P = Pozione"); break;
                case 14: printf("  V = Veleno"); break;
                case 15: printf("  K = Chiave"); break;
                case 16: printf("  B = Bomba"); break;
                case 17: printf("  F = Amuleto forza"); break;
                case 18: printf("  D = Amuleto difesa"); break;
                case 19: printf("  [ = Porta bloccata"); break;
                default: break;
            }
        }
        putchar('\n');
    }
    printf("\n");
}

// ─── HELPER OGGETTI/MOSTRI ────────────────────────────────────────────────────

// Alloca e inizializza un nuovo oggetto.
static Oggetto *nuovo_oggetto(const char *nome, TipoOggetto tipo, int valore){
Oggetto *o = malloc(sizeof(Oggetto));
    if (!o) return NULL;
    strncpy(o->nome, nome, MAX_NOME - 1);
    o->nome[MAX_NOME - 1] = '\0';
    o->tipo   = tipo;
    o->valore = valore;
    o->next   = NULL;
    return o;
}

// Alloca e inizializza un nuovo mostro.
static Mostro *nuovo_mostro(const char *nome, TipoMostro tipo,int hp, int att, int dif, int xp, int oro)
{
    Mostro *m = malloc(sizeof(Mostro));
    if (!m) return NULL;
    strncpy(m->nome, nome, MAX_NOME - 1);
    m->nome[MAX_NOME - 1] = '\0';
    m->tipo           = tipo;
    m->hp             = hp;
    m->hp_max         = hp;
    m->attacco        = att;
    m->difesa         = dif;
    m->xp_ricompensa  = xp;
    m->oro_ricompensa = oro;
    m->vivo           = 1;
    return m;
}

// Restituisce il carattere da mostrare sulla mappa per un dato oggetto.
static char simbolo_oggetto(const Oggetto *o){
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

// Libera una lista collegata di oggetti.
static void libera_oggetto(Oggetto *o){
    while (o) {
        Oggetto *next = o->next;
        free(o);
        o = next;
    }
}