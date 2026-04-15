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

// ─── ENUM ─────────────────────────────────────────────────────────────────────

// typedef enum funziona come typedef struct:
// senza typedef dovresti scrivere "enum TipoOggetto tipo" ogni volta,
// con typedef puoi scrivere solo "TipoOggetto tipo".
// Internamente ogni valore è un numero intero:
// POZIONE=0, POZIONE_VELENO=1, ARMA=2, ecc.
// Usiamo i nomi invece dei numeri per rendere il codice leggibile.
typedef enum {
    POZIONE,
    POZIONE_VELENO,
    ARMA,
    ARMATURA,
    CHIAVE,
    TORCIA,
    BOMBA,
    AMULETO
} TipoOggetto;

// Stesso concetto, ma per i nemici.
typedef enum {
    SCHELETRO,
    GOBLIN,
    ORCO,
    VAMPIRO,
    DRAGO,
    MAGO,
    BOSS
} TipoMostro;

// I comandi che il giocatore può digitare.
// Il parser legge la stringa, la confronta con le parole chiave
// e restituisce uno di questi valori. CMD_SCONOSCIUTO viene
// restituito quando il giocatore scrive qualcosa che non esiste.
typedef enum {
    CMD_VAI,
    CMD_GUARDA,
    CMD_PRENDI,
    CMD_USA,
    CMD_ATTACCA,
    CMD_INVENTARIO,
    CMD_SALVA,
    CMD_CARICA,
    CMD_MAPPA,
    CMD_SCONOSCIUTO
} TipoComando;

// ─── STRUCT OGGETTO ───────────────────────────────────────────────────────────

// Rappresenta un singolo oggetto raccoglibile dal giocatore.
// Il campo "valore" cambia significato in base al tipo:
//   POZIONE        → quanti HP ripristina
//   POZIONE_VELENO → quanti danni fa al nemico
//   ARMA           → quanti danni aggiunge all'attacco
//   BOMBA          → quanti danni fa ad area
//   ARMATURA       → quanta difesa aggiunge
typedef struct Oggetto {
    char        nome[MAX_NOME];
    TipoOggetto tipo;
    int         valore;
} Oggetto;

// ─── LISTA COLLEGATA OGGETTI ──────────────────────────────────────────────────

// Ogni stanza ha una lista collegata di oggetti al suo interno.
// Ogni nodo punta all'oggetto e al nodo successivo.
// Quando "next" è NULL siamo all'ultimo nodo della lista.
// Quando il giocatore raccoglie un oggetto, il nodo viene
// rimosso dalla lista e l'oggetto viene messo nella sua pila.
typedef struct NodoOggetto {
    Oggetto            *oggetto;  // puntatore all'oggetto
    struct NodoOggetto *next;     // puntatore al nodo successivo (NULL se ultimo)
} NodoOggetto;

// ─── STRUCT MOSTRO ────────────────────────────────────────────────────────────

// Rappresenta un nemico presente in una stanza.
// xp_ricompensa e oro_ricompensa vengono aggiunti all'eroe
// quando il mostro viene sconfitto.
typedef struct Mostro {
    char       nome[MAX_NOME];
    TipoMostro tipo;
    int        hp;
    int        attacco;
    int        difesa;
    int        xp_ricompensa;
    int        oro_ricompensa;
} Mostro;

// ─── STRUCT STANZA ────────────────────────────────────────────────────────────

// È il nodo del grafo. Ogni stanza conosce le stanze vicine
// tramite quattro puntatori (nord, sud, est, ovest).
// Se un puntatore è NULL non c'è uscita in quella direzione.
//
// NOTA: dentro la struct usiamo "struct Stanza *nord" e non "Stanza *nord".
// Questo perché quando il compilatore sta leggendo la definizione di Stanza,
// il typedef non è ancora completo — quindi "Stanza" come nome non esiste ancora.
// "struct Stanza" invece funziona perché il compilatore sa già che
// esiste una struttura con quel nome, anche se non ha finito di leggerla.
typedef struct Stanza {
    int           id;
    char          nome[MAX_NOME];
    char          descrizione[MAX_DESCRIZIONE];
    bool          visitata;       // true se il giocatore ci è già entrato
    bool          bloccata;       // true se serve una chiave per entrare
    NodoOggetto  *oggetti;        // testa della lista oggetti (NULL se vuota)
    Mostro       *mostro;         // NULL se non c'è nessun nemico
    struct Stanza *nord;          // puntatore alla stanza a nord (NULL se assente)
    struct Stanza *sud;
    struct Stanza *est;
    struct Stanza *ovest;
} Stanza;

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
// "stanza_corrente" è il puntatore alla stanza dove si trova l'eroe:
// quando si muove, aggiorniamo solo questo puntatore — non copiamo niente.
typedef struct {
    char   nome[MAX_NOME];
    int    hp;
    int    hp_max;
    int    attacco;
    int    difesa;
    int    xp;
    int    livello;
    int    oro;
    Pila   inventario;        // la pila degli oggetti raccolti
    Stanza *stanza_corrente;  // stanza in cui si trova l'eroe in questo momento
} Eroe;

#endif  // fine include guard — tutto sopra viene incluso una volta sola