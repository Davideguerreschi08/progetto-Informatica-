// tipi.h — definizioni condivise da tutti i moduli del progetto.
// Usando le include guard evitiamo che il file venga incluso più volte
// nello stesso file .c, il che causerebbe errori di "ridefinizione".
#ifndef TIPI_H
#define TIPI_H

#include <stdbool.h>  // per il tipo bool (true/false)

// ─── COSTANTI ─────────────────────────────────────────────────────────────────

#define MAX_INVENTARIO  8    // numero massimo di oggetti in inventario
#define MAX_NOME        64   // lunghezza massima di un nome
#define MAX_DESCRIZIONE 256  // lunghezza massima di una descrizione
#define MAX_INPUT       128  // lunghezza massima dell'input utente

// ─── TIPO OGGETTO ─────────────────────────────────────────────────────────────
// Ogni valore corrisponde internamente a un intero (POZIONE=0, ecc.).
// Il campo "valore" in Oggetto cambia significato a seconda del tipo:
//   POZIONE        → HP recuperati
//   POZIONE_VELENO → danni inflitti al nemico (con componente casuale)
//   CHIAVE         → apre porte bloccate
//   BOMBA          → danni fissi inflitti al nemico
//   AMULETO_FORZA  → bonus al danno per l'intero combattimento
//   AMULETO_DIFESA → bonus alla difesa per l'intero combattimento
typedef enum {
    POZIONE,
    POZIONE_VELENO,
    CHIAVE,
    BOMBA,
    AMULETO_FORZA,
    AMULETO_DIFESA
} TipoOggetto;

// ─── TIPO MOSTRO ──────────────────────────────────────────────────────────────
// Identifica la categoria del nemico.
// BOSS è speciale: sconfiggerlo termina la partita.
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

// ─── TIPO COMANDO ─────────────────────────────────────────────────────────────
// Valori restituiti dal parser dopo aver letto l'input utente.
// CMD_SCONOSCIUTO viene restituito per qualsiasi stringa non riconosciuta.
typedef enum {
    CMD_VAI,        // movimento (W/A/S/D o "vai nord" ecc.)
    CMD_USA,        // usa un oggetto dall'inventario
    CMD_INVENTARIO, // mostra l'inventario
    CMD_SALVA,      // salva la partita su file
    CMD_CARICA,     // carica la partita da file
    CMD_SCONOSCIUTO
} TipoComando;

// ─── STRUCT OGGETTO ───────────────────────────────────────────────────────────
// Rappresenta un oggetto raccoglibile o già in inventario.
// "next" serve per liste collegate (non usato in inventario, ma in mappa).
typedef struct Oggetto {
    char        nome[MAX_NOME];
    TipoOggetto tipo;
    int         valore;
    struct Oggetto *next;
} Oggetto;

// ─── STRUCT MOSTRO ────────────────────────────────────────────────────────────
// Rappresenta un nemico posizionato sulla mappa.
typedef struct Mostro {
    char       nome[MAX_NOME];
    TipoMostro tipo;
    int        hp;
    int        hp_max;
    int        attacco;
    int        difesa;
    int        xp_ricompensa;
    int        oro_ricompensa;
    int        vivo;  // 1 = vivo, 0 = sconfitto
} Mostro;

// ─── PILA INVENTARIO ──────────────────────────────────────────────────────────
// Stack (LIFO) di puntatori a Oggetto.
// top = -1 → vuota; top = MAX_INVENTARIO-1 → piena.
typedef struct {
    Oggetto *oggetti[MAX_INVENTARIO];
    int      top;
} Pila;

// ─── STRUCT EROE ──────────────────────────────────────────────────────────────
// Contiene l'intero stato del giocatore.
// La posizione è memorizzata come coordinate (riga, colonna) sulla mappa ASCII.
// bonus_danno è temporaneo: viene resettato all'inizio di ogni combattimento.
typedef struct {
    char nome[MAX_NOME];
    int  hp;
    int  hp_max;
    int  attacco;     // danno base per colpo
    int  difesa;      // riduzione danni ricevuti
    int  bonus_danno; // bonus temporaneo da amuleti
    int  xp;
    int  livello;
    int  oro;
    Pila inventario;
    int  pos_riga;    // coordinata riga sulla mappa
    int  pos_col;     // coordinata colonna sulla mappa
} Eroe;

#endif