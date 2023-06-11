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

/*A fő játékciklus függvénye, amely megkap minden szükséges struktúrát, képet és hangot.
  Ez frissít minden grafikus és eltárolt adatot.*/
void jatek_loop(SDL_Renderer *renderer, MenuAllapot* allapot, Jatekos* jatekos, SDL_Event event, Csillag* cs, Hajo* ellenseg, Hajo* ellov, Kellekek* kellekek) {

    switch (event.type) {
        case SDL_USEREVENT:

            hatter_rajzol(renderer, cs, jatekos);

            hajo_mozgas(event, jatekos);

            hajo_loves(renderer, jatekos);

            hajo_rajzol(renderer, kellekek->hajokep, jatekos->hajo->x, jatekos->hajo->y, HAJO_MERET_X, HAJO_MERET_Y);

            // Minden ellenséget külön kell kezelni (kirajzolni, logikát elvégezni, lövést lebonyolítani)
            for (int i = 0; i < jatekos->ellensegszam; ++i)
                if (ellenseg[i].van) {
                    hajo_rajzol(renderer, kellekek->ellensegkep, ellenseg[i].x, ellenseg[i].y, ELLENSEG_MERET_X, ELLENSEG_MERET_Y);
                    ellenseg_logika(&ellov[i], &ellenseg[i], allapot, jatekos, kellekek);
                    ellenseg_loves(renderer, &ellenseg[i], &ellov[i], allapot, jatekos);
                }

            kor_vege_check(ellenseg, ellov, jatekos);

            SDL_RenderPresent(renderer);

        break;

        // A lenyomott gomb alapján állítja a megfelelő értéket igazra
        case SDL_KEYDOWN:
            switch (event.key.keysym.sym) {
                case SDLK_LEFT:  jatekos->irany_bal = true; break;
                case SDLK_RIGHT: jatekos->irany_jobb = true; break;
                case SDLK_SPACE:
                    if (!jatekos->loves) {
                        Mix_PlayChannel(-1, kellekek->loveshang, 0);
                        jatekos->loves = true;
                        jatekos->lov->x = jatekos->hajo->x;
                        jatekos->lov->y = jatekos->hajo->y;
                    }
                break;
            }
        break;

        // Az elengedett gomb hatására az abba az irányba történő mozgás megszűnik
        // Az "escape" gomb megnyomása majd elengedése után a játékos visszatér a főmenübe
        case SDL_KEYUP:
            switch (event.key.keysym.sym) {
                case SDLK_LEFT:  jatekos->irany_bal = false; break;
                case SDLK_RIGHT: jatekos->irany_jobb = false; break;
                case SDLK_ESCAPE:
                    adat_reset(renderer, allapot, cs, jatekos, ellenseg, ellov);
                break;
            }
        break;
    }
}

// Az adatokat alapértékekre állító függvény. Ez megkapja a játékban használt struktúrákat és a menü állapotát.
// A struktúrákban tárolt adatokat olyanra állítja, hogy az új játék indításánál megfelelő legyen.
// Ezek mellett a játékos a főmenübe tér vissza.
void adat_reset(SDL_Renderer *renderer, MenuAllapot* allapot, Csillag *cs, Jatekos* jatekos, Hajo* ellenseg, Hajo* ellov) {

    // A játékos 3 élettel kezd, minden más adata 0.
    jatekos->menupont = jatekos->kor = 0;
    jatekos->pont = 2000;
    jatekos->elet = 3;

    // A játékos hajója a képernyő alján, a vízszintes tengely közepéről indul.
    jatekos->hajo->x = jatekos->lov->x = (ABLAK - HAJO_MERET_X) / 2;
    jatekos->hajo->y = jatekos->lov->y = ABLAK - HAJO_MERET_Y - 8;
    jatekos->hajo->vx = jatekos->hajo->vy = jatekos->lov->x = jatekos->lov->y = jatekos->lov->vx = jatekos->lov->vy = 0;

    // Az ellenségek pozíciója kinullázásra kerül, a csillagok randomizálása megtörténik.
    for (int i = 0; i < jatekos->ellensegszam; ++i) {
        ellenseg[i].x = ellenseg[i].y = ellenseg[i].vx = ellenseg[i].vy = 0;
        ellov[i].x = ellov[i].y = ellov[i].vx = ellov[i].vy = 0;
        ellenseg[i].van = false;
        ellenseg[i].lott = false;
        csillag_rand(&cs[i]);
    }
    // Az ellenségszámot alaphelyzetbe állítjuk.
    jatekos->ellensegszam = 15;
    jatekos->loves = jatekos->irany_bal = jatekos->irany_jobb = false;

    // Visszakerülünk a főmenübe
    *allapot = fomenu;
}

/*Ez a függvény rajzol a paraméterként megkapott megjelenítõ rendererre egy programablak hosszú
  fekete négyzetet, majd azután a paraméterként megkapott Csillag struktúra elemeit.
  Amennyiben egy csillag elérte az ablak alját, akkor az visszakerül az ablak tetejére,
  véletlenszerû értékekkel. Ha nem, akkor az y-sebességével kerül lejjebb.
  Továbbá ez a függvény végzi el az adott pont, kör és életek számának képernyőre való kiírását is.*/
void hatter_rajzol(SDL_Renderer *renderer, Csillag *cs, Jatekos* jatekos) {
    char s_pont[10] = "Pont: ";
    char s_kor[10] = "Kor: ";
    char s_elet[10] = "Elet: ";
    boxColor(renderer, 0, 0, ABLAK, ABLAK, 0x000000FF);
    for (int i = 0; i < CSILLAG_SZAM; ++i) {
        if (cs[i].y < ABLAK) {
            cs[i].y += cs[i].vym;
            filledCircleRGBA(renderer, cs[i].x, cs[i].y, cs[i].vym, cs[i].rg, cs[i].rg, cs[i].ba, cs[i].ba);
        } else csillag_rand(&cs[i]);
    }

    itoa(jatekos->pont, s_pont + 6, 10);
    itoa(jatekos->kor, s_kor + 5, 10);
    itoa(jatekos->elet, s_elet + 6, 10);
    stringRGBA(renderer, 8, 8, s_pont, 255, 255, 255, 255);
    stringRGBA(renderer, 8, 24, s_kor, 255, 255, 255, 255);
    stringRGBA(renderer, 8, 40, s_elet, 255, 255, 255, 255);

}

/* Ez a függvény a paraméterként megkapott Csillag struktúra adatait randomizálja.
   Ez akkor kerül meghívásra ha egy csillag elérte a képernyő alját, így az az ablak
   tetejére kerül új értékekkel.*/
void csillag_rand(Csillag *cs) {
    cs->x = rand() % ABLAK;
    cs->y = (rand() % 100) - 100;
    cs->vym = 3 + rand() % 5;
    cs->rg = 192 + rand() % 64;
    cs->ba = 32 + rand() % 64;
}

/*Ez a függvény felelõs a játékos által irányított hajó mozgásáért.
  Ha a hajó az ablak szélein kívülre kerül, akkor azt az ablak szélére helyezi vissza.
  A hajó vízszintes sebessége alapból 0, ha valamelyik iránygomb le van nyomva, akkor a hajó
  abba az irányba kap sebességet. Végül a hajó koordinátájához hozzáadódik a sebesség.*/
void hajo_mozgas(SDL_Event event, Jatekos* jatekos) {
    switch (event.type) {
        case SDL_USEREVENT:
            if (jatekos->hajo->x < 0) jatekos->hajo->x = 0;
            if (jatekos->hajo->x > ABLAK - HAJO_MERET_X) jatekos->hajo->x = ABLAK - HAJO_MERET_X;

            jatekos->hajo->vx = 0;
            if (jatekos->irany_bal) jatekos->hajo->vx = -8;
            if (jatekos->irany_jobb) jatekos->hajo->vx = 8;
            jatekos->hajo->x += jatekos->hajo->vx;
            break;
    }
}

/*Ez a függvény bonyolítja le a játékos hajója által kiadott lövés logikáját.
  Ha a lövés a játéktér keretein belül van, akkor annak koordinátájához hozzáadódik a 30-ig
  ötösével gyorsuló sebességhez. Ellenkezõ esetben a lövés koordinátái kinullázódnak,
  és a játékos újra lövésre készen áll.*/
void hajo_loves(SDL_Renderer *renderer, Jatekos* jatekos) {
    if (jatekos->lov->y > 0) {
        if (jatekos->lov->vy < 30) jatekos->lov->vy += 5;
        jatekos->lov->y -= jatekos->lov->vy;
        boxRGBA(renderer, jatekos->lov->x + HAJO_MERET_X / 2 - 2, jatekos->lov->y + 5,
                jatekos->lov->x + HAJO_MERET_X / 2 + 2, jatekos->lov->y + 30, 0xFF, 0xFF, 0xFF, 0xFF);
    } else {
        jatekos->lov->x = 0;
        jatekos->lov->y = 0;
        jatekos->lov->vy = 0;
        jatekos->loves = false;
    }
}

/*Ez a függvény rajzolja ki a paraméterként megkapott, a játékos által
  irányított hajó textúráját a szintén megkapott x és y koordinátákra.*/
void hajo_rajzol(SDL_Renderer *renderer, SDL_Texture *kep, int x, int y, int meretx, int merety) {
    SDL_Rect src = { 0, 0, meretx, merety };
    SDL_Rect dest = { x, y, meretx, merety };
    SDL_RenderCopy(renderer, kep, &src, &dest);
}

/* Ez a függvény bonyolítja le az ellenség cselekvéseit; ilyen a mozgás, lövés, találatra eltűnés*/
void ellenseg_logika(Hajo* ellov, Hajo* ellenseg, MenuAllapot* allapot, Jatekos* jatekos, Kellekek *kellekek) {

    // Ha egy ellenség elérte az ablak jobb szélét, akkor kerüljön lejjebb és iránya változzon meg.
    if (ellenseg->x > ABLAK - ELLENSEG_MERET_X) {
        ellenseg->x = 2 * (ABLAK - ELLENSEG_MERET_X) - ellenseg->x;
        ellenseg->vx = -(jatekos->nehez + jatekos->kor);
        ellenseg->y += ELLENSEG_MERET_Y;
    }
    // Szintén az ablak bal szélével.
    else if (ellenseg->x < 0) {
        ellenseg->x *= -1;
        ellenseg->vx = jatekos->nehez + jatekos->kor;
        ellenseg->y += ELLENSEG_MERET_Y;
    }
    // Ha egy ellenséges űrhajó elérte a játéktér alját, akkor a játékos meghalt.
    if (ellenseg->y >= ABLAK - HAJO_MERET_Y)
        *allapot = gameover;

    // Ha a játékos által leadott lövés talált, akkor az ellenség tűnjön el a játékból, és a játékos ezért kapjon 10 pontot.
    if ((jatekos->lov->x >= ellenseg->x - (2 * ELLENSEG_MERET_X)) && (jatekos->lov->x <= ellenseg->x)
     && (jatekos->lov->y >= ellenseg->y) && (jatekos->lov->y <= ellenseg->y + ELLENSEG_MERET_Y)) {
        Mix_PlayChannel(-1, kellekek->robbanashang, 0);
        jatekos->lov->x = jatekos->lov->y = ellenseg->y = ellov->x = ellov->y = -32;
        jatekos->pont += 10;
        // A játékos minden ezredik pontnál kap egy bónusz életet (ezzel is motiválva a játékost :D)
        if (jatekos->pont % 1000 == 0) {
            ++jatekos->elet;
            Mix_PlayChannel(-1, kellekek->elethang, 0);
        }
        ellenseg->van = false;
    }

    // Az ellenség esélye a lövésre a nehézséggel és az adott körrel áll viszonyban; minél tovább jut a játékos, annál többet lőnek.
    if ((rand() % (1000 - jatekos->nehez * 500 - jatekos->kor * 25) == 1) && (!ellenseg->lott) && (ellenseg->van)) {
        Mix_PlayChannel(-1, kellekek->ellovhang, 0);
        ellenseg->lott = true;
        ellov->x = ellenseg->x;
        ellov->y = ellenseg->y;
    }

    // Végül az ellenség mozog.
    ellenseg->x += ellenseg->vx;
}

/* Ez a függvény az ellenségek által lőtt lövedékek logikáját bonyolítja le.
   Ha egy ellenség lőtt, akkor annak lövedéke az adott sebességgel közelít a játékos felé,
   illetve ha azzal találkozik, akkor egy életet levon.
   Ha a játékosnak nem maradt több élete, akkor a játék véget ér.*/
void ellenseg_loves(SDL_Renderer *renderer, Hajo* ellenseg, Hajo* ellov, MenuAllapot* allapot, Jatekos* jatekos) {
    if (ellenseg->lott) {
        // A lövedék gyorsasága nem haladhatja meg a 15-öt, mivel az már gyakorlatilag kikerülhetetlen lenne.
        ellov->y += 15 < jatekos->nehez + jatekos->kor * 2 ? 15 : jatekos->nehez + jatekos->kor * 2;
        boxRGBA(renderer, ellov->x + ELLENSEG_MERET_X / 2 - 2, ellov->y + 3,
                ellov->x + ELLENSEG_MERET_X / 2 + 2, ellov->y + 30, 0xFF, 0x00, 0x00, 0xFF);

        // Ha egy ellenséges lövedék eltalálta a hajót...
        if ((ellov->x >= jatekos->hajo->x - 8) && (ellov->x <= jatekos->hajo->x + HAJO_MERET_X - 30) &&
            (ellov->y >= jatekos->hajo->y) && (ellov->y <= jatekos->hajo->y + HAJO_MERET_Y)) {
            --jatekos->elet;
            ellov->y = ABLAK;
            if (jatekos->elet == 0)
                *allapot = gameover;
        }

        // Ha egy lövedék leért a pálya aljára, akkor az nullázódjon, és az ellenség újra lövésre kész.
        if (ellov->y > ABLAK) {
            ellov->x = 0;
            ellov->y = 0;
            ellenseg->lott = false;
        }
    }
}

/* Ez a függvény nézi meg, hogy a pályán minden űrhajó el lett-e pusztítva.
   Ha igen, akkor a kört eggyel megnöveli, megnöveli az ellenségek számát,
   és új értéket ad az ellenséges űrhajóknak.*/
void kor_vege_check(Hajo *ellenseg, Hajo *ellov, Jatekos* jatekos) {
    for (int i = 0; i < jatekos->ellensegszam; ++i)
        if (ellenseg[i].van)
            return;

    // Ha már egy ellenség sincs életben akkor jutunk el idáig...
    ++jatekos->kor;
    jatekos->ellensegszam += 3 * jatekos->nehez + 3;
    if (jatekos->ellensegszam > ELLENSEG_LIMIT)
        jatekos->ellensegszam = ELLENSEG_LIMIT;

    for (int i = 0; i < jatekos->ellensegszam; ++i) {
        ellenseg[i].x = i * ELLENSEG_MERET_X;
        ellenseg[i].y = ELLENSEG_MERET_Y;
        ellenseg[i].vx = jatekos->nehez + jatekos->kor;
        ellenseg[i].van = true;
    }
}
