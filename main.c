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

int main(int argc, char *argv[]) {

    // Az inicializáláshoz szükséges változók létrehozása
    srand(time(NULL));
    SDL_Window *window;
    SDL_Renderer *renderer;
    int flags = 0, initted = 0;
    bool quit = false;
    Kellekek *kellekek;
    kellekek = (Kellekek*) malloc(sizeof(Kellekek));

    // Az inicializálás 3 függvényben, külön az ablak, a hang és a kép
    sdl_alap_init(ABLAK, ABLAK, &window, &renderer);
    sdl_hang_init(flags, initted, kellekek);
    sdl_kep_init(renderer, kellekek);

    // Időzítő létrehozása
    SDL_TimerID id = SDL_AddTimer(16, idozit, NULL);

    // Dicsőséglista létrehozása és fájlból beolvasása
    Dicsoseglista *dl = NULL;
    dl = dicsoseglista_beolvas("dicsoseglista.txt", dl);

    // A játékos és az ellenség hajóinak és lövedékeinek, illetve a háttérben lévő csillagok memória lefoglalása
    Hajo *ellenseg, *ellov;
    Csillag* cs;
    ellenseg = (Hajo*) malloc(ELLENSEG_LIMIT * sizeof(Hajo));
    ellov = (Hajo*) malloc(ELLENSEG_LIMIT * sizeof(Hajo));
    cs = (Csillag *) malloc(2 * CSILLAG_SZAM * sizeof(Csillag));

    if ( !ellenseg || !ellov || !cs) {
        perror("Memoriafoglalas sikertelen!");
        exit(40);
    }

    MenuAllapot allapot;
    Jatekos *jatekos;
    jatekos = (Jatekos*) malloc(sizeof(Jatekos));
    jatekos->hajo = (Hajo*) malloc(sizeof(Hajo));
    jatekos->lov = (Hajo*) malloc(sizeof(Hajo));
    jatekos->ellensegszam = 15;

    // Alapértékek beállítása
    adat_reset(renderer, &allapot, cs, jatekos, ellenseg, ellov);

    // A fő ciklus, ami egy menürendszerként funkcionál, aszerint hívja a megfelelő függvényeket
    while (!quit) {
        SDL_Event event;
        SDL_WaitEvent(&event);
        switch (allapot) {

            case fomenu:
                menu_fomenu(renderer, &allapot, event, jatekos, kellekek);
                break;

            case nehezseg:
                menu_nehezseg(renderer, &allapot, event, jatekos, kellekek);
                break;

            case jatek:
                jatek_loop(renderer, &allapot, jatekos, event, cs, ellenseg, ellov, kellekek);
                break;

            case dicsoseglista:
                menu_dicsoseglista(renderer, &allapot, event, dl, kellekek);
                break;

            case gameover:
                menu_gameover(renderer, &allapot, event, cs, ellenseg, ellov, jatekos, dl, kellekek);
                break;

            case kilepes:
                quit = true;
                break;

            default:
                quit = true;
                break;
        }
    }

    //Lefoglalt memória, kép, hang felszabadítása
    TTF_CloseFont(kellekek->font);

    SDL_RemoveTimer(id);
    SDL_DestroyTexture(kellekek->hajokep);
    SDL_DestroyTexture(kellekek->ellensegkep);
    Mix_AllocateChannels(0);
    while(Mix_Init(0))
        Mix_Quit();
    Mix_CloseAudio();
    lista_free(dl);
    free(kellekek);
    free(jatekos->hajo);
    free(jatekos->lov);
    free(jatekos);
    free(ellenseg);
    free(ellov);
    free(cs);
    SDL_Quit();

    return 0;
}
