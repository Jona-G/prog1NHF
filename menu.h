#ifndef MENU_H_INCLUDED
#define MENU_H_INCLUDED

void szoveg_kiir(SDL_Renderer *renderer, char* szoveg, SDL_Rect hova, int x, int y, TTF_Font *font, SDL_Surface *felirat, SDL_Texture *felirat_t, SDL_Color szin);

bool input_text(char *dest, size_t hossz, SDL_Rect teglalap, SDL_Color hatter, SDL_Color szoveg, SDL_Renderer *renderer, Kellekek* kellekek);

void menu_fomenu(SDL_Renderer *renderer, MenuAllapot* allapot, SDL_Event event, Jatekos* jatekos, Kellekek* kellekek);

void menu_nehezseg(SDL_Renderer *renderer, MenuAllapot* allapot, SDL_Event event, Jatekos* jatekos, Kellekek* kellekek);

void menu_dicsoseglista(SDL_Renderer *renderer, MenuAllapot* allapot, SDL_Event event, Dicsoseglista* dl, Kellekek* kellekek);

void menu_gameover(SDL_Renderer *renderer, MenuAllapot* allapot, SDL_Event event, Csillag* cs,
                   Hajo* ellenseg, Hajo* ellov, Jatekos* jatekos, Dicsoseglista* dl, Kellekek* kellekek);

#endif
