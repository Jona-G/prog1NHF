#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL2_gfxPrimitives.h>

#include "init.h"
#include "menu.h"
#include "jatek.h"

#include "debugmalloc.h"

/* Ez a függvény végzi el a dicsőséglista beolvasását egy szöveges fájlból.
   Amennyiben ez a fájl nem létezik, úgy létrehozza azt, előre beállított nevekkel és pontszámokkal.
   Ha létezik akkor onnan beolvassa az 5 pontszámot és a hozzá tartozó nevet is.*/
Dicsoseglista *dicsoseglista_beolvas(char *fajlnev, Dicsoseglista *dl) {
    FILE *fp = fopen(fajlnev, "r+");
    int c = fgetc(fp);
    Dicsoseglista *mozgo;
    if (c == EOF) {
        dl = dicsoseglista_elem_beszur(dl, 2000, "LegendaLaura");
        dl = dicsoseglista_elem_beszur(dl, 1600, "ProfiPeter");
        dl = dicsoseglista_elem_beszur(dl, 1200, "HaladoHunor");
        dl = dicsoseglista_elem_beszur(dl, 800, "AtlagosAnna");
        dl = dicsoseglista_elem_beszur(dl, 400, "KezdoKazmer");

        fclose(fp);
        fp = fopen(fajlnev, "w+");
        for (mozgo = dl; mozgo != NULL; mozgo = mozgo->kov)
            fprintf(fp, "%d %s\n", mozgo->pont, mozgo->nev);
    } else {
        ungetc(c, fp);
        for (int i = 0; i < 5; ++i) {
            int temp_pont;
            char temp_nev[31];
            fscanf(fp, " %d %[^\n]s", &temp_pont, temp_nev);
            dl = dicsoseglista_elem_beszur(dl, temp_pont, temp_nev);
        }
    }
    fclose(fp);
    return dl;
}

Dicsoseglista *dicsoseglista_elem_beszur(Dicsoseglista *eleje, int pont, char *nev) {
    Dicsoseglista *uj = (Dicsoseglista *) malloc(sizeof(Dicsoseglista));

    uj->pont = pont;
    strcpy(uj->nev, nev);
    uj->kov = NULL;
    if (eleje == NULL) {
        return uj;
    } else {
        Dicsoseglista *p = eleje;
        while (p->kov != NULL) p = p->kov;
        p->kov = uj;
        return eleje;
    }
}

void lista_free(Dicsoseglista *eleje) {
    Dicsoseglista *p = eleje;
    while (p != NULL) {
        Dicsoseglista *tmp = p->kov;
        free(p);
        p = tmp;
    }
}

// Itt megtörténik az SDL grafikus környezet inicializálása, a megfelelő hibakezelésekkel együtt.
void sdl_alap_init(int szeles, int magas, SDL_Window **pwindow, SDL_Renderer **prenderer) {
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        SDL_Log("Nem indithato az SDL: %s", SDL_GetError());
        exit(10);
    }

    SDL_Window *window = SDL_CreateWindow("Space Invaders", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, szeles, magas, 0);
    if (window == NULL) {
        SDL_Log("Nem hozhato letre az ablak: %s", SDL_GetError());
        exit(11);
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    if (renderer == NULL) {
        SDL_Log("Nem hozhato letre a megjelenito: %s", SDL_GetError());
        exit(12);
    }

    SDL_RenderClear(renderer);
    *pwindow = window;
    *prenderer = renderer;
}

// Itt megtörténik az SDL grafikus környezet által használt hangok inicializálása, a megfelelő hibakezelésekkel együtt.
void sdl_hang_init(int flags, int initted, Kellekek *kellekek) {
    flags = MIX_INIT_OGG;
    initted = Mix_Init(flags);
    if (initted != flags) {
        SDL_Log("Nem nyithato meg a hangfajl: %s", Mix_GetError());
        exit(20);
    }

    if (SDL_Init(SDL_INIT_AUDIO) == -1) {
        SDL_Log("Problema a hang inicializalasaval: %s", SDL_GetError());
        exit(21);
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024) == -1) {
        SDL_Log("Problema a hangformatum beallitasaval: %s", Mix_GetError());
        exit(22);
    }

    kellekek->loveshang = Mix_LoadWAV("loves.ogg");
    kellekek->robbanashang = Mix_LoadWAV("robbanas.ogg");
    kellekek->ellovhang = Mix_LoadWAV("ellov.ogg");
    kellekek->elethang = Mix_LoadWAV("elet.ogg");
    if (!kellekek->loveshang || !kellekek->robbanashang || !kellekek->ellovhang || !kellekek->elethang) {
        SDL_Log("Nem nyithatok meg a hangfajlok: %s", Mix_GetError());
        exit(23);
    }

    Mix_AllocateChannels(4);
    Mix_Volume(-1, 64);
}

// Itt megtörténik az SDL grafikus környezet által használt képek inicializálása, a megfelelő hibakezelésekkel együtt.
void sdl_kep_init(SDL_Renderer *renderer, Kellekek *kellekek) {
    kellekek->hajokep = IMG_LoadTexture(renderer, "hajo.png");
    kellekek->ellensegkep = IMG_LoadTexture(renderer, "ellenseg.png");
    kellekek->logokep = IMG_LoadTexture(renderer, "logo.png");
    if ((kellekek->hajokep == NULL) || (kellekek->ellensegkep == NULL) || (kellekek->logokep == NULL)) {
        SDL_Log("Nem nyithatok meg a kepfajlok: %s", IMG_GetError());
        exit(30);
    }

    TTF_Init();
    kellekek->font = TTF_OpenFont("LiberationSerif-Regular.ttf", 48);
    if (!kellekek->font) {
        SDL_Log("Nem sikerult megnyitni a fontot! %s\n", TTF_GetError());
        exit(31);
    }
}

/*Ez a függvény egy idõzítõt hoz létre, mely megkapja paraméterként azt, hogy milyen idõközökre
  lesz beállítva, és a tetszõleges felhasználói paramétert. Visszatérési értéke egy egész szám,
  ami megmondja, hogy legközelebb hány ezredmásodperc múlva hívódjon meg.*/
Uint32 idozit(Uint32 ms, void *param) {
    SDL_Event ev;
    ev.type = SDL_USEREVENT;
    SDL_PushEvent(&ev);
    return ms;
}
