#ifndef JATEK_H_INCLUDED
#define JATEK_H_INCLUDED

void jatek_loop(SDL_Renderer *renderer, MenuAllapot* allapot, Jatekos* jatekos, SDL_Event event, Csillag* cs, Hajo* ellenseg, Hajo* ellov, Kellekek* kellekek);

void adat_reset(SDL_Renderer *renderer, MenuAllapot* allapot, Csillag *cs, Jatekos* jatekos, Hajo* ellenseg, Hajo* ellov);

void hatter_rajzol(SDL_Renderer *renderer, Csillag *cs, Jatekos* jatekos);

void csillag_rand(Csillag *cs);

void hajo_mozgas(SDL_Event event, Jatekos* jatekos);

void hajo_loves(SDL_Renderer *renderer, Jatekos* jatekos);

void hajo_rajzol(SDL_Renderer *renderer, SDL_Texture *kep, int x, int y, int meretx, int merety);

void ellenseg_logika(Hajo* ellov, Hajo* ellenseg, MenuAllapot* allapot, Jatekos* jatekos, Kellekek* kellekek);

void ellenseg_loves(SDL_Renderer *renderer, Hajo* ellenseg, Hajo* ellov, MenuAllapot* allapot, Jatekos* jatekos);

void kor_vege_check(Hajo *ellenseg, Hajo *ellov, Jatekos* jatekos);

#endif
