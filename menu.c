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

SDL_Color feher = {255, 255, 255}, sarga = {255, 255, 0}, fekete = { 0, 0, 0 };
SDL_Rect hova = {0, 0, 0, 0}, src_ablak = { 0, 0, 400, 400 }, dest_ablak = { ABLAK / 4, - ABLAK / 8, 400, 400 }, r = { ABLAK / 5, 4 * ABLAK / 5, 400, 40 };

// A paraméterként megadott szöveg az ablakra kíírásra kerül az adott helyre, adott színnel, adott betűtípussal.
void szoveg_kiir(SDL_Renderer *renderer, char* szoveg, SDL_Rect hova, int x, int y, TTF_Font *font, SDL_Surface *felirat, SDL_Texture *felirat_t, SDL_Color szin) {
    felirat = TTF_RenderUTF8_Blended(font, szoveg, szin);
    felirat_t = SDL_CreateTextureFromSurface(renderer, felirat);
    hova.x = x;
    hova.y = y;
    hova.w = felirat->w;
    hova.h = felirat->h;
    SDL_RenderCopy(renderer, felirat_t, NULL, &hova);
}

/* Beolvas egy szoveget a billentyuzetrol.
   A rajzolashoz hasznalt font es a megjelenito az utolso parameterek.
   Az elso a tomb, ahova a beolvasott szoveg kerul.
   A masodik a maximális hossz, ami beolvasható.
   A visszateresi erteke logikai igaz, ha sikerult a beolvasas. */
bool input_text(char *dest, size_t hossz, SDL_Rect teglalap, SDL_Color hatter, SDL_Color szoveg, SDL_Renderer *renderer, Kellekek* kellekek) {
    char composition[SDL_TEXTEDITINGEVENT_TEXT_SIZE];
    composition[0] = '\0';
    char textandcomposition[hossz + SDL_TEXTEDITINGEVENT_TEXT_SIZE + 1];
    int maxw = teglalap.w - 2;
    int maxh = teglalap.h - 2;

    dest[0] = '\0';

    bool enter = false;
    bool kilep = false;

    SDL_StartTextInput();
    while (!kilep && !enter) {
        boxRGBA(renderer, teglalap.x, teglalap.y, teglalap.x + teglalap.w - 1, teglalap.y + teglalap.h - 1, hatter.r, hatter.g, hatter.b, 255);
        rectangleRGBA(renderer, teglalap.x, teglalap.y, teglalap.x + teglalap.w - 1, teglalap.y + teglalap.h - 1, szoveg.r, szoveg.g, szoveg.b, 255);
        int w;
        strcpy(textandcomposition, dest);
        strcat(textandcomposition, composition);
        if (textandcomposition[0] != '\0') {
            SDL_Surface *felirat = TTF_RenderUTF8_Blended(kellekek->font, textandcomposition, szoveg);
            SDL_Texture *felirat_t = SDL_CreateTextureFromSurface(renderer, felirat);
            SDL_Rect cel = { teglalap.x, teglalap.y, felirat->w < maxw ? felirat->w : maxw, felirat->h < maxh ? felirat->h : maxh };
            SDL_RenderCopy(renderer, felirat_t, NULL, &cel);
            SDL_FreeSurface(felirat);
            SDL_DestroyTexture(felirat_t);
            w = cel.w;
        } else {
            w = 0;
        }
        if (w < maxw) {
            vlineRGBA(renderer, teglalap.x + w + 2, teglalap.y + 2, teglalap.y + teglalap.h - 3, szoveg.r, szoveg.g, szoveg.b, 192);
        }
        SDL_RenderPresent(renderer);

        SDL_Event event;
        SDL_WaitEvent(&event);
        switch (event.type) {
            case SDL_KEYDOWN:
                if (event.key.keysym.sym == SDLK_BACKSPACE) {
                    int textlen = strlen(dest);
                    do {
                        if (textlen == 0) {
                            break;
                        }
                        if ((dest[textlen-1] & 0x80) == 0x00)   {
                            dest[textlen-1] = 0x00;
                            break;
                        }
                        if ((dest[textlen-1] & 0xC0) == 0x80) {
                            dest[textlen-1] = 0x00;
                            textlen--;
                        }
                        if ((dest[textlen-1] & 0xC0) == 0xC0) {
                            dest[textlen-1] = 0x00;
                            break;
                        }
                    } while(true);
                }
                if (event.key.keysym.sym == SDLK_RETURN) {
                    enter = true;
                }
                break;

            case SDL_TEXTINPUT:
                if (strlen(dest) + strlen(event.text.text) < hossz) {
                    strcat(dest, event.text.text);
                }

                composition[0] = '\0';
                break;

            case SDL_TEXTEDITING:
                strcpy(composition, event.edit.text);
                break;

            case SDL_QUIT:
                SDL_PushEvent(&event);
                kilep = true;
                break;
        }
    }

    SDL_StopTextInput();
    return enter;
}

/* A főmenüt lebonyolító függvény.
   A játék logója és a három kiválasztási lehetőség kerül kiírásra.
   Ezek közül egyet kiválasztva a játékos a következő menüpontra kerül.*/
void menu_fomenu(SDL_Renderer *renderer, MenuAllapot* allapot, SDL_Event event, Jatekos* jatekos, Kellekek* kellekek) {
    switch (event.type) {
        case SDL_USEREVENT:

            boxRGBA(renderer, 0, 0, ABLAK, ABLAK, 0, 0, 0, 255);

            SDL_RenderCopy(renderer, kellekek->logokep, &src_ablak, &dest_ablak);

            szoveg_kiir(renderer, "\xC3\x9Aj j\xc3\xA1t\xC3\xA9k", hova, ABLAK / 6, 3 * ABLAK / 8, kellekek->font, kellekek->felirat, kellekek->felirat_t, feher);
            szoveg_kiir(renderer, "Dics\xC5\x91s\xC3\xA9glista", hova, ABLAK / 6, 5 * ABLAK / 8, kellekek->font, kellekek->felirat, kellekek->felirat_t, feher);
            szoveg_kiir(renderer, "Kil\xC3\xA9p\xC3\xA9s", hova, ABLAK / 6, 7 * ABLAK / 8, kellekek->font, kellekek->felirat, kellekek->felirat_t, feher);

            switch (jatekos->menupont) {
                case 0 /* Új játék*/ :
                    szoveg_kiir(renderer, "\xC3\x9Aj j\xc3\xA1t\xC3\xA9k", hova, ABLAK / 6, 3 * ABLAK / 8, kellekek->font, kellekek->felirat, kellekek->felirat_t, sarga);
                    break;

                case 1 /* Dicsőséglista */ :
                    szoveg_kiir(renderer, "Dics\xC5\x91s\xC3\xA9glista", hova, ABLAK / 6, 5 * ABLAK / 8, kellekek->font, kellekek->felirat, kellekek->felirat_t, sarga);
                    break;

                case 2 /* Kilépés */ :
                    szoveg_kiir(renderer, "Kil\xC3\xA9p\xC3\xA9s", hova, ABLAK / 6, 7 * ABLAK / 8, kellekek->font, kellekek->felirat, kellekek->felirat_t, sarga);
                    break;
            }

            SDL_RenderPresent(renderer);
            break;

        case SDL_KEYDOWN:
            switch (event.key.keysym.sym) {
                case SDLK_UP:
                    if (jatekos->menupont == 0)
                        jatekos->menupont = 2;
                    else
                        --jatekos->menupont;
                    break;

                case SDLK_DOWN:
                    if (jatekos->menupont == 2)
                        jatekos->menupont = 0;
                    else
                        ++jatekos->menupont;
                    break;

                case SDLK_RETURN:
                    switch (jatekos->menupont) {
                        case 0 /*Új játék*/:
                            *allapot = nehezseg;
                            break;
                        case 1 /*Dicsõséglista*/:
                            *allapot = dicsoseglista;
                            break;
                        case 2 /*Kilépés*/:
                            *allapot = kilepes;
                            break;
                    }
                    break;

                case SDLK_ESCAPE:
                    *allapot = kilepes;
                    break;
            break;
            }
        break;
    }
}

/* A nehézség kiválasztási menüben a játékos két nehézség közül választhat, és a játék aszerint zajlik le.
   A nehezebb fokozatban több ellenség lesz, azok gyorsabban közelítenek a játékos felé, és többet is lőnek.*/
void menu_nehezseg(SDL_Renderer *renderer, MenuAllapot* allapot, SDL_Event event, Jatekos* jatekos, Kellekek* kellekek) {
    switch (event.type) {
        case SDL_USEREVENT:

            boxRGBA(renderer, 0, 0, ABLAK, ABLAK, 0, 0, 0, 255);

            if (jatekos->menupont == 0) {
                szoveg_kiir(renderer, "K\xC3\xB6zepes", hova, ABLAK / 6, ABLAK / 3, kellekek->font, kellekek->felirat, kellekek->felirat_t, sarga);           // Közepes
                szoveg_kiir(renderer, "Nehez\xC3\xADtett", hova, ABLAK / 6, 2 * ABLAK / 3, kellekek->font, kellekek->felirat, kellekek->felirat_t, feher);    // Nehezített
           } else {
                szoveg_kiir(renderer, "K\xC3\xB6zepes", hova, ABLAK / 6, ABLAK / 3, kellekek->font, kellekek->felirat, kellekek->felirat_t, feher);
                szoveg_kiir(renderer, "Nehez\xC3\xADtett", hova, ABLAK / 6, 2 * ABLAK / 3, kellekek->font, kellekek->felirat, kellekek->felirat_t, sarga);
           }

            SDL_RenderPresent(renderer);
            break;

        case SDL_KEYDOWN:
            switch (event.key.keysym.sym) {
                case SDLK_UP:
                case SDLK_DOWN:
                    if (jatekos->menupont == 0)
                        jatekos->menupont = 1;
                    else
                        jatekos->menupont = 0;
                    break;

                case SDLK_RETURN:
                    if (jatekos->menupont == 0)
                        jatekos->nehez = 0;
                    else
                        jatekos->nehez = 1;
                    *allapot = jatek;
                    break;

                case SDLK_ESCAPE:
                    jatekos->menupont = 0;
                    *allapot = fomenu;
                    break;
            }
        break;
    }
}

/* A dicsőséglistát kiíró függvényben a szöveges fájlból beolvasott 5 legjobb játékos neve és pontszáma kerül kiírásra.*/
void menu_dicsoseglista(SDL_Renderer *renderer, MenuAllapot* allapot, SDL_Event event, Dicsoseglista* dl, Kellekek* kellekek) {
    switch (event.type) {
        case SDL_USEREVENT:

            boxRGBA(renderer, 0, 0, ABLAK, ABLAK, 0, 0, 0, 255);
            szoveg_kiir(renderer, "Dics\xC5\x91s\xC3\xA9glista", hova, ABLAK / 3, ABLAK / 10, kellekek->font, kellekek->felirat, kellekek->felirat_t, sarga);

            Dicsoseglista *mozgo;
            int i = 0;
            for (mozgo = dl; mozgo != NULL; mozgo = mozgo->kov) {
                char s[100];
                itoa(i+1, s, 10);
                strcat(s, ". ");
                strcat(s, mozgo->nev);

                szoveg_kiir(renderer, s, hova, ABLAK / 8, (i+2) * ABLAK / 7, kellekek->font, kellekek->felirat, kellekek->felirat_t, feher);

                szoveg_kiir(renderer, itoa(mozgo->pont, s , 10), hova, 3 * ABLAK / 4, (i+2) * ABLAK / 7, kellekek->font, kellekek->felirat, kellekek->felirat_t, feher);
                ++i;
            }

            SDL_RenderPresent(renderer);

            break;

        case SDL_KEYDOWN:
            switch (event.key.keysym.sym) {
                case SDLK_ESCAPE:
                    *allapot = fomenu;
                    break;
            }
            break;
    }
}

/* Ha a játékos minden életét elvesztette, vagy az ellenség leért a pálya aljára, akkor kerül ide.
   Itt tudatja a program a játékossal, hogy a játéknak vége, és kiírja az elért pontszámot.
   Ha ez a pontszám több mint az 5. helyezeté, akkor bekerült a dicsőséglistába, és a neve beírásával ezt
   el is könyvelheti a felhasználó, mivel az a szöveges fájlba beírásra kerül.*/
void menu_gameover(SDL_Renderer *renderer, MenuAllapot* allapot, SDL_Event event, Csillag* cs,
                   Hajo* ellenseg, Hajo* ellov, Jatekos* jatekos, Dicsoseglista* dl, Kellekek* kellekek) {
    bool volt = false;
    switch (event.type) {
        case SDL_USEREVENT:
            boxRGBA(renderer, 0, 0, ABLAK, ABLAK, 0, 0, 0, 255);

            szoveg_kiir(renderer, "J\xc3\xA1t\xC3\xA9k v\xC3\xA9ge", hova, ABLAK / 3, ABLAK / 5, kellekek->font, kellekek->felirat, kellekek->felirat_t, feher);  // Játék vége

            // "Elért pontszám"
            char s[30] = "El\xC3\xA9rt pontsz\xc3\xA1m: ";
            itoa(jatekos->pont, s + strlen(s), 10);
            szoveg_kiir(renderer, s, hova, ABLAK / 4, 2 * ABLAK / 5, kellekek->font, kellekek->felirat, kellekek->felirat_t, feher);
            // Ha az elért pontszám nagyobb mint az 5. helyezeté...
            Dicsoseglista *mozgo;
            for (mozgo = dl; mozgo != NULL; mozgo = mozgo->kov)
                if ((jatekos->pont > mozgo->pont) && !volt) {
                    // "Bekerültél a dicsőséglistába"
                    szoveg_kiir(renderer, "Beker\xC3\xBClt\xC3\xA9l a dics\xC5\x91s\xC3\xA9glistaba!", hova, ABLAK / 7, 3 * ABLAK / 5, kellekek->font, kellekek->felirat, kellekek->felirat_t, feher);
                    char *szoveg = NULL;
                    szoveg = (char*) malloc(30 * sizeof(char));
                    // Itt írhatja be a játékos a nevét.
                    input_text(szoveg, 100, r, fekete, feher, renderer, kellekek);
                    Dicsoseglista *mozgo2 = dl;
                    // Itt derül ki hogy hanyadik is lett a játékos...
                    while ((jatekos->pont < mozgo2->kov->pont) && (mozgo2->kov != NULL))
                        mozgo2 = mozgo2->kov;
                    Dicsoseglista *temp1;
                    temp1 = (Dicsoseglista*) malloc(sizeof(Dicsoseglista));
                    temp1->pont = jatekos->pont;
                    strcpy(temp1->nev, szoveg);
                    temp1->kov = mozgo2->kov;
                    mozgo2->kov = temp1;
                    int sz = 0;
                    FILE* fp;
                    fp = fopen("dicsoseglista.txt", "w");
                    if (!fp)
                        perror("A dicsoseglista megnyitasa nem sikerult!");
                    else
                        for (mozgo2 = dl; mozgo2 != NULL; mozgo2 = mozgo2->kov)
                            if (sz < 5) {
                                fprintf(fp, "%d %s\n", mozgo2->pont, mozgo2->nev);
                                ++sz;
                            }
                    fclose(fp);
                    free(szoveg);
                    volt = true;
                    adat_reset(renderer, allapot, cs, jatekos, ellenseg, ellov);
                }

            SDL_RenderPresent(renderer);

            break;

        case SDL_KEYDOWN:
            switch (event.key.keysym.sym) {
                case SDLK_ESCAPE:
                    adat_reset(renderer, allapot, cs, jatekos, ellenseg, ellov);
                    break;
            }
            break;
    }
}
