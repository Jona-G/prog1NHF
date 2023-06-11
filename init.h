#ifndef INIT_H_INCLUDED
#define INIT_H_INCLUDED

typedef struct Hajo {
    int x, y, vx, vy;
    bool van, lott;
} Hajo;

typedef struct Jatekos {
    int menupont, nehez, kor, pont, elet, ellensegszam;
    bool loves, irany_bal, irany_jobb;
    Hajo *hajo, *lov;
} Jatekos;

typedef struct Csillag {
    int x, y, vym, rg, ba;
} Csillag;

typedef struct Dicsoseglista {
    int pont;
    char nev[31];
    struct Dicsoseglista *kov;
} Dicsoseglista;

typedef enum MenuAllapot {
    fomenu,
    nehezseg,
    jatek,
    dicsoseglista,
    gameover,
    kilepes
} MenuAllapot;

typedef struct Kellekek {
    Mix_Chunk *loveshang, *robbanashang, *ellovhang, *elethang;
    SDL_Texture *hajokep, *ellensegkep, *logokep, *felirat_t;
    TTF_Font *font;
    SDL_Surface *felirat;
} Kellekek;

enum { ABLAK = 720 };
enum { HAJO_MERET_X = 90 };
enum { HAJO_MERET_Y = 56 };
enum { ELLENSEG_MERET_X = 48 };
enum { ELLENSEG_MERET_Y = 28 };
enum { ELLENSEG_LIMIT = 100};
enum { CSILLAG_SZAM = 20};

Dicsoseglista *dicsoseglista_beolvas(char *fajlnev, Dicsoseglista *dl);

Dicsoseglista *dicsoseglista_elem_beszur(Dicsoseglista *eleje, int pont, char *nev);
void lista_free(Dicsoseglista *eleje);

void sdl_alap_init(int szeles, int magas, SDL_Window **pwindow, SDL_Renderer **prenderer);
void sdl_hang_init(int flags, int initted, Kellekek *kellekek);
void sdl_kep_init(SDL_Renderer *renderer, Kellekek *kellekek);

Uint32 idozit(Uint32 ms, void *param);

#endif
