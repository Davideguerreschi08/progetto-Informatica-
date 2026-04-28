#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/tipi.h"
#include "../include/mappa.h"

// ─── VARIABILI GLOBALI ────────────────────────────────────────────────────────

Stanza *tutte_stanze[MAX_STANZE];
int num_stanze = 0;

// ─── FUNZIONI AUSILIARIE ──────────────────────────────────────────────────────

/* Crea una nueva stanza con parametri di default */
static Stanza* crea_stanza(int id, const char *nome, const char *descrizione) {
    Stanza *s = malloc(sizeof(Stanza));
    if (!s) return NULL;
    
    s->id = id;
    strncpy(s->nome, nome, MAX_NOME - 1);
    s->nome[MAX_NOME - 1] = '\0';
    strncpy(s->descrizione, descrizione, MAX_DESCRIZIONE - 1);
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

/* Crea un nuovo mostro */
static Mostro* crea_mostro(const char *nome, TipoMostro tipo, int hp, 
                           int attacco, int difesa, int xp, int oro) {
    Mostro *m = malloc(sizeof(Mostro));
    if (!m) return NULL;
    
    strncpy(m->nome, nome, MAX_NOME - 1);
    m->nome[MAX_NOME - 1] = '\0';
    m->tipo = tipo;
    m->hp = hp;
    m->hp_max = hp;
    m->attacco = attacco;
    m->difesa = difesa;
    m->xp_ricompensa = xp;
    m->oro_ricompensa = oro;
    m->vivo = 1;
    
    return m;
}

/* Crea un nuovo oggetto */
static Oggetto* crea_oggetto(const char *nome, TipoOggetto tipo, int valore) {
    Oggetto *o = malloc(sizeof(Oggetto));
    if (!o) return NULL;
    
    strncpy(o->nome, nome, MAX_NOME - 1);
    o->nome[MAX_NOME - 1] = '\0';
    o->tipo = tipo;
    o->valore = valore;
    o->next = NULL;
    
    return o;
}

/* Aggiunge un oggetto alla stanza */
static void aggiungi_oggetto_stanza(Stanza *stanza, Oggetto *obj) {
    if (!stanza || !obj) return;
    
    obj->next = stanza->oggetti;
    stanza->oggetti = obj;
}

// ─── FUNZIONI PRINCIPALI ──────────────────────────────────────────────────────

/* Costruisce il dungeon: crea le stanze e le collega in un grafo */
Stanza *costruisci_mappa(void) {
    // Creiamo le 8 stanze del dungeon
    Stanza *entrata = crea_stanza(0, "ENTRATA DEL DUNGEON",
        "Sei di fronte a un'enorme porta di pietra. Dentro soffia un vento freddo.\n"
        "Puoi andare a NORD (corridoio principale) o a EST (corridoio laterale).");
    
    Stanza *corridoio_principale = crea_stanza(1, "CORRIDOIO PRINCIPALE",
        "Un lungo corridoio fiancheggiato da torce blu. Senti strani rumori.\n"
        "Puoi andare a SUD (entrata), NORD (camera del tesoro) o OVEST (camera nascosta).");
    
    Stanza *camera_tesoro = crea_stanza(2, "CAMERA DEL TESORO",
        "Una grande sala con colonne di marmo. Qui riposa il BOSS finale!\n"
        "Puoi andare a SUD (corridoio principale).");
    
    Stanza *corridoio_laterale = crea_stanza(3, "CORRIDOIO LATERALE",
        "Un corridoio stretto e umido. Vedi tracce di artigli sulle muri.\n"
        "Puoi andare a OVEST (entrata) o a NORD (camera dei goblin).");
    
    Stanza *camera_goblin = crea_stanza(4, "CAMERA DEI GOBLIN",
        "Una sala puzzolente piena di ossa e rifiuti. Demoni di fuoco sorvegliano il tesoro.\n"
        "Puoi andare a SUD (corridoio laterale).");
    
    Stanza *camera_scheletri = crea_stanza(5, "CAMERA DEGLI SCHELETRI",
        "Una cripta con bare di pietra. Gli scheletri camminano in cerchio.\n"
        "Puoi andare a NORD (torre del mago) o a SUD (corridoio principale).");
    
    Stanza *torre_mago = crea_stanza(6, "TORRE DEL MAGO",
        "Una stanza con scaffali di libri e pozioni. L'aria puzza di magia.\n"
        "Puoi andare a SUD (camera degli scheletri).");
    
    Stanza *camera_nascosta = crea_stanza(7, "CAMERA NASCOSTA",
        "Una stanza segreta scoperta! Qui riposano i migliori tesori e una chiave dorata.\n"
        "Puoi andare a EST (corridoio principale).");
    
    // Salviamo i puntatori nell'array globale
    tutte_stanze[0] = entrata;
    tutte_stanze[1] = corridoio_principale;
    tutte_stanze[2] = camera_tesoro;
    tutte_stanze[3] = corridoio_laterale;
    tutte_stanze[4] = camera_goblin;
    tutte_stanze[5] = camera_scheletri;
    tutte_stanze[6] = torre_mago;
    tutte_stanze[7] = camera_nascosta;
    num_stanze = 8;
    
    // ─── COLLEGAMENTO DELLE STANZE (il grafo) ───────────────────────────────
    
    // ENTRATA (0)
    entrata->nord = corridoio_principale;
    entrata->est = corridoio_laterale;
    
    // CORRIDOIO PRINCIPALE (1)
    corridoio_principale->sud = entrata;
    corridoio_principale->nord = camera_tesoro;
    corridoio_principale->ovest = camera_nascosta;
    
    // CAMERA DEL TESORO (2) - BOSS
    camera_tesoro->sud = corridoio_principale;
    
    // CORRIDOIO LATERALE (3)
    corridoio_laterale->ovest = entrata;
    corridoio_laterale->nord = camera_goblin;
    
    // CAMERA DEI GOBLIN (4)
    camera_goblin->sud = corridoio_laterale;
    
    // CAMERA DEGLI SCHELETRI (5)
    camera_scheletri->nord = torre_mago;
    camera_scheletri->sud = corridoio_principale;
    
    // TORRE DEL MAGO (6)
    torre_mago->sud = camera_scheletri;
    
    // CAMERA NASCOSTA (7)
    camera_nascosta->est = corridoio_principale;
    
    // ─── AGGIUNTA NEMICI ─────────────────────────────────────────────────────
    
    // 2 Goblin in camera_goblin
    camera_goblin->mostro = crea_mostro("Goblin Guerriero", GOBLIN, 20, 8, 3, 50, 30);
    
    // 1 Scheletro in camera_scheletri
    camera_scheletri->mostro = crea_mostro("Scheletro Antico", SCHELETRO, 25, 7, 4, 60, 40);
    
    // 1 Mago in torre_mago
    torre_mago->mostro = crea_mostro("Mago Oscuro", MAGO, 30, 12, 5, 100, 60);
    
    // 1 Drago-Scheletro in corridoio_principale
    corridoio_principale->mostro = crea_mostro("Drago-Scheletro", DRAGO_SCHELETRO, 40, 14, 7, 150, 100);
    
    // 1 BOSS in camera_tesoro
    camera_tesoro->mostro = crea_mostro("LICH SUPREMO", BOSS, 80, 18, 10, 500, 200);
    
    // ─── AGGIUNTA OGGETTI ────────────────────────────────────────────────────
    
    // ENTRATA: pozione e torcia
    aggiungi_oggetto_stanza(entrata, crea_oggetto("Pozione di Guarigione", POZIONE, 20));
    aggiungi_oggetto_stanza(entrata, crea_oggetto("Torcia", TORCIA, 1));
    
    // CORRIDOIO PRINCIPALE: armatura scura
    aggiungi_oggetto_stanza(corridoio_principale, crea_oggetto("Armatura di Ferro", ARMATURA, 5));
    
    // CAMERA DEL TESORO: spada leggendaria e amuleto
    aggiungi_oggetto_stanza(camera_tesoro, crea_oggetto("Spada Leggendaria", ARMA, 20));
    aggiungi_oggetto_stanza(camera_tesoro, crea_oggetto("Amuleto della Protezione", AMULETO, 10));
    
    // CORRIDOIO LATERALE: pozione veleno
    aggiungi_oggetto_stanza(corridoio_laterale, crea_oggetto("Pozione di Veleno", POZIONE_VELENO, 15));
    
    // CAMERA DEI GOBLIN: bomba e chiave
    aggiungi_oggetto_stanza(camera_goblin, crea_oggetto("Bomba a Mano", BOMBA, 30));
    aggiungi_oggetto_stanza(camera_goblin, crea_oggetto("Chiave di Ferro", CHIAVE, 1));
    
    // CAMERA DEGLI SCHELETRI: pozione alta guarigione
    aggiungi_oggetto_stanza(camera_scheletri, crea_oggetto("Pozione di Grande Guarigione", POZIONE, 40));
    
    // TORRE DEL MAGO: armatura magica
    aggiungi_oggetto_stanza(torre_mago, crea_oggetto("Armatura Magica", ARMATURA, 8));
    aggiungi_oggetto_stanza(torre_mago, crea_oggetto("Libro di Magia", AMULETO, 5));
    
    // CAMERA NASCOSTA: il tesoro finale!
    aggiungi_oggetto_stanza(camera_nascosta, crea_oggetto("Chiave Dorata", CHIAVE, 5));
    aggiungi_oggetto_stanza(camera_nascosta, crea_oggetto("Corona del Re", AMULETO, 15));
    aggiungi_oggetto_stanza(camera_nascosta, crea_oggetto("Spada di Platino", ARMA, 25));
    
    return entrata;  // Ritorna la stanza iniziale
}

/* Libera tutta la memoria allocata per il dungeon */
void distruggi_mappa(Stanza **stanze, int n) {
    if (!stanze) return;
    
    for (int i = 0; i < n; i++) {
        if (!stanze[i]) continue;
        
        // Libera tutti gli oggetti della stanza
        Oggetto *obj = stanze[i]->oggetti;
        while (obj) {
            Oggetto *temp = obj;
            obj = obj->next;
            free(temp);
        }
        
        // Libera il mostro (se c'è)
        if (stanze[i]->mostro) {
            free(stanze[i]->mostro);
        }
        
        // Libera la stanza stessa
        free(stanze[i]);
    }
}

/* Stampa una mappa ASCII visualizzando le stanze visitate */
void stampa_mappa(Stanza **stanze, int n, Stanza *corrente) {
    if (!stanze || n <= 0) {
        printf("Mappa non disponibile.\n");
        return;
    }
    
    printf("\n╔════════════════════════════════════════════════════════════╗\n");
    printf("║              MAPPA DEL DUNGEON ESPLORATO                   ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n\n");
    
    // Layout ASCII del dungeon
    printf("                      [TORRE DEL MAGO (6)]\n");
    printf("                             |\n");
    printf("  [CAMERA NASCOSTA]---[CAMERA SCHELETRI (5)]\n");
    printf("        (7)                |\n");
    printf("         |                 |\n");
    printf("[CORRIDOIO PRINCIPALE (1)]-|\n");
    printf("     |           |\n");
    printf("     |       [CAMERA TESORO (2)]\n");
    printf("     |       [BOSS LICH]\n");
    printf(" [ENTRATA (0)]\n");
    printf("     |\n");
    printf("[CORRIDOIO LATERALE (3)]\n");
    printf("     |\n");
    printf("[CAMERA DEI GOBLIN (4)]\n\n");
    
    // Legenda con stanze visitate
    printf("Legenda:\n");
    printf("  ✓ = visitata\n");
    printf("  → = stanza attuale\n");
    printf("  ✗ = non visitata\n\n");
    
    for (int i = 0; i < n; i++) {
        Stanza *s = stanze[i];
        if (!s) continue;
        
        printf("[%d] %s ", s->id, s->nome);
        
        if (s == corrente) {
            printf(" → ATTUALE\n");
        } else if (s->visitata) {
            printf(" ✓ visitata\n");
        } else {
            printf(" ✗ non visitata\n");
        }
    }
    
    printf("\n");
}
