// Questo file viene incluso da più file (.c e .h) del progetto.
// Senza protezione, le struct verrebbero definite più volte → errore.
// Le include guard qui sotto risolvono il problema:
#ifndef TIPI_H  // se TIPI_H non è ancora stato definito (prima inclusione)...
#define TIPI_H  // ...definiscilo, così la prossima volta saltiamo tutto

#include <stdbool.h>  // ci serve per usare il tipo bool (true/false)

// ─── COSTANTI ─────────────────────────────────────────────────────────────────

#define MAX_INVENTARIO  8    // massimo 8 oggetti nell'inventario
#define MAX_NOME        64   // lunghezza massima di un nome
#define MAX_DESCRIZIONE 256  // lunghezza massima di una descrizione
#define MAX_INPUT      128  // lunghezza massima input utente

// ─── ENUM ─────────────────────────────────────────────────────────────────────

// typedef enum funziona come typedef struct:
// senza typedef dovresti scrivere "enum TipoOggetto tipo" ogni volta,
// con typedef puoi scrivere solo "TipoOggetto tipo".
// Internamente ogni valore è un numero intero.
// Usiamo i nomi invece dei numeri per rendere il codice leggibile.
typedef enum {
    POZIONE,
    POZIONE_VELENO,
    CHIAVE,
    BOMBA,
    AMULETO_FORZA,
    AMULETO_DIFESA
} TipoOggetto;

// Stesso concetto, ma per i nemici.
typedef enum {
    SCHELETRO,
    GOBLIN,
    DRAGO,
    MAGO,
    BOSS,
    DRAGO_SCHELETRO,
    DEMONE,
    LICH
} TipoMostro;

// I comandi che il giocatore può digitare.
// Il parser legge la stringa, la confronta con le parole chiave
// e restituisce uno di questi valori. CMD_SCONOSCIUTO viene
// restituito quando il giocatore scrive qualcosa che non esiste.
typedef enum {
    CMD_VAI,
    CMD_PRENDI,
    CMD_USA,
    CMD_ATTACCA,
    CMD_INVENTARIO,
    CMD_SALVA,
    CMD_CARICA,
    CMD_SCONOSCIUTO
} TipoComando;

// ─── STRUCT OGGETTO ───────────────────────────────────────────────────────────

// Rappresenta un singolo oggetto raccoglibile dal giocatore.
// Il campo "valore" cambia significato in base al tipo:
//   POZIONE        → quanti HP ripristina
//   POZIONE_VELENO → quanti danni fa al nemico
//   CHIAVE         → apertura di porte bloccate
//   BOMBA          → quanti danni fa al nemico
//   AMULETO_FORZA  → aumenta il danno in combattimento
//   AMULETO_DIFESA → aumenta la difesa in combattimento
typedef struct Oggetto {
    char        nome[MAX_NOME];
    TipoOggetto tipo;
    int         valore;
    struct Oggetto *next;   // lista collegata di oggetti
} Oggetto;

// ─── STRUCT MOSTRO ────────────────────────────────────────────────────────────

// Rappresenta un nemico sul dungeon.
// xp_ricompensa e oro_ricompensa vengono aggiunti all'eroe
// quando il mostro viene sconfitto.
typedef struct Mostro {
    char       nome[MAX_NOME];
    TipoMostro tipo;
    int        hp;
    int        hp_max;
    int        attacco;
    int        difesa;
    int        xp_ricompensa;
    int        oro_ricompensa;
    int        vivo;
} Mostro;

// ─── PILA INVENTARIO ──────────────────────────────────────────────────────────

// La pila è un array di puntatori a Oggetto con un indice "top".
// top = -1 → pila vuota
// top =  0 → un solo oggetto (in posizione 0)
// top =  7 → pila piena (MAX_INVENTARIO - 1)
// Push aggiunge in cima, pop toglie dalla cima.
// Se la pila è piena, push non aggiunge niente e segnala errore.
typedef struct {
    Oggetto *oggetti[MAX_INVENTARIO];  // array di puntatori agli oggetti
    int      top;                      // indice della cima della pila
} Pila;

// ─── STRUCT EROE ──────────────────────────────────────────────────────────────

// Contiene tutto lo stato del giocatore.
// La posizione dell'eroe viene memorizzata come coordinate sulla mappa.
typedef struct {
    char   nome[MAX_NOME];
    int    hp;
    int    hp_max;
    int    attacco;
    int    difesa;
    int    bonus_danno;
    int    xp;
    int    livello;
    int    oro;
    Pila   inventario;
    int    pos_riga;   // posizione sulla mappa visiva
    int    pos_col;
} Eroe;

#endif  // fine include guard — tutto sopra viene incluso una volta sola