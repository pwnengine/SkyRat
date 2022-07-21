#ifndef RENDERER_H
#define RENDERER_H

#include "microui.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

void r_init(int width, int height);
void r_clean(void);
void r_draw_rect(mu_Rect rect, mu_Color color);
void r_draw_text(const char *text, mu_Vec2 pos, mu_Color color);
void r_draw_icon(int id, mu_Rect rect, mu_Color color);
int* r_get_text_size(const char *text);
void r_clear(mu_Color color);
void r_present(void);

#endif

